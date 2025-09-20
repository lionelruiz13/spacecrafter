#ifndef ORBIT_LOADER_HPP_
#define ORBIT_LOADER_HPP_

#include <string>

class Orbit;

class OrbitLoader {
public:
    virtual std::unique_ptr<Orbit> load(std::map<std::string, std::string> &params) = 0;
};

#endif /* end of include guard: ORBIT_LOADER_HPP_ */
