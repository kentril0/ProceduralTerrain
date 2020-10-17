#include "pch.hpp"
#include "application.hpp"

#include <GLFW/glfw3.h>


// GLFW handler functions
void on_resize(GLFWwindow *window, int width, int height);

void on_mouse_move(GLFWwindow *window, double x, double y);

void on_mouse_pressed(GLFWwindow *window, int button, int action, int mods);

void on_key_pressed(GLFWwindow *window, int key, int scancode, int action, 
                    int mods);

// Callback function to be called when OpenGL emits an error
void APIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, 
                                    GLsizei length, const char *message,
                                    const void *user_parameter);

int main(void)
{
    const size_t initial_width = SCREEN_INIT_WIDTH;
    const size_t initial_height = SCREEN_INIT_HEIGHT;

    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Could not initialize GLFW!" << std::endl;
        return -1;
    }

    // Request OpenGL 4.5 Core context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    // Create window
    GLFWwindow *window = glfwCreateWindow(initial_width, initial_height, 
                                          PROJECT_NAME, NULL, NULL);
    if (!window)
    {
        std::cerr << "Could not create a window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make window's context current
    glfwMakeContextCurrent(window);

    // Load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Could not initialize OpenGL context!" << std::endl;
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_debug_callback, nullptr);

    {
        // Initialize our Application
        Application application(window, initial_width, initial_height);

        // Set window callbacks
        glfwSetWindowUserPointer(window, &application);
        glfwSetWindowSizeCallback(window, on_resize);
        glfwSetCursorPosCallback(window, on_mouse_move);
        glfwSetMouseButtonCallback(window, on_mouse_pressed);
        glfwSetKeyCallback(window, on_key_pressed);

        // Loop
        while (!glfwWindowShouldClose(window))
        {
            // Application main loop
            application.loop();

            // Swap front and back buffers
            glfwSwapBuffers(window);

            // Poll for and process events
            glfwPollEvents();
        }
    }   // Application is freed here, before GLFW

    // Free every allocated resource associated with GLFW
    glfwTerminate();

    return 0;
}

void on_resize(GLFWwindow *window, int width, int height)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_resize(window, width, height);
}

void on_mouse_move(GLFWwindow *window, double x, double y)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_mouse_move(window, x, y);
}

void on_mouse_pressed(GLFWwindow *window, int button, int action, int mods)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_mouse_pressed(window, button, action, mods);
}

void on_key_pressed(GLFWwindow *window, int key, int scancode, int action, 
                    int mods)
{
  Application *application = (Application *)glfwGetWindowUserPointer(window);
  application->on_key_pressed(window, key, scancode, action, mods);
}

void APIENTRY opengl_debug_callback(GLenum source, GLenum type, GLuint id, 
                                    GLenum severity, GLsizei length, 
                                    const char *message,
                                    const void *user_parameter)
{
  switch (type)
  {
      case GL_DEBUG_TYPE_ERROR:
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
          std::cerr << message << std::endl;
          return;
      default:
          return;
  }
}
