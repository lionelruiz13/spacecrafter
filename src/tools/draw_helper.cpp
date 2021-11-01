#include "tools/draw_helper.hpp"
#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"
#include "tools/s_texture.hpp"

DrawHelper::DrawHelper()
{
    thread = std::thread(&DrawHelper::mainloop, this);
}

DrawHelper::~DrawHelper()
{
    queue.close();
    thread.join();
}

void DrawHelper::mainloop()
{
    DrawData *data = nullptr;
    unsigned char subpass = -1;

    queue.acquire();
    while (queue.pop(data)) {
        switch (data->flag) {
            case DRAW_PRINT:
                break;
            case DRAW_PRINTH:
                break;
            case DRAW_HINT:
                break;
            case SIGNAL_PASS:
                break;
            case DRAW_NEBULA:
                break;
            case SIGNAL_NEBULA:
                break;
        }
    }
    queue.release();
}

void DrawHelper::beginDraw(unsigned char subpass)
{
}

void DrawHelper::nextDraw(unsigned char subpass)
{
    endDraw();
    beginDraw(subpass);
}

void DrawHelper::endDraw()
{
    sigpass.emplace_back(s_sigpass{.flag=SIGNAL_PASS, .subpass=255});
    queue.emplace((DrawData *) &sigpass.back());
    queue.flush();
}

void DrawHelper::beginNebulaDraw(const Mat4f &mat)
{
    nebulaMat = mat;
    sigpass.emplace_back(s_sigpass{.flag=SIGNAL_NEBULA, .subpass=0});
    queue.emplace((DrawData *) &sigpass.back());
}

void DrawHelper::endNebulaDraw()
{
    sigpass.emplace_back(s_sigpass{.flag=SIGNAL_NEBULA, .subpass=255});
    queue.emplace((DrawData *) &sigpass.back());
}

void DrawHelper::waitCompletionOf(int frameIdx)
{
    if (!drawer[frameIdx].hasCompleted) {
        drawer[frameIdx].waiter.lock();
        drawer[frameIdx].waiter.unlock();
    }
}
