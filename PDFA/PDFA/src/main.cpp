//
//  main.cpp
//  PDFA
//
//  Created by Yu Mao on 4/1/16.
//  Copyright © 2016 Carnegie Mellon University. All rights reserved.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Application.hpp"


int main(void)
{
    GLFWwindow* window;
    
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(APPLICATION_WWIDTH, APPLICATION_WHEIGHT, "PDFA", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    
    
    /*initialize glew*/
    glewExperimental=GL_TRUE;
    GLenum err=glewInit();
    if(err!=GLEW_OK)
    {
        std::cout<<"glewInit failed, aborting."<<std::endl;
        return -1;
    }
    
    Application::appSetup(window);
    
    glEnable(GL_DEPTH_TEST);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        
        /*Clear Color&Depth Buffer*/
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        /* Render here */
        Application::appLoop();
        
        /* Swap front and back buffers */
        glfwSwapBuffers(window);
        
        /* Poll for and process events */
        glfwPollEvents();
    }
    
    Application::appDestroy();
    
    glfwTerminate();
    return 0;
}
