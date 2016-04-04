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
#include <string>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <SOIL.h>
#include "XRShaderUtils.hpp"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw_gl3.h"

#define NUM_BLENDSHAPE 14

namespace Application
{
    /*const*/
    static const char* ObjFileName          = "res/model/head/FaceDefault.obj";
    static const char* textureFileName      = "res/model/head/headTexture.jpg";
    static const char* vertexShaderName     = "res/shader/defaultShader.vs.glsl";
    static const char* fragmentShaderName   = "res/shader/defaultShader.fs.glsl";
    static const char* blendShapesFileNames[NUM_BLENDSHAPE] =
    {
        "res/model/head/FaceCheekPuff.obj",
        "res/model/head/FaceCheekSuck.obj",
        "res/model/head/FaceJawOpen.obj",
        "res/model/head/FaceKiss.obj",
        "res/model/head/FaceLeftBrowDown.obj",
        "res/model/head/FaceLeftBrowUp.obj",
        "res/model/head/FaceLeftFrown.obj",
        "res/model/head/FaceLeftSmile.obj",
        "res/model/head/FaceLeftSneer.obj",
        "res/model/head/FaceRightBrowDown.obj",
        "res/model/head/FaceRightBrowUp.obj",
        "res/model/head/FaceRightFrown.obj",
        "res/model/head/FaceRightSmile.obj",
        "res/model/head/FaceRightSneer.obj"
    };
    
    
    static const float objScale = 0.25;
    
    /*application*/
    static GLFWwindow* window;
    static bool setWindowInst(GLFWwindow* window);
    static void render();
    static void loadResources();

	/*Graphics User Interface*/
	static void initGUI();
	static void renderGUI();
	static void shutdownGUI();
    
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
    static void loadObj();
    static void loadShader();
	static void initData();
    static void initVBOs();
    static void initVAO();
    static void loadTexture();
    static void initBlendShapes();
    
    /*blend shapes*/
	static float weights[NUM_BLENDSHAPE] = { 0 };
	static GLuint vbo_bs_positions[NUM_BLENDSHAPE];
    
    
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
        
        std::cout << "- Preparing Blendshapes" << std::endl;
        initBlendShapes();
        
        std::cout << "- Initialize Camera..." << std::endl;
        initCamera();

		std::cout << "- Initialize GUI..." << std::endl;
		initGUI();
        
        std::cout << "Start rendering..." << std::endl;
    }
    
    void appLoop()
    {
        updateCamera();
        render();
		renderGUI();
    }
    
    void appDestroy()
    {
		shutdownGUI();

        free(positions);
        free(texcoords);
        glDeleteBuffers     (1, &vbo_positions);
        glDeleteBuffers     (1, &vbo_texcoords);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures	(1, &texture);
		glDeleteBuffers(NUM_BLENDSHAPE, vbo_bs_positions);
    }
    
    /**
     * This function performs the rendering
     */
    static glm::mat4 rotate;
    void render()
    {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        //bind
        glUseProgram(program);
        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        //set uniforms - for transformation
        GLuint m2vlocation =  glGetUniformLocation(program, "m2v");
        GLuint persplocation = glGetUniformLocation(program, "persp");
        glm::mat4 m2w = glm::scale(glm::mat4(), glm::vec3(objScale,objScale,objScale));
        //rotate = glm::rotate(rotate, glm::radians(0.2f), glm::vec3(0,1,0)); //rotate for fun
        m2w = m2w * rotate;
        glUniformMatrix4fv(m2vlocation, 1, GL_FALSE, glm::value_ptr(getWorld2View() * m2w));
        glUniformMatrix4fv(persplocation, 1, GL_FALSE, glm::value_ptr(getPerspective()));
        
        //set uniforms - for blending shapes
        GLuint weightsLocation = glGetUniformLocation(program,"weights");
        glUniform1fv(weightsLocation, NUM_BLENDSHAPE, weights);
        
        //drawcall
        glDrawArrays(GL_TRIANGLES, 0, size_positions/3);
        
        //unbind
        glUseProgram(0);
        glBindVertexArray(0);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
		{
			GLuint location = glGetAttribLocation(program, "vs_position");
			glVertexAttribBinding(location, location);
			glBindVertexBuffer(location, vbo_positions, 0, sizeof(GLfloat)* 3);
			glVertexAttribFormat(location, 3, GL_FLOAT, GL_FALSE, 0);
			glEnableVertexAttribArray(location);
		}
        
        //set up texcoords' attribute bindings
		{
			GLuint location = glGetAttribLocation(program, "vs_texcoord");
			glVertexAttribBinding(location, location);
			glBindVertexBuffer(location, vbo_texcoords, 0, sizeof(GLfloat)* 2);
			glVertexAttribFormat(location, 2, GL_FLOAT, GL_FALSE, 0);
			glEnableVertexAttribArray(location);
		} 

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
     * Load blendshapes obj from files, compute the differneces with neutral expression,
     * and stack the data into a texture buffer.
     */
    void initBlendShapes()
    {
        //Read positions data from blendshape files
        std::vector<std::vector<float>> blendshape_positions(NUM_BLENDSHAPE);
        for(int i=0; i<NUM_BLENDSHAPE; i++)
        {
            const char* ObjFileName = blendShapesFileNames[i];
            
            std::cout << "reading " << ObjFileName << std::endl;
            
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
            
            //initialize positions vector array
            int numf = 0;
            for (size_t i = 0; i < shapes.size(); i++)
            {
                numf += shapes[i].mesh.indices.size()/3;
            }
            blendshape_positions[i].reserve(numf * 9);
            
            
            //fill the positions array
            for (size_t j = 0; j < shapes.size(); j++) {
                
                //make sure the data is good
                assert((shapes[j].mesh.indices.size()   % 3) == 0);
                assert((shapes[j].mesh.positions.size() % 3) == 0);
                assert((shapes[j].mesh.texcoords.size() % 2) == 0);
                
                //for all the triangles
                for (size_t f = 0; f < shapes[j].mesh.indices.size() / 3; f++) {
                    
                    //for all the 3 vertices
                    for (int viter = 0; viter < 3; viter ++)
                    {
                        int vidx = shapes[j].mesh.indices[3*f+viter];
                        blendshape_positions[i].push_back(shapes[j].mesh.positions[vidx*3 + 0]);
                        blendshape_positions[i].push_back(shapes[j].mesh.positions[vidx*3 + 1]);
                        blendshape_positions[i].push_back(shapes[j].mesh.positions[vidx*3 + 2]);
                    }
                }
            }
        }
        
		//compute the different vector
		for (int j = 0; j< size_positions; j++)
		{
			for (int i = 0; i<NUM_BLENDSHAPE; i++)
			{
				blendshape_positions[i][j] -= positions[j];
			}
		}

		//set up vertex buffer objects
		glBindVertexArray(vao);
		glGenBuffers(NUM_BLENDSHAPE, vbo_bs_positions);
		for (int i = 0; i < NUM_BLENDSHAPE; i++)
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo_bs_positions[i]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * blendshape_positions[i].size(), &blendshape_positions[i][0], GL_STATIC_DRAW);
			char attriName[10];
			sprintf(attriName, "pos%d", i);
			GLuint location = glGetAttribLocation(program, attriName);
			glVertexAttribBinding(location, location);
			glBindVertexBuffer(location, vbo_bs_positions[i], 0, sizeof(GLfloat)* 3);
			glVertexAttribFormat(location, 3, GL_FLOAT, GL_FALSE, 0);
			glEnableVertexAttribArray(location);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
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

	void initGUI()
	{
		ImGui_ImplGlfwGL3_Init(window, true);
	}

	void renderGUI()
	{

		/*
		
		"res/model/head/FaceCheekPuff.obj",
		"res/model/head/FaceCheekSuck.obj",
		"res/model/head/FaceJawOpen.obj",
		"res/model/head/FaceKiss.obj",
		"res/model/head/FaceLeftBrowDown.obj",
		"res/model/head/FaceLeftBrowUp.obj",
		"res/model/head/FaceLeftFrown.obj",
		"res/model/head/FaceLeftSmile.obj",
		"res/model/head/FaceLeftSneer.obj",
		"res/model/head/FaceRightBrowDown.obj",
		"res/model/head/FaceRightBrowUp.obj",
		"res/model/head/FaceRightFrown.obj",
		"res/model/head/FaceRightSmile.obj",
		"res/model/head/FaceRightSneer.obj"

		
		
		*/


		ImGui_ImplGlfwGL3_NewFrame();
		{
			static float f = 0.0f;
			ImGui::Text("Facial Blending Shapes");
			ImGui::SliderFloat("CheekPuff", &weights[0], 0.0f, 1.0f);
			ImGui::SliderFloat("CheekSuck", &weights[1], 0.0f, 1.0f);
			ImGui::SliderFloat("JawOpen", &weights[2], 0.0f, 1.0f);
			ImGui::SliderFloat("Kiss", &weights[3], 0.0f, 1.0f);
			ImGui::SliderFloat("LeftBrowDown", &weights[4], 0.0f, 1.0f);
			ImGui::SliderFloat("LeftBrowUp", &weights[5], 0.0f, 1.0f);
			ImGui::SliderFloat("LeftFrown", &weights[6], 0.0f, 1.0f);
			ImGui::SliderFloat("LeftSmile", &weights[7], 0.0f, 1.0f);
			ImGui::SliderFloat("LeftSneer", &weights[8], 0.0f, 1.0f);
			ImGui::SliderFloat("RightBrowDown", &weights[9], 0.0f, 1.0f);
			ImGui::SliderFloat("RightBrowUp", &weights[10], 0.0f, 1.0f);
			ImGui::SliderFloat("RightFrown", &weights[11], 0.0f, 1.0f);
			ImGui::SliderFloat("RightSmile", &weights[12], 0.0f, 1.0f);
			ImGui::SliderFloat("RightSneer", &weights[13], 0.0f, 1.0f);
		}

		ImGui::Render();
	}

	void shutdownGUI()
	{
		ImGui_ImplGlfwGL3_Shutdown();
	}
}

