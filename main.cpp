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
#include <fstream>
#include <iostream>
#include <sstream>

auto shader_utils = ShaderUtils::Program{};

GLFWwindow *initializeWindow(const int *screen_size)
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
    glViewport(0, 0, screen_size[0], screen_size[1]);

    return window;
}

inline auto readFile(const std::string_view path) -> const std::string
{
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
    const std::string basicVertexShaderSource = readFile("../shaders/vertex_shader.glsl");
    const std::string basicFragmentShaderSource = readFile("../shaders/wave_shader_frag.glsl");

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

int main(void)
{
    // Initialize the lib
    if (!glfwInit())
    {
        std::cerr << "could not start GLFW3" << std::endl;
        return -1;
    }

    const int W = 1920;
    const int H = 1080;

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

    const float quadVerts[] = {-1.f, -1.f, 0.f, 0.f, -1.f, 1.f, 0.f, 1.f, 1.f, -1.f, 1.f, 0.f,
                               1.f,  -1.f, 1.f, 0.f, -1.f, 1.f, 0.f, 1.f, 1.f, 1.f,  1.f, 1.f};

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(0);

    // disable vsync
    bool enable_vsync = true;
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
            glUniform1f(glGetUniformLocation(shaderProgram, "iTime"), glfwGetTime());
            glfwGetWindowSize(window, &screen_size[0], &screen_size[1]);
            float screen_size_f[] = {static_cast<float>(screen_size[0]), static_cast<float>(screen_size[1])};
            glUniform2fv(glGetUniformLocation(shaderProgram, "iResolution"), 1, screen_size_f);

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
        glDrawArrays(GL_TRIANGLES, 0, 6);
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