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

#ifndef DSO_NAVIGATOR_HPP
#define DSO_NAVIGATOR_HPP

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include <memory>
#include <map>
#include <string>

class Projector;
class Navigator;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class s_texture;

class DsoNavigator: public NoCopy {
public:
    DsoNavigator(const std::string& tex_file, const std::string &tex3d_file = "dso3d.png", int depth = 256);
    ~DsoNavigator();
    void computePosition(Vec3f posI, const Projector *prj);
    void draw(const Navigator *nav, const Projector *prj);
    // Script version of insert
    void insert(std::map<std::string, std::string> &args);
    // Simplified version of insert
    void insert(const Vec3f &position, const Vec2f &zyrotation, const Vec3f &shaping, float scaling, int textureID);
    //! Insert a dso, textureID is the subtexture of the texture to use (like dso3d)
    void insert(const Mat4f &model, int textureID, float unscale);
    //! Override dsoNavigator resources, allow loading another set of dso
    void overrideCurrent(const std::string& tex_file, const std::string &tex3d_file, int depth);
private:
    void build();

    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertex;
    std::unique_ptr<VertexBuffer> instance;
    SubBuffer index;
    std::unique_ptr<s_texture> texture;
    std::unique_ptr<s_texture> colorTexture;
    struct dso {
        Mat4f model;
        Mat4f invmodel;
        Vec3f data; // texOffset, coefScale, lod
    };
    std::vector<dso> dsoData;
    std::vector<Vec3f> dsoPos;

    int instanceCount = 0;

    std::unique_ptr<Set> set;
    std::unique_ptr<SharedBuffer<Mat4f>> uModelViewMatrix;
    std::unique_ptr<SharedBuffer<Vec3f>> uclipping_fov;
    std::unique_ptr<SharedBuffer<Mat4f>> uCamRotToLocal; // represent a Mat3f
    VkCommandBuffer cmds[3];
    float texScale;
    bool needRebuild[3];
};

#endif /* end of include guard: DSO_NAVIGATOR_HPP */
