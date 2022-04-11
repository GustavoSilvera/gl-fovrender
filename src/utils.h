#ifndef UTILS
#define UTILS

#include <cassert>
#include <cmath> // pow
#include <cstdio>
#include <fstream>
#include <iostream>
#include <vector>

inline bool stob(const std::string &s)
{
    // assuming s is either "true" or "false"
    return (s.at(0) == 't' || s.at(0) == 'T');
}

struct MainParamsStruct
{
    std::string vertex_shader_path, fragment_shader_path;
    bool bEnableVsync;
};

struct WindowParamsStruct
{
    int X0, Y0;
};

struct ParamsStruct
{
    MainParamsStruct MainParams;
    WindowParamsStruct WindowParams;
};

// global params (extern for multiple .o files)
extern ParamsStruct GlobalParams;

inline void ParseParams(const std::string &FilePath)
{
    // create input stream to get file data
    std::ifstream Input(FilePath);
    if (!Input.is_open())
    {
        std::cout << "ERROR: could not open" << FilePath << std::endl;
        exit(1);
    }
    std::cout << "Reading params from " << FilePath << std::endl;

    // read from file into params
    std::string Tmp;
    const std::string Delim = "=";
    while (!Input.eof())
    {
        Input >> Tmp;
        if (Input.bad() || Input.fail())
            break;
        if (Tmp.at(0) == '[' || Tmp.at(0) == '#') // ignoring labels & comments
            continue;
        std::string ParamName = Tmp.substr(0, Tmp.find(Delim));
        std::string ParamValue = Tmp.substr(Tmp.find(Delim) + 1, Tmp.size());
        if (!ParamName.compare("enable_vsync"))
            GlobalParams.MainParams.bEnableVsync = stob(ParamValue);
        else if (!ParamName.compare("vertex_shader"))
            GlobalParams.MainParams.vertex_shader_path = ParamValue;
        else if (!ParamName.compare("fragment_shader"))
            GlobalParams.MainParams.fragment_shader_path = ParamValue;
        else if (!ParamName.compare("init_width"))
            GlobalParams.WindowParams.X0 = std::stoi(ParamValue);
        else if (!ParamName.compare("init_height"))
            GlobalParams.WindowParams.Y0 = std::stoi(ParamValue);
        else
            continue;
    }
}

#endif