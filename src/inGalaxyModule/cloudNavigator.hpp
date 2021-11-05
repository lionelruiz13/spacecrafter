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
#include <memory>

#include "EntityCore/SubBuffer.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

class Projector;
class Navigator;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class s_texture;

class CloudNavigator: public NoCopy {
public:
    CloudNavigator();
    ~CloudNavigator();
    void computePosition(Vec3f posI);
    void draw(const Navigator * nav, const Projector* prj);
    void draw(const Mat4f &mat, const Projector* prj);
    void insert(const Vec4f &color, const Mat4f &model);
private:
    void build(int nbClouds);

    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertex;
    std::unique_ptr<VertexBuffer> instance;
    SubBuffer index;
    std::unique_ptr<s_texture> texture;
    struct cloud {
        Vec4f color;
        Mat4f model;
        Mat4f invmodel;
    };
    std::vector<cloud> cloudData;
    std::vector<Vec3f> cloudPos;

    int instanceCount = 0;
    int cmds[3] = {-1, -1, -1};

    std::unique_ptr<Set> set;
    std::unique_ptr<SharedBuffer<Mat4f>> uModelViewMatrix;
    std::unique_ptr<SharedBuffer<Vec3f>> uclipping_fov;
    std::unique_ptr<SharedBuffer<Mat4f>> uCamRotToLocal; // represent a Mat3f
};

#endif /* end of include guard: CLOUD_NAVIGATOR_HPP */
