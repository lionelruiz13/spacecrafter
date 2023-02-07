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

#include "volumObj3D.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/s_texture.hpp"
#include "tools/context.hpp"
#include "tools/log.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "ojmModule/objl_mgr.hpp"
#include "ojmModule/objl.hpp"

#define vkmgr (*VulkanMgr::instance)
#define context (*Context::instance)

enum PipelineSelection {
    PS_OUT = 0,
    PS_SPLIT = 0,
    PS_ZREFLECT = 1,
    PS_PACKED = 2,
    PS_IN = 4,
    PS_COUNT = 8,
};

// Pipeline Constructor Arguments
#define PCA {vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH}

const char *_names[] = {
    "VolumObj3D external splitted", "VolumObj3D external mirror",
    "VolumObj3D external unified", "VolumObj3D external unified mirror",
    "VolumObj3D internal splitted", "VolumObj3D internal mirror",
    "VolumObj3D internal unified", "VolumObj3D internal unified mirror",
};

struct VolumObj3D::Shared {
    Shared() :
        vertexArray(vkmgr), inVertexArray(vkmgr, context.ojmAlignment),
        layout{{vkmgr}, {vkmgr}, {vkmgr}, {vkmgr}, {vkmgr}, {vkmgr}, {vkmgr}, {vkmgr}},
        pipeline{PCA, PCA, PCA, PCA, PCA, PCA, PCA, PCA}
    {
        // Specialization constants
        const float colorScale = 2;

        // General ressources
        vertexArray.createBindingEntry(3*sizeof(float));
        vertexArray.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        inVertexArray.createBindingEntry(8*sizeof(float)); // Because we use a SphereObjL
        inVertexArray.addInput(VK_FORMAT_R32G32B32_SFLOAT);
        auto blend = BLEND_SRC_ALPHA;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        VkSamplerCreateInfo sampler = PipelineLayout::DEFAULT_SAMPLER;
        sampler.anisotropyEnable = VK_FALSE;

        // Internal Shape
        obj = ObjLMgr::instance->select("EquiSphere");

        // External Shape
        vertex = vertexArray.createBuffer(0, 8, context.globalBuffer.get());
        Vec3f *ptr = (Vec3f *) context.transfer->planCopy(vertex->get());
        ptr[0].set(-1,1,1); ptr[1].set(1,1,1);
        ptr[2].set(-1,-1,1); ptr[3].set(1,-1,1);
        ptr[4].set(-1,-1,-1); ptr[5].set(1,-1,-1);
        ptr[6].set(-1,1,-1); ptr[7].set(1,1,-1);
        index = context.indexBufferMgr->acquireBuffer(3*2*6*sizeof(uint16_t));
        uint16_t tmp[3*2*6] = {
            2,0,1, 1,3,2,
            4,2,3, 3,5,4,
            6,4,5, 5,7,6,
            0,6,7, 7,1,0,
            0,2,4, 4,6,0,
            1,7,5, 5,3,1
        };
        memcpy(context.transfer->planCopy(index), tmp, 3*2*6*sizeof(uint16_t));

        // External View
        for (int i = 0; i < PS_IN; ++i) {
            layout[i].setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 0);
            pipeline[i].setTopology(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
            pipeline[i].setTessellationState(3);
            pipeline[i].bindVertex(vertexArray);
            pipeline[i].bindShader("volumObj3D.vert.spv");
            pipeline[i].setSpecializedConstant(7, context.isFloat64Supported);
            pipeline[i].bindShader("volumObj3D.tesc.spv");
            pipeline[i].bindShader("volumObj3D.tese.spv");
            pipeline[i].setSpecializedConstant(7, context.isFloat64Supported);
            pipeline[i].bindShader((i & PS_PACKED) ? "volumObj3DPacked.frag.spv" : "volumObj3D.frag.spv");
        }

        // Internal View
        auto &screenRect = vkmgr.getScreenRect();
        int radius2 = screenRect.extent.width / 2;
        int center[2] = {screenRect.offset.x+radius2, abs(screenRect.offset.y)+radius2};
        radius2 *= radius2;
        for (int i = PS_IN; i < PS_COUNT; ++i) {
            layout[i].setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
            pipeline[i].setFrontFace();
            pipeline[i].bindVertex(inVertexArray);
            pipeline[i].bindShader("inVolumObj3D.vert.spv");
            pipeline[i].setSpecializedConstant(7, context.isFloat64Supported);
            pipeline[i].bindShader((i & PS_PACKED) ? "inVolumObj3DPacked.frag.spv" : "inVolumObj3D.frag.spv");
            pipeline[i].setSpecializedConstant(2, &radius2, sizeof(radius2));
            pipeline[i].setSpecializedConstant(3, center, sizeof(int));
            pipeline[i].setSpecializedConstant(4, center+1, sizeof(int));
        }

        // Any View
        for (int i = 0; i < PS_COUNT; ++i) {
            VkBool32 zreflect = (i & PS_ZREFLECT) ? VK_TRUE : VK_FALSE;
            sampler.maxLod = (i & PS_IN) ? 0.f : VK_LOD_CLAMP_NONE;
            sampler.addressModeW = zreflect ? VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            layout[i].setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1);
            layout[i].setTextureLocation(2, &sampler);
            if (!(i & PS_PACKED))
                layout[i].setTextureLocation(3, &sampler);
            layout[i].buildLayout();
            layout[i].build();
            pipeline[i].setCullMode(true);
            pipeline[i].setDepthStencilMode();
            pipeline[i].disableSampleShading();
            pipeline[i].setBlendMode(blend);
            pipeline[i].bindLayout(layout[i]);
            pipeline[i].setSpecializedConstant(0, &colorScale, sizeof(colorScale));
            pipeline[i].setSpecializedConstant(1, &zreflect, sizeof(zreflect));
            pipeline[i].build(_names[i]);
        }
    }

    ~Shared() {
        Context::instance->indexBufferMgr->releaseBuffer(index);
    }
    VertexArray vertexArray, inVertexArray;
    PipelineLayout layout[PS_COUNT];
    Pipeline pipeline[PS_COUNT];
    std::unique_ptr<VertexBuffer> vertex;
    ObjL *obj;
    SubBuffer index;
};

std::weak_ptr<VolumObj3D::Shared> VolumObj3D::refShared;

VolumObj3D::VolumObj3D(const std::string& tex_color_file, const std::string &tex_absorbtion_file, bool z_reflection) : transform(*Context::instance->uniformMgr), ray(*Context::instance->uniformMgr), inTransform(*Context::instance->uniformMgr), inCamCoord(*Context::instance->uniformMgr), shared(refShared.lock())
{
    if (!shared)
        refShared = shared = std::make_shared<Shared>();

    for (int i = 0; i < 3; ++i) {
        cmds[i] = context.frame[i]->create(1);
        context.frame[i]->setName(cmds[i], "VolumObj3D " + std::to_string(i));
    }
    if (tex_color_file.empty())
        return;
    reconstruct(tex_color_file, tex_absorbtion_file, rayPoints, z_reflection);
    setModel(Mat4f::translation(Vec3f( -0.0001, -0.0001, -0.005)) * Mat4f::yawPitchRoll(90, 0, 0) * Mat4f::scaling(0.01), Vec3f(1, 1, 1/8.));
}

VolumObj3D::~VolumObj3D()
{
    for (int i = 0; i < 3; ++i) {
        Context::instance->frame[i]->destroy(cmds[i]);
    }
}

void VolumObj3D::reconstruct(const std::string& tex_color_file, const std::string &tex_absorbtion_file, int _rayPoints, bool z_reflection, int colorDepth, int absorbtionDepth, int colorDepthColumn)
{
    selected = tex_absorbtion_file.empty() ? PS_PACKED : PS_SPLIT;
    if (colorDepth == 0) {
        int tmpSize = tex_color_file.find_last_of('.');
        if (*reinterpret_cast<const int*>(tex_color_file.data()+tmpSize) != 0x7761722e) { // Check if the extension is not ".raw"
            int tmpPos = tex_color_file.find_last_of('d', tmpSize) + 1;
            colorDepth = std::stoi(tex_color_file.substr(tmpPos, tmpSize - tmpPos));
        }
    }
    int size;
    if (selected) {
        mapTexture.reset();
        isLoaded = true;
    } else {
        if (absorbtionDepth == 0) {
            int tmpSize = tex_absorbtion_file.find_last_of('.');
            int tmpPos = tex_absorbtion_file.find_last_of('d', tmpSize) + 1;
            absorbtionDepth = std::stoi(tex_absorbtion_file.substr(tmpPos, tmpSize - tmpPos));
        }
        mapTexture = std::make_unique<s_texture>(tex_absorbtion_file, TEX_LOAD_TYPE_PNG_SOLID, false, false, absorbtionDepth, 1, 1, false, true);
        mapTexture->getDimensions(size, size);
        isLoaded = (size >= 8);
    }
    colorTexture = std::make_unique<s_texture>(tex_color_file, TEX_LOAD_TYPE_PNG_SOLID, false, false, colorDepth, 4, 1, false, true, colorDepthColumn);
    colorTexture->getDimensions(size, size);
    if (size < 8)
        isLoaded = false;
    if (isLoaded) {
        rayPoints = (_rayPoints) ? _rayPoints : 512;
        // Assume width and height are equal
        ray->texCoef = Vec3f(1, 1, (z_reflection) ? 2 : 1);
        ray->rayPoints = rayPoints;
        if (z_reflection)
            selected |= PS_ZREFLECT;

        set = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, shared->layout + (selected | PS_OUT), -1, true, true);
        set->bindUniform(transform, 0);
        set->bindUniform(ray, 1);
        inSet = std::make_unique<Set>(*VulkanMgr::instance, *Context::instance->setMgr, shared->layout + (selected | PS_IN), -1, true, true);
        inSet->bindUniform(inTransform, 0);
        inSet->bindUniform(inCamCoord, 1);
        if (selected & PS_PACKED) {
            set->bindTexture(colorTexture->getTexture(), 2);
            inSet->bindTexture(colorTexture->getTexture(), 2);
        } else {
            set->bindTexture(mapTexture->getTexture(), 2);
            set->bindTexture(colorTexture->getTexture(), 3);
            inSet->bindTexture(mapTexture->getTexture(), 2);
            inSet->bindTexture(colorTexture->getTexture(), 3);
        }
    } else {
        cLog::get()->write("Volumetric texture missing", LOG_TYPE::L_WARNING);
    }
}

void VolumObj3D::drop()
{
    isLoaded = false;
    set.reset();
    inSet.reset();
    mapTexture.reset();
    colorTexture.reset();
}

void VolumObj3D::setModel(const Mat4f &_model, const Vec3f &scale)
{
    model = _model * Mat4f::scaling(scale);
    ray->rayCoef = scale;
    inTransform->invScale.v[0] = 1/scale.v[0]/rayPoints;
    inTransform->invScale.v[1] = 1/scale.v[1]/rayPoints;
    inTransform->invScale.v[2] = 1/scale.v[2]/rayPoints;
}

void VolumObj3D::draw(const Navigator * nav, const Projector* prj)
{
    Mat4f mat = nav->getHelioToEyeMat().convert() * model;
    Vec3f camCoord = (mat.inverseUntranslated() * -mat.getTranslation()) / 2 + Vec3f(0.5f, 0.5f, 0.5f);
    Vec3f reClamp;
    for (int i = 0; i < 3; ++i) {
        if (camCoord.v[i] < 0) {
            reClamp.v[i] = -camCoord.v[i];
        } else if (camCoord.v[i] > 1) {
            reClamp.v[i] = 1 - camCoord.v[i];
        } else {
            reClamp.v[i] = 0;
        }
    }

    VkCommandBuffer cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
    if (reClamp.lengthSquared() > 0.04) { // more than 20% outside range
        transform->ModelViewMatrix = mat;
        transform->NormalMatrix = mat.inverseUntranslated();
        transform->clipping_fov = prj->getClippingFov();

        shared->pipeline[selected].bind(cmd);
        shared->layout[selected].bindSet(cmd, *set);
        shared->vertex->bind(cmd);
        vkCmdBindIndexBuffer(cmd, shared->index.buffer, shared->index.offset, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd, 3*2*6, 1, 0, 0, 0);
    } else {
        // camCoord += reClamp; // Disable reclamping
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j)
                inTransform->ModelViewMatrix[i].v[j] = mat.r[i*4+j];
        }
        inTransform->fov = prj->getFov()*M_PI/360;
        inCamCoord = camCoord;

        shared->pipeline[selected | PS_IN].bind(cmd);
        shared->layout[selected | PS_IN].bindSet(cmd, *inSet);
        shared->obj->bind(cmd);
        shared->obj->draw(cmd, 1024);
    }
    context.frame[context.frameIdx]->compile(cmd);
    context.frame[context.frameIdx]->toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

bool VolumObj3D::isInside(const Navigator *nav)
{
    Mat4f mat = nav->getHelioToEyeMat().convert() * model;
    Vec3f camCoord = (mat.inverseUntranslated() * -mat.getTranslation()) / 2 + Vec3f(0.5f, 0.5f, 0.5f);
    Vec3f reClamp;
    for (int i = 0; i < 3; ++i) {
        if (camCoord.v[i] < 0) {
            reClamp.v[i] = -camCoord.v[i];
        } else if (camCoord.v[i] > 1) {
            reClamp.v[i] = 1 - camCoord.v[i];
        } else {
            reClamp.v[i] = 0;
        }
    }
    if (reClamp.lengthSquared() > 0.04) // more than 20% outside range
        return false;

    // camCoord += reClamp;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j)
            inTransform->ModelViewMatrix[i].v[j] = mat.r[i*4+j];
    }
    inCamCoord = camCoord;
    return true;
}

void VolumObj3D::drawInside(const Navigator * nav, const Projector* prj)
{
    VkCommandBuffer cmd = context.frame[context.frameIdx]->begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);

    inTransform->fov = prj->getFov()*M_PI/360;
    shared->pipeline[selected | PS_IN].bind(cmd);
    shared->layout[selected | PS_IN].bindSet(cmd, *inSet);
    shared->obj->bind(cmd);
    shared->obj->draw(cmd, 1024);
    context.frame[context.frameIdx]->compile(cmd);
    context.frame[context.frameIdx]->toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
}

Mat4f VolumObj3D::drawExternal(const Navigator * nav, const Projector* prj)
{
    Mat4f mat = nav->getHelioToEyeMat().convert() * model;
    transform->ModelViewMatrix = mat;
    transform->NormalMatrix = mat.inverseUntranslated();
    transform->clipping_fov = prj->getClippingFov();
    return mat;
}

void VolumObj3D::recordVolumetricObject(VkCommandBuffer cmd)
{
    shared->pipeline[selected].bind(cmd);
    shared->layout[selected].bindSet(cmd, *set);
    shared->vertex->bind(cmd);
    vkCmdBindIndexBuffer(cmd, shared->index.buffer, shared->index.offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, 3*2*6, 1, 0, 0, 0);
}
