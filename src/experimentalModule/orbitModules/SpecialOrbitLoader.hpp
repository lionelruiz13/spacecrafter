class SpecialOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        std::unique_ptr<SpecialOrbit> sorb = std::make_unique<SpecialOrbit>(params["coord_func"]);
    	if(!sorb->isValid()) {
			cLog::get()->write("OrbitCreatorComet::handle unknown type");
            sorb.reset();
    	}
    	return sorb;
    }
};
