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

	std::string map_Ka;
	std::string map_Kd;
	std::string map_Ks;
};


struct OjmHeader {
    ssize_t sourceTimestamp; // Last modification time of the source file
    float radius; // Radius of the ojm before normalization
    uint16_t nbShapes; // Number of shapes in this OJM
    bool poorlyCentered;
};

struct ShapeAttributes {
    Vec3f Ka;
    float Ns;
    Vec3f Kd;
    float T;
    Vec3f Ks;
};

struct ShapeHeader {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint8_t len_map_Ka;
    uint8_t len_map_Kd; // Unused - always 0
    uint8_t len_map_Ks; // Unused - always 0
    bool pushAttr; // True if attr differ from the previous shape
};

struct CompiledShape {
    ShapeAttributes attr;
    std::unique_ptr<s_texture> map_Ka;
    std::unique_ptr<VertexBuffer> vertex;
    SubBuffer index {};
    bool pushAttr;
};

class Ojm {
public:
    static std::shared_ptr<Ojm> load(const std::string &_fileName, const std::string &_pathFile);

	Ojm(const std::string& _fileName, const std::string& _pathFile);
	~Ojm();

	//! returns the state of the object: loaded and operational, negative otherwise
	bool getOk() {
		return is_ok;
	}

	//! loads and initializes an OJM object
	bool init();

	//! draws the object
	//void draw(shaderProgram *shader);

    //! @brief record Ojm draw commands
    //! @param pipelines {pipeline with texture, pipeline without texture}
    int record(VkCommandBuffer cmd, Pipeline *pipelines, PipelineLayout *layout, int selectedPipeline = -1, bool firstRecorded = true);

    // Record draws of the object, for shadow tracing
    void drawShadow(VkCommandBuffer cmd);

	//! for debugging : print
	void print();

    inline float getRadius() const {
        return radius;
    }

private:
    static std::map<std::string, std::weak_ptr<Ojm>> recycler;
	bool is_ok = false; //say if the model is correctly initialised and operationnal
	//! checks if the indices in the object match

	bool testIndices();

	//! loads an OJM object from the hard disk
	bool readOJM(const std::string& filename);

	//! indices the different pieces of the object
	std::vector<Shape> shapes;

    //! compiled shapes
    std::vector<CompiledShape> cshapes;

    //! Read the cache for the given OJM
    bool readCache();

    //! Compile the cache of this OJM
    void compileCache();

	std::string fileName;
	std::string pathFile;
    float radius = 0;
    bool poorlyCentered = false;
};


#endif // OJM_HPP_INCLUDED
