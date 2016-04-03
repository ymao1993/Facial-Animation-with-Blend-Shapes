//
//  Application.cpp
//  PDFA
//
//  Created by Yu Mao on 4/1/16.
//  Copyright © 2016 Carnegie Mellon University. All rights reserved.
//
#include <GL/glew.h>
#include "Application.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "obj/tiny_obj_loader.h"
#include "XRShaderUtils.hpp"
#include "SOIL2.h"

namespace Application
{
    /*const*/
    static const char* ObjFileName          = "/Users/MaoYu/Desktop/PDFA/PDFA/res/model/bunny/bunny.obj";
    static const char* textureFileName      = "/Users/MaoYu/Desktop/PDFA/PDFA/res/model/bunny/bunny-atlas.jpg";
    static const char* vertexShaderName     = "/Users/MaoYu/Desktop/PDFA/PDFA/res/shader/defaultShader.vs.glsl";
    static const char* fragmentShaderName   = "/Users/MaoYu/Desktop/PDFA/PDFA/res/shader/defaultShader.fs.glsl";
    static const float objScale = 0.005;
    
    /*application*/
    static GLFWwindow* window;
    static bool setWindowInst(GLFWwindow* window);
    static void render();
    static void loadResources();
    static void initData();
    
    /*data*/
    static GLfloat* positions;
    static GLfloat* texcoords;
    static int size_positions;
    static int size_texcoords;
    
    /*shader*/
    static GLuint program;
    static GLuint vao;
    static GLuint vbo_positions;
    static GLuint vbo_texcoords;
    static GLuint texture;
    static GLuint sampler;
    static void loadObj();
    static void loadTriangle();
    static void loadShader();
    static void initVBOs();
    static void initVAO();
    static void loadTexture();
    static void initSampler();
    
    /*camera*/
    static float camera_speed;
    static glm::vec3 camera_position;
    static glm::vec3 camera_forward;
    static glm::vec3 camera_up;
    static float camera_fov;
    static float camera_aspect;
    static float camera_zNear;
    static float camera_zFar;
    static void initCamera();
    static void updateCamera();
    static glm::mat4 getPerspective();
    static glm::mat4 getWorld2View();
    
    
    
    void appSetup(GLFWwindow* window)
    {
        std::cout << "Setting up application..." << std::endl;
        
        std::cout << "- Set window instance..." << std::endl;
        setWindowInst(window);
        
        std::cout << "- Load Resources" << std::endl;
        loadResources();
        
        std::cout << "- Initializing Data" << std::endl;
        initData();
        
        std::cout << "- Initialize Camera..." << std::endl;
        initCamera();
        
        std::cout << "Start rendering..." << std::endl;
    }
    
    void appLoop()
    {
        updateCamera();
        render();
    }
    
    void appDestroy()
    {
        free(positions);
        free(texcoords);
        glDeleteBuffers     (1, &vbo_positions);
        glDeleteBuffers     (1, &vbo_texcoords);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &texture);
    }
    
    /**
     * This function performs the rendering
     */
    static glm::mat4 rotate;
    void render()
    {
        //bind
        glUseProgram(program);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        //set uniforms
        GLuint m2vlocation =  glGetUniformLocation(program, "m2v");
        GLuint persplocation = glGetUniformLocation(program, "persp");
        glm::mat4 m2w = glm::scale(glm::mat4(), glm::vec3(objScale,objScale,objScale));
        rotate = glm::rotate(rotate, glm::radians(1.f), glm::vec3(0,1,0)); //rotate for fun
        m2w = m2w * rotate;
        glUniformMatrix4fv(m2vlocation, 1, GL_FALSE, glm::value_ptr(getWorld2View() * m2w));
        glUniformMatrix4fv(persplocation, 1, GL_FALSE, glm::value_ptr(getPerspective()));
        
        GLuint samplerlocation = glGetUniformLocation(program,"sampler");
        glUniform1i(samplerlocation, sampler);
        
        //drawcall
        glDrawArrays(GL_TRIANGLES, 0, size_positions/3);
        
        //unbind
        glUseProgram(0);
        glBindVertexArray(0);
    }
    
    /**
     * load resources including obj model file, texture file and shaders.
     */
    void loadResources()
    {
        loadObj();
        loadTexture();
        loadShader();
    }
    
    /**
     * initialize data needed by opengl, including buffers, uniforms and vao.
     */
    void initData()
    {
        initVBOs();
        initVAO();
        initSampler();
    }
    
    /* Load, compile and link the shader program*/
    void loadShader()
    {
        GLuint shaders[2];
        shaders[0] = XRShaderUtils::loadShader(vertexShaderName, GL_VERTEX_SHADER, true);
        shaders[1] = XRShaderUtils::loadShader(fragmentShaderName, GL_FRAGMENT_SHADER, true);
        program = XRShaderUtils::linkShaderProgram(shaders, 2, true);
    }
    
    /**
     * Load the OBJ model
     */
    void loadObj()
    {
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        
        //load obj file
        std::string err;
        bool ret = tinyobj::LoadObj(shapes, materials, err, ObjFileName);
        
        if (!err.empty()) { // `err` may contain warning message.
            std::cerr << err << std::endl;
        }
        
        if (!ret) {
            exit(1);
        }
        
        //initialize positions and texcoords array
        int numf = 0;
        for (size_t i = 0; i < shapes.size(); i++)
        {
            numf += shapes[i].mesh.indices.size()/3;
        }
        positions = (float*) malloc(sizeof(GLfloat) * numf * 9);
        texcoords = (float*) malloc(sizeof(GLfloat) * numf * 6);
        size_positions = numf * 9;
        size_texcoords = numf * 6;
        
        
        //fill the positions and texcoords array
        int count1 = 0;
        int count2 = 0;
        for (size_t i = 0; i < shapes.size(); i++) {
            
            //make sure the data is good
            assert((shapes[i].mesh.indices.size()   % 3) == 0);
            assert((shapes[i].mesh.positions.size() % 3) == 0);
            assert((shapes[i].mesh.texcoords.size() % 2) == 0);
            
            //for all the triangles
            for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
                
                //for all the 3 vertices
                for (int viter = 0; viter < 3; viter ++)
                {
                    int vidx = shapes[i].mesh.indices[3*f+viter];
                    
                    positions[count1++] = shapes[i].mesh.positions[vidx*3 + 0];
                    positions[count1++] = shapes[i].mesh.positions[vidx*3 + 1];
                    positions[count1++] = shapes[i].mesh.positions[vidx*3 + 2];
                    
                    texcoords[count2++] = shapes[i].mesh.texcoords[vidx*2 + 0];
                    texcoords[count2++] = shapes[i].mesh.texcoords[vidx*2 + 1];
                }
            }
        }
    }
    
    
    /**
     * Initialize all the Vertex Buffer Objects
     */
    void initVBOs()
    {
        {
            //create the buffer object
            glGenBuffers(1, &vbo_positions);
            //bind the buffer object to a binding targets for configuration
            glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
            //allocate memory for the buffer object bound to a binding target
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * size_positions, positions, GL_STATIC_DRAW);
        }
        {
            //create the buffer object
            glGenBuffers(1, &vbo_texcoords);
            //bind the buffer object to a binding targets for configuration
            glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords);
            //allocate memory for the buffer object bound to a binding target
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*size_texcoords, texcoords, GL_STATIC_DRAW);
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    /**
     * Initialize Vertex Array Object, which represents the inputs to the
     * vertex shader.
     */
    void initVAO()
    {
        //initialize and bind the vao to the pipeline
        glGenVertexArrays(1,&vao);
        glBindVertexArray(vao);
        
        //set up positions' attribute bindings
        GLint posAttrib = glGetAttribLocation(program, "vs_position");
        glEnableVertexAttribArray(posAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_positions);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
        
        //set up texcoords' attribute bindings
        GLint tcAttrib = glGetAttribLocation(program, "vs_texcoord");
        glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords);
        glVertexAttribPointer(tcAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(tcAttrib);
        
        //unbind vao
        glBindVertexArray(0);
    }
    
    /**
     * Load Texture
     */
    void loadTexture()
    {
        /* load an image file directly as a new OpenGL texture */
        texture = SOIL_load_OGL_texture(
         textureFileName,
         SOIL_LOAD_AUTO,
         SOIL_CREATE_NEW_ID,
         SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
        
        if(texture == 0)
        {
            std::cout << "texture loading failed!" << std::endl;
        }
        
        glBindTexture(GL_TEXTURE_2D, 0);
        
        return;
    }
    
    /**
     * Initialize Sampler
     */
    static void initSampler()
    {
        glGenSamplers(1,&sampler);
        glBindSampler(texture, sampler);
    }
    
    /**
     * Initialize Camera's configuration paramters
     */
    void initCamera()
    {
        camera_position = glm::vec3(0,0,5);
        camera_forward  = glm::vec3(0,0,-1);
        camera_up       = glm::vec3(0,1,0);
        camera_speed = 0.05f;
        camera_fov = glm::radians(45.f);
        camera_aspect = APPLICATION_WHEIGHT/APPLICATION_WHEIGHT;
        camera_zNear = 0.01f;
        camera_zFar = 100.f;
    }
    
    /**
     * Update Camera
     */
    void updateCamera()
    {
        bool w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        bool s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        bool a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        bool d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        
        if(w) camera_position += camera_forward * camera_speed;
        if(s) camera_position -= camera_forward * camera_speed;
        if(a) camera_position += glm::normalize(glm::cross(camera_up, camera_forward)) * camera_speed;
        if(d) camera_position -= glm::normalize(glm::cross(camera_up, camera_forward)) * camera_speed;
    }
    
    /**
     * Get perspective projection matrix
     */
    static glm::mat4 getPerspective()
    {
        return glm::perspective(camera_fov, camera_aspect, camera_zNear, camera_zFar);
    }
    
    static glm::mat4 getWorld2View()
    {
        return glm::lookAt(camera_position, camera_position + camera_forward, camera_up);
    }
    
    bool setWindowInst(GLFWwindow* window)
    {
        if(window)
        {
            Application::window = window;
            return true;
        }
        else
        {
            std::cout <<"The pointer to window cannot be NULL"<<std::endl;
            return false;
        }
    }
}

