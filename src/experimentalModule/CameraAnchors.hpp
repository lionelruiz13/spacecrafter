#ifndef CAMERA_ANCHORS_HPP_
#define CAMERA_ANCHORS_HPP_

#include <string>
#include "tools/vecmath.hpp"

class Camera;
class ModularBody;

// ============================================================================
// SKETCH - contract only, implementation second-pass (INTENT.md gap: "anchor
// persistence / named-anchor / scripted-animation layer unmigrated").
//
// The old AnchorManager mixes two layers; the new Camera already covers the
// lower one (reference switching, motion, tracking, modes - Camera.hpp).
// What remains ABOVE Camera, and lands here:
// - named-anchor registry: addAnchor(name, ...) / removeAnchor / switchTo
//   by name, anchor.ini loading (old load/initFirstAnchor/displayAnchor)
// - persistence: saveCameraPosition / loadCameraPosition (anchors/ files)
// - scripted animated transitions: transitionToPoint / transitionToBody /
//   alignCameraToBody(duration) / moveToBody(name, time, alt)
// - rotation following: setFollowRotation / setRotationMultiplierCondition
// These are consumed exclusively through the SSystemFactory seam (CoreLink
// camera* commands) - the seam table maps each old call here (INTENT.md 9).
//
// Everything here executes on the render chain (camera state is
// draw-visible); scripted transitions are time-driven updates inside the
// frame task, not blocking waits (C3).
// ============================================================================
class CameraAnchors {
public:
    // To be designed at port time against the actual anchor.ini contents and
    // script command semantics - the method set above is the requirement
    // list extracted from the old surface; signatures land with the port.
};

#endif /* end of include guard: CAMERA_ANCHORS_HPP_ */
