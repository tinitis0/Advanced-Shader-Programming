#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"
#include "Camera.h"

#include <Model.h>
#include "Terrain.h"

#include<string>
#include <iostream>
#include <numeric>


// settings
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

const unsigned int SHADOW_W = 3072;
const unsigned int SHADOW_H = 3072;

const unsigned int LOD = 32;
glm::vec3 dirLightPos(0.1f,1.0f,0.2f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);

void setFBOcolour();
void setFBOdepth();
void renderQuad();

// camera
Camera camera(glm::vec3(69, 91, 437));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//arrays
unsigned int VBO, VAO, quadVAO, quadVBO, FBO;
unsigned int textureColourbuffer;
unsigned int textureDepthBuffer;
unsigned int depthMap;
unsigned int SM;
unsigned int depthFBO;

//terrain
std::vector<float> verticies;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//sky
const float RED = 0.7f; 
const float GREEN = 0.8f;
const float BLUE = 0.9f;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// simple vertex and fragment shader - add your own tess and geo shader
	Shader shader("..\\shaders\\plainVert.vs", "..\\shaders\\plainFrag.fs", "..\\shaders\\tessEvaluationShader.tes", "..\\shaders\\tessControlShader.tcs");
	Shader postProcessor("..\\shaders\\VertShader.vs", "..\\shaders\\fragShader.fs");
	Shader depthShader("..\\shaders\\depthVert.vs", "..\\shaders\\depthFrag.fs");
	Shader ShadowM("..\\shaders\\SMVertShader.vs", "..\\shaders\\SMFragShader.fs");
	unsigned int heightMap = loadTexture("..\\resources\\HeightMap.jpg");
	//unsigned int cat = loadTexture("..\\resources\\download.jfif");
	

	//Terrain Constructor ; number of grids in width, number of grids in height, gridSize
	Terrain terrain(50, 50,10);
	VAO = terrain.getVAO();	
	setFBOcolour();
	setFBOdepth();
	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(RED, GREEN, BLUE, 1.0);


	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;


	glm::vec3 lightPos = glm::vec3(0, 0, 0);
	glm::vec3 lookingAt = glm::vec3(0, 0, 0);


	int showShadow;
	//Shadows


	while (!glfwWindowShouldClose(window))
	{
		showShadow = 0;
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
			showShadow = 1;

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);

		
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", model);
		shader.setVec3("camPos", camera.Position);
		shader.setVec3("viewPos", camera.Position);
		shader.setInt("showShadow", showShadow);
		shader.setInt("heightMap", 0);
		shader.setInt("shadowMap", 1);
		shader.setInt("SM", 3);
		shader.setInt("scale", 100);


		lightPos = glm::vec3(337, 420, 250);
		lookingAt = glm::vec3(- 390, 40, 266);
		glm::mat4 lightView = glm::lookAt(lightPos, lookingAt, glm::vec3(10.0f, 10.0f, 10.0f));
		float near_plane = 0.01f, far_plane = 1000.f, ortho_size = 250.f;
		lightProjection = glm::ortho(-ortho_size, ortho_size, -ortho_size, ortho_size, near_plane, far_plane);
		lightSpaceMatrix = lightProjection * lightView;
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		
		//light properties
		shader.setVec3("dirLight.position", dirLightPos);
		shader.setVec3("dirLight.ambient", 0.5f, 0.5f, 0.5f);
		shader.setVec3("dirLight.diffuse", 0.55f, 0.55f, 0.55f);
		shader.setVec3("dirLight.specular", 0.6f, 0.6f, 0.6f);

		shader.setVec3("sky", glm::vec3(RED, GREEN, BLUE));

		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
			
		//first
		glViewport(0, 0, SHADOW_W, SHADOW_H);
		depthShader.use();
		glBindFramebuffer(GL_FRAMEBUFFER, 1);  // bind FBO		
		glEnable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMap);
		glClearColor(RED, GREEN, BLUE, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(VAO);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_PATCHES, 0, terrain.getSize());
		//second pass
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		shader.use();		
		glEnable(GL_DEPTH_TEST);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glClearColor(RED, GREEN, BLUE, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindVertexArray(VAO);
	    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_PATCHES, 0, terrain.getSize());
		renderQuad();
		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		ShadowM.use();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, SM);		
		glDrawArrays(GL_PATCHES, 0, terrain.getSize());
		renderQuad();





		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			camera.printCameraCoords();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}





// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		std::cout << "Loaded texture at path: " << path << " width " << width << " id " << textureID <<  std::endl;

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		
	}

	return textureID;
}

void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

}


void setFBOcolour()
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	
	glGenTextures(1, &textureColourbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColourbuffer);	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, textureColourbuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

}

void setFBOdepth()
{
	glGenFramebuffers(1, &depthFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float boarderColour[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, boarderColour);

	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

	glDrawBuffer(GL_NONE);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
