#include "MeshLoader.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Texture.h"

#include "Logger.h"

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

MeshLoader::MeshLoader(Asset file)
{
  Assimp::Importer importer;
  log_info("Loading mesh: %s", file.getFileName().c_str());

  const aiScene* scene = importer.ReadFileFromMemory(file.read(), file.getSize(),
                                                     aiProcess_Triangulate |
                                                     aiProcess_GenSmoothNormals |
                                                     aiProcess_FlipUVs |
                                                     aiProcess_CalcTangentSpace);

  if(!scene) {
    log_err("Failed to load mesh: %s", file.getFileName().c_str());
  } else {
    loadScene(scene);
  }

}

MeshLoader::~MeshLoader(void)
{
}

Entity *MeshLoader::getEntity(void)
{
  return m_entity;
}

void MeshLoader::loadScene(const aiScene* scene)
{
  m_entity = new Entity();

  for (int i = 0; i < scene->mNumMeshes; i++) {
    const aiMesh* model = scene->mMeshes[i];

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    const aiVector3D aiZeroVector(0.0f, 0.0f, 0.0f);
    for(unsigned int i = 0; i < model->mNumVertices; i++)
    {
      const aiVector3D* pPos = &(model->mVertices[i]);
      const aiVector3D* pNormal = &(model->mNormals[i]);
      const aiVector3D* pTexCoord = model->HasTextureCoords(0) ? &(model->mTextureCoords[0][i]) : &aiZeroVector;
      const aiVector3D* pTangent = &(model->mTangents[i]);

      Vertex vert(glm::vec3(pPos->x, pPos->y, pPos->z),
                  glm::vec2(pTexCoord->x, pTexCoord->y),
                  glm::vec3(pNormal->x, pNormal->y, pNormal->z),
                  glm::vec3(pTangent->x, pTangent->y, pTangent->z));

      vertices.push_back(vert);
    }

    for(unsigned int i = 0; i < model->mNumFaces; i++)
    {
      const aiFace& face = model->mFaces[i];
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
    }

    const aiMaterial* pMaterial = scene->mMaterials[model->mMaterialIndex];
    log_info("tex num: %i", model->mMaterialIndex);

    std::string texturePath;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
      aiString Path;

      if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
        texturePath = Path.data;
        log_info("Found tex: %s", texturePath.c_str());
      }
    }

    m_entity->addComponent(
      new MeshRenderer(
        new Mesh(&vertices[0], vertices.size(), &indices[0], indices.size()),
        new Material(new Texture(Asset("t.jpg")))
    ));
  }
}
