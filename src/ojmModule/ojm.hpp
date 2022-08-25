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

	//! returns the state of the object: loaded and operational, negative otherwise
	bool getOk() {
		return is_ok;
	}

	//! loads and initializes an OJM object
	bool init(float multiplier = 1.0);

	//! draws the object
	//void draw(shaderProgram *shader);

    //! @brief record Ojm draw commands
    //! @param pipelines {pipeline with texture, pipeline without texture}
    int record(VkCommandBuffer &cmd, Pipeline *pipelines, PipelineLayout *layout, int selectedPipeline = -1, bool firstRecorded = true);

	//! for debugging : print
	void print();

private:
	bool is_ok = false; //say if the model is correctly initialised and operationnal
	//! checks if the indices in the object match

	bool testIndices();

	//! loads an OJM object from the hard disk
	bool readOJM(const std::string& filename, float multiplier= 1.0);

	//! indices the different pieces of the object
	std::vector<Shape> shapes;

	//! initializes all GL parameters of the ojm
	void initGLparam();

	//! removes GL parameters from the ojm
	void delGLparam();

	std::string fileName;
	std::string pathFile;
};


#endif // OJM_HPP_INCLUDED
