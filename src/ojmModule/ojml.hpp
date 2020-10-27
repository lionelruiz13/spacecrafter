#ifndef OJML_HPP_INCLUDED
#define OJML_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include "tools/s_texture.hpp"
//
#include <vulkanModule/Context.hpp>

#include <vector>
#include <string>
#include <memory>

class VertexArray;
class Pipeline;

class OjmL {
public:
	OjmL(const std::string& _fileName, ThreadContext *context, bool mergeVertexArray = false, int *maxVertex = nullptr, int *maxIndex = nullptr);
	~OjmL();

	void initFrom(VertexArray *vertex);

	//! renvoie l'état de l'objet: chargé et opérationnel, négatif sinon
	bool getOk() {
		return is_ok;
	}

	//! définit l'objet à dessiner
	void draw(void *pDrawData);

	VertexArray *getVertexArray() {return dGL.get();}

	//! bind VertexArray
	void bind(CommandMgr *cmdMgr);
	void bind(Pipeline *pipeline);
private:
	bool is_ok = false;

	//! charge et initialise un objet OJM
	bool init(const std::string& _fileName, ThreadContext *context, bool mergeVertexArray, int *maxVertex, int *maxIndex);

	//! charge un objet OJM du disque dur
	bool readOJML(const std::string& _fileName);

	//! initialise tous les parametres GL de l'ojm
	void initGLparam(ThreadContext *context, bool mergeVertexArray, int *maxVertex, int *maxIndex);

	//! supprime les paramètres GL de l'ojm
	void delGLparam();

	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::vector<unsigned int> indices;
	std::unique_ptr<VertexArray> dGL;
	struct {
	    uint32_t    indexCount;
	    uint32_t    instanceCount;
	    uint32_t    firstIndex;
	    int32_t     vertexOffset;
	    uint32_t    firstInstance;
	} drawData;
};


#endif // OJM_HPP_INCLUDED
