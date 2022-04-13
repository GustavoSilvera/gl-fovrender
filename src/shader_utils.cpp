/// NOTE: this code was modified from https://github.com/k0pernicus/opengl-explorer

#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#endif

#include "shader_utils.h"
#include "utils.h"
#include <filesystem>
#include <iostream>
#include <optional>

namespace ShaderUtils
{
Program::Program()
{
}

Program::~Program()
{
    glDeleteProgram(program);
}

bool Program::Reload()
{
    // delete the current program
    glDeleteProgram(GetProgram());

    // load Shaders new
    return loadShaders(Shaders);
}

bool Program::loadShaders(const std::vector<Shader> &ShaderStructList)
{
    std::cout << std::endl;
    Shaders = ShaderStructList;
    for (auto &ShaderStruct : Shaders)
    {
        if (!registerShader(ShaderStruct))
        {
            std::cerr << "failed to register the " << ShaderStruct.title << " shader..." << std::endl;
            return false;
        }
    }
    if (!registerProgram())
    {
        std::cerr << "failed to register the program..." << std::endl;
        return false;
    }
    return true;
}

bool Program::registerShader(Shader &S)
{
    // first read the file
    const std::string shader_src_str = readFile(S.file_path);
    const char *shader_source = shader_src_str.c_str();

    // then create the shader and compile
    int success = {};
    char errorMessage[1024] = {};
    int shader = glCreateShader(S.type);
    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    // ensure shader compilation
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 1024, NULL, errorMessage);
        std::cerr << "Shader compilation error : " << errorMessage << std::endl;
        return false;
    }
    S.ShaderID = shader;
    return true;
}

bool Program::registerProgram()
{
    int success = {};
    char errorMessage[1024] = {};

    program = glCreateProgram();

    for (auto &Shader : Shaders)
    {
        glAttachShader(program, Shader.ShaderID);
    }
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 1024, NULL, errorMessage);
        std::cerr << "Shader linking error: " << errorMessage << std::endl;
        return false;
    }

    // We can now delete our other shaders
    DeleteShaders();
    return true;
}

int Program::GetProgram() const
{
    return program;
}

void Program::DeleteShaders()
{
    for (auto &Shader : Shaders)
    {
        glDeleteShader(Shader.ShaderID);
    }
}

bool MainProgram::loadShaders(const ParamsStruct &P)
{
    Shaders.clear();
    Shaders = {
        ShaderUtils::Shader(P.MainParams.vertex_shader_path, "vertex", GL_VERTEX_SHADER),
        ShaderUtils::Shader(P.MainParams.fragment_shader_dir + P.MainParams.fragment_shader_name, "main",
                            GL_FRAGMENT_SHADER),
        ShaderUtils::Shader(P.bEnableFovRender ? P.FRParams.drop_shader : P.MainParams.non_fr_fragment_shader_path,
                            "fragment", GL_FRAGMENT_SHADER),
    };
    // read all the shaders in the FragmentShaderPath
    for (const auto &file : std::filesystem::directory_iterator(P.MainParams.fragment_shader_dir))
    {
        std::string path = file.path();
        std::cout << "Found shader: \"" << path << "\"" << std::endl;
        OtherShaderPaths.push_back(path);
    }
    std::cout << std::endl;

    // call parent load
    return Program::loadShaders(Shaders);
}

bool MainProgram::Reload(const ParamsStruct &P)
{
    // expects an up-to-date ParamsStruct instance
    if (OtherShaderPaths.size() > 0)
    {
        Shaders.clear();
        Shaders = {
            ShaderUtils::Shader(P.MainParams.vertex_shader_path, "vertex", GL_VERTEX_SHADER),
            ShaderUtils::Shader(OtherShaderPaths[ShaderIdx], "main", GL_FRAGMENT_SHADER),
            ShaderUtils::Shader(P.bEnableFovRender ? P.FRParams.drop_shader : P.MainParams.non_fr_fragment_shader_path,
                                "fragment", GL_FRAGMENT_SHADER),
        };
        ShaderIdx = (ShaderIdx + 1) % OtherShaderPaths.size();
    }
    // call parent reload
    return Program::Reload();
}
}; // namespace ShaderUtils