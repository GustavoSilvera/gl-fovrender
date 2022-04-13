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
    void displayFps(double &lastTime, int &nbFrames, GLFWwindow *pWindow);
    void TalkWithProgram(int program, GLFWwindow *window, int nbFrames, int *screen_size, double *mouse_pos,
                         float currentTime);
    ParamsStruct Params;

    // buffer objects
    GLuint FBO, VBO, VAO, texture_map;

    // screen params
    int *screen_size = nullptr;

    // window
    GLFWwindow *window;

    ShaderUtils::MainProgram main_program;
    ShaderUtils::Program reconstruct_program;

  public:
    Renderer(struct ParamsStruct &P) : Params(P)
    {
    }

    bool Init();
    bool Run();
    bool Exit();
};

#endif
