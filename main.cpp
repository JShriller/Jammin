#include <GLFW/glfw3.h>
#include <iostream>

// This functions is for resizing things related to the winow changing 

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Match the drawing canvas size
    glViewport(0, 0, width, height);
}


int main() {
    // Initialize the GLFW window-management library 
    if(!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl; 
        return -1;
    }

    // Configure the window settings for specific targeting verisons
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //2. Opens the destop window
    GLFWwindow* window = glfwCreateWindow(800, 600, "C++ Engine Window", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Tells GLFW to link all operations to this window
    glfwMakeContextCurrent(window); 

    // continous rendering loop
    // Runs unitl user clicke the close button 
    while (!glfwWindowShouldClose(window)) {

        // If the user pressed Escape to close the window 
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // Rendering Commands: Clear the screen witha specific background color
        // The color is a dark gray color using hex code
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT); 

        // SWAP BUFFERS: OpenGL draws items to an invisible back-buffer to prevent flickering. 
        // This command flips that hidden image forward onto the visible desktop display.
        glfwSwapBuffers(window);

        // POLL EVENTS: Check if the user clicked any window controls, dragged their mouse, or pressed keys
        glfwPollEvents();
    }

    // terminating after while loop ends
    glfwTerminate(); 
    return 0; 
}