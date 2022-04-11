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

auto shader_utils = ShaderUtils::Program{};

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

const bool loadShaderProgram(const bool erase_if_program_registered = true)
{
    const std::string basicVertexShaderSource = readFile(GlobalParams.MainParams.vertex_shader_path);
    const std::string basicFragmentShaderSource = readFile(GlobalParams.MainParams.fragment_shader_path);

    if (!shader_utils.registerShader(ShaderUtils::Type::VERTEX_SHADER_TYPE, basicVertexShaderSource.c_str()))
    {
        std::cerr << "failed to register the vertex shader..." << std::endl;
        return false;
    }

    if (!shader_utils.registerShader(ShaderUtils::Type::FRAGMENT_SHADER_TYPE, basicFragmentShaderSource.c_str()))
    {
        std::cerr << "failed to register the fragment shader..." << std::endl;
        return false;
    }

    if (!shader_utils.registerProgram(erase_if_program_registered))
    {
        std::cerr << "failed to register the program..." << std::endl;
        return false;
    }
    return true;
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
// global params struct
struct ParamsStruct GlobalParams;

int main(int argc, char *argv[])
{
    if (argc == 1)
        ParseParams("../params/params.ini");
    else
    {
        const std::string ParamFile(argv[1]);
        ParseParams(ParamFile);
    }

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

    if (!loadShaderProgram(false))
    {
        std::cerr << "can't load the shaders to initiate the program" << std::endl;
        glfwTerminate();
        return -1;
    }

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    const float quadVerts[] = {
        // top right triangle
        -1.0f, 1.0f, 0.0f, // Top-left
        1.0f, 1.0f, 0.0f,  // Top-right
        1.0f, -1.0f, 0.0f, // Bottom-right
        // bottom left triangle
        -1.0f, -1.0f, 0.0f, // Bottom-left
        -1.0f, 1.0f, 0.0f,  // Top-left
        1.0f, -1.0f, 0.0f,  // Bottom-right
    };
    //{-1.f, -1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 1.f, 1.f, -1.f, 1.f, 0.f,                              1.f,  -1.f, 1.f,
    // 0.f, -1.f, 1.f, 0.f, 1.f, 1.f, 1.f,  1.f, 1.f};

    GLuint VBO;
    glGenBuffers(1, &VBO); // generate 1 vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    const int stride = 3;
    const void *offset = (void *)(0);
    const auto bNoramalize = GL_FALSE;
    glVertexAttribPointer(0, stride, GL_FLOAT, bNoramalize, stride * sizeof(float), offset);
    glEnableVertexAttribArray(0);

    // disable vsync
    bool enable_vsync = GlobalParams.MainParams.bEnableVsync;
    glfwSwapInterval(int(enable_vsync));

    // get mouse pos
    double mouse_pos[] = {0.0, 0.0};
    glfwGetCursorPos(window, &mouse_pos[0], &mouse_pos[1]);

    double lastTime = glfwGetTime();
    int nbFrames = 0;
    while (!glfwWindowShouldClose(window))
    {
        const unsigned int shaderProgram = shader_utils.getProgram().value();
        // send data to the shader program
        {
            // send iTime
            glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), glfwGetTime());

            // send iFrame
            glUniform1i(glGetUniformLocation(shaderProgram, "iFrame"), nbFrames);

            // send iResolution
            glfwGetFramebufferSize(window, &screen_size[0], &screen_size[1]);
            glViewport(0, 0, screen_size[0], screen_size[1]);
            float screen_size_f[] = {static_cast<float>(screen_size[0]), static_cast<float>(screen_size[1])};
            glUniform2fv(glGetUniformLocation(shaderProgram, "iResolution"), 1, screen_size_f);

            // send iMouse
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                // only capture mouse pos when (left) pressed
                glfwGetCursorPos(window, &mouse_pos[0], &mouse_pos[1]);
            }
            float mouse_pos_f[] = {static_cast<float>(mouse_pos[0]), static_cast<float>(mouse_pos[1])};
            glUniform2fv(glGetUniformLocation(shaderProgram, "iMouse"), 1, mouse_pos_f);
        }

        // Render
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
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
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            std::cout << "reloading..." << std::endl;
            loadShaderProgram(true);
        }

        // display fps in title
        displayFps(lastTime, nbFrames, window);

        // Swap front and back buffers
        glfwSwapBuffers(window);
    }

    // ... here, the user closed the window
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}