#ifndef CAMERA_ANCHORS_HPP_
#define CAMERA_ANCHORS_HPP_

#include "ModularBodyPtr.hpp"
#include "tools/utility.hpp"
#include <string>
#include <map>
#include <vector>
#include <iosfwd>

class Camera;
class ModularBody;
class ModularSystem;

enum class AnchorKind : uint8_t {
    ON_ORBIT,    //!< `type = orbit`  - a point on an orbit around a body (PRIMARY)
    ATTACHED,    //!< `type = body`   - attached to a body (+/- follow_rotation)
    FIXED_POINT, //!< `type = point` | `type = observatory` - a fixed point in AU
};

//! Named places the Camera can stand on, declared by anchor.ini (load) or by `camera action` commands (add).
//! ON_ORBIT and FIXED_POINT own a hidden body carrying their motion: the tree stays the only position authority.
class CameraAnchors {
public:
    //! @param root the tree root (the Universe node): the frame FIXED_POINT
    //! anchors are read in, and the parent their bodies are created under.
    explicit CameraAnchors(ModularSystem *root) : root(root) {}
    ~CameraAnchors();

    //! Load an anchor.ini: each [section] block goes to add(), the section name is not read
    void load(const std::string &path);

    //! Declare an anchor with the anchor.ini keys (name, type, ...); false + log if the name is taken or has no body
    bool add(stringHash_t params);

    //! Drop a declared anchor and destroy the body it owns; refuses the current anchor
    bool remove(const std::string &name);

    //! Reference the named anchor; a body name which is not an anchor is an implicit ATTACHED one. Refused while moving
    bool switchTo(const std::string &name, Camera &camera);

    //! true = bound to the surface of the ATTACHED body, false = keep the angle to the body while it spins
    //! Applied now if name is the current anchor, at switchTo() otherwise
    bool setFollowRotation(const std::string &name, bool value, Camera &camera);

    //! Put the current place at posRoot (root frame, AU) now; refused on a body, allowed while moving
    bool placeCurrentAt(const Vec3d &posRoot, Camera &camera, double jd);
    //! Travel the current place to posRoot over seconds of simulation time (0 = now); refused while moving or on a body
    bool travelToPoint(const Vec3d &posRoot, double seconds, Camera &camera, double jd);
    //! Same, toward where the body will be at arrival, stopping altitudeKm above its surface (< 0 = at 5 radii)
    bool travelToBody(const std::string &bodyName, double seconds, double altitudeKm,
                      Camera &camera, double jd);
    //! Turn the observer's position into the FIXED_POINT anchor name (created if needed) and stand on it; nothing moves
    bool transitionToPoint(const std::string &name, Camera &camera, double jd);
    //! Reference the body or ATTACHED anchor name; the observer stays where it is and holds its whole orientation
    bool transitionToBody(const std::string &name, Camera &camera);

    //! Per frame: retire a travel that has landed
    void update(double jd);
    //! True while a travel is in flight: travels and switchTo() are refused meanwhile
    inline bool isMoving() const {
        return moving;
    }

    //! Inputs of the last installed travel (root frame, AU, jd), for the seam recorder
    inline const Vec3d &getTravelStart() const {
        return travelStart;
    }
    inline const Vec3d &getTravelDirection() const {
        return travelDirection;
    }
    inline double getTravelDistance() const {
        return travelDistance;
    }
    inline double getTravelStartTime() const {
        return travelStartTime;
    }
    inline double getTravelEndTime() const {
        return travelEndTime;
    }

    //! Anchor last switched to through this class ("" if none); the camera may have left it by other means
    inline const std::string &getCurrent() const {
        return currentName;
    }

    //! Current anchor, its kind and follow-rotation state, and the declared set, as one JSON object (trace harness)
    void dumpState(std::ostream &out) const;

private:
    struct Anchor {
        std::string name;
        AnchorKind kind;
        stringHash_t params;      //!< the declaration - the re-creation authority
        ModularBodyPtr body;      //!< owned anchor body, or the ATTACHED target
        bool ownsBody = false;
        bool declaresFollowRotation = false; //!< false = switchTo() leaves the camera's bind state alone
        bool followRotation = true;
    };
    //! Resolve `type` to a kind; false + log naming the valid values if absent or unknown
    static bool parseKind(const stringHash_t &params, const std::string &name, AnchorKind &out);
    //! Owned hidden body for ON_ORBIT/FIXED_POINT, the loaded `body_name` for ATTACHED; false + log if impossible
    bool buildBody(Anchor &anchor);
    //! Rebuild a body destroyed by a system reload; validity is asked of the name registry, never of the stale pointer
    bool ensureBody(Anchor &anchor);
    //! Create one hidden anchor body under `parent` with the given orbit params.
    ModularBody *createAnchorBody(const std::string &name, ModularBody *parent, stringHash_t &orbitParams);
    Anchor *find(const std::string &name);
    //! The anchor owning the body the camera stands on, or null: only a place can be travelled, not a body
    Anchor *currentPlace(const Camera &camera);
    //! Give place the travel as its orbit and remember when it lands; targetRoot in root frame, travelDays 0 = now
    bool installTravel(Anchor &place, const Vec3d &targetRoot, double travelDays, double jd);

    ModularSystem *root;
    std::vector<Anchor> anchors;
    std::string currentName;
    //! Travel state; the trajectory itself lives in the anchor body's orbit
    bool moving = false;
    double travelArrival = 0;
    //! Record of the last install's inputs, never read back by the travel (a zero-duration install writes them too)
    Vec3d travelStart{};
    Vec3d travelDirection{};
    double travelDistance = 0;
    double travelStartTime = 0;
    double travelEndTime = 0;
    //! Install counter - the seam recorder's edge, so an install that never
    //! sets `moving` (a zero-duration place) is still seen exactly once.
    unsigned int travelSeq = 0;
public:
    inline unsigned int getTravelSeq() const {
        return travelSeq;
    }
};

#endif /* end of include guard: CAMERA_ANCHORS_HPP_ */
