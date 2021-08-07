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

#include "vulkanModule/Context.hpp"
#include "tools/vecmath.hpp"
#include <memory>

class Projector;
class Navigator;
class Pipeline;
class Set;
class Uniform;
class Buffer;
class VertexArray;
class ObjL;

class StarViewer {
public:
    StarViewer(const Vec3f &pos, const Vec3f &color, const float radius);
    ~StarViewer();
    void draw(const Navigator * nav, const Projector* prj, const Mat4f &mat);
    static void createSC_context(ThreadContext *_context);
private:
    void createLocalContext();
    //! build command
    void build();
    float getOnScreenSize(const Projector* prj, const Vec3f &pos);

    static ThreadContext *context;
    static PipelineLayout *layout;
    static Pipeline *pipeline, *pipelineCorona, *pipelineHalo;
    Mat4f model;
    int commandIndex;
    std::unique_ptr<Set> set;
    std::unique_ptr<Uniform> uVert, uFrag;
    std::unique_ptr<Buffer> drawData;
    std::vector<float> vertices;
    struct {
        Mat4f ModelViewMatrix;
        Vec3f clipping_fov;
        float radius;
    } *pVert;
    struct {
        Vec3f color;
        float radius;
        Vec3f cam_view;
    } *pFrag;
    float radius;
    std::unique_ptr<VertexArray> vertexHalo;
    std::unique_ptr<ObjL> objl;
};
