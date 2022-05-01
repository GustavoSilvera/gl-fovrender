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
    WindowW = Params.WindowParams.X0;
    WindowH = Params.WindowParams.Y0;

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

void Renderer::WindowCallbacks()
{
    // callback on framebuffer/window size
    glfwGetFramebufferSize(window, &WindowW, &WindowH);
    if (WindowW != LastWindowW || WindowH != LastWindowH)
    {
        std::cout << "Detected FB size change from "
                  << "(" << LastWindowW << " x " << LastWindowH << ") to (" << WindowW << " x " << WindowH << ")"
                  << std::endl;
#ifdef __APPLE__
        /// HACK: special case for MacOS hiDPI monitors which exactly 2x or 1/2x the dpi between hi and lo dpi displays
        int ExpectedWindowW, ExpectedWindowH;
        glfwGetWindowSize(window, &ExpectedWindowW, &ExpectedWindowH);
        if (ExpectedWindowH == WindowH / 2 && ExpectedWindowW == WindowW / 2)
        {
            std::cout << "Detected hiDPI display" << std::endl;
            bIsHiDPI = true;
        }
        else
            bIsHiDPI = false;

#endif
        LastWindowW = WindowW;
        LastWindowH = WindowH;

        glViewport(0, 0, WindowW, WindowH);

        if (Params.bEnablePostProcessing)
        {
            // glDeleteBuffers(1, &FBO);
            glDeleteTextures(1, &Tex);

            // generate new FB texture for postprocessing
            GenerateFBO();
        }
    }

    // callback on Mouse coordinates
    glfwGetCursorPos(window, &MouseX, &MouseY);
    if (bIsHiDPI)
    {
        MouseX *= 2;
        MouseY *= 2;
    }
}

bool Renderer::GenerateFBO()
{
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
    return true;
}

void Renderer::DisplayFps()
{
    assert(window != nullptr);
    const double DeltaT = glfwGetTime() - LastTimeFps;
    NumFrames++;
    if (DeltaT > 1.f) // more than a second ago
    {
        const double Fps = double(NumFrames) / DeltaT;
        std::stringstream ss;
        ss << "[" << Fps << " FPS]";
        glfwSetWindowTitle(window, ss.str().c_str());
        NumFrames = 0;
        LastTimeFps = glfwGetTime();
    }
}

void Renderer::CheckInputs()
{
    // check for closing window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    // check for reloading shaders
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !bReloadPressed)
    {
        std::cout << "Reloading..." << std::endl;
        bReloadPressed = true;
        Params.ParseFile();  // reload global params
        Main.Reload(Params); // reload main param & shaders
        PostProc.Reload();   // reload postprocessing shaders
    }
    else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE)
    {
        bReloadPressed = false;
    }

    // previous shader
    const bool bPressPrev =
        (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS);
    const bool bReleasePrev =
        (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE);
    if (bPressPrev && !bPrevPressed)
    {
        std::cout << "Previous shader..." << std::endl;
        bPrevPressed = true;
        Params.ParseFile();      // reload global params
        Main.PrevShader(Params); // previous main param & shaders
        PostProc.Reload();       // reload postprocessing shaders
    }
    else if (bReleasePrev)
    {
        bPrevPressed = false;
    }

    // next shader
    const bool bPressNext =
        (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS);
    const bool bReleaseNext =
        (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE);
    if (bPressNext && !bNextPressed)
    {
        std::cout << "Right shader..." << std::endl;
        bNextPressed = true;
        Params.ParseFile();      // reload global params
        Main.NextShader(Params); // right main param & shaders
        PostProc.Reload();       // reload postprocessing shaders
    }
    else if (bReleaseNext)
    {
        bNextPressed = false;
    }

    // double the stride size
    const bool bPressUp =
        (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS);
    const bool bReleaseUp =
        (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE);
    if (bPressUp && !bUpPressed)
    {
        int old = Params.FRParams.stride;
        int current = std::min(Params.FRParams.stride * 2, 256);
        Params.FRParams.stride = current;
        std::cout << "Increasing block size from " << old << " to " << current << std::endl;
        bUpPressed = true;
    }
    else if (bReleaseUp)
    {
        bUpPressed = false;
    }

    // half the stride size
    const bool bPressDown =
        (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS);
    const bool bReleaseDown =
        (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE);
    if (bPressDown && !bDownPressed)
    {
        int old = Params.FRParams.stride;
        int current = std::max(Params.FRParams.stride / 2, 2);
        Params.FRParams.stride = current;
        std::cout << "Decreasing block size from " << old << " to " << current << std::endl;
        bDownPressed = true;
    }
    else if (bReleaseDown)
    {
        bDownPressed = false;
    }

    // toggle postprocessing (reconstruction) shader
    const bool bPressPPToggle =
        (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS);
    const bool bReleasePPToggle =
        (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE);
    if (bPressPPToggle && !bPPTogglePressed)
    {
        std::cout << "Toggling postprocessing shader..." << std::endl;
        Params.bEnablePostProcessing = !Params.bEnablePostProcessing;
        bPPTogglePressed = true;
    }
    else if (bReleasePPToggle)
    {
        bPPTogglePressed = false;
    }
}

void Renderer::TickClock()
{
    assert(window != nullptr);
    // check for toggling time shaders
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bPausePressed)
    {
        bTickClock = !bTickClock;
        if (!bTickClock)
            std::cout << "Freezing clock @ " << glfwGetTime() << std::endl;
        else
            std::cout << "Resuming clock @ " << glfwGetTime() << std::endl;
        bPausePressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        bPausePressed = false;
    }

    if (bTickClock)
        CurrentTime += glfwGetTime() - LastTime; // using deltas to continue where left off
    LastTime = glfwGetTime();
}

void Renderer::TalkWithProgram(int ProgramIdx)
{
    // send data to the active shader program

    // send iTime
    glUniform1f(glGetUniformLocation(ProgramIdx, "iTime"), CurrentTime);

    // send iFrame
    glUniform1i(glGetUniformLocation(ProgramIdx, "iFrame"), NumFrames);

    // send iResolution
    float ScreenSize[] = {static_cast<float>(WindowW), static_cast<float>(WindowH)};
    glUniform2fv(glGetUniformLocation(ProgramIdx, "iResolution"), 1, ScreenSize);

    // send iMouse
    float mouse_pos_f[] = {static_cast<float>(MouseX), static_cast<float>(MouseY)};
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        // only capture mouse pos when (left) pressed
        glUniform2fv(glGetUniformLocation(ProgramIdx, "iMouse"), 1, mouse_pos_f);
    }
    // pass in to "Mouse" regardless of click+drag (only need move)
    glUniform2fv(glGetUniformLocation(ProgramIdx, "Mouse"), 1, mouse_pos_f);

    // communicate foveated render params
    glUniform1i(glGetUniformLocation(ProgramIdx, "stride"), Params.FRParams.stride);
    const float diag = 0.5f * (WindowW + WindowH);
    assert(Params.FRParams.thresh1 < Params.FRParams.thresh2 && Params.FRParams.thresh2 < Params.FRParams.thresh3);
    const float thresh1 = Params.FRParams.thresh1 * diag;
    const float thresh2 = Params.FRParams.thresh2 * diag;
    const float thresh3 = Params.FRParams.thresh3 * diag;
    glUniform1f(glGetUniformLocation(ProgramIdx, "thresh1"), thresh1);
    glUniform1f(glGetUniformLocation(ProgramIdx, "thresh2"), thresh2);
    glUniform1f(glGetUniformLocation(ProgramIdx, "thresh3"), thresh3);
}

bool Renderer::Init()
{
    // Initialize the lib
    if (!glfwInit())
    {
        std::cerr << "could not start GLFW3" << std::endl;
        return false;
    }

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

    Main = ShaderUtils::MainProgram{};
    bool status = Main.loadShaders(Params);

    if (!status)
    {
        std::cerr << "can't load the shaders to initiate the main program" << std::endl;
        glfwTerminate();
        return false;
    }

    PostProc = ShaderUtils::Program{};
    status = PostProc.loadShaders({
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
    if (!GenerateFBO())
        return false;
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

void Renderer::RenderPass()
{
    int MainProgram = Main.GetProgram();

    // first, render to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear canvas
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw main shader
    glUseProgram(MainProgram);
    TalkWithProgram(MainProgram);
    glBindVertexArray(VAO);

    // peform the drawing
    glDrawArrays(GL_TRIANGLES, 0, 6); // 2 (3 vertex) triangles for rect
}

void Renderer::PostprocessingPass()
{
    if (Params.bEnablePostProcessing)
    {
        int ReconstructionProgram = PostProc.GetProgram();
        // copy framebuffer (current rendered buffer) to FBO (& its texture)
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
        glBlitFramebuffer(0, 0, WindowW, WindowH, 0, 0, WindowW, WindowH, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, Tex); // bind texture to current active texture

        glBindFramebuffer(GL_FRAMEBUFFER, 0); // render on default framebuffer
        glUseProgram(ReconstructionProgram);
        TalkWithProgram(ReconstructionProgram);
        glBindVertexArray(VAO);
        // peform the drawing
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2 (3 vertex) triangles for rect
    }
}

bool Renderer::Run()
{
    assert(window != nullptr);
    while (!glfwWindowShouldClose(window))
    {
        WindowCallbacks(); // check for frame buffer size change

        RenderPass(); // perform main draw pass

        PostprocessingPass(); // perform postprocessing effects

        glfwPollEvents(); // Poll for and process events

        CheckInputs(); // check for miscellaneous input actions

        TickClock(); // tick forward (unless paused) the internal clock

        DisplayFps(); // display fps in title

        glfwSwapBuffers(window); // Swap front and back buffers
    }

    return true;
}

bool Renderer::Exit()
{
    std::cout << std::endl << "Goodbye!" << std::endl;
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &FBO);
    glDeleteTextures(1, &Tex);
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return true;
}