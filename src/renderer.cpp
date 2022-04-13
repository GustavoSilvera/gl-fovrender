#include "renderer.h"
#include <fstream>
#include <iostream>
#include <sstream>

Renderer::Renderer(int argc, char *argv[])
{

    Params.FilePath = (argc > 1) ? argv[1] : "../params/params.ini";
    Params.ParseFile();
}

bool Renderer::CreateWindow()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const auto T0 = "Loading shaders..."; // initial title
    window = glfwCreateWindow(WindowW, WindowH, T0, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "window creation failed" << std::endl;
        return false;
    }
    // Makes the window context current
    glfwMakeContextCurrent(window);
    // Enable the viewport
    glfwGetFramebufferSize(window, &WindowW, &WindowH);
    glViewport(0, 0, WindowW, WindowH);
    return true;
}

void Renderer::DisplayFps()
{
    const double DeltaT = glfwGetTime() - LastTime1Sec;
    NumFrames++;
    if (DeltaT > 1.f) // more than a second ago
    {
        const double Fps = double(NumFrames) / DeltaT;
        std::stringstream ss;
        ss << "[" << Fps << " FPS]";
        glfwSetWindowTitle(window, ss.str().c_str());
        // std::cout << "FPS: " << 1.f / (currentTime - lastTime) << std::endl;
        NumFrames = 0;
        LastTime1Sec = glfwGetTime();
    }
}

void Renderer::TickClock()
{
    assert(window != nullptr);
    // check for toggling time shaders
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bPauseDown)
    {
        bTickClock = !bTickClock;
        if (!bTickClock)
            std::cout << "Freezing clock @ " << glfwGetTime() << std::endl;
        else
            std::cout << "Resuming clock @ " << glfwGetTime() << std::endl;
        bPauseDown = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        bPauseDown = false;
    }

    /// TODO: make this continue from where it left off
    if (bTickClock)
        CurrentTime += glfwGetTime() - LastTime;
    LastTime = glfwGetTime();
}

void Renderer::TalkWithProgram(int program)
{
    // send data to the active shader program

    // send iTime
    glUniform1f(glGetUniformLocation(program, "iTime"), CurrentTime);

    // send iFrame
    glUniform1i(glGetUniformLocation(program, "iFrame"), NumFrames);

    // send iResolution
    glfwGetFramebufferSize(window, &WindowW, &WindowH);
    glViewport(0, 0, WindowW, WindowH);
    float ScreenSize[] = {static_cast<float>(WindowW), static_cast<float>(WindowH)};
    glUniform2fv(glGetUniformLocation(program, "iResolution"), 1, ScreenSize);

    // send iMouse
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        // only capture mouse pos when (left) pressed
        glfwGetCursorPos(window, &MouseX, &MouseY);
    }
    float mouse_pos_f[] = {static_cast<float>(MouseX), static_cast<float>(MouseY)};
    glUniform2fv(glGetUniformLocation(program, "iMouse"), 1, mouse_pos_f);

    // communicate foveated render params
    glUniform1i(glGetUniformLocation(program, "stride"), 32);
    const float diag = 0.5f * (WindowW + WindowH);
    const float thresh1 = 0.1f * diag;
    const float thresh2 = 0.25f * diag;
    const float thresh3 = 0.4f * diag;
    glUniform1f(glGetUniformLocation(program, "thresh1"), thresh1);
    glUniform1f(glGetUniformLocation(program, "thresh2"), thresh2);
    glUniform1f(glGetUniformLocation(program, "thresh3"), thresh3);
}

bool Renderer::Init()
{
    // Initialize the lib
    if (!glfwInit())
    {
        std::cerr << "could not start GLFW3" << std::endl;
        return false;
    }

    WindowW = Params.WindowParams.X0;
    WindowH = Params.WindowParams.Y0;

    if (!CreateWindow())
    {
        std::cerr << "Unable to create window!" << std::endl;
        glfwTerminate();
        return false;
    }

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl << std::endl;

    main_program = ShaderUtils::MainProgram{};
    bool status = main_program.loadShaders(Params);

    if (!status)
    {
        std::cerr << "can't load the shaders to initiate the main program" << std::endl;
        glfwTerminate();
        return false;
    }

    reconstruct_program = ShaderUtils::Program{};
    status = reconstruct_program.loadShaders({
        ShaderUtils::Shader(Params.MainParams.vertex_shader_path, "vertex", GL_VERTEX_SHADER),
        ShaderUtils::Shader(Params.FRParams.reconstruction_shader, "reconstruct", GL_FRAGMENT_SHADER),
    });

    if (!status)
    {
        std::cerr << "can't load the shaders to initiate the FR program" << std::endl;
        glfwTerminate();
        return false;
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
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    // create texture map for FBO
    glGenTextures(1, &Tex);
    glBindTexture(GL_TEXTURE_2D, Tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WindowW, WindowH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // link the texture map with the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Tex, 0);
    // check for problems
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "can't initialize FBO" << std::endl;
        glfwTerminate();
        return false;
    }
    // unbind to not accidentally render to it
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create vertex buffer object
    glGenBuffers(1, &VBO); // generate 1 vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CanvasVerts), CanvasVerts, GL_STATIC_DRAW);

    // create vertex attribute object
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    const int stride = 3;
    const void *offset = (void *)(0);
    const auto bNoramalize = GL_FALSE;
    glVertexAttribPointer(0, stride, GL_FLOAT, bNoramalize, stride * sizeof(float), offset);
    glEnableVertexAttribArray(0);

    // disable vsync
    bEnableVsync = Params.bEnableVsync;
    glfwSwapInterval(bEnableVsync);

    return true;
}

bool Renderer::Run()
{
    assert(window != nullptr);

    // get mouse pos
    glfwGetCursorPos(window, &MouseX, &MouseY);
    LastTime = glfwGetTime();
    int NumFrames = 0;
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
        TalkWithProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2 (3 vertex) triangles for rect

        // copy framebuffer (current rendered buffer) to FBO (& its texture)
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glBlitFramebuffer(0, 0, WindowW, WindowH, 0, 0, WindowW, WindowH, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        // Draw reconstruction shader

        glBindTexture(GL_TEXTURE_2D, Tex); // bind texture to current active texture

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // render on default framebuffer
        glUseProgram(reconstructProgram);
        TalkWithProgram(reconstructProgram);
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

        TickClock();

        /// TODO: use arrow keys to switch between shaders!!

        // display fps in title
        DisplayFps();

        // Swap front and back buffers
        glfwSwapBuffers(window);
    }

    return true;
}

bool Renderer::Exit()
{
    std::cout << std::endl << "Goodbye!" << std::endl;
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &FBO);
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return true;
}