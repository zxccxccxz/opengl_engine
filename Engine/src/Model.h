// Mesh Group
#pragma once

#include <vector>
#include <string>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Texture.h"

class Model {
public:
	Model();
	~Model();

	void LoadModel(const std::string& filepath);
	void RenderModel();
	void ClearModel();

private:
	void LoadNode(aiNode* node, const aiScene* scene);
	void LoadMesh(aiMesh* mesh, const aiScene* scene);
	void LoadMaterials(const aiScene* scene);


	std::vector<std::unique_ptr<Mesh>> _mesh_list;
	std::vector<std::unique_ptr<Texture>> _texture_list;
	std::vector<unsigned int> _mesh_to_tex;
};