#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <future>
#include "AABBImpl.h"
#include "BulletManager.h"

// #define SCREEN_WIDTH 1920
// #define SCREEN_HEIGHT 1080
#define SCREEN_WIDTH 3440
#define SCREEN_HEIGHT 1440

int main()
{
	constexpr int BulletAmount = 100;
	constexpr int BulletThreadsAmount = 1;
	constexpr int BulletPerThreadAmount = BulletAmount / BulletThreadsAmount;
	constexpr int WallLength = 10;
	constexpr int DistanceBetweenWalls = 10;
	constexpr int WallsCollsAmount = 100;
	constexpr int WallsRowsAmount = 10;
	constexpr int WallCoordsNumber = 2;
	constexpr int WallsCoordsAmount = WallsRowsAmount * WallsCollsAmount * WallCoordsNumber;
	constexpr int NumberOfFires = 5;
	constexpr float WallsStartXCoord = 600.0f;
	constexpr float WallsStartYCoord = 810.0f;
	float Speed = 2.0f;
	constexpr float LifeTime = 15.0f;

	GLFWwindow* window;

	// Initialize the library
	if (!glfwInit())
	{
		return -1;
	}

	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Window", NULL, NULL);

	if (!window)
	{
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

	AABBTree* Tree = new AABBTree(1);

	float CurrentXCoord = WallsStartXCoord;
	float CurrentYCoord = WallsStartYCoord;
	for (int i = 0; i < WallsCoordsAmount; i += WallCoordsNumber)
	// for (int i = 0; i < WallsCollsAmount; ++i)
	{
		if (i != 0 && i % (WallsCollsAmount * WallCoordsNumber) == 0)
		{
			CurrentXCoord = WallsStartXCoord;
			CurrentYCoord += 10;
		}
		// float FirstXCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_WIDTH);
		// float SecondXCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_WIDTH);
		// float FirstYCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_HEIGHT);
		// float SecondYCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_HEIGHT);

		Tree->InsertObject(
			// std::make_shared<AABBImpl>(FirstXCoord, SecondXCoord, FirstYCoord, SecondYCoord));
			std::make_shared<AABBImpl>(CurrentXCoord, CurrentXCoord + WallLength, CurrentYCoord, CurrentYCoord));
		CurrentXCoord += WallLength + DistanceBetweenWalls;
	}

	BulletManager* Manager = new BulletManager(*Tree);

	int nbFrames = 0;
	float StartTime = glfwGetTime();
	int FireCount = 0;
	float FireTime = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		double glfw_get_time = glfwGetTime();
		if (NumberOfFires > FireCount && glfw_get_time > FireTime)
		{
			std::future<void> f = std::async(
				[Manager,/* BulletStartDirectionXCoord, BulletEndDirectionXCoord, BulletAmount,*/ BulletPerThreadAmount,
					LifeTime, Speed, FireCount]
				{
					const float StartPosXCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_WIDTH);
					const float StartPosYCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_HEIGHT);
					const Float2 StartPosition = Float2{StartPosXCoord, StartPosYCoord};
					for (int i = 0; i < BulletPerThreadAmount; ++i)
					{
						const float CurrentXCoord = -1.0f + static_cast<float>(std::rand()) / (RAND_MAX / (1.0f - -
							1.0f));
						const float CurrentYCoord = -1.0f + static_cast<float>(std::rand()) / (RAND_MAX / (1.0f - -
							1.0f));
						const Float2 StartDirection = Float2{CurrentXCoord, CurrentYCoord};
						Manager->Fire(StartPosition, StartDirection, glfwGetTime(), LifeTime / FireCount, Speed);
					}
				});
			f.get();
			++FireCount;
			Speed *= 2;
			FireTime += 2;
		}

		const float CurrentTime = glfwGetTime();
		nbFrames++;
		if (CurrentTime - StartTime >= 1.0)
		{
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

		const std::vector<Bullet>& Bullets = Manager->GetBullets(CurrentTime);
		for (const Bullet& b : Bullets)
		{
			const GLfloat pointVertex[] = {b.Position.X, b.Position.Y};
			glVertexPointer(2, GL_FLOAT, 0, pointVertex);
			glDrawArrays(GL_POINTS, 0, 1);
		}

		const std::vector<float>& CurrentWalls = Manager->GetWalls();
		std::vector<GLfloat> LineVertices;
		const size_t WallsDoubleNumber = CurrentWalls.size();
		LineVertices.reserve(WallsDoubleNumber);
		for (int i = 0; i < WallsDoubleNumber; ++i)
		{
			LineVertices.push_back({CurrentWalls[i]});
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
