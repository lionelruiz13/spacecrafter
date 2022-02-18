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

#ifndef STAR_GALAXY_HPP
#define STAR_GALAXY_HPP

#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"
#include <memory>

#include "EntityCore/Resource/SharedBuffer.hpp"

class Projector;
class Navigator;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class s_texture;

class StarGalaxy : public NoCopy {
public:
    StarGalaxy(const std::string &filename = "\0");
    ~StarGalaxy();
    void draw(const Navigator * nav, const Projector* prj);

    void loadCatalog(const std::string &filename);
    bool isLoaded() const {return nbStars > 0;}
private:
    int nbStars = 0;
    int cmds[3] = {-1, -1, -1};
    static const Vec3f color_table[128];
    struct VertexEntry {
        Vec3f pos;
        Vec3f color;
    };
    std::unique_ptr<PipelineLayout> layout;
    std::unique_ptr<Pipeline> pipeline;
    std::unique_ptr<VertexArray> vertexArray;
    std::unique_ptr<VertexBuffer> vertex;
    std::unique_ptr<Set> set;
    struct UniformData {
        Mat4f MV;
        Vec3f clipping_fov;
    };
    SharedBuffer<UniformData> global;
    float scaling = 0.0000008;
};

#endif /* end of include guard: STAR_GALAXY_HPP */
