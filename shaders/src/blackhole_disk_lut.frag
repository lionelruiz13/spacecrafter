#version 420

layout(binding=0) uniform sampler2D Trajectories;
layout(binding=1) uniform sampler2D Density;
layout(binding=2) uniform DiskModel {
    vec4 Camera;       // BH-to-observer; capture radius (negative: distortion disabled)
    vec4 DiskX;        // disk basis in eye coordinates; inner radius
    vec4 DiskY;        // disk basis in eye coordinates; outer radius
    vec4 Projection;   // framebuffer center, viewport radius, half FOV
    vec4 DiskColor;    // rgb, intensity
    vec4 PhotonColor;  // rgb, turbulence
};
layout(constant_id=8) const int projectionType = 0;
layout(location=0) out vec4 FragColor;

#include <blackhole_disk_material.glsl>

const float PI = 3.141592653589793;
const float CRITICAL = 2.598076211353316;
const float MIN_B = 0.0001;
const float CORE_B = 128.0;
const float B_SCALE = 0.002;

vec2 lutUv(float b, float phase)
{
    float d = b - CRITICAL;
    float span = d < 0.0 ? CRITICAL - MIN_B : CORE_B - CRITICAL;
    float x = 0.5 + 0.5 * sign(d) * log(1.0 + abs(d) / B_SCALE) / log(1.0 + span / B_SCALE);
    if (d < 0.0) {
        float lo = log(MIN_B / (CRITICAL - MIN_B));
        float hi = log((CRITICAL - 1e-6) / 1e-6);
        x = 0.5 * (log(max(b, MIN_B) / (-d)) - lo) / (hi - lo);
    }
    vec2 size = vec2(textureSize(Trajectories, 0));
    float column = b > CORE_B ? 1024.0 + 256.0 * (1.0 - CORE_B / b) : clamp(x, 0.0, 1.0) * 1023.0;
    return (vec2(column, clamp(phase, 0.0, 1.0) * (size.y - 1.0)) + 0.5) / size;
}

float inverseRadius(float b, float phi, bool innerBranch)
{
    if (phi <= 0.0)
        return innerBranch ? 1.0 : 0.0;
    if (phi >= 3.0 * PI)
        return innerBranch || b < CRITICAL ? 1.0 : 0.0;
    vec4 entry = textureLod(Trajectories, lutUv(b, sqrt(phi / (3.0 * PI))), 0.0);
    return innerBranch ? entry.b : (b > CORE_B ? entry.r / b : entry.r);
}

float inversePhase(float b, float u, bool innerBranch)
{
    float coordinate = innerBranch ? 3.0 * (1.0 - u) : u;
    if (b > CORE_B && !innerBranch) {
        float apsis = 1.0;
        for (int i = 0; i < 8; ++i)
            apsis = inversesqrt(1.0 - apsis / b);
        coordinate = 1.0 - sqrt(clamp(1.0 - (b * u) / apsis, 0.0, 1.0));
    } else if (b >= CRITICAL) {
        float angle = acos(clamp(1.0 - 2.0 * CRITICAL * CRITICAL / (b * b), -1.0, 1.0));
        float apsis = (1.0 + 2.0 * cos((angle + (innerBranch ? 0.0 : 4.0 * PI)) / 3.0)) / 3.0;
        float remaining = innerBranch ? (u - apsis) / (1.0 - apsis) : 1.0 - u / apsis;
        coordinate = 1.0 - sqrt(clamp(remaining, 0.0, 1.0));
    }
    vec4 entry = textureLod(Trajectories, lutUv(b, coordinate), 0.0);
    return innerBranch ? entry.a : entry.g;
}

float allsphereRadius(float normalizedAngle)
{
    // Same polynomial as custom_project.glsl, shifted by half a FOV to
    // avoid cancellation of large terms in single-precision arithmetic.
    float x = normalizedAngle - 0.5;
    float p = -80.180823948145459;
    p = fma(p, x, 214.05881150609818);
    p = fma(p, x, 88.617637007474571);
    p = fma(p, x, -105.67852344410126);
    p = fma(p, x, -26.941893001076522);
    p = fma(p, x, 17.00345934829549);
    p = fma(p, x, 3.1866807254451852);
    p = fma(p, x, -1.4224842650278533);
    p = fma(p, x, -0.83536392826748052);
    p = fma(p, x, 1.1304434420990701);
    return fma(p, x, 0.6740732889877743);
}

float allsphereAngle(float radius)
{
    // The monotonic interval covers the viewport, including its corners.
    // Bisection stays on this branch even at the optical axis.
    float lo = 0.0;
    float hi = 1.2;
    for (int i = 0; i < 24; ++i) {
        float mid = (lo + hi) * 0.5;
        if (allsphereRadius(mid) < radius)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) * 0.5;
}

vec3 eyeRay()
{
    vec2 xy = (gl_FragCoord.xy - Projection.xy) / Projection.z;
    xy.y = -xy.y;
    float r = length(xy);
    float theta = r * Projection.w;
    if (projectionType == 1)
        theta = allsphereAngle(r) * Projection.w;
    if (projectionType == 3)
        theta = 2.0 * atan(r * tan(Projection.w * 0.5));
    return vec3(xy * (sin(theta) / max(r, 1e-8)), -cos(theta));
}

vec4 diskEmission(vec3 point)
{
    float radius = length(point);
    if (radius < DiskX.w || radius > DiskY.w)
        return vec4(0.0);
    float angle = atan(dot(point, DiskY.xyz), dot(point, DiskX.xyz));
    float r = (radius - DiskX.w) / (DiskY.w - DiskX.w);
    return accretionMaterial(Density, r, angle,
                            DiskColor.rgb, PhotonColor.rgb, DiskColor.a, PhotonColor.a);
}

vec4 undistortedDisk(vec3 ray)
{
    if (length(Camera.xyz) <= -Camera.w)
        return vec4(0.0);
    vec3 normal = normalize(cross(DiskX.xyz, DiskY.xyz));
    float denominator = dot(normal, ray);
    if (abs(denominator) < 1e-7)
        return vec4(0.0);
    float t = -dot(normal, Camera.xyz) / denominator;
    if (t <= 0.0)
        return vec4(0.0);
    // Reject intersections hidden by the horizon, not the foreground disk.
    float along = dot(Camera.xyz, ray);
    float discriminant = along * along - dot(Camera.xyz, Camera.xyz) + Camera.w * Camera.w;
    if (discriminant >= 0.0) {
        float horizon = -along - sqrt(discriminant);
        if (horizon >= 0.0 && horizon < t)
            return vec4(0.0);
    }
    return diskEmission(Camera.xyz + t * ray);
}

void main()
{
    // The common scene pass owns the shadow with or without an accretion disk.
    // This pass contributes only premultiplied disk emission/opacity.
    FragColor = vec4(0.0);
    vec3 ray = eyeRay();
    if (Camera.w < 0.0) {
        FragColor = undistortedDisk(ray);
        return;
    }
    float distance = length(Camera.xyz);
    if (distance <= Camera.w * 1.00001) {
        return; // A static exterior observer is undefined at/inside the horizon.
    }
    vec3 radial = Camera.xyz / distance;
    float inward = -dot(ray, radial);

    vec3 tangent = ray + radial * inward;
    float sinAngle = length(tangent);
    float b = distance * sinAngle / sqrt(1.0 - 1.0 / distance);
    // No disk intersection outside its maximum impact parameter. This also
    // confines the expensive work to the angular neighborhood of the body.
    float limit = DiskY.w / sqrt(1.0 - 1.0 / DiskY.w);
    float captureImpact = Camera.w > 1.5
        ? Camera.w / sqrt(1.0 - 1.0 / Camera.w) : CRITICAL;
    bool innerBranch = distance < 1.5 && b >= CRITICAL;
    bool captured = innerBranch || (inward >= 0.0 && b < captureImpact);
    if (!captured && b > max(limit, CRITICAL))
        return;
    if (sinAngle < 1e-7) {
        return;
    }
    tangent /= sinAngle;
    vec3 normal = normalize(cross(DiskX.xyz, DiskY.xyz));
    float n0 = dot(normal, radial);
    float n1 = dot(normal, tangent);
    if (abs(n0) + abs(n1) < 1e-7) {
        return; // A zero-thickness disk has no defined coplanar intersection.
    }

    float phase = mod(atan(-n0, n1), PI);
    float observerPhase;
    float phaseDirection = 1.0;
    if (innerBranch) {
        observerPhase = inversePhase(b, 1.0 / distance, true);
        // The inner table starts outward at the horizon, unlike the outer one.
        if (inward >= 0.0)
            phaseDirection = -1.0;
    } else {
        observerPhase = inversePhase(b, 1.0 / distance, false);
        if (inward < 0.0) {
            if (b < CRITICAL)
                phaseDirection = -1.0;
            else {
                float apsis = textureLod(Trajectories, lutUv(b, 1.0), 0.0).g;
                observerPhase = 2.0 * apsis - observerPhase;
            }
        }
    }
    float stopPhase = captured && Camera.w > 1.5
        ? inversePhase(b, 1.0 / Camera.w, false) - observerPhase : 3.0 * PI;
    vec4 emission = vec4(0.0);
    for (int image = 0; image < 2; ++image) {
        float phi = phase + float(image) * PI;
        if (phi >= stopPhase)
            break;
        float u = inverseRadius(b, observerPhase + phaseDirection * phi, innerBranch);
        if (u <= 0.0 || u >= 1.0 / Camera.w)
            continue;
        float radius = 1.0 / u;
        if (radius < DiskX.w || radius > DiskY.w)
            continue;
        vec3 point = cos(phi) * radial + sin(phi) * tangent;
        float angle = atan(dot(point, DiskY.xyz), dot(point, DiskX.xyz));
        float r = (radius - DiskX.w) / (DiskY.w - DiskX.w);
        vec4 light = accretionMaterial(Density, r, angle,
                                      DiskColor.rgb, PhotonColor.rgb, DiskColor.a, PhotonColor.a);
        emission += (1.0 - emission.a) * light;
    }
    // Capture terminates disk trajectories; it must not add a second, larger
    // opaque mask over the already distorted scene.
    FragColor = emission;
}
