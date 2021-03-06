#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <functional>

namespace engine
{	
	struct Texture {
		bool valid = false;
		uint32_t gl_id;
		std::string filename;
        std::string name;
		int width, height, numComponents;
        float scale;
        glm::vec2 screenPosition;
		uint8_t* data;
		bool load(const std::string & filename, int nof_components);
        void serialize(std::ofstream& myFile, std::ofstream& txtFile);
        void deserialize(std::ifstream& myFile, std::ifstream& txtFile);
	};

    struct NormalMap
    {
        bool valid = false;
        uint32_t gl_id;
        std::string filename;
        int width, height;
        uint8_t * data;
        bool load(const std::string & filename, int nof_components);
    };
	//////////////////////////////////////////////////////////////////////////////
	// This material class implements a subset of the suggested PBR extension
	// to the OBJ/MTL format, explained at: 
	// http://exocortex.com/blog/extending_wavefront_mtl_to_support_pbr
	// NOTE: A material can have _either_ a (textured) roughness, or a good old
	//       shininess value. We still use shininess for the Blinn mfd in the 
	//       GL labs, but roughness for pathtracing. 
	//////////////////////////////////////////////////////////////////////////////
	struct Material
	{
		std::string m_name;
		glm::vec3	m_color;
		float		m_reflectivity;
		float		m_shininess;
		float		m_metalness; 
		float		m_fresnel; 
		float		m_emission; 
		float		m_transparency; 
		Texture m_color_texture;
		Texture	m_reflectivity_texture;
		Texture	m_shininess_texture;
		Texture	m_metalness_texture;
		Texture	m_fresnel_texture;
		Texture	m_emission_texture;
        void serialize(std::ofstream& myFile, std::ofstream& txtFile);
        void deserialize(std::ifstream& myFile, std::ifstream& txtFile);
        NormalMap normalMap;
	};

	struct Mesh
	{
		std::string m_name;
		uint32_t m_material_idx; 
		// Where this Mesh's vertices start
		uint32_t m_start_index; 
		uint32_t m_number_of_vertices;
        void serialize(std::ofstream& myFile, std::ofstream& txtFile);
        void deserialize(std::ifstream& myFile, std::ifstream& txtFile);
	};

	class Model
	{
	public: 
		~Model(); 
		// The name of the whole model
		std::string m_name; 
		// The filename of this model 
		std::string m_filename;
		// The materials 
		std::vector<Material> m_materials; 
		// A model will contain one or more "Meshes"
		std::vector<Mesh> m_meshes; 
		// Buffers on CPU
		std::vector<glm::vec3> m_positions;
		std::vector<glm::vec3> m_normals;
		std::vector<glm::vec2> m_texture_coordinates;
        
        std::vector<glm::vec3> m_tangents;
        std::vector<glm::vec3> m_bitTangents;

        /*std::vector<glm::vec3> tangent;
        std::vector<glm::vec3> binormal;*/
		// Buffers on GPU
		uint32_t m_positions_bo;
		uint32_t m_normals_bo;
		uint32_t m_texture_coordinates_bo;

        uint32_t m_tangents_bo;
        uint32_t m_bitTangents_bo;

		// Vertex Array Object
		uint32_t m_vaob;
        bool m_loaded;
        bool m_enabled;
        glm::mat4 m_modelMatrix;
        glm::mat4 m_previousModelMatrix;
    public:
        glm::mat4 getModelMatrix();
	};

	Model * loadModelFromOBJ(std::string filename);
	void saveModelToOBJ(Model * model, std::string filename);
	void freeModel(Model * model);
	void render(const Model * model, bool showNormalMap, const bool submitMaterials = true);
    void renderWithDiffOptionForMesh(const Model * model, const std::string& meshName, const std::function<void()>& setFunction, const std::function<void()>& resetFunction, const bool submitMaterials = true);
    void saveModelBinary(Model * model, std::string filename);
    Model * loadModelBinary(std::string filename);
}