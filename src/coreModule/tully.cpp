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
#include "starModule/hip_star_mgr.hpp"
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
#include "coreModule/volumObj3D.hpp"

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
	delete[] pipelineSquare;

	posTully.clear();
	colorTully.clear();
	texTully.clear();
	scaleTully.clear();
	nameTully.clear();
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
	m_pointsGL->createBindingEntry(9 * sizeof(float));
	m_pointsGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // Pos3D
	m_pointsGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // Color
	m_pointsGL->addInput(VK_FORMAT_R32_SFLOAT); // Mag
	m_pointsGL->addInput(VK_FORMAT_R32_SFLOAT); // Scale
	pipelinePoints = new Pipeline[2]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get()}, {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get()}};
	for (int i = 0; i < 2; i++) {
		pipelinePoints[i].setBlendMode(BLEND_SATURATE_ALPHA);
		pipelinePoints[i].setDepthStencilMode();
		pipelinePoints[i].setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
		pipelinePoints[i].bindVertex(*m_pointsGL);
		pipelinePoints[i].bindShader("tully.vert.spv");
		VkBool32 whiteColor = (i & 1);
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

	bigSet = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
	bigSet->bindUniform(uGeom, 0);
	uBigFader = std::make_unique<SharedBuffer<float>>(*context.uniformMgr);
	bigSet->bindUniform(uBigFader, 1);

	m_squareGL = std::make_unique<VertexArray>(vkmgr);
	m_squareGL->createBindingEntry(5 * sizeof(float));
	m_squareGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // Pos3D
	m_squareGL->addInput(VK_FORMAT_R32_SFLOAT); // Mag
	m_squareGL->addInput(VK_FORMAT_R32_SFLOAT); // Scale
	auto blendMode = BLEND_SRC_ALPHA;
	blendMode.colorBlendOp = blendMode.alphaBlendOp = VK_BLEND_OP_MAX;
	pipelineSquare = new Pipeline[1]{{vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get()}};
	for (int i = 0; i < 1; ++i) {
		pipelineSquare[i].setBlendMode(blendMode);
		pipelineSquare[i].setDepthStencilMode();
		pipelineSquare[i].setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
		pipelineSquare[i].bindVertex(*m_squareGL);
		pipelineSquare[i].bindShader("tullyH.vert.spv");
		pipelineSquare[i].bindShader("tullyH.geom.spv");
		pipelineSquare[i].bindShader("tullyH.frag.spv");
		pipelineSquare[i].build();
	}
	drawData = std::make_unique<SharedBuffer<VkDrawIndirectCommand[4]>>(*context.tinyMgr);
	for (int i = 0; i < 4; ++i)
		(**drawData)[i] = VkDrawIndirectCommand{0, 1, 0, 0};
}


bool Tully::loadCatalog(const std::string &cat) noexcept
{
	std::ifstream file(cat, std::ifstream::in);

	if (!file) {
		//~ cout << "ERROR: Unable to open the file " << cat << std::endl;
		cLog::get()->write("TULLY catalog: missing file " + cat + " - Feature disabled",LOG_TYPE::L_ERROR);
		return false;
	}

	std::string line; // variable which will contain each line of the file
	int index, typeGalaxy;
	float r,g,b,x,y,z,xr,yr,zr;

	/*
	*
	* Line format: index, components (r, g ,b) between [0;1]
	* 			   (x,y,z) coordinates in the frame and typeGalaxy: the type of the object
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

		nameTully.push_back(index);
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

	Context &context = *Context::instance;
	const int vertexCount = texTully.size();
	vertexPoints = m_pointsGL->createBuffer(0, vertexCount, context.globalBuffer.get());
	vertexSquare = m_squareGL->createBuffer(0, vertexCount, context.globalBuffer.get());
	// float *staging = (float *) context.transfer->planCopy(vertexPoints->get());
	// vertexPoints->fillEntry(3, vertexCount, posTully.data(), staging);
	// vertexPoints->fillEntry(3, vertexCount, colorTully.data(), staging + 3);
	// vertexPoints->fillEntry(1, vertexCount, texTully.data(), staging + 6);
	// vertexPoints->fillEntry(1, vertexCount, scaleTully.data(), staging + 7);

	cLog::get()->write("Tully catalog successfully loaded: number of items " + std::to_string(nbGalaxy) );

	isAlive = true;
	needRebuild = true;
	return true;
}

bool Tully::loadBigCatalog(const std::string &cat, float optimalDistance) noexcept
{
	std::ifstream file(cat, std::ifstream::in);

	if (!file) {
		//~ cout << "ERROR: Unable to open the file  " << cat << std::endl;
		cLog::get()->write("Tully catalog: file missing " + cat + " - Feature disabled",LOG_TYPE::L_ERROR);
		return false;
	}

	bigCatalogMaxVisibilityAt = optimalDistance;

	std::string line, index; // variable which will contain each line of the file
	int typeGalaxy;
	float r,g,b,x,y,z,xr,yr,zr;
	Context &context = *Context::instance;
	float *staging = (float *) context.transfer->beginPlanCopy(0);
	int vertexCount = 0;

	/*
	*
	* Line format: index, components (r, g ,b) between [0;1]
	* 			   (x,y,z) coordinates in the frame and typeGalaxy: the type of the object
	*
	*	int, 3 floats, 3 floats, un int
	*/
	while (getline(file, line)) {
		if (line[0]=='#')
			continue;
		std::istringstream aGalaxie(line);
		aGalaxie >> index >> r >> g >> b >> x >> y >> z >> typeGalaxy;

		xr=200.f*x;
		yr=-200.f*z;
		zr=200.f*y;

		++vertexCount;
		*(staging++) = xr;
		*(staging++) = yr;
		*(staging++) = zr;
		*(staging++) = r;
		*(staging++) = g;
		*(staging++) = b;
		*(staging++) = typeGalaxy;

		switch (typeGalaxy) {
			case 0  : *(staging++) = 8.0; break;  //Dwarf
			case 13 : *(staging++) = 4.0; break;  // LMC
			case 14 : *(staging++) = 4.0; break;  // SMC
			case 7  : *(staging++) = 0.125; break; // Elliptic
			case 9  : *(staging++) = 64.0; break; // AG
			case 10 : *(staging++) = 128.0; break; // Dark NEB
			case 12 : *(staging++) = 128.0; break; // Bright NEB
			default : *(staging++) = 0.25; break; // GALAXY
		}
	}
	file.close();

	vertexPointsExt = m_pointsGL->createBuffer(0, vertexCount, context.globalBuffer.get());
	context.transfer->endPlanCopy(vertexPointsExt->get(), vertexCount * 9 * sizeof(float));

	needRebuild = true;
	return true;
}

void Tully::buildInternal()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	// create and initialize
	if (!cmdWhiteColor) {
		context.cmdInfo.commandBufferCount = 6;
		vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmdCustomColor);
		cmdWhiteColor = cmdCustomColor + 3;
	}
	std::string headName[2] {"Tully custom ", "Tully white "};
	if (includeObject) {
		headName[0] += "with volumObj3D ";
		headName[1] += "with volumObj3D ";
	}
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 2; ++j) {
			VkCommandBuffer cmd = cmdCustomColor[i+j*3];
			context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
			pipelineSquare[0].bind(cmd);
			layout->bindSets(cmd, {*context.uboSet, *set});
			vertexSquare->bind(cmd);
			vkCmdDrawIndirect(cmd, drawData->getBuffer().buffer, drawData->getOffset(), 1, 0);
			pipelinePoints[j].bind(cmd);
			vertexPoints->bind(cmd);
			if (includeObject) {
				vkCmdDrawIndirect(cmd, drawData->getBuffer().buffer, drawData->getOffset() + 2 * sizeof(VkDrawIndirectCommand), 1, 0);
				withObject->recordVolumetricObject(cmd);
				pipelineSquare[0].bind(cmd);
				layout->bindSets(cmd, {*context.uboSet, *set});
				vertexSquare->bind(cmd);
				vkCmdDrawIndirect(cmd, drawData->getBuffer().buffer, drawData->getOffset() + 1 * sizeof(VkDrawIndirectCommand), 1, 0);
				pipelinePoints[j].bind(cmd);
				vertexPoints->bind(cmd);
				vkCmdDrawIndirect(cmd, drawData->getBuffer().buffer, drawData->getOffset() + 3 * sizeof(VkDrawIndirectCommand), 1, 0);
			} else {
				vkCmdDraw(cmd, vertexPoints->getVertexCount(), 1, 0, 0);
			}
			if (vertexPointsExt) {
				layout->bindSet(cmd, *bigSet, 1);
				vertexPointsExt->bind(cmd);
				vkCmdDraw(cmd, vertexPointsExt->getVertexCount(), 1, 0, 0);
			}
			context.frame[i]->compile(cmd);
			context.frame[i]->setName(cmd, headName[j] + std::to_string(i));
		}
	}
	needRebuild = false;
}

void Tully::buildVertexSplit()
{
	sortedDataTully.resize(nbGalaxy * 9);
	float *staging = sortedDataTully.data();
	if (withObject) {
		drawDataPointFirstOffset = 0;
		drawDataPointSecondSize = 0;
		float *reverseStaging = staging + nbGalaxy * 9;
		float x,y,z;
		// Put the element behind from top to back, and element over from back to top
		for(unsigned int i=0; i< nbGalaxy;i++) {
			x=posTully[3*i];
			y=posTully[3*i+1];
			z=posTully[3*i+2];

			Vec4f pos(x, y, z, 0);
			pos -= withObject->getModel() * Vec4f(0,0,0,1);
			// Now, pos is the position relative to the center of the observed object
			pos = withObject->getModel().inverseUntranslated() * pos;
			// Now, pos is the position in the object coordinates
			if (pos[2] < 0) {
				// Behind the center, regarding to the local z plane
				++drawDataPointFirstOffset;
				*(staging++) = x;
				*(staging++) = y;
				*(staging++) = z;
				*(staging++) = colorTully[i*3+0];
				*(staging++) = colorTully[i*3+1];
				*(staging++) = colorTully[i*3+2];
				*(staging++) = texTully[i];
				*(staging++) = scaleTully[i];
				*(staging++) = nameTully[i];
			} else {
				// Over the center, regarding to the local z plane
				++drawDataPointSecondSize;
				*--reverseStaging = nameTully[i];
				*--reverseStaging = scaleTully[i];
				*--reverseStaging = texTully[i];
				*--reverseStaging = colorTully[i*3+2];
				*--reverseStaging = colorTully[i*3+1];
				*--reverseStaging = colorTully[i*3+0];
				*--reverseStaging = z;
				*--reverseStaging = y;
				*--reverseStaging = x;
			}
		}

		planeOrder = 2;
	} else {
		drawDataPointFirstOffset = nbGalaxy;
		drawDataPointSecondSize = 0;
		// Just put every elements in the vertexPoints
		vertexPoints->fillEntry(3, nbGalaxy, posTully.data(), staging);
		vertexPoints->fillEntry(3, nbGalaxy, colorTully.data(), staging + 3);
		vertexPoints->fillEntry(1, nbGalaxy, texTully.data(), staging + 6);
		vertexPoints->fillEntry(1, nbGalaxy, scaleTully.data(), staging + 7);
		vertexPoints->fillEntry(1, nbGalaxy, nameTully.data(), staging + 8);
		planeOrder = 0;
	}
	std::memcpy(Context::instance->transfer->planCopy(vertexPoints->get()), sortedDataTully.data(), vertexPoints->get().size);
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

	needRebuild = true;
}

bool Tully::compTmpTully(const tmpTully &a,const tmpTully &b)
{
	if (a.planeSide < b.planeSide)
		return true;
	return (a.distance > b.distance);
}

void Tully::computeSquareGalaxies(Vec3f camPosition)
{
	float x,y,z,a,b,c,distance, radius;

	a = camPosition[0];
	b = camPosition[1];
	c = camPosition[2];
	int squareOffset = 0;
	for(unsigned int i=0; i< nbGalaxy;i++) {
		x=sortedDataTully[9*i];
		y=sortedDataTully[9*i+1];
		z=sortedDataTully[9*i+2];

		//only galaxies large enough to be displayed are selected
        distance=sqrt((x-a)*(x-a)+(y-b)*(y-b)+(z-c)*(z-c));
		radius = 3.0/(distance*sortedDataTully[9*i+7]);
		if (radius<2)
			continue;

		/* OPTIMISATION : radius < 2 means that d²< (3/2scale)² */
		tmpTully tmp;
		tmp.position = Vec3f(x,y,z);
		tmp.distance = distance;
		tmp.radius = radius;
		tmp.texture = sortedDataTully[9*i+6];
		tmp.planeSide = (i >= drawDataPointFirstOffset) ^ planeOrder;
		squareOffset += !tmp.planeSide;
		lTmpTully.push_back(tmp);
	}
	int vertexCount = lTmpTully.size();
	// printf("list size: %i\n", lTmpTully.size());
	lTmpTully.sort(compTmpTully);

	float *data = nullptr;
	if (vertexCount)
	 	data = (float *) Context::instance->transfer->planCopy(vertexSquare->get(), 0, vertexCount * 5 * sizeof(float));

	for (int i = 0; i < 2; ++i) {
		for (std::list<tmpTully>::iterator it=lTmpTully.begin(); it!=lTmpTully.end(); ++it) {
			if ((*it).planeSide == i) {
				memcpy(data, (float *) (*it).position, 3 * sizeof(float));
				data += 3;
				*(data++) = (*it).texture;
				*(data++) = (*it).radius;
			}
		}
	}

	lTmpTully.clear();	//data become useless

	if (includeObject) {
		drawData->get()[0].vertexCount = squareOffset;
		drawData->get()[1].vertexCount = vertexCount - squareOffset;
		drawData->get()[1].firstVertex = squareOffset;
	} else
		drawData->get()[0].vertexCount = vertexCount;
}

void Tully::drawGalaxyName(const Projector* prj)
{
	for (auto const& token : galaxyNameToDraw) {
		prj->printGravity180(font, std::get<0>(token), std::get<1>(token), std::get<2>(token), std::get<3>(token), 4,4);
	}
	// std::cout << "Number of the names to print : " << galaxyNameToDraw.size() << "\n";
}


void Tully::draw(double distance, const Navigator *nav, const Projector *prj) noexcept
{
	if (!(isAlive && fader.getInterstate()))
		return;

	galaxyNameToDraw.clear();

	Mat4f matrix= nav->getHelioToEyeMat().convert();
	camPos = nav->getObserverHelioPos();

	if (withObject) {
		if (withObject->isInside(nav) == includeObject) {
			includeObject = !includeObject;
			VulkanMgr::instance->waitIdle(); // Avoid rewriting commands in use - this doesn't check pending submission in the draw_helper
			buildInternal();
		}
		if (planeOrder != ((withObject->drawExternal(nav, prj) * Vec4f {0, 0, 1, 0})[2] < 0)) {
			planeOrder = ((withObject->drawExternal(nav, prj) * Vec4f {0, 0, 1, 0})[2] < 0);
			if (planeOrder) {
				(**drawData)[2].vertexCount = drawDataPointSecondSize;
				(**drawData)[2].firstVertex = drawDataPointFirstOffset;
				(**drawData)[3].vertexCount = drawDataPointFirstOffset;
				(**drawData)[3].firstVertex = 0;
			} else {
				(**drawData)[2].vertexCount = drawDataPointFirstOffset;
				(**drawData)[2].firstVertex = 0;
				(**drawData)[3].vertexCount = drawDataPointSecondSize;
				(**drawData)[3].firstVertex = drawDataPointFirstOffset;
			}
		}
	}

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
	*uBigFader = fader.getInterstate() * ((distance > bigCatalogMaxVisibilityAt) ? 1 : distance / bigCatalogMaxVisibilityAt);
	uGeom->get().nbTextures = nbTextures;

	if (useWhiteColor)
		Context::instance->frame[Context::instance->frameIdx]->toExecute(cmdWhiteColor[Context::instance->frameIdx], PASS_MULTISAMPLE_DEPTH);
	else
		Context::instance->frame[Context::instance->frameIdx]->toExecute(cmdCustomColor[Context::instance->frameIdx], PASS_MULTISAMPLE_DEPTH);

	if (!includeObject)
		withObject->drawInside(nav, prj);

	if (!names_fader.getInterstate())
		return;

	const float names_brightness = names_fader.getInterstate() * fader.getInterstate();
	float x,y,z,a,b,c,distanceGal;
	a = camPos[0];
	b = camPos[1];
	c = camPos[2];
	for(unsigned int i=0; i< nbGalaxy;i++) {
		x=sortedDataTully[9*i];
		y=sortedDataTully[9*i+1];
		z=sortedDataTully[9*i+2];

		Vec3f pos(x, y, z);
		pos = nav->helioToEarthPosEqu(pos);
		//pos[0] = -pos[0];
		Vec3d screenposd;
		prj->projectEarthEqu(pos, screenposd);

        distanceGal=sqrt((x-a)*(x-a)+(y-b)*(y-b)+(z-c)*(z-c));
		if (distanceGal < 0.01) {
			std::string galaxyName = std::to_string(int(sortedDataTully[9*i+8]));
			if (!galaxyName.empty()) {
				// Vec4f Color(HipStarMgr::color_table[int(colorTully[i*3+0]*255)][0]*0.75,
				// 			HipStarMgr::color_table[int(colorTully[i*3+1]*255)][1]*0.75,
				// 			HipStarMgr::color_table[int(colorTully[i*3+2]*255)][2]*0.75,
				// 			names_brightness);
				Vec4f Color(sortedDataTully[9*i+3], sortedDataTully[9*i+4], sortedDataTully[9*i+5], names_brightness);
				// printf("color: %f, %f, %f\n", sortedDataTully[9*i+3], sortedDataTully[9*i+4], sortedDataTully[9*i+5]);
				galaxyNameToDraw.push_back(std::make_tuple(screenposd[0],screenposd[1], galaxyName, Color));
			}
		}
	}
	this->drawGalaxyName(prj);
}
