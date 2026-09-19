#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/vecmath.hpp"
#include "tools/utility.hpp"
#include "tools/sc_const.hpp"
#include "EntityCore/Executor/Task.hpp"
#include <cmath>
#include <atomic>
#include <iosfwd>

class Renderer;
class ModularBody;
class ModularSystem;
namespace ModularSystemFormat {
    class Section;
}

enum class CameraMount : uint8_t {
    ALTAZ, // horizontal coordinate system
    EQUATORIAL, // equatorial coordinate system
};

class Camera {
public:
    Camera(ModularBody *reference, float longitude, float latitude, float altitude);
    // Return the distance to the reference (minus datum_radius) in AU
    float distanceToReference() const;
    //! Calculate velocity scaling factor for camera movements
    float velocityScaling(float deltaTime) const;
    // Serialize the observer state as one JSON object (trace harness)
    void dumpTrace(std::ostream &out) const;
    // Session state; anything in flight is saved settled. The reference body is restored by the caller (a warp)
    void saveSession(ModularSystemFormat::Section &out) const;
    void restoreSession(const ModularSystemFormat::Section &in);
    // Change the reference body without moving
    void switchToBody(ModularBody *dst);
    // Change the reference body without accomodation
    void warpToBody(ModularBody *dst);
    void update(double jd, float deltaTime);
    void draw(Renderer &renderer);
    void setBoundToSurface(bool b);
    void setFreeMode(bool b);

    // Set the view with constant-acceleration smoothing
    // isMaxDuration true  -> smallest constant-acceleration for furthest rotation
    // isMaxDuration false -> smallest constant-acceleration for this rotation
    void lookTo(const Vec3f &direction, float duration = 1, bool isMaxDuration = false);
    void lookTo(float _alt, float _az, float duration = 1, bool isMaxDuration = false);
    // Deltas in the convention of Navigator::updateMove; duration <= 0 snaps
    void lookRel(float deltaAlt, float deltaAz, float duration = 1, bool isMaxDuration = false);

    // TODO move out or make unnecessary
    static inline Vec3f oldLocalToLocal(const Vec3f &v) {
        return Vec3f(v[1], -v[0], v[2]);
    }

    // Change the mount, keeping the current view exactly
    void setMount(CameraMount m);
    inline void toggleMount() {
        setMount((mount == CameraMount::ALTAZ) ? CameraMount::EQUATORIAL : CameraMount::ALTAZ);
    }

    // Hold the view fixed in the equatorial frame of the reference; dormant while tracking, during a view plan and in freeMode
    void setSkyLock(bool b);
    inline bool getSkyLock() const {
        return skyLocked;
    }

    // Readbacks for the trace harness
    inline Vec3f getViewParams() const {
        return Vec3f(alt, az, heading);
    }
    inline const Vec3f &getAbsFwd() const {
        return lastAbsFwd;
    }
    inline Vec3f getForwardLocal() const {
        return fold().transpose().multiplyWithoutTranslation(paramForward());
    }
    inline Vec4f getPlanTimers() const {
        return Vec4f(viewT, hdgT, zoomDuration, moveDuration);
    }
    inline float getZoomSrcHalfFov() const {
        return srcHalfFov;
    }
    inline float getZoomDstHalfFov() const {
        return dstHalfFov;
    }

    void moveHeading(float deltaHeading);
    void setHeading(float heading, float duration = 0);
    inline float getHeading() const {
        return heading;
    }

    void moveRel(const Vec3f &position, float duration = 0, bool calculateDuration = false);
    inline void moveEyeRel(const Vec3f &position, float duration = 0) {
        moveRel(observedToLocalPos(position), duration);
    }
    inline void moveRelLon(float lon, float delay = 0) {
        if (freeMode) {
            lon *= velocityScaling(1) * 5.f;
            moveEyeRel({lon, 0, 0}, delay);
        } else {
            moveRel({lon, 0, 0}, delay);
        }
    }
    inline void moveRelLat(float lat, float delay = 0) {
        if (freeMode) {
            lat *= velocityScaling(1) * 5.f;
            moveEyeRel({0, lat, 0}, delay);
        } else {
            moveRel({0, lat, 0}, delay);
        }
    }
    // With alt in meters
    inline void moveRelAlt(double alt, float delay = 0) {
        alt /= 1000*AU;
        if (freeMode) {
            moveEyeRel({0, 0, static_cast<float>(-alt)}, delay);
        } else {
            moveRel({0, 0, static_cast<float>(alt)}, delay);
        }
    }
    // Multiply the altitude by coef; in free flight, along the view ray (body) or toward the selected body (system)
    void multAlt(float coef);
    // pos = (longitude, latitude, altitude in AU), in both modes
    void moveTo(const Vec3f &pos, float duration = 0, bool calculateDuration = false);

    // viewRotation() = Z(heading+pi) * X(pi/2-alt) * Z(az-pi/2) * fold(); every inverse is derived from it
    Mat4f fold() const;
    Mat4f viewRotation() const;
    // Rotation applied downstream of the view (placement + surface fold)
    Mat4f placementRotation() const;

    // Shift of the dome centre as a fraction of its radius; render-only, inert until armed
    void setViewOffset(double offset);
    inline double getViewOffset() const {
        return viewOffset;
    }
    // Armed by a commanded view move, disarmed by the zoom-out to the initial view
    void armViewOffset(bool armed);
    // armViewOffset with the transition landed
    void restoreViewOffsetLatch(bool armed);

    // Observed (eye) frame -> the frame the view acts on: zenith frame when anchored, body frame in freeMode
    inline Vec3f observedToLocalPos(const Vec3f &observedPos) const {
        return renderViewRotation().transpose().multiplyWithoutTranslation(observedPos);
    }
    // Observed frame -> equatorial frame of the reference, rotation only (the origin stays at the observer)
    Vec3f observedToBodyEquPos(const Vec3f &observedPos) const;
    // Same, with the origin at the centre of the reference (inverse of viewMat)
    Vec3f observedToBodyLocalPos(const Vec3f &observedPos) const;
    // (ra, de) of an observed position, as seen from the observer
    std::pair<float, float> observedPosToRaDe(const Vec3f &observedPos) const;
    inline std::pair<float, float> observedPosToAltAz(const Vec3f &observedPos) const {
        Vec3f direction = observedToLocalPos(observedPos);
        std::pair<float, float> ret;
        if (direction[0] == 0 && direction[1] == 0) { // was x+y==0, see RaDe
            ret.first = std::copysign(M_PI_2, direction[2]);
            ret.second = 0;
        } else {
            Utility::rectToSphe(&ret.second, &ret.first, direction);
        }
        return ret;
    }
    inline void trackBody(nullptr_t) {
        target = nullptr;
    }
    inline void trackBody(ModularBody *body) {
        target = body;
    }
    inline ModularBody *getReferenceBody() const {
        return reference;
    }
    inline ModularBody *getTrackedBody() const {
        return target;
    }
    // Re-seat the reference on the object which replaced it (system reload): nothing moves
    inline void rebindReference(ModularBody *dst) {
        reference = dst;
    }
    inline ModularSystem *getCurrentSystem() const {
        return system;
    }
    inline bool isFreeMode() const {
        return freeMode;
    }
    void setHalfFov(float halfFov, float duration = 0.5);

    // Position of the eye from the centre of the reference, in its equatorial frame
    Vec3f getReferenceRelativePosition() const;
    // Same, in the equatorial frame of body
    Vec3f positionRelativeTo(const ModularBody *body) const;
    // Position of the eye in the root frame
    Vec3d getRootPosition() const;
    // Inverse of getReferenceRelativePosition; holdView keeps the composed orientation
    void placeAt(const Vec3f &pos, bool holdView);
    // Inverse of moveTo: (longitude, latitude, altitude in AU), in both modes
    Vec3f getPlace() const;

    // Compatibility methods, only work while not in freeMode
    inline void setLongitude(float l) {
        longitude = l*M_PI/180;
    }
    inline void setLatitude(float l) {
        latitude = l*M_PI/180;
    }
    void setAltitude(double altitude);
    inline float getLatitude() const {
        return latitude;
    }
    inline float getLongitude() const {
        return longitude;
    }
    // This disallow using multiple cameras, but multiple cameras can't be used simultaneously anyway
    static Camera *instance;
private:
    // Zenith frame -> equatorial frame of the reference
    Vec3f localToBodyEqu(Vec3f v) const;
    // (longitude, latitude, distance) <-> position from the centre of the reference: the one conversion between both
    static inline Vec3f posePart(float lon, float lat, float dist) {
        const float cl = std::cos(lat);
        return Vec3f(dist * cl * std::sin(lon), -dist * cl * std::cos(lon),
                     dist * std::sin(lat));
    }
    // At the centre the angles parametrize nothing: lonAt0/latAt0 are returned
    static inline Vec3f posePartToPose(const Vec3f &p, float lonAt0, float latAt0) {
        const float d = p.length();
        if (d == 0.f)
            return Vec3f(lonAt0, latAt0, 0.f);
        return Vec3f(std::atan2(p[0], -p[1]), std::asin(p[2]/d), d);
    }
    // Calculate intermediate zoom coefficient
    inline float calculateZoomCoef() const {
        float coef = zoomTimer / zoomDuration;
        if (coef > 0.5) {
            coef = 1 - coef;
            coef = 1 - 2*coef*coef;
        } else
            coef *= 2*coef;
        return coef;
    }
    // Calculate zoom coefficient velocity (derivated for one second)
    inline float calculateZoomCoefVelocity() const {
        float coef = zoomTimer / zoomDuration;
        coef = (coef > 0.5) ? 4*(1-coef) : 4*coef;
        return coef / zoomDuration;
    }
    // Deduce (heading, alt, az) giving totalRot under the current modes, mount and placement; drops the view plans
    void recoverParams(const Mat4f &totalRot);
    // Advance the view/heading smoothing plans (constant-min-acceleration law)
    void advanceView(float deltaTime);
    // viewRotation() with the view offset
    Mat4f renderViewRotation() const;
    // Eye <- equatorial frame of the reference, from the current parameters
    Mat4f viewMat() const;
    Mat4f viewOffsetEyeRotation() const;
    // Advance the view-offset transition (old view_offset_transition ramp).
    void advanceViewOffset(float deltaTime);
    inline float effectiveViewOffset() const {
        return static_cast<float>(viewOffset) * viewOffsetTransition;
    }
    // Current forward direction in the PARAM frame (post-fold), from alt/az
    Vec3f paramForward() const;
    // The frame draw, serialized on the render chain with the publish tasks
    class FrameDrawTask : public Task {
    public:
        virtual void start(Taskable *target) override;
        Renderer *renderer = nullptr;
        Camera *camera = nullptr;
        std::atomic<bool> done{true};
    } frameDrawTask;
    ModularBodyPtr reference;
    ModularBodyPtr target;
    ModularSystem *system; // Determined on construction and update
    // View plan: rotate viewFrom about viewAxis, accelerating until viewT1 then decelerating until viewT
    Vec3f viewFrom;        // path start direction (unit, param frame)
    Vec3f viewAxis;        // rotation axis (unit, param frame)
    float viewAngle = 0;   // total planned angle from viewFrom (signed = 0..)
    float viewT1 = 0;      // acceleration phase end
    float viewT = 0;       // plan duration; 0 = no plan
    float viewTimer = 0;
    float viewV0 = 0;      // velocity at plan start (rad/s along the path)
    float viewA = 0;       // phase-1 acceleration (signed)
    // Heading 1-D plan, same law
    float hdgTarget = 0, hdgT1 = 0, hdgT = 0, hdgTimer = 0, hdgV0 = 0, hdgA = 0, hdgFrom = 0;
    float alt = 0;
    float az = 0;
    float heading = 0;
    // Latitude the fold was derived with
    float foldLat = 0;
    CameraMount mount = CameraMount::ALTAZ;
    bool skyLocked = false;
    // viewRotation*placementRotation captured when the sky lock engaged
    Mat4f lockedSkyRot;
    double viewOffset = 0;
    float viewOffsetTransition = 0;
    bool viewOffsetArmed = false;
    Vec3f position;
    Vec3f deltaPosition;
    float moveDuration = 0;
    Mat4f lastDispatchedMat; // harness: the mat handed to dispatchUpdate
    Vec3f lastAbsFwd;
    float longitude;
    float latitude;
    float distance; // In AU
    float zoomDuration = 0;
    float zoomTimer;
    float srcHalfFov;
    float dstHalfFov;
    static float minHalfFov;
    static float maxHalfFov;
    bool freeMode = false;
    bool boundToSurface = true;
};

#endif /* end of include guard: CAMERA_HPP_ */
