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

	//! USE WITH CAUTION - Create an ObjL linked to this one - Destroying this object INVALIDATE all OjmL linked to it
	std::unique_ptr<OjmL> makeLink();

	//! returns the state of the object: loaded and operational, negative otherwise
	bool getOk() {
		return is_ok;
	}

	//! Draw this OjmL, this is compatible with any bind() of an OjmL to this VkCommandBuffer
	void draw(VkCommandBuffer &cmd);

	//! Bind VertexBuffer and IndexBuffer to draw any OjmL
	void bind(VkCommandBuffer &cmd);

	//! bind VertexArray
	void bind(Pipeline &pipeline);

	int getVertexCount() const;
	int getIndexCount() const {
		return indexCount;
	}
	//! Deprecated, only used by ring.cpp
	VertexBuffer *getVertexBuffer() {return vertex.get();}
	//! Deprecated, only used by ring.cpp
	const SubBuffer &getIndexBuffer() const {return index;}
private:
	bool is_ok = false;

	//! loads and initializes an OJM object
	bool init(const std::string& _fileName);

	//! loads an OJM object from the hard disk
	bool readOJML(const std::string& _fileName);

	//! initializes all GL parameters of the ojm
	void initGLparam();

	//! deletes the GL parameters of the ojm
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
