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

#ifndef CLOUD_NAVIGATOR_HPP
#define CLOUD_NAVIGATOR_HPP

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include "vulkanModule/Context.hpp"
#include <memory>

class Projector;
class Navigator;
class VertexArray;
class Pipeline;
class PipelineLayout;
class Set;
class Uniform;
class Buffer;
class Texture;

class CloudNavigator: public NoCopy {
public:
    CloudNavigator(ThreadContext *_context);
    ~CloudNavigator();
    void computePosition(Vec3f posI);
    void draw(const Navigator * nav, const Projector* prj);
    void draw(const Mat4f &mat, const Projector* prj);
    void insert(const Vec4f &color, const Mat4f &model);
private:
    void build(int nbClouds);
    ThreadContext *context;

    int commandIndex;
    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertex;
    std::unique_ptr<Texture> texture;
    struct cloud {
        Vec4f color;
        Mat4f model;
        Mat4f invmodel;
    } *pInstance;
    std::vector<cloud> cloudData;
    std::vector<Vec3f> cloudPos;

    int instanceCount = 0;

    std::unique_ptr<Set> set;
    std::unique_ptr<Uniform> uModelViewMatrix, uclipping_fov, uCamRotToLocal;
    Mat4f *pModelViewMatrix;
    Vec3f *pclipping_fov;
    Mat4f *pCamRotToLocal; // represent a Mat3f
};

#endif /* end of include guard: CLOUD_NAVIGATOR_HPP */
