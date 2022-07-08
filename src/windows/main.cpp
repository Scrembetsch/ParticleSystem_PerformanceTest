#include "../common/test_app.h"
#include "../common/logger.h"
#include "../common/gl/gl.h"
#include "../common/device/file_handler.h"

#define SMALL_WINDOW 0

#if SMALL_WINDOW
const int WIDTH = 1280;
const int HEIGHT = 720;
#else
const int WIDTH = 2560;
const int HEIGHT = 1440;
#endif
TestApp* g_TestApp = nullptr;

bool g_FirstMouse = true;
float g_LastX = 0.0f;
float g_LastY = 0.0f;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    g_TestApp->Resize(width, height);
}

void MouseCallback(GLFWwindow* window, double mouseXPos, double mouseYPospos)
{
    float xpos = static_cast<float>(mouseXPos);
    float ypos = static_cast<float>(mouseYPospos);
    if (g_FirstMouse)
    {
        g_LastX = xpos;
        g_LastY = ypos;
        g_FirstMouse = false;
    }

    float xoffset = xpos - g_LastX;
    float yoffset = g_LastY - ypos; // Reverse: y-coordinates go from bottom to top

    g_LastX = xpos;
    g_LastY = ypos;

    if (g_TestApp != nullptr)
    {
        g_TestApp->ProcessLookInput(xoffset, yoffset);
    }
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_TestApp->Zoom(static_cast<float>(-yoffset));
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void MouseClickCallback(GLFWwindow* window, int button, int action, int mods)
{
    // Currently Unused
}

void ProcessInput(GLFWwindow* window)
{
    // Currently Unused
}


int SetupOpenGl(GLFWwindow** window)
{
    // GLFW Init & Config
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSwapInterval(0);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW Window create
    *window = glfwCreateWindow(WIDTH, HEIGHT, "Particle System - Performance Test", NULL, NULL);
    if (*window == nullptr)
    {
        LOGE("WIN_MAIN", "Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(*window, MouseCallback);
    glfwSetScrollCallback(*window, ScrollCallback);
    glfwSetKeyCallback(*window, KeyCallback);
    glfwSetMouseButtonCallback(*window, MouseClickCallback);

    // Capture mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // GLAD: Load all OpenGL Function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOGE("WIN_MAIN", "Failed to initialize GLAD");
        return -1;
    }
    return 0;
}

int main(int argc, char** argv)
{
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = nullptr;
    if (SetupOpenGl(&window) != 0)
    {
        return -1;
    }
    if (argc > 1)
    {
        FileHandler::Instance()->SetBasePath("../Android/app/src/main/assets/");
    }
    else
    {
        FileHandler::Instance()->SetBasePath("../../../Android/app/src/main/assets/");
    }

    g_TestApp = new TestApp();
    if (!g_TestApp->Init())
    {
        glfwSetWindowShouldClose(window, true);
    }
    g_TestApp->Resize(WIDTH, HEIGHT);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        g_TestApp->Step();
        glfwSwapBuffers(window);

        if (g_TestApp->ShouldClose())
        {
            glfwSetWindowShouldClose(window, true);
        }
    }

    delete g_TestApp;
    glfwTerminate();
    return 0;
}