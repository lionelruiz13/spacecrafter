/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - 2020 all rights reserved
*
*/

#include "inGalaxyModule/dso3d.hpp"
#include "tools/utility.hpp"
#include <string>
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include <iomanip>
#include <math.h>
#include "tools/s_texture.hpp"
// #include "vulkanModule/VertexArray.hpp"
// 
// 
#include "vulkanModule/CommandMgr.hpp"
#include "vulkanModule/Pipeline.hpp"
#include "vulkanModule/PipelineLayout.hpp"
#include "vulkanModule/Set.hpp"
#include "vulkanModule/Uniform.hpp"
#include "vulkanModule/VertexArray.hpp"

Dso3d::Dso3d(ThreadContext *context)
{
	texNebulae = nullptr;
	fader = true;
	createSC_context(context);
	nbNebulae=0;
}

Dso3d::~Dso3d()
{
	if (texNebulae!=nullptr)
		delete texNebulae;

	posDso3d.clear();
	scaleDso3d.clear();
	texDso3d.clear();
}

void Dso3d::createSC_context(ThreadContext *context)
{
	globalSet = context->global->globalSet;
	cmdMgr = context->commandMgr;
	// shaderDso3d = std::make_unique<shaderProgram>();
	// shaderDso3d->init("dso3d.vert", "dso3d.geom","dso3d.frag");
	// shaderDso3d->setUniformLocation({"Mat", "fader", "camPos", "nbTextures"});

	sData = std::make_unique<VertexArray>(context->surface);
	sData->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	sData->registerVertexBuffer(BufferType::MAG, BufferAccess::STATIC);
	sData->registerVertexBuffer(BufferType::SCALE, BufferAccess::STATIC);

	layout = std::make_unique<PipelineLayout>(context->surface);
	layout->setGlobalPipelineLayout(context->global->globalLayout);
	layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->setTextureLocation(2);
	layout->buildLayout();
	layout->build();

	pipeline = std::make_unique<Pipeline>(context->surface, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipeline->setDepthStencilMode();
	pipeline->setBlendMode(BLEND_ADD);
	pipeline->bindVertex(sData.get());
	pipeline->bindShader("dso3d.vert.spv");
	pipeline->bindShader("dso3d.geom.spv");
	pipeline->bindShader("dso3d.frag.spv");
	pipeline->build();

	assert(offsetof(pGeom_s, nbTextures) == 76); // check alignment
	set = std::make_unique<Set>(context->surface, context->setMgr, layout.get());
	uGeom = std::make_unique<Uniform>(context->surface, sizeof(*pGeom));
	pGeom = static_cast<typeof(pGeom)>(uGeom->data);
	set->bindUniform(uGeom.get(), 0);
	uFader = std::make_unique<Uniform>(context->surface, sizeof(*pFader));
	pFader = static_cast<typeof(pFader)>(uFader->data);
	set->bindUniform(uFader.get(), 1);
}

void Dso3d::build()
{
	commandIndex = cmdMgr->initNew(pipeline.get());
	cmdMgr->bindSet(layout.get(), globalSet, 0);
	cmdMgr->bindSet(layout.get(), set.get(), 1);
	cmdMgr->bindVertex(sData.get());
	cmdMgr->draw(nbNebulae);
	cmdMgr->compile();
}

bool Dso3d::loadCatalog(const std::string &cat) noexcept
{
	std::ifstream file(cat, std::ifstream::in);

	if (!file) {
		//~ cout << "ERREUR: Impossible d'ouvrir le fichier " << cat << std::endl;
		cLog::get()->write("DSO3D: Impossible d'ouvrir le fichier "+ cat ,LOG_TYPE::L_ERROR);
		return false;
	}

	std::string index, line; // variable which will contain each line of the file
	//~ cout << "Lecture du catalogue DSO"  << cat << std::endl;
	int typeNebula, xyz;
	float size,x,y,z,xr,yr,zr,alpha,delta,r;

	while (getline(file, line)) {
		if (line[0]=='#')
			continue;
		std::istringstream aNebulae(line);
		aNebulae >> index >> xyz >> alpha >> delta >> r >> size >> typeNebula;
		nbNebulae++;
		//x y z position
		if (xyz==1) {
			posDso3d.push_back(alpha);
			posDso3d.push_back(delta);
			posDso3d.push_back(r);
		} else {
		//x y z calculation
			x=r*cos(alpha*M_PI/180.0)*sin((90.0-delta)*M_PI/180.0);
			y=r*sin(alpha*M_PI/180.0)*sin((90.0-delta)*M_PI/180.0);
			z=r*cos((90.0-delta)*M_PI/180.0);
		// ecliptic coordinates change
			xr=x;
			yr=y*cos(-23.43928*M_PI/180.0)-z*sin(-23.43928*M_PI/180.0);
			zr=y*sin(-23.43928*M_PI/180.0)+z*cos(-23.43928*M_PI/180.0);

			posDso3d.push_back(xr/2.0);
			posDso3d.push_back(yr/2.0);
			posDso3d.push_back(zr/2.0);
		}
		//dso texture
		texDso3d.push_back(typeNebula);
		scaleDso3d.push_back(size);
	}
	file.close();
	sData->build(nbNebulae);
	sData->fillVertexBuffer(BufferType::POS3D, posDso3d);
	sData->fillVertexBuffer(BufferType::SCALE, scaleDso3d);
	sData->fillVertexBuffer(BufferType::MAG, texDso3d);

	return true;
}

void Dso3d::setTexture(const std::string& tex_file)
{
	if (texNebulae != nullptr) {
		delete texNebulae;
		texNebulae = nullptr;
	}
	texNebulae =  new s_texture(/*1,*/tex_file,/*true*/ TEX_LOAD_TYPE_PNG_SOLID);

	int width, height;
	texNebulae->getDimensions(width, height);
	set->bindTexture(texNebulae->getTexture(), 2);
	if (width ==0 || height ==0)
		nbTextures = 0;
	else
		nbTextures = width / height;
	pGeom->nbTextures = nbTextures;
}


void Dso3d::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
	if (!fader.getInterstate() || commandIndex == -1 || nbNebulae==0) return;

	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_ONE, GL_ONE);

	pGeom->Mat = nav->getHelioToEyeMat().convert();

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, texNebulae->getID());

	pGeom->camPos = nav->getObserverHelioPos();

	// shaderDso3d->use();
	// shaderDso3d->setUniform("Mat",matrix);
	// shaderDso3d->setUniform("fader", fader.getInterstate());
	// shaderDso3d->setUniform("camPos", camPos);
	// shaderDso3d->setUniform("nbTextures", nbTextures);
	*pFader = fader.getInterstate();
	cmdMgr->setSubmission(commandIndex);

	// sData->bind();
	// glDrawArrays(VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, nbNebulae);
	// sData->unBind();
	// shaderDso3d->unuse();
	// Renderer::drawArrays(shaderDso3d.get(), sData.get(), VK_PRIMITIVE_TOPOLOGY_POINT_LIST, 0, nbNebulae);
}