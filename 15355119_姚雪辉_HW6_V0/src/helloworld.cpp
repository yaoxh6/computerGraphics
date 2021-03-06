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

Camera camera(glm::vec3(1.8f, 0.9f, 3.8f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

glm::vec3 lightPos(1.2, 1.0f, 2.0f);

bool isOpen = true;
bool isPhongShading = true;
bool isGouraudShading = false;
bool isBouns = false;
bool isChanging = true;
float ambientStrength = 0.0f;
float diffuseStrength = 0.0f;
float specularStrength = 0.0f;

const char *lightVertexShaderSource1 = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   FragPos = vec3(model * vec4(aPos,1.0));\n"
"   Normal = mat3(transpose(inverse(model)))*aNormal;\n"
"   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
"}\0";

const char *lightVertexShaderSource2 = "#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"
"out vec3 LightingColor;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"uniform float ambientStrength;\n"
"uniform float diffuseStrength;\n"
"uniform float specularStrength;\n"
"void main()\n"
"{\n"
"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"	vec3 Position = vec3(model * vec4(aPos, 1.0));\n"
"	vec3 Normal = mat3(transpose(inverse(model))) * aNormal;\n"
//"	float ambientStrength = 0.1;\n"
"	vec3 ambient = ambientStrength * lightColor;\n"
"	vec3 norm = normalize(Normal);\n"
"	vec3 lightDir = normalize(lightPos - Position);\n"
"	float diff = max(dot(norm, lightDir), 0.0);\n"
"	vec3 diffuse = diffuseStrength * diff * lightColor;\n"
//"	float specularStrength = 1.0;\n"
"	vec3 viewDir = normalize(viewPos - Position);\n"
"	vec3 reflectDir = reflect(-lightDir, norm);\n"
"	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
"	vec3 specular = specularStrength * spec * lightColor;\n"
"	LightingColor = ambient + diffuse + specular;\n"
"}\n";

const char *objectFragmentShaderSource2 = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 LightingColor;\n"
"uniform vec3 objectColor;\n"
"void main()\n"
"{\n"
"	FragColor = vec4(LightingColor * objectColor, 1.0);\n"
"}\n";


const char *objectFragmentShaderSource1 = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 viewPos;\n"
"uniform vec3 lightColor;\n"
"uniform vec3 objectColor;\n"
"uniform float ambientStrength;\n"
"uniform float diffuseStrength;\n"
"uniform float specularStrength;\n"
"void main()\n"
"{\n"
//"	float ambientStrength = 0.1;\n"
"	vec3 ambient = ambientStrength * lightColor;\n"
"	vec3 norm = normalize(Normal);\n"
"	vec3 lightDir = normalize(lightPos - FragPos);\n"
"	float diff = max(dot(norm, lightDir), 0.0);\n"
"	vec3 diffuse = diffuseStrength * diff * lightColor;\n"
//"	float specularStrength = 0.5;\n"
"	vec3 viewDir = normalize(viewPos - FragPos);\n"
"	vec3 reflectDir = reflect(-lightDir, norm);\n"
"	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
"	vec3 specular = specularStrength * spec * lightColor;\n"
"	vec3 result = (ambient + diffuse + specular) * objectColor;\n"
"	FragColor = vec4(result, 1.0);\n"
"}\n";


const char *lightFragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f);\n"
"}\n\0";

const char *objectVertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"

"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\0";

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
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Light", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	float vertices[] = {
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	  0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	 -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	 -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	  0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	 -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	 -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	 -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	 -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	 -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	 -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	 -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	 -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	  0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	  0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	 -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	 -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	 -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	 -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	unsigned int VBO, cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	int objectShader;
	int lightingShader;
	while (!glfwWindowShouldClose(window))
	{
		if (isPhongShading && isChanging) {
			isChanging = false;
			int objectVertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(objectVertexShader, 1, &objectVertexShaderSource, NULL);
			glCompileShader(objectVertexShader);
			// check for shader compile errors
			int success;
			char infoLog[512];
			glGetShaderiv(objectVertexShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(objectVertexShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// fragment shader
			int lightFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(lightFragmentShader, 1, &lightFragmentShaderSource, NULL);
			glCompileShader(lightFragmentShader);
			// check for shader compile errors
			glGetShaderiv(lightFragmentShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(lightFragmentShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// link shaders
			lightingShader = glCreateProgram();
			glAttachShader(lightingShader, objectVertexShader);
			glAttachShader(lightingShader, lightFragmentShader);
			glLinkProgram(lightingShader);
			// check for linking errors
			glGetProgramiv(lightingShader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(lightingShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}
			glDeleteShader(lightFragmentShader);
			glDeleteShader(objectVertexShader);


			int lightVertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(lightVertexShader, 1, &lightVertexShaderSource1, NULL);
			glCompileShader(lightVertexShader);

			glGetShaderiv(lightVertexShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(lightVertexShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			}

			int objectFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(objectFragmentShader, 1, &objectFragmentShaderSource1, NULL);
			glCompileShader(objectFragmentShader);
			// check for shader compile errors
			glGetShaderiv(objectFragmentShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(objectFragmentShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// link shaders
			objectShader = glCreateProgram();
			glAttachShader(objectShader, lightVertexShader);
			glAttachShader(objectShader, objectFragmentShader);
			glLinkProgram(objectShader);
			// check for linking errors
			glGetProgramiv(objectShader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(objectShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}
			glDeleteShader(lightVertexShader);
			glDeleteShader(objectFragmentShader);
		}
		else if (isGouraudShading && isChanging) {
			isChanging = false;
			int objectVertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(objectVertexShader, 1, &objectVertexShaderSource, NULL);
			glCompileShader(objectVertexShader);
			// check for shader compile errors
			int success;
			char infoLog[512];
			glGetShaderiv(objectVertexShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(objectVertexShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// fragment shader
			int lightFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(lightFragmentShader, 1, &lightFragmentShaderSource, NULL);
			glCompileShader(lightFragmentShader);
			// check for shader compile errors
			glGetShaderiv(lightFragmentShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(lightFragmentShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// link shaders
			lightingShader = glCreateProgram();
			glAttachShader(lightingShader, objectVertexShader);
			glAttachShader(lightingShader, lightFragmentShader);
			glLinkProgram(lightingShader);
			// check for linking errors
			glGetProgramiv(lightingShader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(lightingShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}
			glDeleteShader(lightFragmentShader);
			glDeleteShader(objectVertexShader);


			int lightVertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(lightVertexShader, 1, &lightVertexShaderSource2, NULL);
			glCompileShader(lightVertexShader);

			glGetShaderiv(lightVertexShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(lightVertexShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
			}

			int objectFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(objectFragmentShader, 1, &objectFragmentShaderSource2, NULL);
			glCompileShader(objectFragmentShader);
			// check for shader compile errors
			glGetShaderiv(objectFragmentShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(objectFragmentShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
			}
			// link shaders
			objectShader = glCreateProgram();
			glAttachShader(objectShader, lightVertexShader);
			glAttachShader(objectShader, objectFragmentShader);
			glLinkProgram(objectShader);
			// check for linking errors
			glGetProgramiv(objectShader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(objectShader, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}
			glDeleteShader(lightVertexShader);
			glDeleteShader(objectFragmentShader);
		}
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Light",&isOpen, ImGuiWindowFlags_MenuBar);
		ImGui::Checkbox("isPhongShading", &isPhongShading);
		if (isPhongShading) {
			isGouraudShading = false;
			isChanging = true;
		}
		ImGui::Checkbox("isGouraudShading", &isGouraudShading);
		if (isGouraudShading) {
			isPhongShading = false;
			isChanging = true;
		}
		ImGui::Checkbox("isBouns", &isBouns);

		ImGui::SliderFloat("ambientFactor", &ambientStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("ridiffuseFactorght", &diffuseStrength, 0.0f, 1.0f);
		ImGui::SliderFloat("specularFactor", &specularStrength, 0.0f, 1.0f);
		ImGui::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (isBouns) {
			lightPos.x = 1.0f + sin(glfwGetTime());
			lightPos.y = sin(glfwGetTime() / 2.0f);
		}
		
		glUseProgram(objectShader);
		unsigned int objectColorLoc = glGetUniformLocation(objectShader, "objectColor");
		glUniform3f(objectColorLoc,1.0f, 0.5f, 0.31f);
		unsigned int lightColorLoc = glGetUniformLocation(objectShader, "lightColor");
		glUniform3f(lightColorLoc,1.0f, 1.0f, 1.0f);
		unsigned int lightPosLoc = glGetUniformLocation(objectShader, "lightPos");
		glUniform3f(lightPosLoc, lightPos.x,lightPos.y,lightPos.z);
		unsigned int viewPosLoc = glGetUniformLocation(objectShader, "viewPos");
		glUniform3f(viewPosLoc, camera.Position.x,camera.Position.y,camera.Position.z);
		unsigned int ambientStrengthLoc = glGetUniformLocation(objectShader, "ambientStrength");
		glUniform1f(ambientStrengthLoc, ambientStrength);
		unsigned int diffuseStrengthLoc = glGetUniformLocation(objectShader, "diffuseStrength");
		glUniform1f(diffuseStrengthLoc, diffuseStrength);
		unsigned int specularStrengthLoc = glGetUniformLocation(objectShader, "specularStrength");
		glUniform1f(specularStrengthLoc, specularStrength);


		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		view = glm::lookAt(camera.Position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		unsigned int projectionLoc = glGetUniformLocation(objectShader, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		unsigned int viewLoc = glGetUniformLocation(objectShader, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-0.5f, -0.5f, 0.0f));
		unsigned int modelLoc = glGetUniformLocation(objectShader, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(cubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glUseProgram(lightingShader);
		
		unsigned int objectProjectionLoc = glGetUniformLocation(lightingShader, "projection");
		glUniformMatrix4fv(objectProjectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		unsigned int objectViewLoc = glGetUniformLocation(lightingShader, "view");
		glUniformMatrix4fv(objectViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f));
		
		unsigned int objectModelLoc = glGetUniformLocation(lightingShader, "model");
		glUniformMatrix4fv(objectModelLoc, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(lightVAO);
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
	std::cout << camera.Position.x << " " << camera.Position.y << " " << camera.Position.z << std::endl;
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}