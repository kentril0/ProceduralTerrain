/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "Skybox.h"

#include <memory>
#include <array>
#include <glm/glm.hpp>

#define SGL_DEBUG
#define SGL_ENABLE_ASSERTS
#include <SGL/SGL.h>


std::unique_ptr<Skybox> Skybox::CreateUniq(
        const std::shared_ptr<sgl::Shader>& shader,
        const FacesPaths& faces)
{
    return std::make_unique<Skybox>(shader, faces);
}

Skybox::Skybox(const std::shared_ptr<sgl::Shader>& shader, 
               const FacesPaths& facesPaths)
    : m_Shader(shader)
{
    CreateCubeMap(facesPaths);
    CreateCubeVAO();
}

Skybox::~Skybox()
{

}

void Skybox::Render(const glm::mat4& viewMat, const glm::mat4& projMat) const
{
    // Expects depth test to be run BEFORE

    // Changes depth function to render the skybox as the furthest object
    glDepthFunc(GL_LEQUAL);
    
    m_Shader->Use();

    // Remove the translation part from the view matrix
    glm::mat4 tempViewMat = glm::mat4(glm::mat3(viewMat));

    m_Shader->SetMat4("projview", projMat * tempViewMat);
    
    m_CubeVAO->Bind();

    //glActiveTexture(GL_TEXTURE0);
    m_CubeMap->Bind();

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glDepthFunc(GL_LESS);
}

void Skybox::CreateCubeMap(const FacesPaths& facesPaths)
{
    FacesData imagesData{};

    int width, height, channels;
    LoadCubeMapFacesData(facesPaths, width, height, channels, imagesData);

    m_CubeMap = std::make_unique<sgl::CubeMapTexture>(
        width,
        height,
        channels,
        imagesData
    );

    FreeCubeMapFacesData(imagesData);
}

void Skybox::CreateCubeVAO()
{
    const float kVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
    };

    auto vbo = sgl::VertexBuffer::Create(kVertices, sizeof(kVertices) );

    vbo->SetLayout({
        { sgl::ElementType::Float3, "Position" }
    });

    m_CubeVAO = sgl::VertexArray::CreateUniq();
    m_CubeVAO->AddVertexBuffer(vbo);
}

void Skybox::LoadCubeMapFacesData(const FacesPaths& facesPaths,
    int& outWidth, int& outHeight, int& outChannels, 
    FacesData& imagesData) const
{
    for (uint32_t i = 0; i < imagesData.size(); ++i)
    {
        imagesData[i] = sgl::LoadImageData(facesPaths[i],
                                           outWidth, outHeight,
                                           outChannels);
        SGL_ASSERT_MSG(imagesData[i] != NULL,
                       "Failed to load cube map image '{}'",
                       facesPaths[i]);
    }
}

void Skybox::FreeCubeMapFacesData(const FacesData& imagesData) const
{
    for (uint32_t i = 0; i < imagesData.size(); ++i)
    {
        sgl::FreeImageData(imagesData[i]);
    }
}
