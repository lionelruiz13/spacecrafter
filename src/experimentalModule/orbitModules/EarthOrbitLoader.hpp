class EarthOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        // emb_special returns the Earth-Moon BARYCENTER; the primary's position
        // needs the secondary's orbit subtracted (ratio = lunar mass fraction,
        // same constant as old path, protosystem.cpp). The secondary is wired by
        // name when that body loads (ModularSystem::loadBody); "binary_secondary"
        // in the data overrides the default pairing.
        auto it = params.find("binary_secondary");
        return std::make_unique<BinaryOrbit>(std::make_unique<SpecialOrbit>("emb_special"), 0.0121505677733761,
                                             (it != params.end()) ? it->second : "Moon");
    }
};
