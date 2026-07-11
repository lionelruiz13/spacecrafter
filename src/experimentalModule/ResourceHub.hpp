#ifndef RESOURCE_HUB_HPP_
#define RESOURCE_HUB_HPP_

#include <cstdint>

// ============================================================================
// Generic resource layer - CONTRACT (INTENT.md 8.2.6-8, D6/D7).
// DRAFT declarations, normative contract: pending convergence with Vixy.
// Home: experimentalModule for now; may migrate to EntityCore once proven
// (second pass).
//
// The unit of loading is the RESOURCE, not the (body, module) pair - some
// textures and most models are heavily shared (D7). Therefore:
// - One keyed registry deduplicates by resource identity across textures AND
//   models. Existing backends to adapt behind it (decision 2026-07-11):
//   s_texture (its weak_ptr texCache registry, triple-buffered deferred
//   VkImage destruction, and dedicated transfer-queue loader thread stay
//   load-bearing) and LazyOjmL for models (native-form cache, build-once,
//   no eviction pressure - D6).
// - Resource objects are refcounted (the AsyncBuilder::useCount pattern);
//   attachment/detachment happen on the render chain, completion fan-out
//   schedules one publish task per attached (body, module).
// - Textures carry a 2-3 level LoD ladder: the LOWEST level is always
//   resident (with drawLoaded, something drawable always exists); higher
//   levels load/unload by relevance - the ResourcePriority below, fed by the
//   visibility pipeline (preUpdate/screenSize) - or by anticipation.
// - Eviction is relevance-driven with hysteresis (load above a threshold,
//   unload below a lower one, plus a grace period - the gc.hpp
//   preservationCycles pattern; reuse-vs-inline decided at implementation
//   time). Eviction decisions are made on the render chain (owner of
//   relevance); GPU object destruction is deferred by frames-in-flight
//   (s_texture's releaseTexture[3] pattern, generalized).
// - Heavy work (decode, build, upload, fence-wait on the transfer) is
//   work-domain (C4: may block); the render chain receives only O(small)
//   publish tasks (pointer/descriptor swaps). A publish task carries the
//   body pin it inherited from request emission and unpins at execution
//   (C2 - see RenderChain.hpp).
//
// Relation to EntityCore's LoadPriority (AsyncBase.hpp): LoadPriority is the
// per-TASK lifecycle+priority word (fused encoding, includes states);
// ResourcePriority is the per-RESOURCE residency level. A ResourcePriority
// change re-prioritizes the resource's pending tasks (atomic store on their
// LoadPriority). Deliberately not merged: different levels, different
// lifetimes.
// ============================================================================

//! @brief The level of prioritisation of a resource
//! (Moved from ModularBody.hpp - it is a resource concept, not a body one.)
enum class ResourcePriority : uint8_t {
    UNLOADED, // No resources acquired: explicitly unloaded, or owned by an inner ModularBody (or child of it) while the camera is outside its area of influence
    LAZY, // Only minimal resources shall be loaded, in background (default)
    BACKGROUND, // High resolution will probably be needed (lower resolution in use)
    PRELOAD, // High resolution needed in the near future (preload request)
    ACTIVE, // Currently needed (ex: missing resolution expected, resource in use)
};

#endif /* end of include guard: RESOURCE_HUB_HPP_ */
