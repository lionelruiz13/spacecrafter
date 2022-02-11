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

#ifndef VOLUM_OBJ_3D_HPP_
#define VOLUM_OBJ_3D_HPP_

#include "EntityCore/Forward.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class Navigator;
class Projector;
class s_texture;

// Volumetric 3D object
class VolumObj3D {
public:
    VolumObj3D(const std::string& tex_color_file, const std::string &tex_absorbtion_file, bool z_reflection);
    ~VolumObj3D();
    void setModel(const Mat4f &model, const Vec3f &scale);
    void draw(const Navigator * nav, const Projector* prj);
    bool loaded() const {return isLoaded;}
private:
    struct Transform {
        Mat4f ModelViewMatrix;
        Mat4f NormalMatrix; // Inverse rotation and scaling of View matrix, and rotation of Model matrix
        Vec3f clipping_fov;
    };
    struct Ray {
        Vec3f texCoef; // Coefficient for the texture
        float rayPoints; // Number of points sampled for a ray of length 1
        Vec3f rayCoef; // Length of the ray relative
    };
    struct Shared {
        Shared();
        ~Shared();
        std::unique_ptr<Pipeline> pipeline;
        std::unique_ptr<PipelineLayout> layout;
        std::unique_ptr<VertexArray> vertexArray;
        std::unique_ptr<VertexBuffer> vertex;
        SubBuffer index;
    };
    SharedBuffer<Transform> transform;
    SharedBuffer<Ray> ray;
    std::unique_ptr<s_texture> mapTexture;
    std::unique_ptr<s_texture> colorTexture;
    std::unique_ptr<Set> set;
    std::shared_ptr<Shared> shared;
    Mat4f model;
    int cmds[3] {-1, -1, -1};
    bool isLoaded = false;
    static std::weak_ptr<Shared> refShared;
};

#endif /* end of include guard: VOLUM_OBJ_3D_HPP_ */
