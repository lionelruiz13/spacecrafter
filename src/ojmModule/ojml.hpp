#ifndef OJML_HPP_INCLUDED
#define OJML_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include "tools/s_texture.hpp"
#include <vulkan/vulkan.h>

#include "EntityCore/SubBuffer.hpp"

#include <vector>
#include <string>
#include <memory>

class VertexArray;
class VertexBuffer;
class Pipeline;

class OjmL {
public:
	OjmL(const std::string& _fileName);
	OjmL(std::shared_ptr<VertexBuffer> vertex, SubBuffer index, unsigned int indexCount);
	~OjmL();

	//! renvoie l'état de l'objet: chargé et opérationnel, négatif sinon
	bool getOk() {
		return is_ok;
	}

	//! Draw this OjmL, this is compatible with any bind() of an OjmL to this VkCommandBuffer
	void draw(VkCommandBuffer &cmd);

	//! Bind VertexBuffer and IndexBuffer to draw any OjmL
	void bind(VkCommandBuffer &cmd);

	//! bind VertexArray
	void bind(Pipeline &pipeline);

	//! Deprecated, only used by ring.cpp
	VertexBuffer *getVertexBuffer() {return vertex.get();}
	//! Deprecated, only used by ring.cpp
	const SubBuffer &getIndexBuffer() {return index;}
private:
	bool is_ok = false;

	//! charge et initialise un objet OJM
	bool init(const std::string& _fileName);

	//! charge un objet OJM du disque dur
	bool readOJML(const std::string& _fileName);

	//! initialise tous les parametres GL de l'ojm
	void initGLparam();

	//! supprime les paramètres GL de l'ojm
	void delGLparam();

	std::vector<float> vertices;
	std::vector<float> uvs;
	std::vector<float> normals;
	std::shared_ptr<VertexBuffer> vertex;
	SubBuffer index {};
	unsigned int *pIndex = nullptr;
	unsigned int indexCount = 0;
};


#endif // OJM_HPP_INCLUDED
