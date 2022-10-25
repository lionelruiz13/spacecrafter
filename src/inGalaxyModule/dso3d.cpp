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

#include <cassert>
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
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

Dso3d::Dso3d()
{
	texNebulae = nullptr;
	fader = true;
	createSC_context();
	nbNebulae=0;
}

Dso3d::~Dso3d()
{
	posDso3d.clear();
	scaleDso3d.clear();
	texDso3d.clear();
	nameDso3d.clear();
}

void Dso3d::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	sData = std::make_unique<VertexArray>(vkmgr);
	sData->createBindingEntry(5*sizeof(float));
	sData->addInput(VK_FORMAT_R32G32B32_SFLOAT); // POS3D
	sData->addInput(VK_FORMAT_R32_SFLOAT); // MAG
	sData->addInput(VK_FORMAT_R32_SFLOAT); // SCALE

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	layout->buildLayout();
	layout->build();

	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipeline->setDepthStencilMode();
	pipeline->setBlendMode(BLEND_ADD);
	pipeline->bindVertex(*sData);
	pipeline->bindShader("dso3d.vert.spv");
	pipeline->bindShader("dso3d.geom.spv");
	pipeline->bindShader("dso3d.frag.spv");
	pipeline->build();

	assert(offsetof(pGeom_s, nbTextures) == 76); // check alignment
	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get(), -1, true, true);
	uGeom = std::make_unique<SharedBuffer<pGeom_s>>(*context.uniformMgr);
	set->bindUniform(uGeom, 0);
	uFader = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
	set->bindUniform(uFader, 1);
}

void Dso3d::build()
{
	Context &context = *Context::instance;

	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i) {
		context.frame[i]->begin(cmds[i], PASS_MULTISAMPLE_DEPTH);
		pipeline->bind(cmds[i]);
		layout->bindSets(cmds[i], {*context.uboSet, *set});
		vertex->bind(cmds[i]);
		vkCmdDraw(cmds[i], nbNebulae, 1, 0, 0);
		context.frame[i]->compile(cmds[i]);
		context.frame[i]->setName(cmds[i], "Dso3D " + std::to_string(i));
	}
}

bool Dso3d::loadCatalog(const std::string &cat) noexcept
{
	std::ifstream file(cat, std::ifstream::in);

	if (!file) {
		//~ cout << "ERREUR: Impossible d'ouvrir le fichier " << cat << std::endl;
		cLog::get()->write("DSO3D catalog: missing file "+ cat + " - Feature disabled",LOG_TYPE::L_ERROR);
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
		//dso name
		nameDso3d.push_back(index);
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
	vertex = sData->createBuffer(0, nbNebulae, Context::instance->globalBuffer.get());
	float *data = (float *) Context::instance->transfer->planCopy(vertex->get());
	vertex->fillEntry(3, nbNebulae, posDso3d.data(), data);
	vertex->fillEntry(1, nbNebulae, texDso3d.data(), data + 3);
	vertex->fillEntry(1, nbNebulae, scaleDso3d.data(), data + 4);

	return true;
}

void Dso3d::setTexture(const std::string& tex_file)
{
	texNebulae = std::make_unique<s_texture>(tex_file, TEX_LOAD_TYPE_PNG_SOLID);

	int width, height;
	texNebulae->getDimensions(width, height);
	set->bindTexture(texNebulae->getTexture(), 2);
	if (width ==0 || height ==0)
		nbTextures = 0;
	else
		nbTextures = width / height;
	uGeom->get().nbTextures = nbTextures;
}

void Dso3d::drawDsoName(const Projector* prj, const Navigator *nav)
{
	const float names_brightness = names_fader.getInterstate() * fader.getInterstate();
	Vec4f color(labelColor);
	color[3] = names_brightness;
	for(unsigned int i=0; i< nbNebulae;i++) {
		Vec3f posXYZ(posDso3d[i*3], posDso3d[i*3+1], posDso3d[i*3+2]);
		Vec3f pos;
		pos = nav->helioToEarthPosEqu(posXYZ);
		pos[0] = -pos[0];
		Vec3d screenposd;
		prj->projectEarthEqu(pos, screenposd);

		prj->printGravity180(font, screenposd[0], screenposd[1], nameDso3d[i], color, 4,4);
	}
}

void Dso3d::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
	if (!fader.getInterstate() || nbNebulae==0) return;

	uGeom->get().Mat = nav->getHelioToEyeMat().convert();
	uGeom->get().camPos = nav->getObserverHelioPos();
	*uFader = fader.getInterstate();
	const int frameIdx = Context::instance->frameIdx;
	Context::instance->frame[frameIdx]->toExecute(cmds[frameIdx], PASS_MULTISAMPLE_DEPTH);
	if (names_fader.getInterstate())
		drawDsoName(prj, nav);
}
