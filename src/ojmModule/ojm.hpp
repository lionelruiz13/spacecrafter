#ifndef OJM_HPP_INCLUDED
#define OJM_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include "tools/s_texture.hpp"
#include "renderGL/shader.hpp"
#include <vector>
#include <string>
#include <memory>

class VertexArray;
class CommandMgr;
class Set;
class Pipeline;
class PipelineLayout;
class VirtualSurface;

struct Shape {
    std::string name;

	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::vector<unsigned int> indices;

	Vec3f Ka;
	Vec3f Kd;
	Vec3f Ks;
	float Ns;
	float T=1.0;

	s_texture *map_Ka=nullptr;
	s_texture *map_Kd=nullptr;
	s_texture *map_Ks=nullptr;

	std::unique_ptr<VertexArray> dGL;
};

class Ojm {
public:
	Ojm(const std::string& _fileName, VirtualSurface *surface);
	Ojm(const std::string& _fileName, const std::string& _pathFile, float multiplier, VirtualSurface *_surface);
	~Ojm();

	//! renvoie l'état de l'objet: chargé et opérationnel, négatif sinon
	bool getOk() {
		return is_ok;
	}

	//! charge et initialise un objet OJM
	bool init(float multiplier = 1.0);

	//! dessine l'objet
	//void draw(shaderProgram *shader);

    //! @brief record Ojm draw commands
    //! @param pipelines {pipeline with texture, pipeline without texture}
    int record(CommandMgr *cmdMgr, Pipeline *pipelines, PipelineLayout *layout, Set *set, int selectedPipeline = -1);

	//! pour debugger : print
	void print();

private:
    VirtualSurface *surface;
	bool is_ok = false; //say if the model is correctly initialised and operationnal
	//! vérifie si les indices coincident dans l'objet

	bool testIndices();

	//! charge un objet OJM du disque dur
	bool readOJM(const std::string& filename, float multiplier= 1.0);

	//! indices des différents morceaux de l'objet
	std::vector<Shape> shapes;

	//! initialise tous les parametres GL de l'ojm
	void initGLparam();

	//! supprime les paramètres GL de l'ojm
	void delGLparam();

	std::string fileName;
	std::string pathFile;
};


#endif // OJM_HPP_INCLUDED
