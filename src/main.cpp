#include "renderer.h"
#include "utils.h"
#include <string>

int main(int argc, char *argv[])
{
    struct ParamsStruct GlobalParams;
    if (argc == 1)
        GlobalParams.FilePath = "../params/params.ini";
    else
        GlobalParams.FilePath = std::string(argv[1]);
    GlobalParams.ParseFile();

    auto R = Renderer(GlobalParams);

    // Try to initialize, run, and exit the renderer
    return (R.Init() && R.Run() && R.Exit()) ? 0 : -1;
}