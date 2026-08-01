#include "LayeredMesh.hpp"
#include "tools/s_texture.hpp"
#include "tools/file_path.hpp"
#include "tools/context.hpp"
#include "bodyModule/body_tesselation.hpp"
#include "EntityCore/Resource/Pipeline.hpp"
#include "EntityCore/Resource/PipelineLayout.hpp"
#include "ojmModule/objl.hpp"
#include "experimentalModule/Renderer.hpp"
#include "experimentalModule/ModularBody.hpp"
#include "experimentalModule/ModularSystem.hpp"
#include "experimentalModule/meshModules/meshShadowFill.hpp"
#include "experimentalModule/bodyModules/TraceFamily.hpp"
#include <cmath>

// Regime gate constants (header comment carries the derivation; convergence
// point - tunable one-liners).
constexpr float RAYMARCH_MAX_DISTANCE_RADII = 64.f;
// The one screenSize threshold left in FRACTION units after §5.54 respelled the
// G4 family in px. Deliberately not converted with them: it is a single-site
// named constant (no I2 duplication to close) and it is a raymarch CAPABILITY
// gate, not a G4 regime boundary, so converting it is a behaviour change at
// non-2048 widths that this task had no mandate for. The px-intent argument
// does apply to it - 0.025 is ~51 px at 2048, and this file's own comments
// reason in px - so it is recorded with the §5.54 veto point rather than
// silently left in a second unit.
constexpr float RAYMARCH_MIN_SCREEN_SIZE = 0.025f;

static std::unique_ptr<s_texture> makeTex(const std::string &path)
{
    if (path.empty())
        return nullptr;
    return std::make_unique<s_texture>(FilePath(path, FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true);
}

LayeredMesh::LayeredMesh(ObjL *mesh, Config &&cfg_) : BodyModule(BodyModuleType::MESH),
    mesh(mesh), cfg(std::move(cfg_)),
    day(FilePath(cfg.day, FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID, true, true),
    night(makeTex(cfg.night)), specular(makeTex(cfg.specular)),
    normal(makeTex(cfg.normal)), heightmap(makeTex(cfg.heightmap)),
    midFamily(cfg.tessellated ? MeshFamilies::meshTes() : MeshFamilies::meshLayered()),
    rayFamily(cfg.rayCapable ? MeshFamilies::meshRayMarch() : PipelineFamily()),
    midSet(Context::instance->renderer.allocSet(midFamily, 0)),
    raySet(cfg.rayCapable ? Context::instance->renderer.allocSet(rayFamily, 0) : nullptr),
    vert(*Context::instance->uniformMgr), frag(*Context::instance->uniformMgr)
{
    frag->nbShadowingBodies = 0;
    if (cfg.tessellated) {
        // Contract "bodyTes": 3 day, 4 night, 5 specular, 6 normal, 7 heightmap(TESE), 8 shadows
        midSlots[0] = &day; midSlots[1] = night.get(); midSlots[2] = specular.get();
        midSlots[3] = normal.get(); midSlots[4] = heightmap.get();
        midSlotCount = 5; midFirstBinding = 3; midShadowBinding = 8;
        tescGeom = std::make_unique<SharedBuffer<meshTescGeom>>(*Context::instance->uniformMgr);
    } else {
        // Contract "bodyLayered": 2 day, 3 night, 4 normal, 5 shadows
        midSlots[0] = &day; midSlots[1] = night.get(); midSlots[2] = normal.get();
        midSlots[3] = nullptr; midSlots[4] = nullptr;
        midSlotCount = 3; midFirstBinding = 2; midShadowBinding = 5;
    }
    // Contract "bodyRayMarch": 2 heightmap, 3 normal, 4 day, 5 night, 6 specular, 7 shadows
    raySlots[0] = heightmap.get(); raySlots[1] = normal.get(); raySlots[2] = &day;
    raySlots[3] = night.get(); raySlots[4] = specular.get();
    if (cfg.rayCapable) {
        rayVert = std::make_unique<SharedBuffer<rayMarchVert>>(*Context::instance->uniformMgr);
        rayFrag = std::make_unique<SharedBuffer<rayMarchFrag>>(*Context::instance->uniformMgr);
        (*rayFrag)->nbShadowingBodies = 0;
    }
}

LayeredMesh::~LayeredMesh()
{
}

float LayeredMesh::altimetryLevel() const
{
    auto &tes = ModularBody::getTesselation();
    if (!tes)
        return 0;
    return cfg.moonClass ? tes->getMoonAltimetryFactor() : tes->getPlanetAltimetryFactor();
}

void LayeredMesh::rebind(bool ray, Texture *const *big)
{
    // Slot resolution: absent slots AND the day slot itself route through
    // dayTex() - old binds tex_current at exactly those bindings (the skin
    // replaces day everywhere day would be sampled or placeheld).
    if (ray) {
        raySet->uninit();
        raySet->bindUniform(rayVert, 0);
        raySet->bindUniform(rayFrag, 1);
        for (int i = 0; i < 5; ++i) {
            s_texture *slot = raySlots[i];
            if (!slot || slot == &day)
                slot = &dayTex();
            raySet->bindTexture((big && big[i]) ? *big[i] : slot->getTexture(), 2 + i);
        }
        raySet->bindTexture(*Context::instance->renderer.shadow.layerArray(), 7);
    } else {
        midSet->uninit();
        midSet->bindUniform(vert, 0);
        midSet->bindUniform(frag, 1);
        if (tescGeom)
            midSet->bindUniform(*tescGeom, 2);
        for (int i = 0; i < midSlotCount; ++i) {
            s_texture *slot = midSlots[i];
            if (!slot || slot == &day)
                slot = &dayTex();
            midSet->bindTexture((big && big[i]) ? *big[i] : slot->getTexture(), midFirstBinding + i);
        }
        midSet->bindTexture(*Context::instance->renderer.shadow.layerArray(), midShadowBinding);
    }
}

void LayeredMesh::refreshSkinState()
{
    const bool ready = skinUse && skinTexture && !skinTexture->isLoading();
    if (ready != skinBound) {
        skinBound = ready;
        midBigMapping = rayBigMapping = 0xFFFF; // force both rebinds (sentinel)
    }
}

void LayeredMesh::createTexSkin(const std::string &texName)
{
    // Old parity (Body::createTexSkin): creating/replacing never activates;
    // load flags mirror old exactly (PNG_SOLID_REPEAT, mipmap, resolution).
    skinUse = false;
    skinTexture = std::make_unique<s_texture>(FilePath(texName, FilePath::TFP::TEXTURE).toString(), TEX_LOAD_TYPE_PNG_SOLID_REPEAT, true, true);
}

void LayeredMesh::switchTexSkin(bool use)
{
    if (use && !skinTexture)
        return; // old parity: activation requires an existing skin
    skinUse = use;
}

bool LayeredMesh::isLoaded()
{
    if (loaded)
        return true;
    if (day.isLoading()
        || (night && night->isLoading()) || (specular && specular->isLoading())
        || (normal && normal->isLoading()) || (heightmap && heightmap->isLoading()))
        return false;
    rebind(false, nullptr);
    if (cfg.rayCapable)
        rebind(true, nullptr);
    loaded = true;
    return true;
}

void LayeredMesh::preload(ModularBody *body, int keepFrames)
{
    int tmp = s_texture::setBigTextureLifetime(keepFrames);
    for (s_texture *t : {&day, night.get(), specular.get(), normal.get(), heightmap.get()}) {
        if (t) {
            t->prioritize(LoadPriority::PRELOAD);
            t->getBigTexture();
        }
    }
    s_texture::setBigTextureLifetime(tmp);
}

bool LayeredMesh::update(ModularBody *body, float scaledRadius)
{
    boundingRadius = (cfg.tessellated || cfg.rayCapable)
        ? scaledRadius * (1.f + 0.01f * altimetryLevel())
        : scaledRadius;
    return true;
}

void LayeredMesh::fillVert(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    vert->ModelViewMatrix = mat;
    vert->NormalMatrix = mat.inverseUntranslated().transpose();
    vert->clipping_fov = renderer.getClippingFov();
    vert->planetRadius = body->getRadius();
    vert->LightPosition = ModularBody::getLightPosition();
    // Old parity: planetScaledRadius = the body's scaled radius (the tese
    // adds the altimetry displacement on top; module boundingRadius is the
    // depth-slice value, deliberately NOT this one).
    vert->planetScaledRadius = body->getScaledRadius();
    vert->planetOneMinusOblateness = body->getOneMinusOblateness();
}

void LayeredMesh::drawMid(Renderer &renderer, ModularBody *body, const Mat4f &mat, uint16_t wanted, bool low)
{
    const FamilyBound bound = renderer.bind(midFamily, wanted);
    if (!bound.layout)
        return; // pass unavailable - logged at its definition site (C3)
    refreshSkinState();
    mesh->bind(renderer);
    fillVert(renderer, body, mat);
    if (tescGeom) {
        auto &tes = ModularBody::getTesselation();
        (*tescGeom)->TesParam = tes
            ? Vec3i(tes->getMinTesLevel(), tes->getMaxTesLevel(),
                    cfg.moonClass ? tes->getMoonAltimetryFactor() : tes->getPlanetAltimetryFactor())
            : Vec3i(1, 1, 1);
    }
    fillPlainShadows(frag, body, this);
    const auto screenSize = body->getScreenSize();
    if (screenSize > ModularBody::bigTextureGate() && !low) {
        Texture *big[5] = {};
        uint16_t map = 0;
        for (int i = 0; i < midSlotCount; ++i) {
            // Day slot resolves through the skin (which has no big texture ->
            // bit drops, and the mapping change itself triggers the rebind).
            s_texture *slot = (midSlots[i] == &day) ? &dayTex() : midSlots[i];
            if (slot) {
                big[i] = slot->getBigTexture();
                map |= (big[i] != nullptr) << i;
            }
        }
        if (map != midBigMapping) {
            rebind(false, big);
            midBigMapping = map;
        }
    } else if (midBigMapping) {
        rebind(false, nullptr);
        midBigMapping = 0;
    }
    bound.layout->bindSets(renderer, {*midSet, *Context::instance->uboSet});
    if (low)
        mesh->drawLow(renderer);
    else
        mesh->draw(renderer, screenSize * 1024);
}

void LayeredMesh::drawRay(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // DEPTH ON for BOTH ray rows (INTENT 5.30). The old myEarthShadowed
    // depth-OFF quirk this used to reproduce was never Earth-specific and its
    // requirement is dead at source: `13d846c8` turned depth off on BOTH ray
    // rows in the same commit that made body_tes_shadow.vert emit a CONSTANT
    // gl_Position.z = 0 (the shell became a pure rasterization carrier, so a
    // stage emitting no depth must not write depth); `39a8f235` restored depth
    // to the moon-class row when planet_grid needed the disc to occlude its
    // far-side lines, touching only that row; and `b323db09` gave the vertex
    // stage a real projected depth again (custom_projectNoMV), retiring the
    // premise for both rows - the Earth row simply never got the two updates.
    // The fragment now overrides that depth with the TRUE ray hit anyway
    // (bodyRayMarchNight.frag, same commit - without it this line would swap
    // Earth's "no depth" for the SHELL depth, i.e. defect 5.29 on Earth).
    const uint16_t wanted = cfg.rayVariant;
    const FamilyBound bound = renderer.bind(rayFamily, wanted);
    if (!bound.layout)
        return; // pass unavailable - logged at its definition site (C3)
    refreshSkinState();
    mesh->bind(renderer);
    const float altimetryFactor = 0.01f * altimetryLevel();
    const float scaledRadius = body->getScaledRadius();
    const float distance = body->getDistanceToObserver();
    // Old drawCenterOfInterest math, verbatim (body_bigbody.cpp:655-680):
    const float finalRadius = std::min(scaledRadius * (1.f + altimetryFactor), distance - scaledRadius / 64.f);
    // The near-component matrix carries computeBodyToSurface()'s +PI/2 (mesh
    // texcoord convention); the ray-march reconstructs texture longitude from
    // atan(y,x) - the old CoI convention without the +90: remove it.
    const Mat4f m = mat * Mat4f::zrotation(-M_PI_2);
    auto &rv = **rayVert;
    rv.ModelViewMatrix = m * Mat4f::scaling(Vec3f(1.f, 1.f, body->getOneMinusOblateness()));
    Mat4f m2 = m.transpose();
    m2.setMat3(rv.WorldToModelMatrix);
    rv.radius = finalRadius;
    const Vec3f &clipping = renderer.getClippingFov();
    rv.zNear = clipping.v[0];
    rv.zRange = clipping.v[1] - clipping.v[0];
    rv.fov = clipping.v[2];
    auto &rf = **rayFrag;
    {
        Vec3f tmp = m2 * (mat.getTranslation() - ModularBody::getLightPosition());
        tmp.normalize();
        rf.lightDirection = tmp; // body-local, sun -> body (old: m2*(eye_planet-eye_sun))
    }
    {
        // sinSunAngle = 2*sin(sun half-angle at the receiver): the terrain
        // self-shadow penumbra scale. Star of the body's system (valid since
        // the S5 star-assignment fix); guard the degenerate cases.
        ModularSystem *system = ModularSystem::systemOf(body);
        ModularBody *star = system ? system->getSystemStar() : nullptr;
        const float lightDist = (mat.getTranslation() - ModularBody::getLightPosition()).length();
        rf.sinSunAngle = (star && lightDist > 0)
            ? std::max(2.f * star->getScaledRadius() / lightDist, 1e-6f) : 1e-6f;
    }
    const float altimetryCoef = scaledRadius / finalRadius;
    rf.heightMapDepthLevel = altimetryCoef;
    rf.heightMapDepth = altimetryFactor * altimetryCoef;
    rf.squaredHeightMapDepthLevel = altimetryCoef * altimetryCoef;
    rf.sunDeviation = cfg.sunDeviation;
    rf.atmColor = cfg.atmColor;
    rf.atmDeviation = cfg.atmDeviation;
    fillFoldedShadows(*rayFrag, body, rv.ModelViewMatrix, finalRadius, this);
    // Big textures: the close-range regime is exactly where they engage
    // (old getSet >= 180px; the ray gate's 0.025 floor ~ 51px keeps the
    // 0.2 threshold check meaningful).
    if (body->getScreenSize() > ModularBody::bigTextureGate()) {
        Texture *big[5] = {};
        uint16_t map = 0;
        for (int i = 0; i < 5; ++i) {
            // Day slot resolves through the skin (no big texture -> bit drops).
            s_texture *slot = (raySlots[i] == &day) ? &dayTex() : raySlots[i];
            if (slot) {
                big[i] = slot->getBigTexture();
                map |= (big[i] != nullptr) << i;
            }
        }
        if (map != rayBigMapping) {
            rebind(true, big);
            rayBigMapping = map;
        }
    } else if (rayBigMapping) {
        rebind(true, nullptr);
        rayBigMapping = 0;
    }
    bound.layout->bindSets(renderer, {*raySet, *Context::instance->uboSet});
    mesh->draw(renderer, body->getScreenSize() * 1024);
}

void LayeredMesh::draw(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    if (cfg.rayCapable
        && body->getDistanceToObserver() < body->getScaledRadius() * RAYMARCH_MAX_DISTANCE_RADII
        && body->getScreenSize() > RAYMARCH_MIN_SCREEN_SIZE) {
        drawRay(renderer, body, mat);
    } else {
        drawMid(renderer, body, mat, cfg.midVariant, false);
    }
}

void LayeredMesh::drawNoDepth(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // The ray gate (RAYMARCH_MIN_SCREEN_SIZE) never reaches the noDepth band
    // (at or below BODY_FULL_VISIBILITY_BOUNDING_SIZE): always the mid family here.
    drawMid(renderer, body, mat, cfg.midVariant | VARIANT_NO_DEPTH, true);
}

void LayeredMesh::drawShadow(Renderer &renderer, ModularBody *body, const Mat4f &mat, int idx)
{
    renderer.shadow.produce(idx, mat, mesh);
}

void LayeredMesh::drawTrace(Renderer &renderer, ModularBody *body, const Mat4f &mat)
{
    // Row-8 TRACE prepass: identical shape to BasicMesh::drawTrace (the shared
    // sphere-trace family). The heightmap headroom in boundingRadius is <1% -
    // the base scaled radius is the parity value with the old sphere trace.
    const FamilyBound bound = renderer.bind(TraceFamily::sphere());
    if (!bound.layout)
        return;
    TraceInfo info;
    info.ModelViewMatrix = mat;
    info.clipping_fov = renderer.getClippingFov();
    info.planetScaledRadius = body->getScaledRadius();
    info.planetOneMinusOblateness = body->getOneMinusOblateness();
    bound.layout->pushConstant(renderer, 0, &info);
    mesh->bind(renderer);
    mesh->draw(renderer, body->getScreenSize() * 1024);
}
