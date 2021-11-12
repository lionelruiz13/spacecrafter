#ifndef DRAW_HELPER_HPP_
#define DRAW_HELPER_HPP_

#include "EntityCore/Tools/SafeQueue.hpp"
#include <vulkan/vulkan.h>
#include <thread>
#include <chrono>
#include <list>
#include "EntityCore/Forward.hpp"
#include "tools/vecmath.hpp"

class Hints;
class s_texture;

enum DrawFlag {
    DRAW_PRINT = 1,
    DRAW_PRINTH,
    DRAW_HINT,
    DRAW_NEBULA,
    SIGNAL_PASS,
    SIGNAL_NEBULA,
};

struct s_print {
    unsigned char flag;
    float x;
    float y;
    float h;
    float w;
    Vec4f Color;
    Texture *string;
    Mat4f MVP;
};

struct s_printh {
    unsigned char flag;
    float theta;
    float psi;
    Vec2d center;
    float d;
    float d_sub_textureH;
    Vec3f texColor;
    Texture *border;
    Texture *string;
};

struct s_sigpass {
    unsigned char flag;
    unsigned char subpass;
};

typedef union {
    unsigned char flag;
    s_print print;
    s_printh printh;
    struct s_hint {
        unsigned char flag;
        float fader;
        Vec4f color;
        Hints *self;
    } hint;
    struct {
        unsigned char flag;
        float color;
        s_texture *neb_tex;
    } nebula;
    s_sigpass sigPass;
} DrawData; // sizeof(DrawData) == 32

/**
 * @file draw_helper.hpp
 * @class DrawHelper
 *
 * @brief Regroup several draw and move work to a dedicated thread
 *
 */
class DrawHelper {
public:
    DrawHelper();
    ~DrawHelper();

    template <typename T>
    inline void draw(T *data) {
        while (!queue.push(reinterpret_cast<DrawData *&>(data))) {
            queue.flush();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
    void beginDraw(unsigned char subpass, FrameMgr &frame);
    void nextDraw(unsigned char subpass);
    void endDraw();
    void beginNebulaDraw(const Mat4f &mat);
    void endNebulaDraw();
    void submit();
    // Wait until the worker thread has compiled every submitted commands for this frame
    // void waitCompletionOf(int frameIdx);
private:
    void beginDraw(unsigned char subpass);
    void beginDrawCommand(unsigned char subpass); // Start draw command recording
    void endDrawCommand(unsigned char subpass); // Stop draw command recording
    void drawPrint(s_print &data);
    void drawPrintH(s_printh &data);
    void drawHint(DrawData::s_hint &data);
    VkCommandBuffer getCmd();
    void pushCommand();
    void mainloop();
    std::thread thread;
    std::list<s_sigpass> sigpass;
    WorkQueue<DrawData *> queue;
    Mat4f nebulaMat;
    FrameMgr *frame = nullptr;
    unsigned char internalVFrameIdx = 0;
    unsigned char externalVFrameIdx = 0;
    unsigned char extCmdIdx;
    bool hasRecorded = false;
    bool hasRecordedNebula = false;
    unsigned char internalSubpass;
    unsigned char externalSubpass;
    unsigned char lastFlag = SIGNAL_PASS;
    struct {
        VkCommandPool cmdPool;
        VkCommandBuffer nebula;
        unsigned char intCmdIdx;
        std::vector<VkCommandBuffer> cmds;
        std::vector<VkCommandBuffer> cancelledCmds;
        std::mutex waitMutex;
        // bool hasDraw; // Tell if the next command must be submitted on nextDraw/endDraw or not
        bool hasCompleted = false; // Tell if every submitted commands were compiled
    } drawer[3];
};

#endif /* end of include guard: DRAW_HELPER_HPP_ */
