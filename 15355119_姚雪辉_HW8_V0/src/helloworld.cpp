#include "stdafx.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <cmath>
//#define DEBUG
#define DEBUGD

class point
{
public:
	GLfloat x;
	GLfloat y;
	point() {
		x = 0;
		y = 0;
	}
	point(GLfloat inputX,GLfloat inputY) {
		x = inputX;
		y = inputY;
	}
};
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void getBezier(int inputPointSize,int numOfPointToDraw);
float B(int i, int n, float T);
point getPoint(point A, point B, float T);
void drawDynamicLine(std::vector<point>, float deltaT);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
int size = SCR_WIDTH;

bool isDynamic = false;
int pointNum = 0;
GLfloat mouse_x, mouse_y;//表示当前鼠标位置
std::vector<point> inputPoint;//鼠标点击的位置
std::vector<point> outputPoint(1001);//贝塞尔曲线的点

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

int fac(int x)  
{
	int f;
	if (x == 0 || x == 1)
		f = 1;
	else
		f = fac(x - 1)*x;
	return f;
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

void drawLine(point begin,point end) {
	float vertices[] = {
		begin.x,begin.y,0.0f, 0.0f,0.0f,0.0f,
		end.x, end.y, 0.0f, 0.0f, 0.0f, 0.0f
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
	glDrawArrays(GL_LINES, 0, 2);
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

int main()
{
#ifdef DEBUG
	std::cout << B(0, 3, 0.5) << std::endl;
#endif // DEBUG

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
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}


	genShaderProgram();

	while (!glfwWindowShouldClose(window))
	{

		processInput(window);
		glfwPollEvents();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (inputPoint.size() > 1) {
			for (int i = 0; i < inputPoint.size() - 1; i++) {
				drawLine(inputPoint[i], inputPoint[i + 1]);
			}
		}

		if (outputPoint.size() > 1) {
			for (int i = 0; i < outputPoint.size(); i++) {
				drawPoint(outputPoint[i].x, outputPoint[i].y);
#ifdef DEBUG
				//std::cout << outputPoint[i].x << " " << outputPoint[i].y << std::endl;
#endif
			}
		}

		if (isDynamic) {
			float currentTime = (float)glfwGetTime();
			float deltaT = currentTime - int(currentTime / 4) * 4;
			std::vector<point> tempPoint = inputPoint;
			drawDynamicLine(tempPoint,deltaT);
		}

		glfwSwapBuffers(window);
	}

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

void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
	mouse_x = xpos;
	mouse_y = ypos;
#ifdef DEBUG
	//std::cout << mouse_x << " " << mouse_y << " " << std::endl;
#endif
}


void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		auto tempX = float(mouse_x - (SCR_WIDTH / 2)) / (SCR_WIDTH / 2);
		auto tempY = float((SCR_HEIGHT / 2) - mouse_y) / (SCR_HEIGHT / 2);
		point tempPoint(tempX, tempY);
		inputPoint.push_back(tempPoint);
		getBezier(inputPoint.size(), 500);

#ifdef DEBUG
	std::cout << "clicked : " << tempX << " " << tempY << " " << std::endl;
#endif
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		inputPoint.pop_back();
		getBezier(inputPoint.size(), 500);
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		isDynamic = true;
	}
}


float B(int i, int n, float T) {
#ifdef DEBUG
	std::cout << "fac(n) = " << fac(n) << std::endl;
	std::cout << "fac(i) = " << fac(i) << std::endl;
	std::cout << "fac(n-i) = " << fac(n-i) << std::endl;
	std::cout << "pow(T,i) = " << pow(T, i) << std::endl;
	std::cout << "pow(1-T, n-i) = " << pow(1-T, n-i) << std::endl;
#endif

	return fac(n)*1.0f / (fac(i)*fac(n - i)) * pow(T, i)*pow(1 - T, n - i);
}

void getBezier(int inputPointSize,int numOfPointToDraw) {
	float deltaT = 1.0 / numOfPointToDraw;
	float T;
	for (int i = 0; i < numOfPointToDraw; i++) {
		float tempX = 0;
		float tempY = 0;
		T = i * deltaT;
		for (int j = 0; j < inputPointSize; j++) {
#ifdef DEBUG
			std::cout << "B = " << B(j, inputPointSize, T) << std::endl;
#endif 
			tempX += B(j, inputPointSize - 1, T) * inputPoint[j].x;
			tempY += B(j, inputPointSize - 1, T) * inputPoint[j].y;
		}
#ifdef DEBUG
		std::cout <<"tempX = " << tempX << " " << "tempY = " <<tempY << std::endl;
#endif 

		outputPoint[i] = point(tempX, tempY);
	}
#ifdef DEBUG
		std::cout << "getBezier" << std::endl;
		std::cout << "outputPoint.size()" << outputPoint.size() << std::endl;
#endif
}

point getPoint(point A, point B, float t) {
	float k = (B.y - A.y)*1.0f / (B.x - A.x);
	float deltaX = (B.x - A.x)*1.0f / 500;
	float currentX = A.x + t / 4 * deltaX * 500;
	float curretnY = A.y + t / 4 * deltaX * 500 * k;
	return point(currentX, curretnY);
}

void drawDynamicLine(std::vector<point> tempPoint, float deltaT) {

	if (tempPoint.size() < 3) {
		return;
	}
	std::vector<point> ttempPoint;
	for (int i = 0; i < tempPoint.size() - 2; i++) {
		drawLine(getPoint(tempPoint[i], tempPoint[i + 1], deltaT), getPoint(tempPoint[i + 1], tempPoint[i + 2], deltaT));
		if (i == 0) {
			ttempPoint.push_back(getPoint(tempPoint[i], tempPoint[i + 1], deltaT));
		}
		ttempPoint.push_back(getPoint(tempPoint[i + 1], tempPoint[i + 2], deltaT));
	}
	drawDynamicLine(ttempPoint, deltaT);
}