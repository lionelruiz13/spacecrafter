#ifndef LIGHT_ABERRATION_HPP
#define LIGHT_ABERRATION_HPP

#include "navModule/navigator.hpp"
#include "tools/sc_const.hpp"
#include "tools/vecmath.hpp"

#include <cmath>

namespace LightAberration {

inline Vec3d makeBeta(const Vec3d& observerVelocity, const Mat4d& velocityToTargetFrame)
{
	const Vec3d beta = velocityToTargetFrame.multiplyWithoutTranslation(observerVelocity)
	             * (AU / SPEED_OF_LIGHT);
	const double betaSquared = beta.lengthSquared();
	if (!std::isfinite(betaSquared) || betaSquared >= 1.0)
		return Vec3d(0., 0., 0.);
	return beta;
}

inline Vec3d makeEyeBeta(const Vec3d& observerVelocity, const Navigator *nav)
{
	return makeBeta(observerVelocity, nav->getHelioToEyeMat());
}

inline Vec3d makeLocalBeta(const Vec3d& observerVelocity, const Navigator *nav)
{
	Vec3d beta = mat_vsop87_to_j2000.multiplyWithoutTranslation(observerVelocity)
	             * (AU / SPEED_OF_LIGHT);
	beta = nav->earthEquToLocal(nav->j2000ToEarthEqu(beta));

	const double betaSquared = beta.lengthSquared();
	if (!std::isfinite(betaSquared) || betaSquared >= 1.0)
		return Vec3d(0., 0., 0.);
	return beta;
}

inline Vec3d applyToDirection(const Vec3d& inputDirection, const Vec3d& beta)
{
	const double betaSquared = beta.lengthSquared();
	if (betaSquared < 1e-24)
		return inputDirection;

	Vec3d direction = inputDirection;
	direction.normalize();
	const double gamma = 1.0 / std::sqrt(1.0 - betaSquared);
	const double directionAlongBeta = direction.dot(beta);
	const double denominator = 1.0 + directionAlongBeta;
	Vec3d refracted = (direction / gamma + beta
	                  + beta * (gamma / (gamma + 1.0) * directionAlongBeta))
	                 / denominator;
	refracted.normalize();
	return refracted;
}

inline Vec3d applyToPosition(const Vec3d& position, const Vec3d& beta)
{
	const double distance = position.length();
	if (distance < 1e-24 || beta.lengthSquared() < 1e-24)
		return position;
	return applyToDirection(position, beta) * distance;
}

}

#endif
