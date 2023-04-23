#include "Model.h"
#include "Log.h"

Model::Model()
{
}

Model::~Model()
{
}

void Model::LoadModel(const std::string& filepath)
{
	LOG_CORE_TRACE("[MODEL] Loading model");
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath, 
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
	
	if (!scene)
	{
		LOG_CORE_ERROR("[MODEL] Failed to load model: {0}", importer.GetErrorString());
		return;
	}

	LoadNode(scene->mRootNode, scene);
	LoadMaterials(scene);
	LOG_CORE_INFO("[MODEL] Loaded");
}

// private
void Model::LoadNode(aiNode* node, const aiScene* scene)
{
	for (size_t i = 0; i < node->mNumMeshes; ++i)
	{
		LoadMesh(scene->mMeshes[node->mMeshes[i]], scene);
	}

	for (size_t i = 0; i < node->mNumChildren; ++i)
	{
		LoadNode(node->mChildren[i], scene);
	}
}

void Model::LoadMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<GLfloat> vertices;
	std::vector<unsigned int> indices;

	for (size_t i = 0; i < mesh->mNumVertices; ++i)
	{
		vertices.insert(vertices.end(), 
			{ mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z }
		);
		if (mesh->mTextureCoords[0])
		{
			vertices.insert(vertices.end(), 
				{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y });
		}
		else
		{
			vertices.insert(vertices.end(),{ 0.0f, 0.0f });
		}
		vertices.insert(vertices.end(),
			{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z }
		);
	}

	// face - 3 indices, which make triangle
	for (size_t i = 0; i < mesh->mNumFaces; ++i)
	{
		aiFace face = mesh->mFaces[i];
		for (size_t j = 0; j < face.mNumIndices; ++j)
		{
			indices.emplace_back(face.mIndices[j]);
		}
	}

	_mesh_list.emplace_back(std::make_unique<Mesh>())->CreateMesh
	(vertices.data(), indices.data(), vertices.size(), indices.size());

	_mesh_to_tex.emplace_back(mesh->mMaterialIndex);
}

void Model::LoadMaterials(const aiScene* scene)
{
	_texture_list.resize(scene->mNumMaterials);
	
	for (size_t i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* material = scene->mMaterials[i];

		if (material->GetTextureCount(aiTextureType_DIFFUSE))
		{
			aiString path;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				int idx = std::string(path.data).rfind("\\");
				std::string filepath = std::string(path.data).substr(idx + 1);

				std::string tex_path = std::string("res/textures/") + filepath;

				_texture_list[i] = std::make_unique<Texture>(tex_path);

				if (!_texture_list[i]->LoadTexture(false, false))
				{
					LOG_CORE_ERROR("[Model::LoadMaterials()] Failed to load texture, fp: {0}", tex_path.data());
					_texture_list[i].reset();
				}
			}
		}
		// If no texture assigned, use default texture
		if (_texture_list[i].get() == nullptr)
		{
			_texture_list[i] = std::make_unique<Texture>("res/textures/plain.png");
			_texture_list[i]->LoadTexture(false, false);
		}
	}
}

void Model::RenderModel()
{
	for (size_t i = 0; i < _mesh_list.size(); ++i)
	{
		unsigned int material_index = _mesh_to_tex[i];

		if ((material_index < _texture_list.size()) && 
			(_texture_list[material_index].get() != nullptr))
		{
			//printf();
			_texture_list[material_index]->UseTexture();
		}

		_mesh_list[i]->RenderMesh();
	}
}

void Model::ClearModel()
{
	// Smart pointers do this for me :3
}
