#include "stdafx.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "Vector3.h"
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const unsigned int NUM_OF_POINT = 1001;
int size = SCR_WIDTH;

const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos, 1.0);\n"
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

int shaderProgram;

void bresenham(float y[], int p0, int numOfPoints, int deltaX, int deltaY) {
	if (numOfPoints < 1) {
		return;
	}
	for (int i = 0; i < numOfPoints; i++) {
		int pnext;
		if (p0 <= 0) {
			y[i + 1] = y[i];
			pnext = p0 + 2 * deltaY;
		}
		else {
			y[i + 1] = y[i] + 1;
			pnext = p0 + 2 * deltaY - 2 * deltaX;
		}
		p0 = pnext;
	}
}

float normlizeX(int inputX) {
	return float(inputX) / SCR_WIDTH;
}

float normlizeY(int inputY) {
	return float(inputY) / SCR_HEIGHT;
}

int getAllPointsBetweenTwoPoints(int x0, int y0, int x1, int y1, float x[], float y[]) {
	int deltaX;
	int deltaY;
	int doubleDeltaY;
	int doubleDeltaYMinusDoubleDeltaX;
	int p0;
	int numOfPoints = 0;

	if (x0 == x1) {
		if (y0 > y1) {
			std::swap(y0, y1);
		}
		numOfPoints = y1 - y0 + 1;
		for (int i = 0; i < numOfPoints; i++) {
			x[i] = x0;
			y[i] = y0 + i;
		}
	}
	else {
		deltaX = fabs(x0 - x1);
		deltaY = fabs(y0 - y1);
		doubleDeltaY = 2 * deltaY;
		doubleDeltaYMinusDoubleDeltaX = 2 * deltaY - 2 * deltaX;
		p0 = 2 * deltaY - deltaX;
		float slope = float(y1 - y0) / float(x1 - x0);
		if (slope <= 1 && slope >= 0) {
			if (x1 < x0) {
				std::swap(x1, x0);
				std::swap(y1, y0);
			}
			numOfPoints = x1 - x0 + 1;

			for (int i = 0; i<numOfPoints; i++) {
				x[i] = x0 + i;
			}
			y[0] = y0;
			y[numOfPoints - 1] = y1;
			bresenham(y, p0, numOfPoints, deltaX, deltaY);
		}
		else if (slope < 0 && slope >= -1) {
			if (x1 < x0) {
				std::swap(x1, x0);
				std::swap(y1, y0);
			}
			numOfPoints = x1 - x0 + 1;
			for (int i = 0; i<numOfPoints; i++) {
				x[i] = x0 + i;
			}
			y[0] = -y0;
			y[numOfPoints - 1] = -y1;
			bresenham(y, p0, numOfPoints, deltaX, deltaY);
			for (int i = 0; i < numOfPoints; i++) {
				y[i] *= -1;
			}
		}
		else if (slope > 1) {
			if (y0 > y1) {
				std::swap(x0, x1);
				std::swap(y0, y1);
			}
			numOfPoints = y1 - y0 + 1;
			for (int i = 0; i < numOfPoints; i++) {
				y[i] = y0 + i;
			}
			x[0] = x0;
			x[numOfPoints - 1] = x1;
			p0 = 2 * deltaX - deltaY;
			bresenham(x, p0, numOfPoints, deltaY, deltaX);
		}
		else {
			if (y0 > y1) {
				std::swap(x0, x1);
				std::swap(y0, y1);
			}
			numOfPoints = y1 - y0 + 1;
			for (int i = 0; i < numOfPoints; i++) {
				y[i] = y0 + i;
			}
			x[0] = -x0;
			x[numOfPoints - 1] = -x1;
			p0 = 2 * deltaX - deltaY;
			bresenham(x, p0, numOfPoints, deltaY, deltaX);
			for (int i = 0; i < numOfPoints; i++) {
				x[i] *= -1;
			}
		}
	}
	return numOfPoints;
}

void genShaderProgram() {
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
	shaderProgram = glCreateProgram();
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
}

void drawPoint(float x, float y) {
	float vertices[] = {
		x,y,0.0f, 0.0f,0.0f,0.0f
	};
	unsigned int VAO;
	unsigned int VBO;
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
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, 1);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void drawPoint(float x, float y,float r,float g,float b) {
	float vertices[] = {
		x,y,0.0f, r,g,b
	};
	unsigned int VAO;
	unsigned int VBO;
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
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, 1);
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void drawLine(float x0, float y0, float x1, float y1) {
	float lineX[NUM_OF_POINT];
	float lineY[NUM_OF_POINT];
	int numOfPoints = getAllPointsBetweenTwoPoints(x0, y0, x1, y1, lineX, lineY);
	for (int i = 0; i < numOfPoints; i++) {
		lineX[i] = normlizeX(lineX[i]);
		lineY[i] = normlizeY(lineY[i]);
	}
	for (int i = 0; i < numOfPoints; i++) {
		drawPoint(lineX[i], lineY[i]);
	}
}

void drawLine(float x0, float y0,float x1,float y1,float r,float g,float b) {
	float lineX[NUM_OF_POINT];
	float lineY[NUM_OF_POINT];
	int numOfPoints = getAllPointsBetweenTwoPoints(x0, y0, x1, y1, lineX, lineY);
	for (int i = 0; i < numOfPoints; i++) {
		lineX[i] = normlizeX(lineX[i]);
		lineY[i] = normlizeY(lineY[i]);
	}
	for (int i = 0; i < numOfPoints; i++) {
		drawPoint(lineX[i], lineY[i], r, g, b);
	}
}

void drawCircle(float inputX, float inputY, int inputR) {
	float newX = normlizeX(inputX);
	float newY = normlizeY(inputY);
	int x = 0;
	int y = inputR;
	int d = 1 - inputR;

	while (y >= x) {
		float realX = normlizeX(x);
		float realY = normlizeY(y);

		drawPoint(newX + realX, newY + realY);
		drawPoint(newX + realY, newY + realX);
		drawPoint(newX + realY, newY - realX);
		drawPoint(newX + realX, newY - realY);
		drawPoint(newX - realX, newY - realY);
		drawPoint(newX - realY, newY - realX);
		drawPoint(newX - realY, newY + realX);
		drawPoint(newX - realX, newY + realY);

		if (d < 0) {
			d = d + 2 * x + 3;
		}
		else {
			d = d + 2 * (x - y) + 5;
			y--;
		}
		x++;
	}
}

int maxOfThree(int a, int b, int c) {
	return std::max(a, std::max(b, c));
}

int minOfThree(int a, int b, int c) {
	return std::min(a, std::min(b, c));
}

bool isPointinTriangle(Vector3 A, Vector3 B, Vector3 C, Vector3 P)
{
	Vector3 v0 = C - A;
	Vector3 v1 = B - A;
	Vector3 v2 = P - A;

	float dot00 = v0.Dot(v0);
	float dot01 = v0.Dot(v1);
	float dot02 = v0.Dot(v2);
	float dot11 = v1.Dot(v1);
	float dot12 = v1.Dot(v2);

	float inverDeno = 1 / (dot00 * dot11 - dot01 * dot01);

	float u = (dot11 * dot02 - dot01 * dot12) * inverDeno;
	if (u < 0 || u > 1) // if u out of range, return directly
	{
		return false;
	}

	float v = (dot00 * dot12 - dot01 * dot02) * inverDeno;
	if (v < 0 || v > 1) // if v out of range, return directly
	{
		return false;
	}

	return u + v <= 1;
}

void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, float r, float g, float b) {
	int minX = minOfThree(x0, x1, x2);
	int minY = minOfThree(y0, y1, y2);
	int maxX = maxOfThree(x0, x1, x2);
	int maxY = maxOfThree(y0, y1, y2);
	Vector3 A = Vector3(x0, y0, 0);
	Vector3 B = Vector3(x1, y1, 0);
	Vector3 C = Vector3(x2, y2, 0);
	Vector3 P = Vector3(0, 0, 0);
	for (int i = minX; i <= maxX; i++) {
		for (int j = minY; j <= maxY; j++) {
			Vector3 P = Vector3(i, j, 0);
			if (isPointinTriangle(A, B, C, P)) {
				float newX = normlizeX(i);
				float newY = normlizeY(j);
				drawPoint(newX, newY, r, g, b);
			}
		}
	}
}

int main()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

														 
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

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

	genShaderProgram();

	bool isLine = true;
	bool isCircle = false;
	bool isOpen = true;
	bool isFillTriangle = false;
	int x0 = 0, y0 = 100;
	int x1 = 50, y1 = -100;
	int x2 = -50, y2 = -100;
	int radius = 100;
	int circleX = 0, circleY = 0;
	ImVec4 color = ImVec4(1, 0.1, 0.1, 1);
	while (!glfwWindowShouldClose(window))
	{

		processInput(window);
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("DrawLine and DrawCircle", &isOpen,ImGuiWindowFlags_MenuBar);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("DrawLine")) {
					isLine = true;
					isCircle = false;
				}
				if (ImGui::MenuItem("DrawCircle")) {
					isLine = false;
					isCircle = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		if (isLine) {
			ImGui::SliderInt("x0", &x0, -size, size);
			ImGui::SliderInt("y0", &y0, -size, size);
			ImGui::SliderInt("x1", &x1, -size, size);
			ImGui::SliderInt("y1", &y1, -size, size);
			ImGui::SliderInt("x2", &x2, -size, size);
			ImGui::SliderInt("y2", &y2, -size, size);
			ImGui::Checkbox("fillTriangle", &isFillTriangle);
			ImGui::ColorEdit3("color", (float*)&color);
		}
		if (isCircle) {
			ImGui::SliderInt("circleX", &circleX, -size, size);            // 设置圆圆心的横坐标，范围为【-500，500】
			ImGui::SliderInt("circleY", &circleY, -size, size);            // 设置圆圆心的纵坐标，范围为【-500，500】
			ImGui::SliderInt("radius", &radius, 0, size);
		}
		ImGui::End();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (isLine) {
			drawLine(x0, y0, x1, y1, color.x, color.y, color.z);
			drawLine(x1, y1, x2, y2, color.x, color.y, color.z);
			drawLine(x2, y2, x0, y0, color.x, color.y, color.z);
			if (isFillTriangle) {
				fillTriangle(x0, y0, x1, y1, x2, y2, color.x, color.y, color.z);
			}
		}

		if (isCircle) {
			drawCircle(circleX, circleY, radius);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
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