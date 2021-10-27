#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "BulletManager.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

int main() {
	constexpr int BulletAmount = 20;
	constexpr float BulletStartDirectionXCoord = -0.8f;
	constexpr float BulletEndDirectionXCoord = 0.8f;
	constexpr int WallsCollsAmount = 100;
	constexpr int WallsRowsAmount = 10;
	constexpr int WallCoordsNumber = 2;
	constexpr int WallsCoordsAmount = WallsRowsAmount * WallsCollsAmount * WallCoordsNumber;
	constexpr float WallsStartXCoord = 480.0f;
	constexpr float WallsStartYCoord = 810.0f;
	constexpr float Speed = 3.0f;
	constexpr float LifeTime = 15.0f;

	GLFWwindow* window;

	// Initialize the library
	if (!glfwInit()) {
		return -1;
	}

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Window", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Make the window's context current
	glfwMakeContextCurrent(window);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	// specifies the part of the window to which OpenGL will draw (in pixels), convert from normalised to pixels
	glMatrixMode(GL_PROJECTION);
	// projection matrix defines the properties of the camera that views the objects in the world coordinate frame. Here you typically set the zoom factor, aspect ratio and the near and far clipping planes
	glLoadIdentity();
	// replace the current matrix with the identity matrix and starts us a fresh because matrix transforms such as glOrpho and glRotate cumulate, basically puts us at (0, 0, 0)
	glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, 0, 1); // essentially set coordinate system
	glMatrixMode(GL_MODELVIEW);
	// (default matrix mode) modelview matrix defines how your objects are transformed (meaning translation, rotation and scaling) in your world
	glLoadIdentity(); // same as above comment

	std::vector<float> WallCoords;
	WallCoords.reserve(WallsCollsAmount);

	float CurrentXCoord = WallsStartXCoord;
	float CurrentYCoord = WallsStartYCoord;
	for (int i = 0; i < WallsCoordsAmount; i += WallCoordsNumber) {
		if (i % (WallsCollsAmount * WallCoordsNumber) == 0) {
			CurrentXCoord = WallsStartXCoord;
			CurrentYCoord += 10;
		}
		WallCoords.push_back(CurrentXCoord);
		WallCoords.push_back(CurrentYCoord);
		CurrentXCoord += 10;
	}

	BulletManager* Manager = new BulletManager(WallCoords);

	int nbFrames = 0;
	const Float2 StartPos = Float2{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	float StartTime = glfwGetTime();

	const float XCoordRange = abs(BulletStartDirectionXCoord) + abs(BulletEndDirectionXCoord);
	const float XCoordRangeStep = XCoordRange / BulletAmount;
	for (int i = 0; i < BulletAmount; ++i) {
		const float CurrentXCoord = BulletStartDirectionXCoord + i * XCoordRangeStep;
		const float CurrentYCoord = sqrt(1 - CurrentXCoord * CurrentXCoord);
		const Float2 StartDirection = Float2{ CurrentXCoord, CurrentYCoord };
		Manager->Fire(StartPos, StartDirection, LifeTime, StartTime, Speed);
	}

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {
		const float CurrentTime = glfwGetTime();
		nbFrames++;
		if (CurrentTime - StartTime >= 1.0) {
			printf("%f ms/frame\n", 1000.0 / static_cast<float>(nbFrames));
			nbFrames = 0;
			StartTime += 1.0;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		// Render OpenGL here
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
		glPushAttrib(GL_LINE_BIT);
		glLineWidth(1);
		glPointSize(1);
		glEnableClientState(GL_VERTEX_ARRAY);

		Manager->Update(CurrentTime);

		const std::vector<Bullet>& Bullets = Manager->GetBullets();
		for (const Bullet& b : Bullets) 
		{
			const GLfloat pointVertex[] = { b.Position.X, b.Position.Y };
			glVertexPointer(2, GL_FLOAT, 0, pointVertex);
			glDrawArrays(GL_POINTS, 0, 1);
		}

		const std::vector<float>& CurrentWalls = Manager->GetWalls();
		std::vector<GLfloat> LineVertices;
		const size_t WallsDoubleNumber = CurrentWalls.size();
		LineVertices.reserve(WallsDoubleNumber);
		for (int i = 0; i < WallsDoubleNumber; ++i) {
			LineVertices.push_back({ CurrentWalls[i] });
		}

		glVertexPointer(WallCoordsNumber, GL_FLOAT, 0, LineVertices.data());
		glDrawArrays(GL_LINES, 0, WallsDoubleNumber / 2);

		glDisableClientState(GL_VERTEX_ARRAY);
		glPopAttrib();
		glDisable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);


		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
