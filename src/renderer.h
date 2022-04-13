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

    // render thread
    void RenderPass();
    void PostprocessingPass();

    ParamsStruct Params;

    // buffer objects
    GLuint FBO, VBO, VAO, Tex;

    // window params
    int WindowW, WindowH;
    bool bEnableVsync = false;

    // other
    double CurrentTime;
    double LastTime;
    double LastTime1Sec;
    int NumFrames;
    bool bTickClock = true; // start ticking

    // input params
    double MouseX, MouseY;
    // button press rising-edge actions
    bool bPauseDown = false;  // only pause on rising edge
    bool bReloadDown = false; // only reload on rising edge

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
