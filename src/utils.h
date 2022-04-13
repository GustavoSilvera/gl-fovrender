#ifndef UTILS
#define UTILS

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

inline bool stob(const std::string &s)
{
    // assuming s is either "true" or "false"
    return (s.at(0) == 't' || s.at(0) == 'T');
}

inline auto readFile(const std::string_view path) -> const std::string
{
    std::cout << "Reading shader: \"" << path << "\"" << std::endl;
    std::ifstream Input(path);
    if (!Input.is_open())
    {
        std::cout << "Unable to read file \"" << path << "\"" << std::endl;
        exit(1);
    }
    // Avoid dynamic allocation: read the 4096 first bytes
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size))
    {
        out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
}

struct MainShaderParams
{
    std::string vertex_shader_path, non_fr_fragment_shader_path, fragment_shader_dir, fragment_shader_name;
};

struct FRShaderParams
{
    std::string drop_shader, reconstruction_shader;
};

struct WindowParamsStruct
{
    int X0, Y0;
};

struct ParamsStruct
{
    bool bEnableVsync;
    bool bEnableFovRender, bEnablePostProcessing;

    MainShaderParams MainParams;
    FRShaderParams FRParams;
    WindowParamsStruct WindowParams;
    std::string FilePath;
    void ParseFile()
    {
        // create input stream to get file data
        std::ifstream Input(FilePath);
        if (!Input.is_open())
        {
            std::cout << "ERROR: could not open \"" << FilePath << "\"" << std::endl;
            exit(1);
        }
        std::cout << "Reading params from \"" << FilePath << "\"" << std::endl;

        // read from file into params
        std::string Tmp;
        const std::string Delim = "=";
        while (!Input.eof())
        {
            Input >> Tmp;
            if (Input.bad() || Input.fail())
                break;
            if (Tmp.at(0) == '[' || Tmp.at(0) == '#' || Tmp.at(0) == ';') // ignoring labels & comments
                continue;
            std::string ParamName = Tmp.substr(0, Tmp.find(Delim));
            std::string ParamValue = Tmp.substr(Tmp.find(Delim) + 1, Tmp.size());
            if (!ParamName.compare("enable_vsync"))
                bEnableVsync = stob(ParamValue);
            else if (!ParamName.compare("enable_foveated_render"))
                bEnableFovRender = stob(ParamValue);
            else if (!ParamName.compare("enable_postprocessing"))
                bEnablePostProcessing = stob(ParamValue);
            else if (!ParamName.compare("vertex_shader"))
                MainParams.vertex_shader_path = ParamValue;
            else if (!ParamName.compare("fragment_shaders"))
                MainParams.fragment_shader_dir = ParamValue;
            else if (!ParamName.compare("non_fr_fragment_shader"))
                MainParams.non_fr_fragment_shader_path = ParamValue;
            else if (!ParamName.compare("start_frag_shader"))
                MainParams.fragment_shader_name = ParamValue;
            else if (!ParamName.compare("fr_fragment_shader"))
                FRParams.drop_shader = ParamValue;
            else if (!ParamName.compare("fr_reconstruction_shader"))
                FRParams.reconstruction_shader = ParamValue;
            else if (!ParamName.compare("init_width"))
                WindowParams.X0 = std::stoi(ParamValue);
            else if (!ParamName.compare("init_height"))
                WindowParams.Y0 = std::stoi(ParamValue);
            else
                continue;
        }
    }
};

#endif