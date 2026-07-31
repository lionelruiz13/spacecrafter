#ifndef DRAW_HELPER_HPP_
#define DRAW_HELPER_HPP_

#include "EntityCore/Tools/SafeQueue.hpp"
#include <vulkan/vulkan.h>
#include <thread>
#include <chrono>
#include <list>
#include <functional>
#include "EntityCore/Forward.hpp"
#include "EntityCore/SubTexture.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "tools/vecmath.hpp"

#define MAX_IDX 64*1024
// This work while there is no more than 96 vertices, otherwise...
#define MAX_HINT_IDX_ 32

class Hints;
class s_texture;
class VideoPlayer;
class Body;

enum DrawFlag {
    DRAW_PRINT = 1,
    DRAW_PRINTH,
    DRAW_HINT,
    DRAW_NEBULA,
    SIGNAL_PASS,
    SIGNAL_NEBULA,
    FRAME_SUBMIT,
};

struct s_print {
    unsigned char flag;
    float x;
    float y;
    float h;
    float w;
    Vec4f Color;
    SubTexture *texture;
    Mat4f MVP;
};

struct s_printh {
    unsigned char flag;
    float theta;
    float psi;
    Vec2d center;
    float d;
    float d_sub_textureH;
    Vec4f texColor;
    SubTexture *texture; // string, then border
};

struct s_sigpass {
    unsigned char flag;
    unsigned char subpass;
};

struct s_submit {
    unsigned char flag;
    unsigned char frameIdx;
    unsigned char lastFrameIdx;
};

typedef union {
    unsigned char flag;
    s_print print;
    s_printh printh;
    struct s_hint {
        unsigned char flag;
        Vec4f color;
        Hints *self;
    } hint;
    struct s_nebula {
        unsigned char flag;
        float color;
        Set *set;
        int64_t *data; // all 4 vertices
    } nebula;
    s_sigpass sigPass;
    s_submit submit;
} DrawData; // sizeof(DrawData) == 32

struct ShadowingData {
    Body *body;
    float radius;
    uint8_t idx;
};

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

    //! Stop and join the worker thread, draining any queued draws while their
    //! resources are still alive. MUST run before the registry pipelines those
    //! queued draws bind (and the ShadowService preFrameRecorder they invoke)
    //! are destroyed: ~Context calls it before releaseRegistry(), ~DrawHelper
    //! calls it again (idempotent) before freeing the helper's own resources.
    void stop();

    template <typename T>
    inline void draw(T *data) {
        while (!queue.emplace(reinterpret_cast<DrawData *>(data))) {
            queue.flush();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }
    void beginDraw(unsigned char subpass, FrameMgr &frame);
    void nextDraw(unsigned char subpass);
    void endDraw();
    void beginNebulaDraw(const Mat4f &mat);
    void endNebulaDraw();
    void nextFrame();
    VkResult acquireNextFrame();
    //! Wait until the worker has finished compiling and submitting the frame
    //! recorded under `frameIdx`. Returns FALSE only when that wait was
    //! abandoned by abandonPendingFrames(): the frame is then NOT complete and
    //! its resources must not be reused - the caller skips the frame.
    bool waitFrame(unsigned char frameIdx);
    //! Wait until the worker has consumed every queued command AND completed
    //! every frame it was given. Observing only: it leaves the per-frame
    //! bookkeeping alone, so a following waitFrame() behaves exactly as it
    //! would have. Half of the mid-session release precondition
    //! (Context::quiesceFrames - the other half is the device wait).
    void waitAllFrames();
    //! Give up on frames that will never be needed again. TEARDOWN ONLY.
    //! waitFrame() ends in an atomic wait with no timeout and no cancellation,
    //! and the teardown request is serviced by the very main loop that is
    //! holding it - so a frame the worker has not completed stops the process
    //! from exiting at all (INTENT 5.59). This stores a distinguished
    //! completion value, which is what an atomic wait already watches for: the
    //! waiter observes a changed value and returns. The frame path is
    //! untouched - no new atomic, no new store, no new wakeup per frame.
    void abandonPendingFrames();
    //! hasCompleted value meaning "waited-for, never completed" (see above).
    //! Any non-zero value ends the wait; this one says WHY, so waitFrame can
    //! tell its caller the frame is not usable.
    static constexpr int FRAME_ABANDONED = 2;
    void submitFrame(unsigned char frameIdx, unsigned char lastFrameIdx);
    void setPlayer(VideoPlayer *_player) {player = _player;}
    // Wait until the worker thread has compiled every submitted commands for this frame
    // void waitCompletionOf(int frameIdx);
    //! This can be called from the submitFunc only
    unsigned char getLastFrameIdx() const {
        return currentLastFrameIdx;
    }
    //! Initialize shadowing system
    void initShadow(float shadowRes) {
        halfShadowRes = shadowRes / 2.f;
    }
    //! Sumbit self-shadowing for ojm
    void selfShadow(Body *target) {
        drawer[externalVFrameIdx].selfShadow = target;
    }
    //! Sumbit shadowing body
    uint8_t drawShadower(Body *target, float radius);
    //! Pre-color recording hook (new-path ShadowService): invoked in submit()
    //! right after the old-path shadow recording, on the helper thread, with
    //! the frame's primary cmd outside any render pass - the same window the
    //! old shadow passes record in. One consumer by design (the service);
    //! nullptr clears. Set from the registration domain before first use.
    void setPreFrameRecorder(std::function<void(VkCommandBuffer, unsigned char)> recorder) {
        preFrameRecorder = std::move(recorder);
    }
private:
    void beginDraw(unsigned char subpass);
    void beginDrawCommand(unsigned char subpass); // Start draw command recording
    void endDrawCommand(unsigned char subpass); // Stop draw command recording
    void drawPrint(s_print &data);
    void drawPrintH(s_printh &data);
    void drawHint(DrawData::s_hint &data);
    void drawNebula(DrawData::s_nebula &data);
    void bindPrint(VkCommandBuffer cmd);
    void bindPrintH(VkCommandBuffer cmd);
    void init();
    VkCommandBuffer getCmd();
    void pushCommand();
    void mainloop();
    void submit(unsigned char frameIdx, unsigned char lastFrameIdx);
    VideoPlayer *player = nullptr;
    std::thread thread;
    std::unique_ptr<VertexArray> vertexPrint;
    std::unique_ptr<VertexArray> vertexPrintH;
    std::unique_ptr<PipelineLayout> layoutPrint;
    std::unique_ptr<PipelineLayout> layoutPrintH;
    std::unique_ptr<Set> setPrints;
    std::unique_ptr<Set> setNebula;
    std::vector<std::unique_ptr<Pipeline>> pipelinePrint;
    std::vector<std::unique_ptr<Pipeline>> pipelinePrintH;
    WorkQueue<DrawData *, 4095> queue;
    SharedBuffer<Mat4f> nebulaMat;
    FrameMgr *frame = nullptr;
    unsigned char internalVFrameIdx = 0;
    unsigned char externalVFrameIdx = 0;
    unsigned char extCmdIdx;
    bool hasRecorded = false;
    bool hasRecordedNebula = false;
    unsigned char internalSubpass;
    unsigned char externalSubpass;
    unsigned char lastFlag = SIGNAL_PASS;
    float halfShadowRes;
    std::function<void(VkCommandBuffer, unsigned char)> preFrameRecorder;
    unsigned short drawIdx = 0;
    unsigned char currentLastFrameIdx = 2;
    bool notInitialized = true;
    Vec4f hintColor;
    PipelineLayout *layoutNebula = nullptr;
    struct {
        FrameMgr *frame = nullptr;
        VkCommandPool cmdPool;
        VkCommandBuffer nebula;
        std::vector<VkCommandBuffer> cmds;
        std::vector<VkCommandBuffer> cancelledCmds;
        Body *selfShadow = nullptr;
        std::vector<ShadowingData> shadowers;
        std::list<s_sigpass> sigpass;
        // bool hasDraw; // Tell if the next command must be submitted on nextDraw/endDraw or not
        s_submit submitData {FRAME_SUBMIT, UINT8_MAX, UINT8_MAX};
        std::atomic<int> hasCompleted = 0; // Tell if every submitted commands were compiled
        unsigned char intCmdIdx;
        unsigned char realFrameIdx;
    } drawer[3];
};

#endif /* end of include guard: DRAW_HELPER_HPP_ */
