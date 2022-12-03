/*
 * Copyright (C) 2020 of the Association Andromède
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
#include "starGalaxy.hpp"
#include "EntityCore/Core/BufferMgr.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Resource/VertexArray.hpp"
#include "EntityCore/Resource/VertexBuffer.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "EntityCore/Resource/TransferMgr.hpp"
#include "EntityCore/Resource/Set.hpp"
#include "EntityCore/Resource/Texture.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/context.hpp"
#include "tools/s_texture.hpp"

const Vec3f StarGalaxy::color_table[128] {
	Vec3f(0.780392,0.866666,1),
	Vec3f(0.788235,0.870588,1),
	Vec3f(0.8,0.878431,1),
	Vec3f(0.807843,0.882352,1),
	Vec3f(0.815686,0.886274,1),
	Vec3f(0.82745,0.894117,1),
	Vec3f(0.835294,0.898039,1),
	Vec3f(0.843137,0.905882,1),
	Vec3f(0.854901,0.909803,1),
	Vec3f(0.866666,0.917647,1),
	Vec3f(0.874509,0.921568,1),
	Vec3f(0.886274,0.92549,1),
	Vec3f(0.894117,0.933333,1),
	Vec3f(0.905882,0.941176,1),
	Vec3f(0.917647,0.945098,1),
	Vec3f(0.92549,0.952941,1),
	Vec3f(0.937254,0.956862,1),
	Vec3f(0.949019,0.964705,1),
	Vec3f(0.956862,0.968627,1),
	Vec3f(0.968627,0.97647,1),
	Vec3f(0.980392,0.980392,1),
	Vec3f(0.988235,0.988235,1),
	Vec3f(1,0.992156,0.996078),
	Vec3f(1,0.988235,0.984313),
	Vec3f(1,0.980392,0.972549),
	Vec3f(1,0.97647,0.960784),
	Vec3f(1,0.972549,0.952941),
	Vec3f(1,0.968627,0.941176),
	Vec3f(1,0.964705,0.929411),
	Vec3f(1,0.960784,0.917647),
	Vec3f(1,0.956862,0.905882),
	Vec3f(1,0.949019,0.898039),
	Vec3f(1,0.945098,0.886274),
	Vec3f(1,0.941176,0.878431),
	Vec3f(1,0.937254,0.866666),
	Vec3f(1,0.933333,0.854901),
	Vec3f(1,0.929411,0.843137),
	Vec3f(1,0.92549,0.835294),
	Vec3f(1,0.917647,0.823529),
	Vec3f(1,0.913725,0.815686),
	Vec3f(1,0.909803,0.803921),
	Vec3f(1,0.905882,0.796078),
	Vec3f(1,0.90196,0.784313),
	Vec3f(1,0.898039,0.77647),
	Vec3f(1,0.894117,0.764705),
	Vec3f(1,0.890196,0.752941),
	Vec3f(1,0.886274,0.745098),
	Vec3f(1,0.882352,0.737254),
	Vec3f(1,0.878431,0.72549),
	Vec3f(1,0.870588,0.713725),
	Vec3f(1,0.866666,0.705882),
	Vec3f(1,0.862745,0.694117),
	Vec3f(1,0.858823,0.686274),
	Vec3f(1,0.854901,0.674509),
	Vec3f(1,0.85098,0.666666),
	Vec3f(1,0.847058,0.654901),
	Vec3f(1,0.843137,0.643137),
	Vec3f(1,0.839215,0.635294),
	Vec3f(1,0.835294,0.62745),
	Vec3f(1,0.831372,0.615686),
	Vec3f(1,0.82745,0.607843),
	Vec3f(1,0.823529,0.596078),
	Vec3f(1,0.819607,0.588235),
	Vec3f(1,0.815686,0.57647),
	Vec3f(1,0.811764,0.568627),
	Vec3f(1,0.807843,0.556862),
	Vec3f(1,0.803921,0.545098),
	Vec3f(1,0.8,0.533333),
	Vec3f(1,0.796078,0.529411),
	Vec3f(1,0.792156,0.517647),
	Vec3f(1,0.788235,0.505882),
	Vec3f(1,0.784313,0.494117),
	Vec3f(1,0.780392,0.486274),
	Vec3f(1,0.77647,0.474509),
	Vec3f(1,0.772549,0.458823),
	Vec3f(1,0.768627,0.45098),
	Vec3f(1,0.764705,0.439215),
	Vec3f(1,0.760784,0.423529),
	Vec3f(1,0.756862,0.415686),
	Vec3f(1,0.752941,0.4),
	Vec3f(1,0.749019,0.392156),
	Vec3f(1,0.745098,0.37647),
	Vec3f(1,0.741176,0.360784),
	Vec3f(1,0.737254,0.349019),
	Vec3f(1,0.733333,0.329411),
	Vec3f(1,0.729411,0.313725),
	Vec3f(1,0.729411,0.30196),
	Vec3f(1,0.721568,0.278431),
	Vec3f(1,0.717647,0.258823),
	Vec3f(1,0.717647,0.235294),
	Vec3f(1,0.709803,0.215686),
	Vec3f(1,0.709803,0.196078),
	Vec3f(1,0.70196,0.164705),
	Vec3f(1,0.70196,0.141176),
	Vec3f(1,0.698039,0.109803),
	Vec3f(1,0.690196,0.062745),
	Vec3f(1,0.690196,0),
	Vec3f(1,0.686274,0),
	Vec3f(1,0.682352,0),
	Vec3f(1,0.678431,0),
	Vec3f(1,0.678431,0),
	Vec3f(1,0.674509,0),
	Vec3f(1,0.670588,0),
	Vec3f(1,0.666666,0),
	Vec3f(1,0.666666,0),
	Vec3f(1,0.662745,0),
	Vec3f(1,0.658823,0),
	Vec3f(1,0.654901,0),
	Vec3f(1,0.654901,0),
	Vec3f(1,0.65098,0),
	Vec3f(1,0.647058,0),
	Vec3f(1,0.643137,0),
	Vec3f(1,0.643137,0),
	Vec3f(1,0.639215,0),
	Vec3f(1,0.635294,0),
	Vec3f(1,0.631372,0),
	Vec3f(1,0.631372,0),
	Vec3f(1,0.62745,0),
	Vec3f(1,0.62745,0),
	Vec3f(1,0.623529,0),
	Vec3f(1,0.619607,0),
	Vec3f(1,0.615686,0),
	Vec3f(1,0.611764,0),
	Vec3f(1,0.611764,0),
	Vec3f(1,0.607843,0),
	Vec3f(1,0.603921,0),
	Vec3f(1,0.6,0),
	Vec3f(1,0.6,0)
};

StarGalaxy::StarGalaxy(const std::string &filename) : global(*Context::instance->uniformMgr)
{
    Context &context = *Context::instance;
    VulkanMgr &vkmgr = *VulkanMgr::instance;

	vertexArray = std::make_unique<VertexArray>(vkmgr);
	vertexArray->createBindingEntry(6*sizeof(float));
	vertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	vertexArray->addInput(VK_FORMAT_R32G32B32_SFLOAT);

	layout = std::make_unique<PipelineLayout>(vkmgr);
    layout->setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    layout->buildLayout();
    layout->build();

	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipeline->setDepthStencilMode(VK_TRUE, VK_FALSE);
    pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
    pipeline->bindVertex(*vertexArray);
    pipeline->bindShader("quickStar.vert.spv");
    pipeline->bindShader("quickStar.frag.spv");
    pipeline->build();

	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
    set->bindUniform(global, 0);

	for (int i = 0; i < 3; ++i) {
        cmds[i] = context.frame[i]->create(1);
        context.frame[i]->setName(cmds[i], "starGalaxy-draw " + std::to_string(i));
    }
	if (!filename.empty())
        loadCatalog(filename);
}

StarGalaxy::~StarGalaxy() {}

void StarGalaxy::draw(const Navigator * nav, const Projector* prj)
{
	FrameMgr &frame = *Context::instance->frame[Context::instance->frameIdx];
    global->MV = nav->getHelioToEyeMat().convert();
    global->clipping_fov = prj->getClippingFov();
	auto &cmd = frame.begin(cmds[Context::instance->frameIdx], PASS_MULTISAMPLE_DEPTH);
	pipeline->bind(cmd);
    layout->bindSet(cmd, *set);
	vertex->bind(cmd);
	vkCmdDraw(cmd, nbStars, 1, 0, 0);
	frame.compile(cmd);
    frame.toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

void StarGalaxy::loadCatalog(const std::string &filename)
{
    std::ifstream file(filename);
	std::vector<VertexEntry> tmp;
	Vec3f pos;
    // float x, y, z;
	int colorID =  rand()%128; // A "random" initial value
    while (file) {
        int t = -1;
		file >> t >> pos.v[0] >> pos.v[1] >> pos.v[2];
		colorID = (colorID + 7) % 32;
		switch (t) {
			case 1:
				break;
			case 2:
				colorID += 32;
				break;
			case 3:
				colorID += 0;
				break;
			case 4:
				colorID += 0;
				break;
			case 5:
				break;
			case 6:
				colorID += 96;
				break;
			case 7:
				colorID += 32;
				break;
			default:
				continue;
		}
		pos *= scaling;
		pos = Mat4f::translation(Vec3f( -0.0001, -0.0001, -0.005)) * Mat4f::yawPitchRoll(90, 0, 0) *pos;
		tmp.push_back({pos, color_table[colorID]});
    }
	nbStars = tmp.size();
	vertex = vertexArray->createBuffer(0, nbStars, Context::instance->globalBuffer.get(), nullptr);
	memcpy(Context::instance->transfer->planCopy(vertex->get()), tmp.data(), nbStars * sizeof(VertexEntry));
}
