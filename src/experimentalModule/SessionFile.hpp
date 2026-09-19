#ifndef SESSION_FILE_HPP_
#define SESSION_FILE_HPP_

#include <functional>
#include <string>

#include "tools/vecmath.hpp"

// A session is a machine-owned preset the operator saves and loads explicitly: nothing at quit, nothing at start.
// It references the system files by path and never copies their content; anything in flight is saved settled.
namespace SessionFile {

// What a session needs from the application besides the camera and the body tree.
// Implementer: every setter drives the old Observer/Navigator/Projector too, the sky is still drawn from them.
class Host {
public:
    virtual ~Host() = default;
    virtual double getJDay() const = 0;
    virtual void setJDay(double jd) = 0;
    virtual double getTimeSpeed() const = 0;
    virtual void setTimeSpeed(double speed) = 0;
    virtual bool getTimePaused() const = 0;
    virtual void setTimePaused(bool paused) = 0;
    virtual std::string getSelectedName() const = 0;
    virtual bool selectByName(const std::string &name) = 0;
    virtual void deselect() = 0;
    virtual bool getTracking() const = 0;
    virtual void setTracking(bool on) = 0;
    virtual bool warpToBody(const std::string &name) = 0;
    virtual void moveObserverTo(double latDeg, double lonDeg, double altMetres) = 0;
    virtual void setFov(double degrees) = 0;
    virtual void setSkyLock(bool locked) = 0;
    // View direction of the old Navigator (local_vision), which no setter above ties to the camera.
    // set = recompute the old view from the place and date just restored, then aim it there.
    virtual void getSkyVision(double &x, double &y, double &z) const = 0;
    virtual void setSkyVision(double x, double y, double z) = 0;
    // Re-aims the old navigator: call before setSkyVision
    virtual void setViewOffset(double offset, bool armed) = 0;
};

// The flags, `set` values and colours, through the command interface which owns their names.
// Nothing is enumerated here: the file carries whatever the surface reports.
class CommandSurface {
public:
    virtual ~CommandSurface() = default;
    // Emit every name which can be read, with its current value; a name without a read half is not emitted
    virtual void forEachFlag(const std::function<void(const std::string &, bool)> &emit) const = 0;
    virtual void forEachValue(const std::function<void(const std::string &, const std::string &)> &emit) const = 0;
    virtual void forEachColor(const std::function<void(const std::string &, const Vec3f &)> &emit) const = 0;
    // The write halves, by name. False = this build does not know that name,
    // which is what a session written by another build looks like from here.
    virtual bool applyFlagByName(const std::string &name, bool value) = 0;
    virtual bool applyValueByName(const std::string &name, const std::string &value) = 0;
    virtual bool applyColorByName(const std::string &name, const Vec3f &value) = 0;
    // How many names the surface holds in each family, readable or not - so
    // the file can state what it did NOT carry instead of silently omitting it.
    virtual void countNames(int &flags, int &values, int &colors) const = 0;
};

// Write DIRECTORY/<name>.ini atomically; name is a plain file name, a path is refused. cmds may be null.
// Every body named is brought to the current date first. false + log on refusal or failure, an existing file is kept.
bool save(Host &host, CommandSurface *cmds, const std::string &name);

// Idempotent: every value is assigned. A value which cannot be re-established is reported and the rest still applies.
bool load(Host &host, CommandSurface *cmds, const std::string &name);

// The default session name, used when the operator names none.
constexpr const char *DEFAULT_NAME = "session";
// Relative to the working directory (~/.spacecrafter); created by save, never shipped
constexpr const char *DIRECTORY = "sessions";
// load refuses any other version instead of half-applying
constexpr int FORMAT_VERSION = 1;

} // namespace SessionFile

#endif /* end of include guard: SESSION_FILE_HPP_ */
