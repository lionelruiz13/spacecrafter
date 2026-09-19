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

//! Which of the three R3 kinds an anchor is. The `type` data value maps here
//! once, in CameraAnchors.cpp (parseKind) - no consumer re-reads the string.
enum class AnchorKind : uint8_t {
    ON_ORBIT,    //!< `type = orbit`  - a point on an orbit around a body (PRIMARY)
    ATTACHED,    //!< `type = body`   - attached to a body (+/- follow_rotation)
    FIXED_POINT, //!< `type = point` | `type = observatory` - a fixed point in AU
};

class CameraAnchors {
public:
    //! @param root the tree root (the Universe node): the frame FIXED_POINT
    //! anchors are read in, and the parent their bodies are created under.
    explicit CameraAnchors(ModularSystem *root) : root(root) {}
    ~CameraAnchors();

    void load(const std::string &path);

    bool add(stringHash_t params);

    //! Drop a declared anchor and destroy the body it owns. REFUSES to drop the
    //! current anchor (the old path's own rule, AnchorManager::removeAnchor:194).
    bool remove(const std::string &name);

    bool switchTo(const std::string &name, Camera &camera);

    bool setFollowRotation(const std::string &name, bool value, Camera &camera);

    bool placeCurrentAt(const Vec3d &posRoot, Camera &camera, double jd);
    bool travelToPoint(const Vec3d &posRoot, double seconds, Camera &camera, double jd);
    bool travelToBody(const std::string &bodyName, double seconds, double altitudeKm,
                      Camera &camera, double jd);
    bool transitionToPoint(const std::string &name, Camera &camera, double jd);
    bool transitionToBody(const std::string &name, Camera &camera);

    void update(double jd);
    //! True while a travel is in flight - the state the refusals above and
    //! switchTo() read (old AnchorManager::moving).
    inline bool isMoving() const {
        return moving;
    }

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

    inline const std::string &getCurrent() const {
        return currentName;
    }

    void dumpState(std::ostream &out) const;

private:
    struct Anchor {
        std::string name;
        AnchorKind kind;
        stringHash_t params;      //!< the declaration - the re-creation authority
        ModularBodyPtr body;      //!< owned anchor body, or the ATTACHED target
        bool ownsBody = false;
        bool declaresFollowRotation = false;
        bool followRotation = true;
    };
    //! Resolve `type` to a kind; false (+ S2(f) log naming the valid values) if
    //! absent or unknown.
    static bool parseKind(const stringHash_t &params, const std::string &name, AnchorKind &out);
    bool buildBody(Anchor &anchor);
    bool ensureBody(Anchor &anchor);
    //! Create one hidden anchor body under `parent` with the given orbit params.
    ModularBody *createAnchorBody(const std::string &name, ModularBody *parent, stringHash_t &orbitParams);
    Anchor *find(const std::string &name);
    Anchor *currentPlace(const Camera &camera);
    bool installTravel(Anchor &place, const Vec3d &targetRoot, double travelDays, double jd);

    ModularSystem *root;
    std::vector<Anchor> anchors;
    std::string currentName;
    //! Travel state - the exact pair the old manager keeps (`moving` +
    //! `arrivalTime`); the trajectory itself lives in the anchor body's orbit.
    bool moving = false;
    double travelArrival = 0;
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
