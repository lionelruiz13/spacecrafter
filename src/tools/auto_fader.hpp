#ifndef AUTO_FADER_HPP_
#define AUTO_FADER_HPP_

#include "EntityCore/Executor/AFader.hpp"
#include "coreModule/coreLink.hpp"
#include "coreModule/core.hpp"

class LinearInterp {
public:
    constexpr float operator()(float v) const {
        return v;
    }
    constexpr float zero() const {
        return 0.f;
    }
    constexpr float one() const {
        return 1.f;
    }
};

class ParabolicInterp {
public:
    constexpr float operator()(float v) const {
        return v*v;
    }
    constexpr float zero() const {
        return 0.f;
    }
    constexpr float one() const {
        return 1.f;
    }
};

typedef AFader<CoreLink, LinearInterp> ALinearFader;
typedef AFader<CoreLink, ParabolicInterp> AParabolicFader;

#endif /* end of include guard: AUTO_FADER_HPP_ */
