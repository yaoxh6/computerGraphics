#include "stdafx.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 transform;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * transform * vec4(aPos, 1.0);\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 0.1f);\n"
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

	float vertices[] = {
		-lengthOfSide, -lengthOfSide, -lengthOfSide,
		lengthOfSide, -lengthOfSide, -lengthOfSide,
		lengthOfSide, lengthOfSide, -lengthOfSide,
		lengthOfSide, lengthOfSide, -lengthOfSide,
		-lengthOfSide, lengthOfSide, -lengthOfSide,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,

		-lengthOfSide, -lengthOfSide, lengthOfSide,
		lengthOfSide, -lengthOfSide, lengthOfSide,
		lengthOfSide, lengthOfSide, lengthOfSide,
		lengthOfSide, lengthOfSide, lengthOfSide,
		-lengthOfSide, lengthOfSide, lengthOfSide,
		-lengthOfSide, -lengthOfSide, lengthOfSide,

		-lengthOfSide, lengthOfSide, lengthOfSide,
		-lengthOfSide, lengthOfSide, -lengthOfSide,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,
		-lengthOfSide, -lengthOfSide, lengthOfSide,
		-lengthOfSide, lengthOfSide, lengthOfSide,

		lengthOfSide, lengthOfSide, lengthOfSide,
		lengthOfSide, lengthOfSide, -lengthOfSide,
		lengthOfSide, -lengthOfSide, -lengthOfSide,
		lengthOfSide, -lengthOfSide, -lengthOfSide,
		lengthOfSide, -lengthOfSide, lengthOfSide,
		lengthOfSide, lengthOfSide, lengthOfSide,

		-lengthOfSide, -lengthOfSide, -lengthOfSide,
		lengthOfSide, -lengthOfSide, -lengthOfSide,
		lengthOfSide, -lengthOfSide, lengthOfSide,
		lengthOfSide, -lengthOfSide, lengthOfSide,
		-lengthOfSide, -lengthOfSide, lengthOfSide,
		-lengthOfSide, -lengthOfSide, -lengthOfSide,

		-lengthOfSide, lengthOfSide, -lengthOfSide,
		lengthOfSide, lengthOfSide, -lengthOfSide,
		lengthOfSide, lengthOfSide, lengthOfSide,
		lengthOfSide, lengthOfSide, lengthOfSide,
		-lengthOfSide, lengthOfSide, lengthOfSide,
		-lengthOfSide, lengthOfSide, -lengthOfSide,
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glUseProgram(shaderProgram);

	bool isOpen = true;
	bool isDeepTest = false;
	bool isTranslate = false;
	bool isRotate = false;
	bool isScale = false;
	float transX = 0.0f;

	glm::mat4 projection = glm::mat4(1.0f);
	projection= glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (!glfwWindowShouldClose(window))
	{
		//glEnable(GL_DEPTH_TEST);
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("DrawCube",&isOpen, ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("Menu")) {
				if (ImGui::MenuItem("Translate")) {
					isTranslate = true;
					isRotate = false;
					isScale = false;
				}
				if (ImGui::MenuItem("Rotate")) {
					isTranslate = false;
					isRotate = true;
					isScale = false;
				}
				if (ImGui::MenuItem("Scale")) {
					isTranslate = false;
					isRotate = false;
					isScale = true;
				}
				if (ImGui::MenuItem("Bonus")) {
					isTranslate = true;
					isRotate = true;
					isScale = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glm::mat4 view = glm::mat4(1.0f);

		view = glm::lookAt(glm::vec3(6.0f , 4.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glBindVertexArray(VAO);
		glm::mat4 transform = glm::mat4(1.0f);
		
		if (isScale) {
			scaleFactor = sin((float)glfwGetTime());
			transform = glm::scale(transform, glm::vec3(scaleFactor, scaleFactor, scaleFactor));
		}
		if (isRotate) {
			transform = glm::rotate(transform, (float)glfwGetTime(), glm::vec3(1.0f, 0.0, 1.0f));
		}
		if (isTranslate) {
			transX = sin((float)glfwGetTime()) / 2;
			transform = glm::translate(transform, glm::vec3(transX, 0.0f, 0.0f));
		}
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
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}