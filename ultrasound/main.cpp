#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "shader.h"
#include "camera.h"

#include "densitymap.h"

#define PI 3.141592653589

// If this is false, then the application will run in windowed mode
// If this is true, then the application will run in fullscren mode
#define FULLSCREEN 0

// Adjust these numbers depending on your monitor resolution
#if FULLSCREEN
#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080
#else
#define SCR_WIDTH 800
#define SCR_HEIGHT 600
#endif

// Keyboard and mouse input functions
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void processKeyboardInput(GLFWwindow* window);

// Demo functions to show what the volume map looks like
void sphereDemo(DensityMap& grid);
void fanDemo(DensityMap& grid);

// Used in the mouse movement callback
struct MouseData {
	double lastMouseX;
	double lastMouseY;
	bool firstMouse;
};

// Creating a Camera object
Camera cam;

int main() {
	// Window title
	std::string windowTitle = "Ultrasound";

	// Initializing the OpenGL context
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Creating the window object
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowTitle.c_str(), FULLSCREEN ? glfwGetPrimaryMonitor() : NULL, NULL);
	
	// If the window is not created (for any reason)
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Setting callbacks
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, cursorPosCallback);

	// Lock the cursor to the center of the window
	// and make it invisible
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
	// Load the OpenGL functions from the graphics card
	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Creating the shaders for the cells in the cube
	// and for the lines of the border of the cube
	Shader cellShader("cells.vs", "cells.fs");
	Shader lineShader("lines.vs", "lines.fs");

	// Allows blending (translucent drawing)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initializing mouse info
	MouseData mouseData;
	mouseData.lastMouseX = SCR_WIDTH / 2.0;
	mouseData.lastMouseY = SCR_HEIGHT / 2.0;
	mouseData.firstMouse = true;

	glfwSetWindowUserPointer(window, &mouseData);
	
	// Creating the density map
	int dim = 31;
	DensityMap grid(dim);

	// (Optional) Adds a fan-shaped arrangement of cells to the volume map
	fanDemo(grid);

	// Get the vertices from the volume map
	// in a form useful to OpenGL
	std::vector<float> cellVertices = grid.getVertices();
	
	// Initializing the buffer storing the vertices
	// of the volume map on the graphics card
	unsigned int cellVAO, cellVBO;
	glGenBuffers(1, &cellVBO);
	glGenVertexArrays(1, &cellVAO);

	glBindVertexArray(cellVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cellVBO);
	glBufferData(GL_ARRAY_BUFFER, cellVertices.size() * sizeof(float), cellVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Array containing the coordinates of the vertices
	// of the white lines
	float lines[72] = {
		-5, -5, -5,
		 5, -5, -5,

		-5,  5, -5,
		 5,  5, -5,

		-5, -5,  5,
		 5, -5,  5,

		-5,  5,  5,
		 5,  5,  5,

		 // -----

		-5, -5, -5,
		-5,  5, -5,

		 5, -5, -5,
		 5,  5, -5,

		-5, -5,  5,
		-5,  5,  5,

		 5, -5,  5,
		 5,  5,  5,

		 // -----

		-5, -5, -5,
		-5, -5,  5,

		 5, -5, -5,
		 5, -5,  5,

		-5,  5, -5,
		-5,  5,  5,
		
		 5,  5, -5,
		 5,  5,  5,
	};

	// Initializing the buffer storing the vertices
	// of the white lines on the graphics card
	unsigned int lineVAO, lineVBO;
	glGenBuffers(1, &lineVBO);
	glGenVertexArrays(1, &lineVAO);

	glBindVertexArray(lineVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	// Main event loop
	while (!glfwWindowShouldClose(window)) {
		double currentFrame = glfwGetTime();
		cam.deltaTime = currentFrame - cam.lastFrame;
		cam.lastFrame = currentFrame;

		// Self-explanatory
		processKeyboardInput(window);

		// Clears the screen and fills it a dark grey color
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Creating matrices to transform the vertices into NDC (screen) coordinates
		// between -1 and 1 that OpenGL can use
		glm::dmat4 projection = glm::perspective(glm::radians(cam.fov), double(SCR_WIDTH) / SCR_HEIGHT, 0.01, 500.0);
		glm::dmat4 camView = cam.getViewMatrix();
		glm::dmat4 model = glm::scale(glm::dmat4{}, glm::dvec3(10.0 / (grid.dim - 1), 10.0 / (grid.dim - 1), 10.0 / (grid.dim - 1)));
		model = glm::translate(model, glm::dvec3(-(grid.dim - 1) / 2.0, -(grid.dim - 1) / 2.0, -(grid.dim - 1) / 2.0));

		// Drawing the volume map
		cellShader.use();
		cellShader.setMat4("projection", projection);
		cellShader.setMat4("view", camView);
		cellShader.setMat4("model", model);

		glBindVertexArray(cellVAO);
		glDrawArrays(GL_TRIANGLES, 0, cellVertices.size() / 4);

		// Drawing the white lines
		lineShader.use();
		lineShader.setMat4("projection", projection);
		lineShader.setMat4("view", camView);
		lineShader.setMat4("model", glm::dmat4{});

		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINES, 0, 24);

		cam.prevPos = cam.position;

		// Update the screen
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// GLFW cleanup
	glfwTerminate();

	// Stops the window from closing immediately
	system("pause");
}

void processKeyboardInput(GLFWwindow *window) {
	// If shift is held down, then the camera moves faster
	bool sprinting = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);

	// WASD + Q and E movement
	if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W))
		cam.processKeyboard(FORWARD, sprinting);
	if (glfwGetKey(window, GLFW_KEY_S))
		cam.processKeyboard(BACKWARD, sprinting);
	if (glfwGetKey(window, GLFW_KEY_A))
		cam.processKeyboard(LEFT, sprinting);
	if (glfwGetKey(window, GLFW_KEY_D))
		cam.processKeyboard(RIGHT, sprinting);
	if (glfwGetKey(window, GLFW_KEY_Q))
		cam.processKeyboard(DOWN, sprinting);
	if (glfwGetKey(window, GLFW_KEY_E))
		cam.processKeyboard(UP, sprinting);

	// Hold C to zoom in
	if (glfwGetKey(window, GLFW_KEY_C)) {
		cam.fov = 30;
	}
	else {
		cam.fov = 70;
	}
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
	MouseData& mouseData = *(MouseData*)glfwGetWindowUserPointer(window);

	// Ensures that the viewer faces forward on startup
	if (mouseData.firstMouse) {
		mouseData.lastMouseX = xpos;
		mouseData.lastMouseY = ypos;
		mouseData.firstMouse = false;
	}

	// Updating the camera angle
	double xoffset = xpos - mouseData.lastMouseX;
	double yoffset = mouseData.lastMouseY - ypos;
	mouseData.lastMouseX = xpos;
	mouseData.lastMouseY = ypos;

	cam.processMouseMovement(xoffset, yoffset);
}

void sphereDemo(DensityMap& grid) {
	// Adds a sphere to the center of the volume map

	for (int i = 0; i < grid.dim; i++) {
		for (int j = 0; j < grid.dim; j++) {
			for (int k = 0; k < grid.dim; k++) {
				float xd = i - ((grid.dim - 1) / 2.0);
				float yd = j - ((grid.dim - 1) / 2.0);
				float zd = k - ((grid.dim - 1) / 2.0);

				float mxd = (grid.dim - 1) / 2.0;
				float myd = (grid.dim - 1) / 2.0;
				float mzd = (grid.dim - 1) / 2.0;

				float distance = sqrt(xd * xd + yd * yd + zd * zd);
				float maxDistance = sqrt(mxd * mxd + myd * myd + mzd * mzd);
				float shade = (maxDistance - distance) / maxDistance;
				shade = shade * shade;

				grid.cells[i][j][k] = shade;
			}
		}
	}
}

void fanDemo(DensityMap& grid) {
	// Adds a fan shape to the volume map
	// using the DensityMap.addLine() function

	glm::vec3 vertex = { 0.5, 0.5, 0.5 };

	float a1 = 1;
	float a2 = 1;

	float r = 0.3;

	for (; a2 <= 3; a2 += 0.05) {
		float x = r * sin(a1) * cos(a2);
		float y = r * sin(a1) * sin(a2);
		float z = r * cos(a1);

		std::vector<float> vals;

		for (int i = 0; i < 50; i++) {
			vals.push_back(1);
		}

		grid.addLine(vertex, vertex + glm::vec3(x, y, z), vals);
	}
}