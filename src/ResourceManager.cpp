/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "ResourceManager.h"

#define SGL_DEBUG
#define SGL_ENABLE_ASSERTS 
#include <SGL/SGL.h>


std::unordered_map<
    std::string, std::shared_ptr<sgl::Shader>
> ResourceManager::s_Shaders;


void ResourceManager::AddShader(const std::shared_ptr<sgl::Shader>& shader,
    const std::string& name)
{
    if (ShaderExists(name))
        return;
    
    s_Shaders[name] = shader;
}

std::shared_ptr<sgl::Shader> ResourceManager::GetShader(const std::string& name)
{
    SGL_ASSERT_MSG(ShaderExists(name), "Shader {} does not exist!", name);
    return s_Shaders[name];
}

bool ResourceManager::ShaderExists(const std::string& name)
{
    return s_Shaders.find(name) != s_Shaders.end();
}

void ResourceManager::ClearAll()
{
    s_Shaders.clear();
}