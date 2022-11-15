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

 #include "EntityCore/Resource/SharedBuffer.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class Projector;
class Navigator;
class Pipeline;
class PipelineLayout;
class Set;
class VertexArray;
class VertexBuffer;
class ObjL;

class StarViewer {
public:
    StarViewer(const Vec3f &color, const float radius, ObjL *_objl);
    ~StarViewer();
    inline void setRadius(float _radius) {
        radius = _radius;
        (*uFrag)->radius = _radius;
        (*uVert)->radius = _radius;
    }
    void draw(const Navigator * nav, const Projector* prj, const Mat4d &mat, float screen_size);
    static void createSC_context();
private:
    void createLocalContext();

    static PipelineLayout *layout;
    static Pipeline *pipeline, *pipelineCorona;
    static std::unique_ptr<VertexArray> modelHalo;
    int cmds[3];
    std::unique_ptr<Set> set;
    struct s_vert {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
        float radius;
    };
    struct s_frag {
        Vec3f color;
        float radius;
        Vec3f cam_view;
    };
    std::unique_ptr<SharedBuffer<s_vert>> uVert;
    std::unique_ptr<SharedBuffer<s_frag>> uFrag;
    std::unique_ptr<VertexBuffer> vertexHalo;
    float *pVertexHalo;
    float radius;
    ObjL *objl;
};
