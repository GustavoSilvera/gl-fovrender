/// NOTE: this code was modified from https://github.com/k0pernicus/opengl-explorer

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
#include <fstream>
#include <iostream>
#include <sstream>

// global params struct
struct ParamsStruct GlobalParams;

GLFWwindow *initializeWindow(int *screen_size)
{
    assert(screen_size != nullptr); // should be {W, H}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const auto T0 = "Loading shaders..."; // initial title
    GLFWwindow *window = glfwCreateWindow(screen_size[0], screen_size[1], T0, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "window creation failed" << std::endl;
        return nullptr;
    }
    // Makes the window context current
    glfwMakeContextCurrent(window);
    // Enable the viewport
    glfwGetFramebufferSize(window, &screen_size[0], &screen_size[1]);
    glViewport(0, 0, screen_size[0], screen_size[1]);

    return window;
}

void displayFps(double &lastTime, int &nbFrames, GLFWwindow *pWindow)
{
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    nbFrames++;
    if (delta > 1.f) // more than a second ago
    {
        const double fps = double(nbFrames) / delta;
        std::stringstream ss;
        ss << "[" << fps << " FPS]";
        glfwSetWindowTitle(pWindow, ss.str().c_str());
        // std::cout << "FPS: " << 1.f / (currentTime - lastTime) << std::endl;
        nbFrames = 0;
        lastTime = currentTime;
    }
}

void TalkWithProgram(int program, GLFWwindow *window, int nbFrames, int *screen_size, double *mouse_pos,
                     float currentTime)
{
    // send data to the active shader program

    // send iTime
    glUniform1f(glGetUniformLocation(program, "iTime"), currentTime);

    // send iFrame
    glUniform1i(glGetUniformLocation(program, "iFrame"), nbFrames);

    // send iResolution
    glfwGetFramebufferSize(window, &screen_size[0], &screen_size[1]);
    glViewport(0, 0, screen_size[0], screen_size[1]);
    float screen_size_f[] = {static_cast<float>(screen_size[0]), static_cast<float>(screen_size[1])};
    glUniform2fv(glGetUniformLocation(program, "iResolution"), 1, screen_size_f);

    // send iMouse
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        // only capture mouse pos when (left) pressed
        glfwGetCursorPos(window, &mouse_pos[0], &mouse_pos[1]);
    }
    float mouse_pos_f[] = {static_cast<float>(mouse_pos[0]), static_cast<float>(mouse_pos[1])};
    glUniform2fv(glGetUniformLocation(program, "iMouse"), 1, mouse_pos_f);

    // communicate foveated render params
    glUniform1i(glGetUniformLocation(program, "stride"), 32);
    const float diag = 0.5f * (screen_size[0] + screen_size[1]);
    const float thresh1 = 0.1f * diag;
    const float thresh2 = 0.25f * diag;
    const float thresh3 = 0.4f * diag;
    glUniform1f(glGetUniformLocation(program, "thresh1"), thresh1);
    glUniform1f(glGetUniformLocation(program, "thresh2"), thresh2);
    glUniform1f(glGetUniformLocation(program, "thresh3"), thresh3);
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        GlobalParams.FilePath = "../params/params.ini";
    else
        GlobalParams.FilePath = std::string(argv[1]);
    GlobalParams.ParseFile();

    // Initialize the lib
    if (!glfwInit())
    {
        std::cerr << "could not start GLFW3" << std::endl;
        return -1;
    }

    const int W = GlobalParams.WindowParams.X0;
    const int H = GlobalParams.WindowParams.Y0;

    int screen_size[] = {W, H};
    GLFWwindow *window = initializeWindow(screen_size);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;

    auto main_program = ShaderUtils::MainProgram{};
    bool status = main_program.loadShaders(GlobalParams);

    if (!status)
    {
        std::cerr << "can't load the shaders to initiate the main program" << std::endl;
        glfwTerminate();
        return -1;
    }

    auto reconstruct_program = ShaderUtils::Program{};
    status = reconstruct_program.loadShaders({
        ShaderUtils::Shader(GlobalParams.MainParams.vertex_shader_path, "vertex", GL_VERTEX_SHADER),
        ShaderUtils::Shader(GlobalParams.FRParams.reconstruction_shader, "reconstruct", GL_FRAGMENT_SHADER),
    });

    if (!status)
    {
        std::cerr << "can't load the shaders to initiate the FR program" << std::endl;
        glfwTerminate();
        return -1;
    }

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    const float CanvasVerts[] = {
        // top right triangle
        -1.0f, 1.0f, 0.0f, // Top-left
        1.0f, 1.0f, 0.0f,  // Top-right
        1.0f, -1.0f, 0.0f, // Bottom-right
        // bottom left triangle
        -1.0f, -1.0f, 0.0f, // Bottom-left
        -1.0f, 1.0f, 0.0f,  // Top-left
        1.0f, -1.0f, 0.0f,  // Bottom-right
    };

    // create frame buffer object
    GLuint FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // create texture map for FBO
    GLuint texture_map;
    glGenTextures(1, &texture_map);
    glBindTexture(GL_TEXTURE_2D, texture_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_size[0], screen_size[1], 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // link the texture map with the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_map, 0);
    // check for problems
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "can't initialize FBO" << std::endl;
        glfwTerminate();
        return -1;
    }
    // unbind to not accidentally render to it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create vertex buffer object
    GLuint VBO;
    glGenBuffers(1, &VBO); // generate 1 vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CanvasVerts), CanvasVerts, GL_STATIC_DRAW);

    // create vertex attribute object
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    const int stride = 3;
    const void *offset = (void *)(0);
    const auto bNoramalize = GL_FALSE;
    glVertexAttribPointer(0, stride, GL_FLOAT, bNoramalize, stride * sizeof(float), offset);
    glEnableVertexAttribArray(0);

    // disable vsync
    bool enable_vsync = GlobalParams.bEnableVsync;
    glfwSwapInterval(int(enable_vsync));

    // get mouse pos
    double mouse_pos[] = {0.0, 0.0};
    glfwGetCursorPos(window, &mouse_pos[0], &mouse_pos[1]);

    double lastTime = glfwGetTime();
    int nbFrames = 0;
    bool bReloadDown = false; // only reload on rising edge
    bool bPauseDown = false;  // only pause on rising edge
    bool bTickClock = true;   // tick clock initially
    float currentTime = 0.f;

    while (!glfwWindowShouldClose(window))
    {
        int shaderProgram = main_program.GetProgram();
        int reconstructProgram = reconstruct_program.GetProgram();

        // first, render to default
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Clear canvas
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw main shader
        glUseProgram(shaderProgram);
        TalkWithProgram(shaderProgram, window, nbFrames, screen_size, mouse_pos, currentTime);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2 (3 vertex) triangles for rect

        // copy framebuffer (current rendered buffer) to FBO (& its texture)
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glBlitFramebuffer(0, 0, screen_size[0], screen_size[1], 0, 0, screen_size[0], screen_size[1],
                          GL_COLOR_BUFFER_BIT, GL_LINEAR);
        // Draw reconstruction shader

        // render FBO as fullscreen quad on default FBO
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);      // unbind your FBO to set the default FBO
        glBindTexture(GL_TEXTURE_2D, texture_map); // bind texture to current active texture

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(reconstructProgram);
        TalkWithProgram(reconstructProgram, window, nbFrames, screen_size, mouse_pos, currentTime);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2 (3 vertex) triangles for rect

        // Poll for and process events
        glfwPollEvents();

        // check for closing window
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        // check for reloading shaders
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !bReloadDown)
        {
            std::cout << "Reloading shaders..." << std::endl;
            bReloadDown = true;
            /// TODO: reload shaders with new params
            main_program.Reload();
            reconstruct_program.Reload();
            std::cout << "Done!" << std::endl;
        }
        else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
        {
            bReloadDown = false;
        }

        // check for toggling time shaders
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bPauseDown)
        {
            std::cout << "Pausing shaders..." << std::endl;
            bTickClock = !bTickClock;
            bPauseDown = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
        {
            bPauseDown = false;
        }

        /// TODO: make this continue from where it left off
        if (bTickClock)
            currentTime = glfwGetTime();

        /// TODO: use arrow keys to switch between shaders!!

        // display fps in title
        displayFps(lastTime, nbFrames, window);

        // Swap front and back buffers
        glfwSwapBuffers(window);
    }

    // ... here, the user closed the window
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &FBO);
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}