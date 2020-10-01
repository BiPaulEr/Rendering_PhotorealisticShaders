// ----------------------------------------------
// Base code for practical computer graphics
// assignments.
//
// Copyright (C) 2018 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#define _USE_MATH_DEFINES

#include <glad/glad.h>

#include <cstdlib>
#include <cstdio>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <memory>
#include <algorithm>
#include <exception>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Error.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Mesh.h"
#include "MeshLoader.h"
#include "Material.h"
#include "LightSource.h"

static const std::string SHADER_PATH ("../Resources/Shaders/");

static const std::string DEFAULT_MESH_FILENAME ("../Resources/Models/rhino.off");

using namespace std;

// Window parameters
static GLFWwindow * windowPtr = nullptr;

// Pointer to the current camera model
static std::shared_ptr<Camera> cameraPtr;

// Pointer to the displayed mesh
static std::shared_ptr<Mesh> meshPtr;

// Pointer to GPU shader pipeline i.e., set of shaders structured in a GPU program
static std::shared_ptr<ShaderProgram> shaderProgramPtr; // A GPU program contains at least a vertex shader and a fragment shader

// Camera control variables
static float meshScale = 1.0; // To update based on the mesh size, so that navigation runs at scale
static bool isRotating (false);
static bool isPanning (false);
static bool isZooming (false);
static double baseX (0.0), baseY (0.0);
static glm::vec3 baseTrans (0.0);
static glm::vec3 baseRot (0.0);

//Rendering mode (0 : PBR, 1 : toon shading, 2 : x-toon shading)
static float renderingMode = 0.f;
static bool microFacet = true;	//Blinn-Phong BRDF / micro facet BRDF
static bool ggx = true;			//Cook-Torrance micro facet BRDF / GGX micro facet BRDF
static bool schlick = true;
void clear ();

void printHelp () {
	std::cout << "> Help:" << std::endl
			  << "    Mouse commands:" << std::endl
			  << "    * Left button: rotate camera" << std::endl
			  << "    * Middle button: zoom" << std::endl
			  << "    * Right button: pan camera" << std::endl
			  << "    Keyboard commands:" << std::endl
   			  << "    * H: print this help" << std::endl
   			  << "    * F1: toggle wireframe rendering" << std::endl
   			  << "    * ESC: quit the program" << std::endl;
}

// Executed each time the window is resized. Adjust the aspect ratio and the rendering viewport to the current window.
void windowSizeCallback (GLFWwindow * windowPtr, int width, int height) {
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));
	glViewport (0, 0, (GLint)width, (GLint)height); // Dimension of the rendering region in the window
}

/// Executed each time a key is entered.
void keyCallback (GLFWwindow * windowPtr, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS && key == GLFW_KEY_H) {
		printHelp ();
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_F1) {
		GLint mode[2];
		glGetIntegerv (GL_POLYGON_MODE, mode);
		glPolygonMode (GL_FRONT_AND_BACK, mode[1] == GL_FILL ? GL_LINE : GL_FILL);
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose (windowPtr, true); // Closes the application if the escape key is pressed
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_T) {
		renderingMode = (int)(renderingMode+1.f)%3; //change the rendering mode if the T key is pressed
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_V) {
		microFacet = !microFacet;
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_G) {
		ggx = !ggx;
	}
	else if (action == GLFW_PRESS && key == GLFW_KEY_S) {
		schlick = !schlick;
	}
}

/// Called each time the mouse cursor moves
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	float normalizer = static_cast<float> ((width + height)/2);
	float dx = static_cast<float> ((baseX - xpos) / normalizer);
	float dy = static_cast<float> ((ypos - baseY) / normalizer);
	if (isRotating) {
		glm::vec3 dRot (-dy * M_PI, dx * M_PI, 0.0);
		cameraPtr->setRotation (baseRot + dRot);
	}
	else if (isPanning) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (dx, dy, 0.0));
	} else if (isZooming) {
		cameraPtr->setTranslation (baseTrans + meshScale * glm::vec3 (0.0, 0.0, dy));
	}
}

/// Called each time a mouse button is pressed
void mouseButtonCallback (GLFWwindow * window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    	if (!isRotating) {
    		isRotating = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseRot = cameraPtr->getRotation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
    	isRotating = false;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    	if (!isPanning) {
    		isPanning = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
    	isPanning = false;
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
    	if (!isZooming) {
    		isZooming = true;
    		glfwGetCursorPos (window, &baseX, &baseY);
    		baseTrans = cameraPtr->getTranslation ();
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
    	isZooming = false;
    }
}

void initGLFW () {
	// Initialize GLFW, the library responsible for window management
	if (!glfwInit ()) {
		std::cerr << "ERROR: Failed to init GLFW" << std::endl;
		std::exit (EXIT_FAILURE);
	}

	// Before creating the window, set some option flags
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);

	// Create the window
	windowPtr = glfwCreateWindow (1024, 768, "Computer Graphics - Practical Assignment", nullptr, nullptr);
	if (!windowPtr) {
		std::cerr << "ERROR: Failed to open window" << std::endl;
		glfwTerminate ();
		std::exit (EXIT_FAILURE);
	}

	// Load the OpenGL context in the GLFW window using GLAD OpenGL wrangler
	glfwMakeContextCurrent (windowPtr);

	/// Connect the callbacks for interactive control
	glfwSetWindowSizeCallback (windowPtr, windowSizeCallback);
	glfwSetKeyCallback (windowPtr, keyCallback);
	glfwSetCursorPosCallback(windowPtr, cursorPosCallback);
	glfwSetMouseButtonCallback (windowPtr, mouseButtonCallback);
}

void exitOnCriticalError (const std::string & message) {
	std::cerr << "> [Critical error]" << message << std::endl;
	std::cerr << "> [Clearing resources]" << std::endl;
	clear ();
	std::cerr << "> [Exit]" << std::endl;
	std::exit (EXIT_FAILURE);
}

void initOpenGL () {
	// Load extensions for modern OpenGL
	if (!gladLoadGLLoader ((GLADloadproc)glfwGetProcAddress))
		exitOnCriticalError ("[Failed to initialize OpenGL context]");

	glEnable (GL_DEBUG_OUTPUT); // Modern error callback functionnality
	glEnable (GL_DEBUG_OUTPUT_SYNCHRONOUS); // For recovering the line where the error occurs, set a debugger breakpoint in DebugMessageCallback
    glDebugMessageCallback (debugMessageCallback, 0); // Specifies the function to call when an error message is generated.
	glCullFace (GL_BACK);     // Specifies the faces to cull (here the ones pointing away from the camera)
	glEnable (GL_CULL_FACE); // Enables face culling (based on the orientation defined by the CW/CCW enumeration).
	glDepthFunc (GL_LESS); // Specify the depth test for the z-buffer
	glEnable (GL_DEPTH_TEST); // Enable the z-buffer test in the rasterization

	// Loads and compile the programmable shader pipeline
	try {
		shaderProgramPtr = ShaderProgram::genBasicShaderProgram (SHADER_PATH + "VertexShader.glsl",
													         	 SHADER_PATH + "ComplexFragmentShader.glsl");
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading shader program]") + e.what ());
	}
}

#define NB_LIGHTSOURCES 4
LightSource lightSourcesArray[NB_LIGHTSOURCES];

void initScene (const std::string & meshFilename) {
	// Camera
	int width, height;
	glfwGetWindowSize (windowPtr, &width, &height);
	cameraPtr = std::make_shared<Camera> ();
	cameraPtr->setAspectRatio (static_cast<float>(width) / static_cast<float>(height));

	// Mesh
	meshPtr = std::make_shared<Mesh> ();
	try {
		MeshLoader::loadOFF (meshFilename, meshPtr);
	} catch (std::exception & e) {
		exitOnCriticalError (std::string ("[Error loading mesh]") + e.what ());
	}
	meshPtr->init ();

	// Lighting
	lightSourcesArray[0] = LightSource(glm::vec3 (5.0, 5.0, 5.0), glm::vec3 (1.0, 1.0, 1.0), 10.f, 1.f, 0.1f, 0.01f, M_PI/8, glm::vec3 (-1.0, -1.0, -1.0)); //position, color, intensity, a_c, a_l, a_q, coneAngle, direction
	lightSourcesArray[1] = LightSource(glm::vec3 (-10.0, -10.0, -10.0), glm::vec3 (1.0, 0.0, 0.0), 10.f, 1.f, 0.1f, 0.01f, M_PI/8, glm::vec3 (1.0, 1.0, 1.0));
	lightSourcesArray[2] = LightSource(glm::vec3(-5.0, 0, 0), glm::vec3(0.0, 1.0, 0.0), 0.5f, 1.f, 0.1f, 0.01f, M_PI / 8, glm::vec3(5.0, 0.0, 0.1));
	lightSourcesArray[3] = LightSource(glm::vec3(0.0, -5.0, 0), glm::vec3(0.0, 0.0, 1.0), 0.5f, 1.f, 0.1f, 0.01f, M_PI / 8, glm::vec3(0.1, 5.0, 0.0));
	for(int i = 0; i < NB_LIGHTSOURCES; i++) {
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].color", lightSourcesArray[i].getColor());
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].intensity", lightSourcesArray[i].getIntensity());
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].a_c", lightSourcesArray[i].getA_c());
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].a_l", lightSourcesArray[i].getA_l());
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].a_q", lightSourcesArray[i].getA_q());
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].coneAngle", lightSourcesArray[i].getConeAngle());
	}
	shaderProgramPtr->set("microFacet", microFacet);		//Blinn-Phong BRDF / micro facet BRDF
	shaderProgramPtr->set("ggx", ggx);			//Cook-Torrance micro facet BRDF / GGX micro facet BRDF
	shaderProgramPtr->set("schlick", schlick); // Approximation de schlick
	// Material
	Material material = Material(glm::vec3 (0.4, 0.6, 0.2), 0.01, glm::vec3 (0.91, 0.92, 0.92));

	string dirName = "..\\Resources\\Materials\\Metal\\";
	GLuint albedoTex = material.loadTextureFromFileToGPU(dirName + "Base_Color.png");

	GLuint roughnessTex = material.loadTextureFromFileToGPU(dirName + "Roughness.png");

	GLuint metallicTex = material.loadTextureFromFileToGPU(dirName + "Metallic.png");

	GLuint ambientTex = material.loadTextureFromFileToGPU(dirName + "Ambient_Occlusion.png");

	GLuint toonTex = material.loadTextureFromFileToGPU(dirName + "X_toon.png");

	shaderProgramPtr->set ("material.albedoTex", 0u);
	shaderProgramPtr->set ("material.roughnessTex", 1u);
	shaderProgramPtr->set ("material.metallicTex", 2u);
	shaderProgramPtr->set ("material.ambientTex", 3u);
	shaderProgramPtr->set ("material.toonTex", 4u);

	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, albedoTex);

	glActiveTexture (GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, roughnessTex);

	glActiveTexture (GL_TEXTURE2);
	glBindTexture (GL_TEXTURE_2D, metallicTex);

	glActiveTexture (GL_TEXTURE3);
	glBindTexture (GL_TEXTURE_2D, ambientTex);

	glActiveTexture (GL_TEXTURE4);
	glBindTexture (GL_TEXTURE_2D, toonTex);

	//zMin and zMax for the computation of the detail value
	shaderProgramPtr->set ("zMin", meshScale);
	shaderProgramPtr->set ("zMax", meshScale*5);

	// Adjust the camera to the actual mesh
	glm::vec3 center;
	meshPtr->computeBoundingSphere (center, meshScale);
	cameraPtr->setTranslation (center + glm::vec3 (0.0, 0.0, 3.0 * meshScale));
	cameraPtr->setNear (meshScale / 100.f);
	cameraPtr->setFar (6.f * meshScale);
}

void init (const std::string & meshFilename) {
	initGLFW (); // Windowing system
	initOpenGL (); // OpenGL Context and shader pipeline
	initScene (meshFilename); // Actual scene to render
}

void clear () {
	cameraPtr.reset ();
	meshPtr.reset ();
	shaderProgramPtr.reset ();
	glfwDestroyWindow (windowPtr);
	glfwTerminate ();
}

// The main rendering call
void render () {
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Erase the color and z buffers.

	// specify the background color, used any time the framebuffer is cleared
	if (renderingMode == 0.f) {
		glClearColor (0.0f, 0.0f, 0.0f, 1.0f); }
	else {
		glClearColor (1.0f, 1.0f, 1.0f, 1.0f); }

	shaderProgramPtr->use (); // Activate the program to be used for upcoming primitive
	glm::mat4 projectionMatrix = cameraPtr->computeProjectionMatrix ();
	shaderProgramPtr->set ("projectionMat", projectionMatrix); // Compute the projection matrix of the camera and pass it to the GPU program
	glm::mat4 modelMatrix = meshPtr->computeTransformMatrix ();
	glm::mat4 viewMatrix = cameraPtr->computeViewMatrix ();
	glm::mat4 modelViewMatrix = viewMatrix * modelMatrix;
	glm::mat4 normalMatrix = glm::transpose (glm::inverse (modelViewMatrix));
	shaderProgramPtr->set ("modelViewMat", modelViewMatrix);
	shaderProgramPtr->set ("normalMat", normalMatrix);
	meshPtr->render ();
	shaderProgramPtr->stop ();
}

// Update any accessible variable based on the current time
void update (float currentTime) {
	// Animate any entity of the program here
	static const float initialTime = currentTime;
	float dt = currentTime - initialTime;
	// <---- Update here what needs to be animated over time ---->
	shaderProgramPtr->use();

	glm::mat4 matrix = cameraPtr->computeViewMatrix();

	//updating the cone angle of a lightsource
	//lightSourcesArray[1].setConeAngle(abs(sin(dt))/24);
	//shaderProgramPtr->set ("lightSourcesArray[" + to_string(1) + "].coneAngle", lightSourcesArray[1].getConeAngle());

	/*compose the transformation matrix of lightsources with the one of the camera
	so that the lights are not “attached” to the camera*/
	for(int i = 0; i < NB_LIGHTSOURCES; i++) {
		glm::vec4 pos = glm::vec4 (lightSourcesArray[i].getPosition(), 1);
		pos = matrix*pos;
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].position", glm::vec3 (pos)/pos.w);

		glm::vec4 dir = glm::vec4 (lightSourcesArray[i].getDirection(), 0);
		dir = matrix*dir;
		shaderProgramPtr->set ("lightSourcesArray[" + to_string(i) + "].direction", glm::vec3 (dir));
	}

	shaderProgramPtr->set("microFacet", microFacet);		//Blinn-Phong BRDF / micro facet BRDF
	shaderProgramPtr->set("ggx", ggx);			//Cook-Torrance micro facet BRDF / GGX micro facet BRDF
	shaderProgramPtr->set("schlick", schlick); // Approximation de schlick

	//updating the rendering mode
	shaderProgramPtr->set ("renderingMode", renderingMode);
}

void usage (const char * command) {
	std::cerr << "Usage : " << command << " [<file.off>]" << std::endl;
	std::exit (EXIT_FAILURE);
}

int main (int argc, char ** argv) {
	if (argc > 2)
		usage (argv[0]);
	init (argc == 1 ? DEFAULT_MESH_FILENAME : argv[1]); // Your initialization code (user interface, OpenGL states, scene with geometry, material, lights, etc)
	while (!glfwWindowShouldClose (windowPtr)) {
		update (static_cast<float> (glfwGetTime ()));
		render ();
		glfwSwapBuffers (windowPtr);
		glfwPollEvents ();
	}
	clear ();
	std::cout << " > Quit" << std::endl;
	return EXIT_SUCCESS;
}
