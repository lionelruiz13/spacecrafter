#ifndef OJML_HPP_INCLUDED
#define OJML_HPP_INCLUDED

#include "tools/vecmath.hpp"
#include "tools/s_texture.hpp"
//#include "renderGL/shader.hpp"
#include <vulkan/vulkan.h>

#include <vector>
#include <string>
#include <memory>

class VertexArray;

class OjmL {
public:
	OjmL(const std::string& _fileName);
	~OjmL();

	//! renvoie l'état de l'objet: chargé et opérationnel, négatif sinon
	bool getOk() {
		return is_ok;
	}

	//! dessine l'objet
	void draw(VkPrimitiveTopology mode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

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
	std::vector<unsigned int> indices;
	std::unique_ptr<VertexArray> dGL;
};


#endif // OJM_HPP_INCLUDED
