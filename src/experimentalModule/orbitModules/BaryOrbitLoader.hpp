class BaryOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        if(params["a"].empty() || params["b"].empty()) {
    		cLog::get()->write("OrbitCreatorBary::handle missing barycenter coefficients");
    		return nullptr;
    	}
        if(params["body_A"].empty() || params["body_B"].empty()) {
    		cLog::get()->write("OrbitCreatorBary::handle missing parents");
    		return nullptr;
    	}
        ModularBody *bodyA = ModularBody::findBodyOnce(params["body_A"]);
        ModularBody *bodyB = ModularBody::findBodyOnce(params["body_B"]);
        if(bodyA == nullptr || bodyB == nullptr) {
    		cLog::get()->write("OrbitCreatorBary::couldn't find one of the bodies");
    		return nullptr;
    	}
        return std::make_unique<BarycenterOrbit2>(bodyA, bodyB, stod(params["a"]), stod(params["b"]));
    }
};
