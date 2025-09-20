class EarthOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        return std::make_unique<BinaryOrbit>(std::make_unique<SpecialOrbit>("emb_special"), 0.0121505677733761);
    }
};
