#ifndef RENDERER_H
#define RENDERER_H

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

#include "shader_utils.h"
#include "utils.h"

class Renderer
{
  private:
    bool CreateWindow();
    void DisplayFps();
    void TalkWithProgram(int ProgramIdx);
    void CheckInputs();
    void TickClock();
    bool GenerateFBO();

    // callbacks
    void WindowCallbacks();

    // render thread
    void RenderPass();
    void PostprocessingPass();

    ParamsStruct Params;

    // buffer objects
    GLuint FBO, VBO, VAO, Tex;

    // window params
    int WindowW, WindowH;
    int LastWindowW, LastWindowH; // checking for window resize
    bool bEnableVsync = false;
    bool bIsHiDPI = false; // assume not hiDPI, check on window resize

    // other
    double CurrentTime = 0.0;
    double LastTime = 0.0;
    double LastTimeFps = 0.0; // last time but only refreshed for the fps counter
    int NumFrames = 0;
    bool bTickClock = true; // start ticking

    // input params
    double MouseX, MouseY;
    // button press rising-edge actions
    bool bPauseDown = false;
    bool bReloadDown = false;
    bool bPrevDown = false;
    bool bNextDown = false;
    bool bPPToggleDown = false;

    // window
    GLFWwindow *window = nullptr;

    // shader programs
    ShaderUtils::MainProgram Main;
    ShaderUtils::Program PostProc;

  public:
    Renderer(int argc, char *argv[]);

    bool Init();
    bool Run();
    bool Exit();
};

#endif
