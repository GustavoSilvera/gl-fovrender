#include "renderer.h"

int main(int argc, char *argv[])
{
    auto R = Renderer(argc, argv);

    // Try to initialize, run, and exit the renderer
    return !(R.Init() && R.Run() && R.Exit());
}