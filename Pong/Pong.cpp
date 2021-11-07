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
	constexpr int WallMaxLength = 500;
	constexpr int WallMinLength = 30;
	constexpr int WallsAmount = 1000;
	constexpr int WallCoordsNumber = 2;
	constexpr int NumberOfFires = 10;
	constexpr float Speed = 5.0f;
	constexpr float LifeTime = 60.0f;

	GLFWwindow* window;

	if (!glfwInit())
	{
		return -1;
	}

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Window", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	glMatrixMode(GL_PROJECTION);
	
	glLoadIdentity();
	
	glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, 0, 1); // essentially set coordinate system
	glMatrixMode(GL_MODELVIEW);
	
	glLoadIdentity();

	AABBTree* Tree = new AABBTree(1);

	for (int i = 0; i < WallsAmount; ++i)
	{
		float FirstXCoord = 0.0;
		float SecondXCoord = 0.0;
		float FirstYCoord = 0.0;
		float SecondYCoord = 0.0;
		float CurrentWallLength = 0.0;
		while (CurrentWallLength < WallMinLength || CurrentWallLength > WallMaxLength)
		{
			FirstXCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_WIDTH);
			SecondXCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_WIDTH);
			FirstYCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_HEIGHT);
			SecondYCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_HEIGHT);

			CurrentWallLength = sqrt(
				(SecondXCoord - FirstXCoord) * (SecondXCoord - FirstXCoord) + (SecondYCoord - FirstYCoord) * (
					SecondYCoord -
					FirstYCoord));
		}

		Tree->InsertObject(
			std::make_shared<AABBImpl>(FirstXCoord, SecondXCoord, FirstYCoord, SecondYCoord));
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
			++FireCount;
			std::future<void> f = std::async([BulletAmount, Manager, LifeTime, Speed, FireCount]
			{
				const float StartPosXCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_WIDTH);
				const float StartPosYCoord = static_cast<float>(std::rand()) / (RAND_MAX / SCREEN_HEIGHT);

				const Float2 StartPosition = Float2{StartPosXCoord, StartPosYCoord};
				for (int i = 0; i < BulletAmount; ++i)
				{
					const float CurrentXCoord = -1.0f + static_cast<float>(std::rand()) / (RAND_MAX / 2.0f);
					const float CurrentYCoord = -1.0f + static_cast<float>(std::rand()) / (RAND_MAX / 2.0f);
					const Float2 StartDirection = Float2{CurrentXCoord, CurrentYCoord};
					Manager->Fire(StartPosition, StartDirection, glfwGetTime(), LifeTime / FireCount, Speed);
				}
			});
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

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}
