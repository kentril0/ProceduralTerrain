/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <memory>
#include <array>

#include <SGL/opengl/Shader.h>
#include <SGL/opengl/VertexArray.h>
#include <SGL/opengl/CubeMapTexture.h>


class Skybox
{
public:
    using FacesPaths = 
        std::array<const char*, sgl::CubeMapTexture::CubeFaceCount>;

    using FacesData = sgl::CubeMapTexture::FacesData;

    static std::unique_ptr<Skybox> CreateUniq(
        const std::shared_ptr<sgl::Shader>& shader, 
        const FacesPaths& faces);
public:
    // TODO pass just the cubemap
    /**
     * @brief Loads and sets up skybox faces and VAO for rendering.
     * @param shader Shader program used to render the skybox.
     * @param faces Paths to textures for each face of the cubemap
     */
    Skybox(const std::shared_ptr<sgl::Shader>& shader, 
           const FacesPaths& faces);

    ~Skybox();

    /**
     * @brief Binds its VAO and shader to render the cubemap
     * @pre Expects Depth Test to be Run BEFORE
     * @param viewMat Camera view matrix.
     * @param projMat Camera projection matrix.
     */
    void Render(const glm::mat4& viewMat,
                const glm::mat4& projMat) const;

private:    
    
    void CreateCubeMap(const FacesPaths& facesPaths);
    void CreateCubeVAO();

    void LoadCubeMapFacesData(const FacesPaths& facesPaths,
                              int& outWidth,
                              int& outHeight,
                              int& outChannels,
                              FacesData& imagesData) const;

    void FreeCubeMapFacesData(const FacesData& imagesData) const;

private:    
    std::shared_ptr<sgl::Shader> m_Shader;

    std::unique_ptr<sgl::CubeMapTexture> m_CubeMap;
    std::unique_ptr<sgl::VertexArray> m_CubeVAO;
};
