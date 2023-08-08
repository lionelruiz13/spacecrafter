class CometOrbitLoader : public OrbitLoader {
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) override {
        auto parent = ModularBody::findBody(params["parent"]);

        double parent_rot_obliquity = 0.0;
        double parent_rot_asc_node = 0.0;
        double parent_rot_J2000_longitude = 0.0;

        if(!parent) {
            parent_rot_obliquity = Utility::strToDouble(params["parent_rot_obliquity"]);
            parent_rot_asc_node = Utility::strToDouble(params["parent_rot_asc_node"]);
            parent_rot_J2000_longitude = Utility::strToDouble(params["parent_rot_J2000_longitude"]);
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

        // Read the orbital elements
    	const double eccentricity = Utility::strToDouble(params["orbit_eccentricity"],0.0);

    	double pericenter_distance = Utility::strToDouble(params["orbit_pericenterdistance"],-1e100);

    	double semi_major_axis;

    	if (pericenter_distance <= 0.0) {
    		semi_major_axis = Utility::strToDouble(params["orbit_semimajoraxis"],-1e100);

    		if (semi_major_axis <= -1e100) {
    			cLog::get()->write("OrbitCreatorComet::handle you must provide orbit_pericenterdistance or orbit_semimajoraxis");
    			return nullptr;
    		}
    		else {
    			if (eccentricity == 1.0) {
    				cLog::get()->write("OrbitCreatorComet::handle parabolic orbits have no semi_major_axis");
    				return nullptr;
    			}
    			pericenter_distance = semi_major_axis * (1.0-eccentricity);
    		}
    	}
    	else {
    		semi_major_axis = (eccentricity == 1.0)
    		                  ? 0.0 // parabolic orbits have no semi_major_axis
    		                  : pericenter_distance / (1.0-eccentricity);
    	}
    	double mean_motion = Utility::strToDouble(params["orbit_meanmotion"],-1e100);
    	double period;
    	if (mean_motion <= -1e100) {
    		period = Utility::strToDouble(params["orbit_period"],-1e100);
    		if (period <= -1e100) {
    			if (parent->getParent()) {
    				cLog::get()->write("OrbitCreatorComet::handle When the parent body is not the Sun\nyou must provide orbit_MeanMotion or orbit_Period");
    				return nullptr;
    			}
    			mean_motion = (eccentricity == 1.0)
    			              ? 0.01720209895 * (1.5/pericenter_distance)
    			              * sqrt(0.5/pericenter_distance)
    			              : (semi_major_axis > 0.0)
    			              ? 0.01720209895 / (semi_major_axis*sqrt(semi_major_axis))
    			              : 0.01720209895 / (-semi_major_axis*sqrt(-semi_major_axis));
    		}
    		else {
    			mean_motion = 2.0*M_PI/period;
    		}
    	}
    	else {
    		mean_motion *= (M_PI/180.0);
    	}

    	double time_at_pericenter = Utility::strToDouble(params["orbit_timeatpericenter"],-1e100);

    	if (time_at_pericenter <= -1e100) {
    		const double epoch = Utility::strToDouble(params["orbit_epoch"],-1e100);
    		double mean_anomaly = Utility::strToDouble(params["orbit_meananomaly"],-1e100);
    		if (epoch <= -1e100 || mean_anomaly <= -1e100) {
    			cLog::get()->write("OrbitCreatorComet::handle when you do not provide orbit_TimeAtPericenter, you must provide both orbit_Epoch and orbit_MeanAnomaly");
    			return nullptr;
    		}
    		else {
    			mean_anomaly *= (M_PI/180.0);
    			time_at_pericenter = epoch - mean_anomaly / mean_motion;
    		}
    	}

    	const double inclination = Utility::strToDouble(params["orbit_inclination"])*(M_PI/180.0);
    	const double ascending_node = Utility::strToDouble(params["orbit_ascendingnode"])*(M_PI/180.0);
    	const double arg_of_pericenter = Utility::strToDouble(params["orbit_argofpericenter"])*(M_PI/180.0);

    	return std::make_unique<CometOrbit>(
            pericenter_distance, eccentricity,
            inclination, ascending_node,
            arg_of_pericenter, time_at_pericenter,
            mean_motion, parent_rot_obliquity,
            parent_rot_asc_node, parent_rot_J2000_longitude
        );
    }
};
