/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017-2020 of the LSS Team & Association Sirius
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include <iomanip>
#include <math.h>

#include "coreModule/tully.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/s_texture.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/insert_all.hpp"

Tully::Tully()
{
	texGalaxy = nullptr;
	fader = true;
	createSC_context();
	nbGalaxy=0;
	nbTextures = 0;
}

Tully::~Tully()
{
	if (texGalaxy!=nullptr)
		delete texGalaxy;
	delete[] pipelinePoints;

	posTully.clear();
	colorTully.clear();
	texTully.clear();
	scaleTully.clear();
}

void Tully::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	layout->setTextureLocation(2, &PipelineLayout::DEFAULT_SAMPLER);
	layout->buildLayout();
	layout->build();

	m_pointsGL = std::make_unique<VertexArray>(vkmgr);
	m_pointsGL->createBindingEntry(8 * sizeof(float));
	m_pointsGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // Pos3D
	m_pointsGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // Color
	m_pointsGL->addInput(VK_FORMAT_R32_SFLOAT); // Mag
	m_pointsGL->addInput(VK_FORMAT_R32_SFLOAT); // Scale
	pipelinePoints = new Pipeline[2]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get()}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get()}};
	for (int i = 0; i < 2; i++) {
		pipelinePoints[i].setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
		pipelinePoints[i].setDepthStencilMode();
		pipelinePoints[i].bindVertex(*m_pointsGL);
		pipelinePoints[i].bindShader("tully.vert.spv");
		VkBool32 whiteColor = (i == 1);
		pipelinePoints[i].setSpecializedConstant(0, &whiteColor, sizeof(whiteColor));
		pipelinePoints[i].bindShader("tully.geom.spv");
		pipelinePoints[i].bindShader("tully.frag.spv");
		pipelinePoints[i].build();
	}
	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
	uGeom = std::make_unique<SharedBuffer<s_geom>>(*context.uniformMgr);
	set->bindUniform(uGeom, 0);
	uFader = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
	set->bindUniform(uFader, 1);

	m_squareGL = std::make_unique<VertexArray>(vkmgr);
	m_squareGL->createBindingEntry(5 * sizeof(float));
	m_squareGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // Pos3D
	m_squareGL->addInput(VK_FORMAT_R32_SFLOAT); // Mag
	m_squareGL->addInput(VK_FORMAT_R32_SFLOAT); // Scale
	auto blendMode = BLEND_SRC_ALPHA;
	blendMode.colorBlendOp = blendMode.alphaBlendOp = VK_BLEND_OP_MAX;
	pipelineSquare = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipelineSquare->setBlendMode(blendMode);
	pipelineSquare->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipelineSquare->setDepthStencilMode();
	pipelineSquare->bindVertex(*m_squareGL);
	pipelineSquare->bindShader("tullyH.vert.spv");
	pipelineSquare->bindShader("tullyH.geom.spv");
	pipelineSquare->bindShader("tullyH.frag.spv");
	pipelineSquare->build();
	drawDataSquare = std::make_unique<SharedBuffer<VkDrawIndirectCommand>>(*context.tinyMgr);
	*drawDataSquare = VkDrawIndirectCommand{0, 1, 0, 0};
}


bool Tully::loadCatalog(const std::string &cat) noexcept
{
	std::ifstream file(cat, std::ifstream::in);

	if (!file) {
		//~ cout << "ERREUR: Impossible d'ouvrir le fichier " << cat << std::endl;
		cLog::get()->write("TULLY: Impossible d'ouvrir le fichier " + cat ,LOG_TYPE::L_ERROR);
		cLog::get()->write("TULLY: classe désactivée "+ cat ,LOG_TYPE::L_WARNING);
		return false;
	}

	std::string line, index; // variable which will contain each line of the file
	int typeGalaxy;
	float r,g,b,x,y,z,xr,yr,zr;

	/*
	*
	* Format de ligne : index , composantes (r, g ,b) entre [0;1]
	*					(x,y,z) coordonnées dans le repère et typeGalaxy: le type de l'objet
	*
	*	int, 3 floats, 3 floats, un int
	*/
	while (getline(file, line)) {
		if (line[0]=='#')
			continue;
		std::istringstream aGalaxie(line);
		aGalaxie >> index >> r >> g >> b >> x >> y >> z >> typeGalaxy;
		nbGalaxy++;

		xr=200.f*x;
		yr=-200.f*z;
		zr=200.f*y;

		insert_all(posTully, xr, yr, zr);
		insert_all(colorTully, r, g, b);

		texTully.push_back(typeGalaxy);

		switch (typeGalaxy) {
			case 0  : scaleTully.push_back(8.0); break;  //Dwarf
			case 13 : scaleTully.push_back(4.0); break;  // LMC
			case 14 : scaleTully.push_back(4.0); break;  // SMC
			case 7  : scaleTully.push_back(0.125); break; // Elliptic
			case 9  : scaleTully.push_back(64.0); break; // AG
			case 10 : scaleTully.push_back(128.0); break; // Dark NEB
			case 12 : scaleTully.push_back(128.0); break; // Bright NEB
			default : scaleTully.push_back(0.25); break; // GALAXY
		}
	}

	file.close();
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;
	const int vertexCount = texTully.size();
	vertexPoints = m_pointsGL->createBuffer(0, vertexCount, context.globalBuffer.get());
	vertexSquare = m_pointsGL->createBuffer(0, vertexCount, context.globalBuffer.get());
	float *staging = (float *) context.transfer->planCopy(vertexPoints->get());
	vertexPoints->fillEntry(3, vertexCount, posTully.data(), staging);
	vertexPoints->fillEntry(3, vertexCount, colorTully.data(), staging + 3);
	vertexPoints->fillEntry(1, vertexCount, texTully.data(), staging + 6);
	vertexPoints->fillEntry(1, vertexCount, scaleTully.data(), staging + 7);

	// create and initialize
	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmdCustomColor);
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmdWhiteColor);
	for (int i = 0; i < 3; ++i) {
		VkCommandBuffer cmd = cmdCustomColor[i];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipelinePoints->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set});
		vertexPoints->bind(cmd);
		vkCmdDraw(cmd, vertexCount, 1, 0, 0);
		pipelineSquare->bind(cmd);
		vertexSquare->bind(cmd);
		vkCmdDrawIndirect(cmd, drawDataSquare->getBuffer().buffer, drawDataSquare->getOffset(), 1, 0);
		context.frame[i]->compile(cmd);

		cmd = cmdWhiteColor[i];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipelinePoints[1].bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set});
		vertexPoints->bind(cmd);
		vkCmdDraw(cmd, vertexCount, 1, 0, 0);
		pipelineSquare->bind(cmd);
		vertexSquare->bind(cmd);
		vkCmdDrawIndirect(cmd, drawDataSquare->getBuffer().buffer, drawDataSquare->getOffset(), 1, 0);
		context.frame[i]->compile(cmd);
	}

	cLog::get()->write("Tully chargement réussi du catalogue : nombre d'items " + std::to_string(nbGalaxy) );

	isAlive = true;
	return true;
}


void Tully::setTexture(const std::string& tex_file)
{
	if (texGalaxy != nullptr) {
		delete texGalaxy;
		texGalaxy = nullptr;
	}
	texGalaxy = new s_texture(tex_file,/*true*/ TEX_LOAD_TYPE_PNG_SOLID);

	int width, height;
	texGalaxy->getDimensions(width, height);
	set->bindTexture(texGalaxy->getTexture(), 2);
	if (width ==0 || height ==0)
		nbTextures = 0;
	else
		nbTextures = width / height;
}

bool Tully::compTmpTully(const tmpTully &a,const tmpTully &b)
{
	if (a.distance>b.distance)
		return true;
	else
		return false;
}

void Tully::computeSquareGalaxies(Vec3f camPosition)
{
	float x,y,z,a,b,c,distance, radius;

	a = camPosition[0];
	b = camPosition[1];
	c = camPosition[2];
	for(unsigned int i=0; i< nbGalaxy;i++) {
		x=posTully[3*i];
		y=posTully[3*i+1];
		z=posTully[3*i+2];

		//on ne sélectionne que les galaxies assez grandes pour être affichées
        distance=sqrt((x-a)*(x-a)+(y-b)*(y-b)+(z-c)*(z-c));
		radius = 3.0/(distance*scaleTully[i]);
		if (radius<2)
			continue;

		/* OPTIMISATION : radius < 2 signifie que d²< (3/2scale)² */
		tmpTully tmp;
		tmp.position = Vec3f(x,y,z);
		tmp.distance = distance;
		tmp.radius = radius;
		tmp.texture = texTully[i];
		lTmpTully.push_back(tmp);
	}
	// printf("taille de la liste: %i\n", lTmpTully.size());
	lTmpTully.sort(compTmpTully);

	int vertexCount = 0;
	float *data = (float *) Context::instance->transfer->beginPlanCopy(vertexSquare->get().size);

	for (std::list<tmpTully>::iterator it=lTmpTully.begin(); it!=lTmpTully.end(); ++it) {
		memcpy(data, (float *) (*it).position, 3 * sizeof(float));
		data += 3;
		*(data++) = (*it).texture;
		*(data++) = (*it).radius;
		++vertexCount;
	}
	Context::instance->transfer->endPlanCopy(vertexSquare->get(), vertexCount * 5 * sizeof(float));

	lTmpTully.clear();	//données devenues inutiles

	drawDataSquare->get().vertexCount = vertexCount;
}


void Tully::draw(double distance, const Projector *prj,const Navigator *nav) noexcept
{
	if (!fader.getInterstate()) return;
	if (!isAlive) return;

	Mat4f matrix= nav->getHelioToEyeMat().convert();
	camPos = nav->getObserverHelioPos();

	computeSquareGalaxies(camPos);

	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, texGalaxy->getID());

	//tracé des galaxies de taille <1 px
	// StateGL::disable(GL_DEPTH_TEST);
	// StateGL::enable(GL_BLEND);
	// StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	// shaderPoints->use();
	// shaderPoints->setUniform("Mat",matrix);
	// shaderPoints->setUniform("fader", fader.getInterstate());
	// shaderPoints->setUniform("camPos", camPos);
	// shaderPoints->setUniform("nbTextures", nbTextures);
	uGeom->get().mat = matrix;
	uGeom->get().camPos = camPos;
	*uFader = fader.getInterstate();
	uGeom->get().nbTextures = nbTextures;

	if (useWhiteColor)
		Context::instance->frame[Context::instance->frameIdx]->toExecute(cmdWhiteColor[Context::instance->frameIdx], PASS_MULTISAMPLE_DEPTH);
	else
		Context::instance->frame[Context::instance->frameIdx]->toExecute(cmdCustomColor[Context::instance->frameIdx], PASS_MULTISAMPLE_DEPTH);
}
