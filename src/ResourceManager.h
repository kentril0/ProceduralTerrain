/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <SGL/opengl/Shader.h>


class ResourceManager
{
public:
    static void AddShader(const std::shared_ptr<sgl::Shader>& shader,
                          const std::string& name);

    static std::shared_ptr<sgl::Shader> GetShader(const std::string& name);

    static bool ShaderExists(const std::string& name);

    static void ClearAll();

private:
    static std::unordered_map<
        std::string, std::shared_ptr<sgl::Shader>
    > s_Shaders;
};
