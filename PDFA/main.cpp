//
//  main.cpp
//  PDFA
//
//  Created by Yu Mao on 4/1/16.
//  Copyright © 2016 Carnegie Mellon University. All rights reserved.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>

int main(void)
{
    GLFWwindow* window;
    
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "PDFA", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    /*initialize glew*/
    if(!glewInit())
    {
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    glfwTerminate();
    return 0;
}