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
    ON_ORBIT,    // type = orbit
    ATTACHED,    // type = body
    FIXED_POINT, // type = point or observatory, in AU
};

// Named places for the Camera; ON_ORBIT and FIXED_POINT own a hidden body
class CameraAnchors {
public:
    explicit CameraAnchors(ModularSystem *root) : root(root) {}
    ~CameraAnchors();

    void load(const std::string &path);

    bool add(stringHash_t params);

    // Destroy the owned body too, refused on the current anchor
    bool remove(const std::string &name);

    // A body name is an implicit ATTACHED anchor, refused while moving
    bool switchTo(const std::string &name, Camera &camera);

    // true = bound to the surface, false = keep the angle to the spinning body
    bool setFollowRotation(const std::string &name, bool value, Camera &camera);

    // posRoot in root frame, AU; refused on a body
    bool placeCurrentAt(const Vec3d &posRoot, Camera &camera, double jd);
    // seconds of simulation time, 0 = now; refused while moving or on a body
    bool travelToPoint(const Vec3d &posRoot, double seconds, Camera &camera, double jd);
    // Aim where the body will be at arrival; altitudeKm < 0 = stop at 5 radii
    bool travelToBody(const std::string &bodyName, double seconds, double altitudeKm,
                      Camera &camera, double jd);
    // Change the reference without moving nor turning; the point is created if needed
    bool transitionToPoint(const std::string &name, Camera &camera, double jd);
    bool transitionToBody(const std::string &name, Camera &camera);

    void update(double jd);
    inline bool isMoving() const {
        return moving;
    }

    // Record of the last installed travel, in root frame, AU and jd
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

    // Return the last anchor switched to, the camera may have left it since
    inline const std::string &getCurrent() const {
        return currentName;
    }

    void dumpState(std::ostream &out) const;

private:
    struct Anchor {
        std::string name;
        AnchorKind kind;
        stringHash_t params;      // Declaration, used to re-create the body
        ModularBodyPtr body;      // Owned anchor body, or the ATTACHED target
        bool ownsBody = false;
        bool declaresFollowRotation = false; // If false, switchTo() leaves the camera binding alone
        bool followRotation = true;
    };
    static bool parseKind(const stringHash_t &params, const std::string &name, AnchorKind &out);
    bool buildBody(Anchor &anchor);
    // Rebuild a body destroyed by a system reload
    bool ensureBody(Anchor &anchor);
    ModularBody *createAnchorBody(const std::string &name, ModularBody *parent, stringHash_t &orbitParams);
    Anchor *find(const std::string &name);
    Anchor *currentPlace(const Camera &camera);
    bool installTravel(Anchor &place, const Vec3d &targetRoot, double travelDays, double jd);

    ModularSystem *root;
    std::vector<Anchor> anchors;
    std::string currentName;
    bool moving = false;
    double travelArrival = 0;
    Vec3d travelStart{};
    Vec3d travelDirection{};
    double travelDistance = 0;
    double travelStartTime = 0;
    double travelEndTime = 0;
    unsigned int travelSeq = 0; // Counts every install, even a zero-duration one
public:
    inline unsigned int getTravelSeq() const {
        return travelSeq;
    }
};

#endif /* end of include guard: CAMERA_ANCHORS_HPP_ */
