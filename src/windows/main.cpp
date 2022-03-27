#include "../common/test_app.h"
#include "../common/logger.h"
#include "../common/gl.h"

const int WIDTH = 1920;
const int HEIGHT = 1080;
TestApp* g_TestApp;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    g_TestApp->Resize(width, height);
}

void MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
}

void MouseClickCallback(GLFWwindow* window, int button, int action, int mods)
{
}

void ProcessInput(GLFWwindow* window)
{
}


int SetupOpenGl(GLFWwindow*& window)
{
    // GLFW Init & Config
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW Window create
    window = glfwCreateWindow(WIDTH, HEIGHT, "Particle System - Performance Test", NULL, NULL);
    if (window == nullptr)
    {
        LOGE("WIN_MAIN", "Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseClickCallback);

    // Capture mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    if (SetupOpenGl(window) != 0)
    {
        return -1;
    }
    g_TestApp = new TestApp();
    g_TestApp->Init();

    while (!glfwWindowShouldClose(window))
    {
        g_TestApp->Step();
    }

    delete g_TestApp;
    return 0;
}