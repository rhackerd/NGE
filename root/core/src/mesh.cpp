#include "mesh.h"
#include "buffer.h"
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cglm/vec3.h>
#include <vector>
namespace Nova::GE {
    bool Mesh::init(const std::string& path, VmaAllocator allocator) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            path,
            aiProcess_Triangulate |
            aiProcess_GenUVCoords |
            aiProcess_FlipUVs |
            aiProcess_GenNormals |
            aiProcess_JoinIdenticalVertices
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            printf("Mesh init(): failed to load '%s': %s\n", path.c_str(), importer.GetErrorString());
            return false;
        }

        std::vector<Vertex> vertices;
        std::vector<u32>    indices;

        loadNode(scene->mRootNode, scene, vertices, indices);

        m_indexCount = static_cast<u32>(indices.size());

        //
        // Upload vertex bufferP
        //

        auto vertCI = CreateInfo::Buffer::Builder()
            .setAllocator(allocator)
            .setSize(sizeof(Vertex) * vertices.size())
            .asVertex()
            .build();
        m_vertexBuffer.init(vertCI);
        m_vertexBuffer.upload(vertices.data(), sizeof(Vertex) * vertices.size());

        //
        // Upload index buffer
        //
        auto idxCI = CreateInfo::Buffer::Builder()
            .setAllocator(allocator)
            .setSize(sizeof(u32) * indices.size())
            .asIndex()
            .build();
        m_indexBuffer.init(idxCI);
        m_indexBuffer.upload(indices.data(), sizeof(u32) * indices.size());

        this->vertices = vertices;
        this->indices = indices;

        return isValid();
    }

    void Mesh::shutdown() {
        if (!isValid()) return;
        m_vertexBuffer.shutdown();
        m_indexBuffer.shutdown();
        m_indexCount = 0;
    };

    void Mesh::loadNode(aiNode* node, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
            loadMesh(scene->mMeshes[node->mMeshes[i]], scene, vertices, indices);

        for (uint32_t i = 0; i < node->mNumChildren; i++)
            loadNode(node->mChildren[i], scene, vertices, indices);
    }

    void Mesh::loadMesh(aiMesh* mesh, const aiScene* scene, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
        uint32_t vertexOffset = static_cast<uint32_t>(vertices.size());

        //
        // Vertices
        //
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            Vertex v{};

            v.pos[0] = mesh->mVertices[i].x;
            v.pos[1] = mesh->mVertices[i].y;
            v.pos[2] = mesh->mVertices[i].z;

            if (mesh->HasNormals()) {
                v.normal[0] = mesh->mNormals[i].x;
                v.normal[1] = mesh->mNormals[i].y;
                v.normal[2] = mesh->mNormals[i].z;
            }

            if (mesh->mTextureCoords[0]) {
                v.uv[0] = mesh->mTextureCoords[0][i].x;
                v.uv[1] = mesh->mTextureCoords[0][i].y;
            }

            vertices.push_back(v);
        }

        //
        // Indices
        //
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace& face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++)
                indices.push_back(vertexOffset + face.mIndices[j]);
        }
    }
};  