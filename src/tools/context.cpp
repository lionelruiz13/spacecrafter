#include "context.hpp"
#include "draw_helper.hpp"
#include "EntityCore/EntityCore.hpp"
#include "EntityCore/Core/RenderMgr.hpp"
#include "EntityCore/Resource/SetMgr.hpp"
#include "EntityCore/Tools/CaptureMetrics.hpp"

Context::Context()
{
    instance = this;
}

Context::~Context()
{
    instance = nullptr;
    helper.reset();
    for (auto p : pipelineArray)
        delete[] p;
}
