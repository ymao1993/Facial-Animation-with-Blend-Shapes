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

#define NUM_BLENDSHAPE 6

namespace Application
{
#pragma region variables

    /*constants*/
    static const char* ObjFileName          = "res/model/humanHead/head-reference.obj";
    static const char* textureFileName      = "res/model/humanHead/headTexture.jpg";
    static const char* vertexShaderName     = "res/shader/defaultShader.vs.glsl";
    static const char* fragmentShaderName   = "res/shader/defaultShader.fs.glsl";
    static const char* blendShapesFileNames[NUM_BLENDSHAPE] =
    {
        "res/model/humanHead/head-01-anger.obj",
        "res/model/humanHead/head-02-cry.obj",
        "res/model/humanHead/head-03-fury.obj",
        "res/model/humanHead/head-04-grin.obj",
        "res/model/humanHead/head-05-laugh.obj",
        "res/model/humanHead/head-06-rage.obj",
        //"res/model/humanHead/head-07-sad.obj",
        //"res/model/humanHead/head-08-smile.obj",
        //"res/model/humanHead/head-09-surprise.obj",
    };
    static const float objScale = 0.12f;
    
    /*application*/
    static GLFWwindow* window = NULL;
    static void render();
    static void loadResources();
	static bool GUIready = false;

	/*Graphics User Interface*/
	static void initGUI();
	static void renderGUI();
	static void shutdownGUI();
    
    /*data*/
	static GLfloat* positions = NULL;
	static GLfloat* texcoords = NULL;
	static GLfloat* normals   = NULL;
    static int size_positions = 0;
	static int size_normals = 0;
    static int size_texcoords = 0;
	bool hasTC = true; //has texture coordinates?
    
    /*shader*/
    static GLuint program = 0;
	static GLuint vao = 0;
	static GLuint vbo_positions = 0;
	static GLuint vbo_normals = 0;
	static GLuint vbo_texcoords = 0;
	static GLuint texture = 0;
    static void loadObj();
    static void loadShader();
	static void initData();
    static void initVBOs();
    static void initVAO();
    static void loadTexture();
    static void initBlendShapes();

	/*lighting*/
	static glm::vec3 lightDir(-0.57735, -0.57735, -0.57735);
	static float ambientf  = 0.3f;
	static float diffusef  = 0.4f;
	static float specularf = 0.8f;
	static float shininessf = 30.f;
    
    /*blend shapes*/
	static float weights[NUM_BLENDSHAPE] = { 0 };
	static GLuint vbo_bs_positions[NUM_BLENDSHAPE];
	static GLuint vbo_bs_normals[NUM_BLENDSHAPE];
    
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

#pragma endregion
    
#pragma region Application Logic
    void appSetup()
    {
        std::cout << "Setting up application..." << std::endl;
        
        std::cout << "- Load Resources" << std::endl;
        loadResources();
        
        std::cout << "- Initializing Data" << std::endl;
        initData();
        
        std::cout << "- Preparing Blendshapes" << std::endl;
        initBlendShapes();
        
        std::cout << "- Initialize Camera..." << std::endl;
        initCamera();
        
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

		if (positions != NULL) { free(positions); }
		if (texcoords != NULL) { free(texcoords); }
		if (normals != NULL)   { free(normals); }
        glDeleteBuffers     (1, &vbo_positions);
        glDeleteBuffers     (1, &vbo_texcoords);
		glDeleteBuffers		(1, &vbo_normals);
        glDeleteTextures	(1, &texture);
		glDeleteBuffers(NUM_BLENDSHAPE, vbo_bs_positions);
		glDeleteVertexArrays(1, &vao);
    }

#pragma endregion

#pragma region subRoutines
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
        
        //set uniforms - for transformation
		GLuint m2wlocation = glGetUniformLocation(program, "m2w");
		glm::mat4 m2w = glm::scale(glm::mat4(), glm::vec3(objScale, objScale, objScale));
		//rotate = glm::rotate(rotate, glm::radians(0.2f), glm::vec3(0,1,0)); //rotate for fun
		m2w = m2w * rotate;
		glUniformMatrix4fv(m2wlocation, 1, GL_FALSE, glm::value_ptr(m2w));
        GLuint w2vlocation =  glGetUniformLocation(program, "w2v");
		glUniformMatrix4fv(w2vlocation, 1, GL_FALSE, glm::value_ptr(getWorld2View()));
        GLuint persplocation = glGetUniformLocation(program, "persp");
        glUniformMatrix4fv(persplocation, 1, GL_FALSE, glm::value_ptr(getPerspective()));
        
        //set uniforms - for blending shapes
        GLuint weightsLocation = glGetUniformLocation(program,"weights");
        glUniform1fv(weightsLocation, NUM_BLENDSHAPE, weights);

		//set uniforms - for texture
		GLuint hasTextureLocation = glGetUniformLocation(program, "hasTexture");
		glUniform1d(hasTextureLocation, hasTC);

		//set uniforms - for lighting
		GLuint lightlocation = glGetUniformLocation(program, "light");
		glUniform3f(lightlocation, lightDir.x, lightDir.y, lightDir.z);
		GLuint eyeposlocation = glGetUniformLocation(program, "eyepos");
		glUniform3f(eyeposlocation, camera_position.x, camera_position.y, camera_position.z);
		GLuint ambientlocation = glGetUniformLocation(program, "ambient");
		glUniform3f(ambientlocation, ambientf, ambientf, ambientf);
		GLuint diffuselocation = glGetUniformLocation(program, "diffuse");
		glUniform3f(diffuselocation, diffusef, diffusef, diffusef);
		GLuint specularlocation = glGetUniformLocation(program, "specular");
		glUniform3f(specularlocation, specularf, specularf, specularf);
		GLuint deltalocation = glGetUniformLocation(program, "delta");
		glUniform1f(deltalocation, shininessf);

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
		loadShader();
		loadTexture();
        loadObj();
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
			if (shapes[i].mesh.texcoords.size() == 0) hasTC = false;
        }
        positions	   = (float*) malloc(sizeof(GLfloat) * numf * 9);
		normals		   = (float*)malloc(sizeof(GLfloat)* numf * 9);
		texcoords	   = (float*)malloc(sizeof(GLfloat)* numf * 6);
        size_positions = numf * 9;
		size_normals   = numf * 9;
		size_texcoords = numf * 6;
        
        
        //fill the positions and texcoords array
        int count1 = 0;
		int count2 = 0;
        int count3 = 0;
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
					normals  [count2++] = shapes[i].mesh.normals  [vidx*3 + 0];
					normals  [count2++] = shapes[i].mesh.normals  [vidx*3 + 1];
					normals  [count2++] = shapes[i].mesh.normals  [vidx*3 + 2];
					if (hasTC)
					{
						texcoords[count3++] = shapes[i].mesh.texcoords[vidx*2 + 0];
						texcoords[count3++] = shapes[i].mesh.texcoords[vidx*2 + 1];
					}
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
		{
			//for the normals
			glGenBuffers(1, &vbo_normals);
			glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*size_normals, normals, GL_STATIC_DRAW);
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

		//set up normals' attribute bindings
		{
			GLuint location = glGetAttribLocation(program, "vs_norm");
			glVertexAttribBinding(location, location);
			glBindVertexBuffer(location, vbo_normals, 0, sizeof(GLfloat)* 3);
			glVertexAttribFormat(location, 3, GL_FLOAT, GL_FALSE, 0);
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
		std::vector<std::vector<float>> blendshape_normals(NUM_BLENDSHAPE);
        for(int i=0; i<NUM_BLENDSHAPE; i++)
        {
            const char* ObjFileName = blendShapesFileNames[i];
            
            std::cout << "-- Reading " << ObjFileName << std::endl;
            
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
            blendshape_positions[i].reserve(size_positions);
			blendshape_normals[i].reserve(size_normals);
            
            
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
						blendshape_normals[i].push_back(shapes[j].mesh.normals[vidx * 3 + 0]);
						blendshape_normals[i].push_back(shapes[j].mesh.normals[vidx * 3 + 1]);
						blendshape_normals[i].push_back(shapes[j].mesh.normals[vidx * 3 + 2]);
                    }
                }
            }
        }
        
		//compute the difference vector
		for (int j = 0; j< size_positions; j++)
		{
			for (int i = 0; i<NUM_BLENDSHAPE; i++)
			{
				blendshape_positions[i][j] -= positions[j];
				blendshape_normals[i][j] -= normals[j];
			}
		}

		//set up vertex buffer objects
		glBindVertexArray(vao);
		glGenBuffers(NUM_BLENDSHAPE, vbo_bs_positions);
		glGenBuffers(NUM_BLENDSHAPE, vbo_bs_normals);
		for (int i = 0; i < NUM_BLENDSHAPE; i++)
		{
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_bs_positions[i]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* blendshape_positions[i].size(), &blendshape_positions[i][0], GL_STATIC_DRAW);
				char attriName[10];
				sprintf(attriName, "pos%d", i);
				GLuint location = glGetAttribLocation(program, attriName);
				glVertexAttribBinding(location, location);
				glBindVertexBuffer(location, vbo_bs_positions[i], 0, sizeof(GLfloat)* 3);
				glVertexAttribFormat(location, 3, GL_FLOAT, GL_FALSE, 0);
				glEnableVertexAttribArray(location);
			}
			{
				glBindBuffer(GL_ARRAY_BUFFER, vbo_bs_normals[i]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)* blendshape_normals[i].size(), &blendshape_normals[i][0], GL_STATIC_DRAW);
				char attriName[10];
				sprintf(attriName, "norm%d", i);
				GLuint location = glGetAttribLocation(program, attriName);
				glVertexAttribBinding(location, location);
				glBindVertexBuffer(location, vbo_bs_normals[i], 0, sizeof(GLfloat)* 3);
				glVertexAttribFormat(location, 3, GL_FLOAT, GL_FALSE, 0);
				glEnableVertexAttribArray(location);
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
#pragma endregion

#pragma region Camera
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

    void bindWindow(GLFWwindow* window)
    {
        if(window)
        {
            Application::window = window;
        }
        else
        {
            std::cout <<"The pointer to window cannot be NULL"<<std::endl;
			exit(-1);
        }
    }
#pragma endregion

#pragma region Graphics User Interface(GUI)
	void initGUI()
	{
		ImGui_ImplGlfwGL3_Init(window, true);
	}

	void renderGUI()
	{
		if (!GUIready) initGUI();

		ImGui_ImplGlfwGL3_NewFrame();
		{
			static float f = 0.0f;
			ImGui::Text("Facial Blending Shapes");
			ImGui::SliderFloat("Angry", &weights[0], 0.0f, 1.0f);
			ImGui::SliderFloat("Cry", &weights[1], 0.0f, 1.0f);
			ImGui::SliderFloat("Fury", &weights[2], 0.0f, 1.0f);
			ImGui::SliderFloat("Grin", &weights[3], 0.0f, 1.0f);
			ImGui::SliderFloat("Laugh", &weights[4], 0.0f, 1.0f);
			ImGui::SliderFloat("Rage", &weights[5], 0.0f, 1.0f);

			ImGui::Text("Lighting");
			ImGui::SliderFloat("Ambient", &ambientf, 0.0f, 1.0f);
			ImGui::SliderFloat("Diffuse", &diffusef, 0.0f, 1.0f);
			ImGui::SliderFloat("Specular", &specularf, 0.0f, 1.0f);
			ImGui::SliderFloat("Shininess", &shininessf, 1.0f, 50.0f);
		}

		ImGui::Render();
	}

	void shutdownGUI()
	{
		ImGui_ImplGlfwGL3_Shutdown();
	}
#pragma endregion
}
