#ifndef OJM_HPP_INCLUDED
#define OJM_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include "tools/s_texture.hpp"

#include <vector>
#include <string>
#include <memory>
#include "EntityCore/Forward.hpp"
#include "EntityCore/SubBuffer.hpp"

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

	std::unique_ptr<s_texture> map_Ka;
	std::unique_ptr<s_texture> map_Kd;
	std::unique_ptr<s_texture> map_Ks;

    std::unique_ptr<VertexBuffer> vertex;
    SubBuffer index {};
};

class Ojm {
public:
	Ojm(const std::string& _fileName);
	Ojm(const std::string& _fileName, const std::string& _pathFile, float multiplier);
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
    int record(VkCommandBuffer &cmd, Pipeline *pipelines, PipelineLayout *layout, int selectedPipeline = -1, bool firstRecorded = true);

	//! pour debugger : print
	void print();

private:
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
