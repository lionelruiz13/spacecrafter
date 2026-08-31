#ifndef RENDER_CHAIN_HPP_
#define RENDER_CHAIN_HPP_

#include "EntityCore/Executor/Taskable.hpp"
#include <memory>
#include <vector>

class ModularBody;

// ============================================================================
// The render chain - the "effective thread" of the rendering domain.
// Normative contract (INTENT.md S8.2); on divergence between this header and
// INTENT.md, reconcile explicitly - neither silently wins.
//
// All draw-visible state (the body tree, slots, regime lists, notableBody,
// descriptors bound to rendering, ...) is protected by THIS chain: everything
// touching that state runs as a Task submitted here. Sequential consistency
// comes from the Taskable ordering promise, NOT from thread identity - a task
// may execute on the main thread (inplace execute), on a worker, or on the
// events thread; all are correct by construction. C1 = chain serialization,
// not thread affinity. Per-body publish ordering is automatic: a single FIFO
// chain preserves per-key order.
//
// Invariants (each is load-bearing; violating any invalidates the design):
// - C1: no draw-visible state is touched outside a task of this chain.
// - C2: a ModularBody with pending work-domain tasks is pinned (see
//   ModularBody pin contract). Pins are plain non-atomic ints BECAUSE both
//   pin and unpin only ever execute inside chain tasks - moving either
//   outside the chain breaks the model, not just a convention.
// - C3: the frame task never blocks on loading. Fence discipline: fence
//   waits happen as late as possible, only where the touched resource
//   requires them, and only in the rendering part - with frames in flight,
//   the acquire/present semaphore structure makes an actual block genuinely
//   unlikely; the fence is the guarantee, not the expected wait.
// - C4: work-domain tasks (loading, building, encoding) may block freely and
//   hold asynchronously (start() returning != done), and NEVER touch
//   draw-visible state; their final step schedules a lightweight publish
//   task here. A publish is O(small): a pointer or descriptor swap -
//   anything heavier belongs to the work domain. Violations show up as
//   frame spikes attributable at the offending task's definition site.
// - Stack: endTask cascades nest on the stack (unless tail-call optimized);
//   the chain going idle between frames resets the depth. Bounded bursts are
//   the sizing assumption - do not design workloads that keep this chain
//   busy without interruption.
//
// Frame flow: the main thread is a pure cadencer - it execute()s the frame
// task at the target framerate and waits in-between. Update+draw are fused
// in that single frame task (the residual synchronous update part is cheap,
// cache-local, and uses the exact elapsed time in non-recording mode). When
// the chain is idle, the frame task runs inplace on the main thread (lowest
// latency); otherwise it chains behind pending publishes - so publishes
// always execute between two frames, never inside one.
//
// Thread model (INTENT.md S8.2.10):
//   1 video-player thread (video frame loading)
//   1 main thread (cadencer, usually runs the frame task inplace)
//   1 events/scripts thread (SDL polling, scripts, non-recurrent actions:
//     bodies are BUILT offline there and ATTACHED via a task on this chain)
//   1 compute thread (shadow projection, owns a compute VkQueue)
//   1 ACTIVE-only worker (guaranteed free lane for the "needed now" class)
//   nproc-3 generic workers (default, configurable), executing every task
// The dedicated threads are rarely stressed simultaneously (< 3 active at a
// time in real-world use); REALTIME FIFO scheduling is viable by design.
// ============================================================================
class RenderChain : public Taskable {
public:
    // Park a body whose destruction must wait for its pins to drain (C2).
    // Contract: call only from a task of this chain, with the body already
    // detached from the tree - the world-visible disappearance is immediate
    // (ModularBodyPtr::redirect philosophy); only the memory lingers here.
    void park(std::unique_ptr<ModularBody> body);

    // Notify that a parked body's pin count reached zero - destroys it.
    // Called from the body's unpin path; runs inside the chain by
    // construction (unpin is chain-only), so no synchronization is needed.
    void onPinDrained(ModularBody *body);

    // The unique render chain. Application lifetime - never destroyed
    // (all Taskables live for the whole duration of the application;
    // the TASKABLE_* reference-counting flags stay off, see Taskable.hpp).
    static RenderChain instance;
private:
    std::vector<std::unique_ptr<ModularBody>> parked;
};

#endif /* end of include guard: RENDER_CHAIN_HPP_ */
