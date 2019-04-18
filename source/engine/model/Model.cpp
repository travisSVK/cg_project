#include "Model.h"
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include <tiny_obj_loader.h>
//#include <experimental/tinyobj_loader_opt.h>
#include <algorithm>
#include <sstream>
#include <iomanip> 
#include <GL/glew.h>
#include <stb_image.h>
#include <filesystem>

namespace engine
{
	bool Texture::load(const std::string & _filename, int _components) {
		filename = _filename;
        numComponents = _components;
		valid = true; 
		int components; 
		data = stbi_load(filename.c_str(), &width, &height, &components, _components);
		if (data == nullptr) {
			std::cout << "ERROR: loadModelFromOBJ(): Failed to load texture: " << filename << "\n";
			exit(1);
		}
		glGenTextures(1, &gl_id);
		glBindTexture(GL_TEXTURE_2D, gl_id);
		GLenum format, internal_format;
		if (_components == 1) { format = GL_R;  internal_format = GL_R8; }
		else if (_components == 3) { format = GL_RGB; internal_format = GL_SRGB; }
		else if (_components == 4) { format = GL_RGBA;  internal_format = GL_SRGB_ALPHA; }
		else {
			std::cout << "Texture loading not implemented for this number of compenents.\n";
			exit(1);
		}
		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
		return true; 
	}

    bool NormalMap::load(const std::string & _filename, int _components)
    {
        filename = _filename;
        valid = true;
        int components;
        data = stbi_load(filename.c_str(), &width, &height, &components, _components);
        if (data == nullptr) {
            std::cout << "ERROR: loadModelFromOBJ(): Failed to load texture: " << filename << "\n";
            exit(1);
        }
        glGenTextures(1, &gl_id);
        glBindTexture(GL_TEXTURE_2D, gl_id);
        GLenum format, internal_format;
        if (_components == 1) { format = GL_R;  internal_format = GL_R8; }
        else if (_components == 3) { format = GL_RGB; internal_format = GL_SRGB; }
        else if (_components == 4) { format = GL_RGBA;  internal_format = GL_SRGB_ALPHA; }
        else {
            std::cout << "Texture loading not implemented for this number of compenents.\n";
            exit(1);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
        return true;
    }

	///////////////////////////////////////////////////////////////////////////
	// Destructor
	///////////////////////////////////////////////////////////////////////////
	Model::~Model()
	{
		for (auto & material : m_materials) {
			if (material.m_color_texture.valid) glDeleteTextures(1, &material.m_color_texture.gl_id);
			if (material.m_reflectivity_texture.valid) glDeleteTextures(1, &material.m_reflectivity_texture.gl_id);
			if (material.m_shininess_texture.valid) glDeleteTextures(1, &material.m_shininess_texture.gl_id);
			if (material.m_metalness_texture.valid) glDeleteTextures(1, &material.m_metalness_texture.gl_id);
			if (material.m_fresnel_texture.valid) glDeleteTextures(1, &material.m_fresnel_texture.gl_id);
			if (material.m_emission_texture.valid) glDeleteTextures(1, &material.m_emission_texture.gl_id);
            if (material.normalMap.valid) glDeleteTextures(1, &material.normalMap.gl_id);
		}
		glDeleteBuffers(1, &m_positions_bo);
		glDeleteBuffers(1, &m_normals_bo);
		glDeleteBuffers(1, &m_texture_coordinates_bo);
        glDeleteVertexArrays(1, &m_vaob);
	}

    glm::mat4 Model::getModelMatrix()
    {
        return m_modelMatrix;
    }

	Model * loadModelFromOBJ(std::string path)
	{
		///////////////////////////////////////////////////////////////////////
		// Separate filename into directory, base filename and extension
		// NOTE: This can be made a LOT simpler as soon as compilers properly 
		//		 support std::filesystem (C++17)
		///////////////////////////////////////////////////////////////////////
		size_t separator = path.find_last_of("\\/");
		std::string filename, extension, directory; 
		if (separator != std::string::npos) {
			filename = path.substr(separator + 1, path.size() - separator - 1); 
			directory = path.substr(0, separator + 1); 
		}
		else {
			filename = path; 
			directory = "./";
		}
		separator = filename.find_last_of(".");
		if (separator == std::string::npos) {
			std::cout << "Fatal: loadModelFromOBJ(): Expecting filename ending in '.obj'\n";
			exit(1);
		}
		extension = filename.substr(separator, filename.size() - separator);
		filename = filename.substr(0, separator);
        // check if model loaded before
        Model* model = nullptr; // loadModelBinary(directory + filename);
        if (!model)
        {
            ///////////////////////////////////////////////////////////////////////
        // Parse the OBJ file using tinyobj
        ///////////////////////////////////////////////////////////////////////
            std::cout << "Loading " << path << "..." << std::flush;
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string err;
            // Expect '.mtl' file in the same directory and triangulate meshes 
            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
                (directory + filename + extension).c_str(), directory.c_str(),
                true);
            if (!err.empty()) { // `err` may contain warning message.
                std::cerr << err << std::endl;
            }
            if (!ret) { exit(1); }
            model = new Model;
            model->m_name = filename;
            model->m_filename = path;

            ///////////////////////////////////////////////////////////////////////
            // Transform all materials into our datastructure
            ///////////////////////////////////////////////////////////////////////
            for (const auto & m : materials) {
                Material material;
                material.m_name = m.name;
                material.m_color = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
                if (m.diffuse_texname != "") {
                    material.m_color_texture.load(directory + m.diffuse_texname, 4);
                }
                material.m_reflectivity = m.specular[0];
                if (m.specular_texname != "") {
                    material.m_reflectivity_texture.load(directory + m.specular_texname, 1);
                }
                material.m_metalness = m.metallic;
                if (m.metallic_texname != "") {
                    material.m_metalness_texture.load(directory + m.metallic_texname, 1);
                }
                material.m_fresnel = m.sheen;
                if (m.sheen_texname != "") {
                    material.m_fresnel_texture.load(directory + m.sheen_texname, 1);
                }
                material.m_shininess = m.roughness;
                if (m.roughness_texname != "") {
                    material.m_fresnel_texture.load(directory + m.sheen_texname, 1);
                }
                material.m_emission = m.emission[0];
                if (m.emissive_texname != "") {
                    material.m_emission_texture.load(directory + m.emissive_texname, 4);
                }
                if (m.bump_texname != "")
                {
                    material.normalMap.load(directory + m.bump_texname, 3);
                }
                material.m_transparency = m.transmittance[0];
                model->m_materials.push_back(material);
            }

            ///////////////////////////////////////////////////////////////////////
            // A vertex in the OBJ file may have different indices for position, 
            // normal and texture coordinate. We will not even attempt to use 
            // indexed lookups, but will store a simple vertex stream per mesh. 
            ///////////////////////////////////////////////////////////////////////
            int number_of_vertices = 0;
            for (const auto & shape : shapes) {
                number_of_vertices += shape.mesh.indices.size();
            }
            model->m_positions.resize(number_of_vertices);
            model->m_normals.resize(number_of_vertices);
            model->m_texture_coordinates.resize(number_of_vertices);

            ///////////////////////////////////////////////////////////////////////
            // For each vertex _position_ auto generate a normal that will be used
            // if no normal is supplied. 
            ///////////////////////////////////////////////////////////////////////
            std::vector<glm::vec4> auto_normals(attrib.vertices.size() / 3);
            for (const auto & shape : shapes) {
                for (int face = 0; face < int(shape.mesh.indices.size()) / 3; face++) {

                    glm::vec3 v0 = glm::vec3(
                        attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 0],
                        attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 1],
                        attrib.vertices[shape.mesh.indices[face * 3 + 0].vertex_index * 3 + 2]
                    );
                    glm::vec3 v1 = glm::vec3(
                        attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 0],
                        attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 1],
                        attrib.vertices[shape.mesh.indices[face * 3 + 1].vertex_index * 3 + 2]
                    );
                    glm::vec3 v2 = glm::vec3(
                        attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 0],
                        attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 1],
                        attrib.vertices[shape.mesh.indices[face * 3 + 2].vertex_index * 3 + 2]
                    );

                    glm::vec3 e0 = glm::normalize(v1 - v0);
                    glm::vec3 e1 = glm::normalize(v2 - v0);
                    glm::vec3 face_normal = cross(e0, e1);

                    auto_normals[shape.mesh.indices[face * 3 + 0].vertex_index] += glm::vec4(face_normal, 1.0f);
                    auto_normals[shape.mesh.indices[face * 3 + 1].vertex_index] += glm::vec4(face_normal, 1.0f);
                    auto_normals[shape.mesh.indices[face * 3 + 2].vertex_index] += glm::vec4(face_normal, 1.0f);
                }
            }
            for (auto & normal : auto_normals) {
                normal = (1.0f / normal.w) * normal;
            }

            ///////////////////////////////////////////////////////////////////////
            // Now we will turn all shapes into Meshes. A shape that has several 
            // materials will be split into several meshes with unique names
            ///////////////////////////////////////////////////////////////////////
            int vertices_so_far = 0;
            for (const auto & shape : shapes)
            {
                ///////////////////////////////////////////////////////////////////
                // The shapes in an OBJ file may several different materials. 
                // If so, we will split the shape into one Mesh per Material
                ///////////////////////////////////////////////////////////////////
                int next_material_index = shape.mesh.material_ids[0];
                int next_material_starting_face = 0;
                std::vector<bool> finished_materials(materials.size(), false);
                int number_of_materials_in_shape = 0;
                while (next_material_index != -1)
                {
                    int current_material_index = next_material_index;
                    int current_material_starting_face = next_material_starting_face;
                    next_material_index = -1;
                    next_material_starting_face = -1;
                    // Process a new Mesh with a unique material
                    Mesh mesh;
                    mesh.m_name = shape.name + "_" + materials[current_material_index].name;
                    mesh.m_material_idx = current_material_index;
                    mesh.m_start_index = vertices_so_far;
                    number_of_materials_in_shape += 1;
                    int number_of_faces = shape.mesh.indices.size() / 3;
                    for (int i = current_material_starting_face; i < number_of_faces; i++)
                    {
                        if (shape.mesh.material_ids[i] != current_material_index) {
                            if (next_material_index >= 0) continue;
                            else if (finished_materials[shape.mesh.material_ids[i]]) continue;
                            else { // Found a new material that we have not processed.
                                next_material_index = shape.mesh.material_ids[i];
                                next_material_starting_face = i;
                            }
                        }
                        else {
                            ///////////////////////////////////////////////////////
                            // Now we generate the vertices
                            ///////////////////////////////////////////////////////
                            for (int j = 0; j < 3; j++) {
                                int v = shape.mesh.indices[i * 3 + j].vertex_index;
                                model->m_positions[vertices_so_far + j] = glm::vec3(
                                    attrib.vertices[shape.mesh.indices[i * 3 + j].vertex_index * 3 + 0],
                                    attrib.vertices[shape.mesh.indices[i * 3 + j].vertex_index * 3 + 1],
                                    attrib.vertices[shape.mesh.indices[i * 3 + j].vertex_index * 3 + 2]);
                                if (shape.mesh.indices[i * 3 + j].normal_index == -1) {
                                    // No normal, use the autogenerated
                                    model->m_normals[vertices_so_far + j] = glm::vec3(auto_normals[shape.mesh.indices[i * 3 + j].vertex_index]);
                                }
                                else {
                                    model->m_normals[vertices_so_far + j] = glm::vec3(
                                        attrib.normals[shape.mesh.indices[i * 3 + j].normal_index * 3 + 0],
                                        attrib.normals[shape.mesh.indices[i * 3 + j].normal_index * 3 + 1],
                                        attrib.normals[shape.mesh.indices[i * 3 + j].normal_index * 3 + 2]);
                                }
                                if (shape.mesh.indices[i * 3 + j].texcoord_index == -1) {
                                    // No UV coordinates. Use null. 
                                    model->m_texture_coordinates[vertices_so_far + j] = glm::vec2(0.0f);
                                }
                                else {
                                    model->m_texture_coordinates[vertices_so_far + j] = glm::vec2(
                                        attrib.texcoords[shape.mesh.indices[i * 3 + j].texcoord_index * 2 + 0],
                                        attrib.texcoords[shape.mesh.indices[i * 3 + j].texcoord_index * 2 + 1]);
                                }
                            }
                            vertices_so_far += 3;
                        }
                    }
                    ///////////////////////////////////////////////////////////////
                    // Finalize and push this mesh to the list
                    ///////////////////////////////////////////////////////////////
                    mesh.m_number_of_vertices = vertices_so_far - mesh.m_start_index;
                    model->m_meshes.push_back(mesh);
                    finished_materials[current_material_index] = true;
                }
                if (number_of_materials_in_shape == 1) {
                    model->m_meshes.back().m_name = shape.name;
                }
            }
            // save object as binary to load it faster next time
            //saveModelBinary(model, directory + filename);
        }
        model->m_loaded = true;
        for (int i = 0; i < model->m_positions.size(); i += 3)
        {
            glm::vec3 pos1 = model->m_positions.at(i);
            glm::vec3 pos2 = model->m_positions.at(i + 1);
            glm::vec3 pos3 = model->m_positions.at(i + 2);

            // texture coordinates
            glm::vec2 uv1 = model->m_texture_coordinates.at(i);
            glm::vec2 uv2 = model->m_texture_coordinates.at(i + 1);
            glm::vec2 uv3 = model->m_texture_coordinates.at(i + 2);

            glm::vec3 tangent1, bitangent1;

            //calculate triangle edge and delta UV coords 
            glm::vec3 edge1 = pos2 - pos1;
            glm::vec3 edge2 = pos3 - pos1;
            glm::vec2 deltaUV1 = uv2 - uv1;
            glm::vec2 deltaUV2 = uv3 - uv1;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            /*tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            tangent1 = glm::normalize(tangent1);

            bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
            bitangent1 = glm::normalize(bitangent1);*/
            tangent1 = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * f;
            bitangent1 = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * f;

            tangent1 = glm::normalize(tangent1);
            bitangent1 = glm::normalize(bitangent1);


            model->m_tangents.push_back(tangent1);
            model->m_tangents.push_back(tangent1);
            model->m_tangents.push_back(tangent1);
            model->m_bitTangents.push_back(bitangent1);
            model->m_bitTangents.push_back(bitangent1);
            model->m_bitTangents.push_back(bitangent1);

        }
		///////////////////////////////////////////////////////////////////////
		// Upload to GPU
		///////////////////////////////////////////////////////////////////////
		glGenVertexArrays(1, &model->m_vaob);
		glBindVertexArray(model->m_vaob);
		glGenBuffers(1, &model->m_positions_bo);
		glBindBuffer(GL_ARRAY_BUFFER, model->m_positions_bo);
		glBufferData(GL_ARRAY_BUFFER, model->m_positions.size() * sizeof(glm::vec3),
			&model->m_positions[0].x, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(0);
		glGenBuffers(1, &model->m_normals_bo);
		glBindBuffer(GL_ARRAY_BUFFER, model->m_normals_bo);
		glBufferData(GL_ARRAY_BUFFER, model->m_normals.size() * sizeof(glm::vec3),
			&model->m_normals[0].x, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);
		glGenBuffers(1, &model->m_texture_coordinates_bo);
		glBindBuffer(GL_ARRAY_BUFFER, model->m_texture_coordinates_bo);
		glBufferData(GL_ARRAY_BUFFER, model->m_texture_coordinates.size() * sizeof(glm::vec2),
			&model->m_texture_coordinates[0].x, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(2);

        glGenBuffers(1, &model->m_tangents_bo);
        glBindBuffer(GL_ARRAY_BUFFER, model->m_tangents_bo);
        glBufferData(GL_ARRAY_BUFFER, model->m_tangents.size() * sizeof(glm::vec3),
            &model->m_tangents[0].x, GL_STATIC_DRAW);
        glVertexAttribPointer(4, 3, GL_FLOAT, false, 0, 0);
        glEnableVertexAttribArray(4);

        glGenBuffers(1, &model->m_bitTangents_bo);
        glBindBuffer(GL_ARRAY_BUFFER, model->m_bitTangents_bo);
        glBufferData(GL_ARRAY_BUFFER, model->m_bitTangents.size() * sizeof(glm::vec3),
            &model->m_bitTangents[0].x, GL_STATIC_DRAW);
        glVertexAttribPointer(5, 3, GL_FLOAT, false, 0, 0);
        glEnableVertexAttribArray(5);

		std::cout << "done.\n";
		return model; 
	}

    void Texture::serialize(std::ofstream& myFile, std::ofstream& txtFile)
    {
        myFile.write(filename.c_str(), filename.size());
        txtFile << filename.size() << "\n";
        myFile.write((char*)&numComponents, sizeof(int));
    }

    void Texture::deserialize(std::ifstream& myFile, std::ifstream& txtFile)
    {
        std::string line;
        getline(txtFile, line);
        int len = std::stoi(line);
        std::string mystr(len, '\0');
        myFile.read(&mystr[0], len);
        filename = mystr;
        myFile.read((char*)&numComponents, sizeof(int));
        load(filename, numComponents);
    }

    void Material::serialize(std::ofstream& myFile, std::ofstream& txtFile)
    {
        myFile.write(m_name.c_str(), m_name.size());
        txtFile << m_name.size() << "\n";
        myFile.write((char*)&m_color, sizeof(glm::vec3));
        myFile.write((char*)&m_reflectivity, sizeof(float));
        myFile.write((char*)&m_shininess, sizeof(float));
        myFile.write((char*)&m_metalness, sizeof(float));
        myFile.write((char*)&m_fresnel, sizeof(float));
        myFile.write((char*)&m_emission, sizeof(float));
        myFile.write((char*)&m_transparency, sizeof(float));
        if (m_color_texture.filename.empty())
        {
            txtFile << 0 << "\n";
        }
        else
        {
            txtFile << 1 << "\n";
            m_color_texture.serialize(myFile, txtFile);
        }
        if (m_reflectivity_texture.filename.empty())
        {
            txtFile << 0 << "\n";
        }
        else
        {
            txtFile << 1 << "\n";
            m_reflectivity_texture.serialize(myFile, txtFile);
        }
        if (m_shininess_texture.filename.empty())
        {
            txtFile << 0 << "\n";
        }
        else
        {
            txtFile << 1 << "\n";
            m_shininess_texture.serialize(myFile, txtFile);
        }
        if (m_metalness_texture.filename.empty())
        {
            txtFile << 0 << "\n";
        }
        else
        {
            txtFile << 1 << "\n";
            m_metalness_texture.serialize(myFile, txtFile);
        }
        if (m_fresnel_texture.filename.empty())
        {
            txtFile << 0 << "\n";
        }
        else
        {
            txtFile << 1 << "\n";
            m_fresnel_texture.serialize(myFile, txtFile);
        }
        if (m_emission_texture.filename.empty())
        {
            txtFile << 0 << "\n";
        }
        else
        {
            txtFile << 1 << "\n";
            m_emission_texture.serialize(myFile, txtFile);
        }
    }

    void Material::deserialize(std::ifstream& myFile, std::ifstream& txtFile)
    {
        std::string line;
        getline(txtFile, line);
        int len = std::stoi(line);
        std::string mystr(len, '\0');
        myFile.read(&mystr[0], len);
        m_name = mystr;
        myFile.read((char*)&m_color, sizeof(glm::vec3));
        myFile.read((char*)&m_reflectivity, sizeof(float));
        myFile.read((char*)&m_shininess, sizeof(float));
        myFile.read((char*)&m_metalness, sizeof(float));
        myFile.read((char*)&m_fresnel, sizeof(float));
        myFile.read((char*)&m_emission, sizeof(float));
        myFile.read((char*)&m_transparency, sizeof(float));
        getline(txtFile, line);
        int dataSize = std::stoi(line);
        if (dataSize != 0)
        {
            m_color_texture.deserialize(myFile, txtFile);
        }
        getline(txtFile, line);
        dataSize = std::stoi(line);
        if (dataSize != 0)
        {
            m_reflectivity_texture.deserialize(myFile, txtFile);
        }
        getline(txtFile, line);
        dataSize = std::stoi(line);
        if (dataSize != 0)
        {
            m_shininess_texture.deserialize(myFile, txtFile);
        }
        getline(txtFile, line);
        dataSize = std::stoi(line);
        if (dataSize != 0)
        {
            m_metalness_texture.deserialize(myFile, txtFile);
        }
        getline(txtFile, line);
        dataSize = std::stoi(line);
        if (dataSize != 0)
        {
            m_fresnel_texture.deserialize(myFile, txtFile);
        }
        getline(txtFile, line);
        dataSize = std::stoi(line);
        if (dataSize != 0)
        {
            m_emission_texture.deserialize(myFile, txtFile);
        }
    }

    void Mesh::serialize(std::ofstream& myFile, std::ofstream& txtFile)
    {
        myFile.write(m_name.c_str(), m_name.size());
        txtFile << m_name.size() << "\n";
        myFile.write((char*)&m_material_idx, sizeof(uint32_t));
        myFile.write((char*)&m_start_index, sizeof(uint32_t));
        myFile.write((char*)&m_number_of_vertices, sizeof(uint32_t));
    }

    void Mesh::deserialize(std::ifstream& myFile, std::ifstream& txtFile)
    {
        std::string line;
        getline(txtFile, line);
        int len = std::stoi(line);
        std::string mystr(len, '\0');
        myFile.read(&mystr[0], len);
        m_name = mystr;
        myFile.read((char*)&m_material_idx, sizeof(uint32_t));
        myFile.read((char*)&m_start_index, sizeof(uint32_t));
        myFile.read((char*)&m_number_of_vertices, sizeof(uint32_t));
    }

    void saveModelBinary(Model * model, std::string filename)
    {
        // open bin file for obj and text file for info about count
        std::ofstream myFile(filename + ".bin", std::ios::out | std::ios::binary);
        std::ofstream txtFile(filename + ".txt");
        myFile.seekp(0);
        txtFile.seekp(0);
        myFile.write(model->m_name.c_str(), model->m_name.size());
        txtFile << model->m_name.size() << "\n";
        myFile.write(model->m_filename.c_str(), model->m_filename.size());
        txtFile << model->m_filename.size() << "\n";
        txtFile << model->m_materials.size() << "\n";
        for (auto& material : model->m_materials)
        {
            material.serialize(myFile, txtFile);
        }
        txtFile << model->m_meshes.size() << "\n";
        for (auto& mesh : model->m_meshes)
        {
            mesh.serialize(myFile, txtFile);
        }
        txtFile << model->m_positions.size() << "\n";
        for (auto& position : model->m_positions)
        {
            myFile.write((char*)&position, sizeof(glm::vec3));
        }
        txtFile << model->m_normals.size() << "\n";
        for (auto& normal : model->m_normals)
        {
            myFile.write((char*)&normal, sizeof(glm::vec3));
        }
        txtFile << model->m_texture_coordinates.size() << "\n";
        for (auto& texture_coordinate : model->m_texture_coordinates)
        {
            myFile.write((char*)&texture_coordinate, sizeof(glm::vec3));
        }
        
        myFile.write((char*)&model->m_positions_bo, sizeof(uint32_t));
        myFile.write((char*)&model->m_normals_bo, sizeof(uint32_t));
        myFile.write((char*)&model->m_texture_coordinates_bo, sizeof(uint32_t));
        myFile.write((char*)&model->m_vaob, sizeof(uint32_t));
        myFile.write((char*)&model->m_loaded, sizeof(bool));
        myFile.close();
        txtFile.close();
    }

    Model * loadModelBinary(std::string filename)
    {
        bool exists = std::experimental::filesystem::exists(filename + ".bin") && std::experimental::filesystem::exists(filename + ".txt");
        if (exists)
        {
            Model* model = new Model();
            std::ifstream myFile(filename + ".bin", std::ios::in | std::ios::binary);
            std::ifstream txtFile(filename + ".txt");
            myFile.seekg(0);
            txtFile.seekg(0);
            std::string line;
            getline(txtFile, line);
            int len = std::stoi(line);
            std::string mystr(len, '\0');
            myFile.read(&mystr[0], len);
            model->m_name = mystr;
            getline(txtFile, line);
            len = std::stoi(line);
            std::string mystr2(len, '\0');
            myFile.read(&mystr2[0], len);
            model->m_filename = mystr2;
            
            getline(txtFile, line);
            for (int i = 0; i < std::stoi(line); i++)
            {
                Material m;
                m.deserialize(myFile, txtFile);
                model->m_materials.push_back(m);
            }
            getline(txtFile, line);
            for (int i = 0; i < std::stoi(line); i++)
            {
                Mesh m;
                m.deserialize(myFile, txtFile);
                model->m_meshes.push_back(m);
            }
            getline(txtFile, line);
            for (int i = 0; i < std::stoi(line); i++)
            {
                glm::vec3 position;
                myFile.read((char*)&position, sizeof(glm::vec3));
                model->m_positions.push_back(position);
            }
            getline(txtFile, line);
            for (int i = 0; i < std::stoi(line); i++)
            {
                glm::vec3 normal;
                myFile.read((char*)&normal, sizeof(glm::vec3));
                model->m_normals.push_back(normal);
            }
            getline(txtFile, line);
            for (int i = 0; i < std::stoi(line); i++)
            {
                glm::vec3 texture_coordinate;
                myFile.read((char*)&texture_coordinate, sizeof(glm::vec3));
                model->m_texture_coordinates.push_back(texture_coordinate);
            }

            myFile.read((char*)&model->m_positions_bo, sizeof(uint32_t));
            myFile.read((char*)&model->m_normals_bo, sizeof(uint32_t));
            myFile.read((char*)&model->m_texture_coordinates_bo, sizeof(uint32_t));
            myFile.read((char*)&model->m_vaob, sizeof(uint32_t));
            myFile.read((char*)&model->m_loaded, sizeof(bool));
            txtFile.close();
            myFile.close();
            return model;
        }
        return nullptr;
    }

	void saveModelToOBJ(Model * model, std::string path)
	{
		///////////////////////////////////////////////////////////////////////
		// Separate filename into directory, base filename and extension
		// NOTE: This can be made a LOT simpler as soon as compilers properly 
		//		 support std::filesystem (C++17)
		///////////////////////////////////////////////////////////////////////
		size_t separator = path.find_last_of("\\/");
		std::string filename, extension, directory;
		if (separator != std::string::npos) {
			filename = path.substr(separator + 1, path.size() - separator - 1);
			directory = path.substr(0, separator + 1);
		}
		else {
			filename = path;
			directory = "./";
		}
		separator = filename.find_last_of(".");
		if (separator == std::string::npos) {
			std::cout << "Fatal: loadModelFromOBJ(): Expecting filename ending in '.obj'\n";
			exit(1);
		}
		extension = filename.substr(separator, filename.size() - separator);
		filename = filename.substr(0, separator);

		///////////////////////////////////////////////////////////////////////
		// Save Materials
		///////////////////////////////////////////////////////////////////////
		std::ofstream mat_file(directory + filename + ".mtl");
		if (!mat_file.is_open()) {
			std::cout << "Could not open file " << filename << " for writing.\n";
			return; 
		}
		mat_file << "# Exported by Chalmers Graphics Group\n";
		for (auto mat : model->m_materials)
		{
			mat_file << "newmtl " << mat.m_name << "\n";
			mat_file << "Kd " << mat.m_color.x << " " << mat.m_color.y << " " << mat.m_color.z << "\n";
			mat_file << "Ks " << mat.m_reflectivity << " " << mat.m_reflectivity << " " << mat.m_reflectivity << "\n";
			mat_file << "Pm " << mat.m_metalness << "\n";
			mat_file << "Ps " << mat.m_fresnel << "\n";
			mat_file << "Pr " << mat.m_shininess << "\n";
			mat_file << "Ke " << mat.m_emission << " " << mat.m_emission << " " << mat.m_emission << "\n";
			mat_file << "Tf " << mat.m_transparency << " " << mat.m_transparency << " " << mat.m_transparency << "\n";
			if (mat.m_color_texture.valid)
				mat_file << "map_Kd " << directory + mat.m_color_texture.filename << "\n";
			if (mat.m_reflectivity_texture.valid)
				mat_file << "map_Ks " << directory + mat.m_reflectivity_texture.filename << "\n";
			if (mat.m_metalness_texture.valid)
				mat_file << "map_Pm " << directory + mat.m_metalness_texture.filename << "\n";
			if (mat.m_fresnel_texture.valid)
				mat_file << "map_Ps " << directory + mat.m_fresnel_texture.filename << "\n";
			if (mat.m_shininess_texture.valid)
				mat_file << "map_Pr " << directory + mat.m_shininess_texture.filename << "\n";
			if (mat.m_emission_texture.valid)
				mat_file << "map_Ke " << directory + mat.m_emission_texture.filename << "\n";
            if (mat.m_emission_texture.valid)
				mat_file << "map_bump " << directory + mat.normalMap.filename << "\n";
		}
		mat_file.close(); 

		///////////////////////////////////////////////////////////////////////
		// Save geometry
		///////////////////////////////////////////////////////////////////////
		std::ofstream obj_file(directory + filename + ".obj");
		if (!obj_file.is_open()) {
			std::cout << "Could not open file " << filename << " for writing.\n";
			return;
		}
		obj_file << "# Exported by Chalmers Graphics Group\n";
		obj_file << "mtllib " << filename << ".mtl\n";
		int vertex_counter = 1; 
		for (auto mesh : model->m_meshes)
		{
			obj_file << "o " << mesh.m_name << "\n";
			obj_file << "g " << mesh.m_name << "\n";
			obj_file << "usemtl " << model->m_materials[mesh.m_material_idx].m_name << "\n";
			for (uint32_t i = mesh.m_start_index; i < mesh.m_start_index + mesh.m_number_of_vertices; i++)
			{
				obj_file << "v " << model->m_positions[i].x << " "
					<< model->m_positions[i].y << " "
					<< model->m_positions[i].z << "\n";
			}
			for (uint32_t i = mesh.m_start_index; i < mesh.m_start_index + mesh.m_number_of_vertices; i++)
			{
				obj_file << "vn " << model->m_normals[i].x << " "
					<< model->m_normals[i].y << " "
					<< model->m_normals[i].z << "\n";
			}
			for (uint32_t i = mesh.m_start_index; i < mesh.m_start_index + mesh.m_number_of_vertices; i++)
			{
				obj_file << "vt " << model->m_texture_coordinates[i].x << " "
					<< model->m_texture_coordinates[i].y << "\n";
			}
			int number_of_faces = mesh.m_number_of_vertices / 3; 
			for (int i = 0; i < number_of_faces; i++)
			{
				obj_file << "f " 
					<< vertex_counter << "/" << vertex_counter << "/" << vertex_counter << " "
					<< vertex_counter + 1 << "/" << vertex_counter + 1 << "/" << vertex_counter + 1 << " "
					<< vertex_counter + 2 << "/" << vertex_counter + 2 << "/" << vertex_counter + 2 << "\n";
				vertex_counter += 3; 
			}
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Free model 
	///////////////////////////////////////////////////////////////////////
	void freeModel(Model * model) {
		if(model != nullptr) delete model; 
	}

	///////////////////////////////////////////////////////////////////////
	// Loop through all Meshes in the Model and render them
	///////////////////////////////////////////////////////////////////////
	void render(const Model * model, const bool submitMaterials)
	{
		glBindVertexArray(model->m_vaob);
		for (auto & mesh : model->m_meshes)
		{
			if (submitMaterials) {
				const Material & material = model->m_materials[mesh.m_material_idx];

				bool has_color_texture = material.m_color_texture.valid;
				bool has_reflectivity_texture = material.m_reflectivity_texture.valid;
				bool has_metalness_texture = material.m_metalness_texture.valid;
				bool has_fresnel_texture = material.m_fresnel_texture.valid;
				bool has_shininess_texture = material.m_shininess_texture.valid;
				bool has_emission_texture = material.m_emission_texture.valid;
                bool has_normalMap = material.normalMap.valid;
				if (has_color_texture) glBindTextures(0, 1, &material.m_color_texture.gl_id);
				if (has_reflectivity_texture) glBindTextures(1, 1, &material.m_reflectivity_texture.gl_id);
				if (has_metalness_texture) glBindTextures(2, 1, &material.m_metalness_texture.gl_id);
				if (has_fresnel_texture) glBindTextures(3, 1, &material.m_fresnel_texture.gl_id);
				if (has_shininess_texture) glBindTextures(4, 1, &material.m_shininess_texture.gl_id);
				if (has_emission_texture) glBindTextures(5, 1, &material.m_emission_texture.gl_id);
				GLint current_program = 0;
				glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
				glUniform1i(glGetUniformLocation(current_program, "has_color_texture"), has_color_texture);
				glUniform1i(glGetUniformLocation(current_program, "has_diffuse_texture"), has_color_texture ? 1 : 0); // FIXME
				glUniform1i(glGetUniformLocation(current_program, "has_reflectivity_texture"), has_reflectivity_texture);
				glUniform1i(glGetUniformLocation(current_program, "has_metalness_texture"), has_metalness_texture);
				glUniform1i(glGetUniformLocation(current_program, "has_fresnel_texture"), has_fresnel_texture);
				glUniform1i(glGetUniformLocation(current_program, "has_shininess_texture"), has_shininess_texture);
				glUniform1i(glGetUniformLocation(current_program, "has_emission_texture"), has_emission_texture);
				glUniform3fv(glGetUniformLocation(current_program, "material_color"), 1, &material.m_color.x);
                glUniform3fv(glGetUniformLocation(current_program, "material_diffuse_color"), 1, &material.m_color.x);

                // @todo Stupid hack!
                if (material.m_color_texture.valid)
                {
                    glActiveTexture(GL_TEXTURE10);
                    glBindTexture(GL_TEXTURE_2D, material.m_color_texture.gl_id);
                    glActiveTexture(GL_TEXTURE11);
                    glBindTexture(GL_TEXTURE_2D, material.normalMap.gl_id);
				    glUniform1i(glGetUniformLocation(current_program, "material_texture"), 10); //FIXME: Compatibility with old shading model of lab3.
                    glUniform1i(glGetUniformLocation(current_program, "normalMap"), 11);
				    glUniform1i(glGetUniformLocation(current_program, "has_texture"), 1); //FIXME: Compatibility with old shading model of lab3.
                }
                else 
                {
                    glUniform1i(glGetUniformLocation(current_program, "has_texture"), 0); //FIXME: Compatibility with old shading model of lab3.
                }
				glUniform3fv(glGetUniformLocation(current_program, "material_diffuse_color"), 1, &material.m_color.x); //FIXME: Compatibility with old shading model of lab3.
				glUniform3fv(glGetUniformLocation(current_program, "material_emissive_color"), 1, &material.m_color.x); //FIXME: Compatibility with old shading model of lab3.
				glUniform1i(glGetUniformLocation(current_program, "has_diffuse_texture"), has_color_texture);//FIXME: Compatibility with old shading model of lab3.
				glUniform1fv(glGetUniformLocation(current_program, "material_reflectivity"), 1, &material.m_reflectivity);
				glUniform1fv(glGetUniformLocation(current_program, "material_metalness"), 1, &material.m_metalness);
				glUniform1fv(glGetUniformLocation(current_program, "material_fresnel"), 1, &material.m_fresnel);
				glUniform1fv(glGetUniformLocation(current_program, "material_shininess"), 1, &material.m_shininess);
				glUniform1fv(glGetUniformLocation(current_program, "material_emission"), 1, &material.m_emission);
			}
			glDrawArrays(GL_TRIANGLES, mesh.m_start_index, (GLsizei)mesh.m_number_of_vertices);
		}
	}
}
