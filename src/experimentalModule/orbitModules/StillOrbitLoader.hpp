class StillOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        return std::make_unique<stillOrbit>(
            Utility::strToDouble(params["orbit_x"]),
		    Utility::strToDouble(params["orbit_y"]),
            Utility::strToDouble(params["orbit_z"])
        );
    }
};
