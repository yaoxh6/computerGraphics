#include "stdafx.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Camera.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"uniform mat4 transform;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * transform * vec4(aPos, 1.0);\n"
"   ourColor = aColor;\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n"
"}\n\0";

const char* glsl_version = "#version 130";

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

														 // glfw window creation
														 // --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "DrawCube", NULL, NULL);
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

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// fragment shader
	int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);


	unsigned int VBO, VAO;
	float lengthOfSide = 2.0f;
	float scaleFactor = 0.9f;
	glm::vec3 rotateNomarl = glm::vec3(1.0f, 0.0f, 1.0f);//沿着x=z轴旋转
	ImVec4 color1 = ImVec4(1, 0, 0, 1);
	ImVec4 color2 = ImVec4(0, 1, 0, 1);
	ImVec4 color3 = ImVec4(0, 0, 1, 1);
	ImVec4 color4 = ImVec4(0.5, 0, 1, 1);
	ImVec4 color5 = ImVec4(1, 0.5, 0, 1);
	ImVec4 color6 = ImVec4(1, 1, 1, 1);


	float vertices[] = {
		-lengthOfSide, -lengthOfSide, -lengthOfSide,color1.x,color1.y,color1.z,
		lengthOfSide, -lengthOfSide, -lengthOfSide,color1.x,color1.y,color1.z,
		lengthOfSide, lengthOfSide, -lengthOfSide,color1.x,color1.y,color1.z,
		lengthOfSide, lengthOfSide, -lengthOfSide,color1.x,color1.y,color1.z,
		-lengthOfSide, lengthOfSide, -lengthOfSide,color1.x,color1.y,color1.z,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,color1.x,color1.y,color1.z,

		-lengthOfSide, -lengthOfSide, lengthOfSide,color2.x,color2.y,color2.z,
		lengthOfSide, -lengthOfSide, lengthOfSide,color2.x,color2.y,color2.z,
		lengthOfSide, lengthOfSide, lengthOfSide,color2.x,color2.y,color2.z,
		lengthOfSide, lengthOfSide, lengthOfSide,color2.x,color2.y,color2.z,
		-lengthOfSide, lengthOfSide, lengthOfSide,color2.x,color2.y,color2.z,
		-lengthOfSide, -lengthOfSide, lengthOfSide,color2.x,color2.y,color2.z,

		-lengthOfSide, lengthOfSide, lengthOfSide,color3.x,color3.y,color3.z,
		-lengthOfSide, lengthOfSide, -lengthOfSide,color3.x,color3.y,color3.z,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,color3.x,color3.y,color3.z,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,color3.x,color3.y,color3.z,
		-lengthOfSide, -lengthOfSide, lengthOfSide,color3.x,color3.y,color3.z,
		-lengthOfSide, lengthOfSide, lengthOfSide,color3.x,color3.y,color3.z,

		lengthOfSide, lengthOfSide, lengthOfSide,color4.x,color4.y,color4.z,
		lengthOfSide, lengthOfSide, -lengthOfSide,color4.x,color4.y,color4.z,
		lengthOfSide, -lengthOfSide, -lengthOfSide,color4.x,color4.y,color4.z,
		lengthOfSide, -lengthOfSide, -lengthOfSide,color4.x,color4.y,color4.z,
		lengthOfSide, -lengthOfSide, lengthOfSide,color4.x,color4.y,color4.z,
		lengthOfSide, lengthOfSide, lengthOfSide,color4.x,color4.y,color4.z,

		-lengthOfSide, -lengthOfSide, -lengthOfSide,color5.x,color5.y,color5.z,
		lengthOfSide, -lengthOfSide, -lengthOfSide,color5.x,color5.y,color5.z,
		lengthOfSide, -lengthOfSide, lengthOfSide,color5.x,color5.y,color5.z,
		lengthOfSide, -lengthOfSide, lengthOfSide,color5.x,color5.y,color5.z,
		-lengthOfSide, -lengthOfSide, lengthOfSide,color5.x,color5.y,color5.z,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,color5.x,color5.y,color5.z,

		-lengthOfSide, lengthOfSide, -lengthOfSide,color6.x,color6.y,color6.z,
		lengthOfSide, lengthOfSide, -lengthOfSide,color6.x,color6.y,color6.z,
		lengthOfSide, lengthOfSide, lengthOfSide,color6.x,color6.y,color6.z,
		lengthOfSide, lengthOfSide, lengthOfSide,color6.x,color6.y,color6.z,
		-lengthOfSide, lengthOfSide, lengthOfSide,color6.x,color6.y,color6.z,
		-lengthOfSide, lengthOfSide, -lengthOfSide,color6.x,color6.y,color6.z,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glUseProgram(shaderProgram);

	bool isOpen = true;
	bool isFirstProbelm = true;
	bool isSecondProblem = false;
	bool isBouns = false;
	bool isOrthographic = true;
	bool isPerspective = false;
	float myLeft = -10.0f;
	float myRight = 10.0f;
	float myBottom = -10.0f;
	float myTop = 10.0f;
	float myNear = 0.1f;
	float myFar = 100.0f;

	float fov = 45.0f;
	float ratio = 1.0f;
	float myNear2 = 0.1f;
	float myFar2 = 100.0f;

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glEnable(GL_DEPTH_TEST);
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("DrawCube",&isOpen, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Menu")) {
				if (ImGui::MenuItem("FirstProblem")) {
					isFirstProbelm = true;
					isSecondProblem = false;
					isBouns = false;
				}
				if (ImGui::MenuItem("SecondProblem")) {
					isFirstProbelm = false;
					isSecondProblem = true;
					isBouns = false;
				}
				if (ImGui::MenuItem("Bouns")) {
					isFirstProbelm = false;
					isSecondProblem = false;
					isBouns = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		if (isFirstProbelm) {
			ImGui::Checkbox("Orthographic", &isOrthographic);
			if (isOrthographic) {
				isPerspective = false;
			}
			ImGui::Checkbox("Perspective", &isPerspective);
			if (isPerspective) {
				isOrthographic = false;
			}
		}

		if (isFirstProbelm && isOrthographic) {
			ImGui::SliderFloat("left", &myLeft, -100.0f, 100.0f);
			ImGui::SliderFloat("right", &myRight, -100.0f, 100.0f);
			ImGui::SliderFloat("bottom", &myBottom, -100.0f, 100.0f);
			ImGui::SliderFloat("top", &myTop, -100.0f, 100.0f);
			ImGui::SliderFloat("near", &myNear, -1.0f, 1.0f);
			ImGui::SliderFloat("far", &myFar, 80.0f, 120.0f);
		}
		if (isFirstProbelm && isPerspective) {
			ImGui::SliderFloat("fov", &fov, 0.0f, 90.0f);
			ImGui::SliderFloat("ratio", &ratio, 0.1f, 2.0f);
			ImGui::SliderFloat("near", &myNear2, 0.0f, 1.0f);
			ImGui::SliderFloat("far", &myFar2, 80.0f, 120.0f);
		}
		ImGui::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glm::mat4 transform = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 projection = glm::mat4(1.0f);
		glBindVertexArray(VAO);

		if (isSecondProblem) {
			projection = glm::perspective(glm::radians(fov), ratio, myNear2, myFar2);
			float radius = 10.0f;
			float camX = sin(glfwGetTime()) * radius;
			float camZ = cos(glfwGetTime()) * radius;
			view = glm::lookAt(glm::vec3(camX, 10.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}
		
		if (isFirstProbelm) {
			if (isPerspective) {
				projection = glm::perspective(glm::radians(fov), ratio, myNear2, myFar2);
			}
			else if (isOrthographic) {
				projection = glm::ortho(myLeft, myRight, myBottom, myTop, myNear, myFar);
			}
			transform = glm::translate(transform, glm::vec3(-1.5f, 0.5f, -1.5f));
			view = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		}

		if (isBouns) {
			projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			view = camera.GetViewMatrix();
		}

		unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}