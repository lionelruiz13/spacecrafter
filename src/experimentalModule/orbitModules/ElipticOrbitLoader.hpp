class ElipticOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        auto parent = ModularBody::findBody(params["parent"]);

    	double parent_rot_obliquity = 0.0;
    	double parent_rot_asc_node = 0.0;
    	double parent_rot_J2000_longitude = 0.0;

    	if(!parent) {
    		parent_rot_obliquity = Utility::strToDouble(params["parent_rot_obliquity"], 0.0);
    		parent_rot_asc_node = Utility::strToDouble(params["parent_rot_asc_node"], 0.0);
    		parent_rot_J2000_longitude = Utility::strToDouble(params["parent_rot_J2000_longitude"], 0.0);
    	} else {
    		parent_rot_obliquity = parent && parent->getParent()
    		                       ? parent->getRotObliquity() : 0.0;
    		parent_rot_asc_node = parent && parent->getParent()
    		                      ? parent->getRotAscendingnode() : 0.0;

    		if (parent && parent->getParent()) {
    			const double c_obl = cos(parent_rot_obliquity);
    			const double s_obl = sin(parent_rot_obliquity);
    			const double c_nod = cos(parent_rot_asc_node);
    			const double s_nod = sin(parent_rot_asc_node);
    			const Vec3d OrbitAxis0( c_nod,       s_nod,        0.0);
    			const Vec3d OrbitAxis1(-s_nod*c_obl, c_nod*c_obl,s_obl);
    			const Vec3d OrbitPole(  s_nod*s_obl,-c_nod*s_obl,c_obl);
    			const Vec3d J2000Pole(mat_j2000_to_vsop87.multiplyWithoutTranslation(Vec3d(0,0,1)));
    			Vec3d J2000NodeOrigin(J2000Pole^OrbitPole);
    			J2000NodeOrigin.normalize();
    			parent_rot_J2000_longitude = atan2(J2000NodeOrigin*OrbitAxis1,J2000NodeOrigin*OrbitAxis0);
    		}
    	}

    	double period = Utility::strToDouble(params["orbit_period"]);
    	double epoch = Utility::strToDouble(params["orbit_epoch"],J2000);
    	double semi_major_axis = Utility::strToDouble(params["orbit_semimajoraxis"])/AU;
    	double eccentricity = Utility::strToDouble(params["orbit_eccentricity"]);
    	double inclination = Utility::strToDouble(params["orbit_inclination"])*M_PI/180.;
    	double ascending_node = Utility::strToDouble(params["orbit_ascendingnode"])*M_PI/180.;
    	double long_of_pericenter = Utility::strToDouble(params["orbit_longofpericenter"])*M_PI/180.;
    	double mean_longitude = Utility::strToDouble(params["orbit_meanlongitude"])*M_PI/180.;

    	double arg_of_pericenter = long_of_pericenter - ascending_node;
    	double anomaly_at_epoch = mean_longitude - (arg_of_pericenter + ascending_node);
    	double pericenter_distance = semi_major_axis * (1.0 - eccentricity);

    	// Create an elliptical orbit
    	return std::make_unique<EllipticalOrbit>(
            pericenter_distance, eccentricity,
            inclination, ascending_node,
            arg_of_pericenter, anomaly_at_epoch,
            period, epoch,
            parent_rot_obliquity, parent_rot_asc_node,
            parent_rot_J2000_longitude
        );
    }
};
