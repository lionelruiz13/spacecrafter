class LunarOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        return std::make_unique<MixedOrbit>(
            std::make_unique<SpecialOrbit>("lunar_special"),
            Utility::strToDouble(params["orbit_period"]),
            SpaceDate::JulianDayFromDateTime(-10000, 1, 1, 1, 1, 1),
            SpaceDate::JulianDayFromDateTime(10000, 1, 1, 1, 1, 1),
            EARTH_MASS + LUNAR_MASS,
            0, 0, 0,
            false
        );
    }
};
