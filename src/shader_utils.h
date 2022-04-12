/// NOTE: this code was modified from https://github.com/k0pernicus/opengl-explorer

#ifndef _SHADER_UTILS_H
#define _SHADER_UTILS_H

#include <string>
#include <vector>

namespace ShaderUtils
{

struct Shader
{
    Shader(const std::string &f, const std::string &t, const int ty) : file_path(f), title(t), type(ty), ShaderID(-1)
    {
    }
    std::string file_path = "";
    std::string title = "shader";
    int type;
    int ShaderID = -1; // -1 for unregistered
};

struct Program
{

  private:
    int program = 0;

    bool registerShader(Shader &S);
    bool registerProgram();
    void DeleteShaders();

    std::vector<Shader> Shaders;

  public:
    Program();
    ~Program();

    bool loadShaders(const std::vector<Shader> &shaders);
    bool Reload();
    int GetProgram() const;
};

} // namespace ShaderUtils

#endif /* SHADER_UTILS_H */
