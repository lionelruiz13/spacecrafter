class LocationOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        auto parent = ModularBody::findBody(params["parent"]);

		return std::make_unique<LocationOrbit>(
			Utility::strToDouble(params["orbit_lon"]),
			Utility::strToDouble(params["orbit_lat"]),
			Utility::strToDouble(params["orbit_alt"]),
			parent->getRadius()
		);
    }
};
