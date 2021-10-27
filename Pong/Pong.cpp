#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// #include "BulletManager.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define SEGMENT_COORDS_AMOUNT 4

struct Float2 {
	float X;
	float Y;
};

struct Point {
	Float2 Position;
	Float2 Direction;
	float LifeTime;
	float BirthTime;

	bool IsAlive(float CurrentTime) const {
		return BirthTime + LifeTime > CurrentTime;
	}
};

bool Compare(float a, float b) {
	return std::fabs(a - b) < 0.1;
}

float Area(Float2 A, Float2 B, Float2 C) {
	return (B.X - A.X) * (C.Y - A.Y) - (B.Y - A.Y) * (C.X - A.X);
}

bool Intersect(float a, float b, float c, float d) {
	if (a > b) std::swap(a, b);
	if (c > d) std::swap(c, d);
	return std::max(a, c) <= std::min(b, d);
}

bool Intersect(Float2 A, Float2 B, Float2 C, Float2 D) {
	return Intersect(A.X, B.X, C.X, D.X)
		&& Intersect(A.Y, B.Y, C.Y, D.Y)
		&& Area(A, B, C) * Area(A, B, D) <= 0
		&& Area(C, D, A) * Area(C, D, B) <= 0;
}

bool DoesPointHitTheWall(const std::vector<float>& Walls, const Float2& Point, const Float2& PrevPoint, size_t i) {
	return Intersect(Float2{ Walls[i], Walls[i + 1] }, Float2{ Walls[i + 2], Walls[i + 3] },
		Float2{ PrevPoint.X, PrevPoint.Y },
		Float2{ Point.X, Point.Y });
}


int DoesHitTheWall(const std::vector<float>& Walls, const Float2& Point, const Float2& PrevPoint) {
	for (size_t i = 0; i < Walls.size(); i += SEGMENT_COORDS_AMOUNT) {
		if (DoesPointHitTheWall(Walls, Point, PrevPoint, i)) {
			return i;
		}
	}
	return -1;
}

int main() {
	constexpr int BulletAmount = 400;
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
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "TestTask Window", NULL, NULL);

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

	int nbFrames = 0;
	const Float2 StartPos = Float2{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	std::vector<Point> Points;
	Points.reserve(BulletAmount);
	float StartTime = glfwGetTime();

	const float XCoordRange = abs(BulletStartDirectionXCoord) + abs(BulletEndDirectionXCoord);
	const float XCoordRangeStep = XCoordRange / BulletAmount;
	for (int i = 0; i < BulletAmount; ++i) {
		const float CurrentXCoord = BulletStartDirectionXCoord + i * XCoordRangeStep;
		const float CurrentYCoord = sqrt(1 - CurrentXCoord * CurrentXCoord);
		const Float2 StartDirection = Float2{ CurrentXCoord, CurrentYCoord };
		const float StartDirLength = sqrt(StartDirection.X * StartDirection.X + StartDirection.Y * StartDirection.Y);
		const Float2 StartDirNormalized = { StartDirection.X / StartDirLength, StartDirection.Y / StartDirLength };
		Points.push_back({ StartPos, StartDirNormalized, LifeTime, StartTime });
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

		for (Point& p : Points) {
			if (p.IsAlive(CurrentTime)) {
				const GLfloat pointVertex[] = { p.Position.X, p.Position.Y };
				glVertexPointer(2, GL_FLOAT, 0, pointVertex); // point to the vertices to be used
				glDrawArrays(GL_POINTS, 0, 1);
				Float2 PrevPos = { p.Position.X, p.Position.Y };
				p.Position.X += p.Direction.X * Speed;
				p.Position.Y += p.Direction.Y * Speed;

				const int HitCoord = DoesHitTheWall(WallCoords, p.Position, PrevPos);
				if (HitCoord != -1) {
					const Float2 FallingVector = { p.Position.X - PrevPos.X, p.Position.Y - PrevPos.Y };
					const Float2 Normal = {
						WallCoords[HitCoord + 1] - WallCoords[HitCoord + 3],
						WallCoords[HitCoord + 2] - WallCoords[HitCoord]
					};
					const float NormalLength = sqrt(Normal.X * Normal.X + Normal.Y * Normal.Y);
					const Float2 NormalNormalized = { Normal.X / NormalLength, Normal.Y / NormalLength };
					const float floatDotProduct = 2 * (FallingVector.X * NormalNormalized.X + FallingVector.Y *
						NormalNormalized.Y);
					const Float2 Subtrahend = {
						floatDotProduct * NormalNormalized.X, floatDotProduct * NormalNormalized.Y
					};
					const Float2 Reflected = { FallingVector.X - Subtrahend.X, FallingVector.Y - Subtrahend.Y };
					const float ReflectedLength = sqrt(Reflected.X * Reflected.X + Reflected.Y * Reflected.Y);
					const Float2 NormalizedReflected = { Reflected.X / ReflectedLength, Reflected.Y / ReflectedLength };
					p.Direction.X = NormalizedReflected.X;
					p.Direction.Y = NormalizedReflected.Y;

					for (int i = 0; i < SEGMENT_COORDS_AMOUNT; ++i) {
						WallCoords.erase(WallCoords.begin() + HitCoord);
					}
				}
			}
		}

		std::vector<GLfloat> lineVertices;
		const size_t WallsDoubleNumber = WallCoords.size();
		lineVertices.reserve(WallsDoubleNumber);
		for (int i = 0; i < WallsDoubleNumber; ++i) {
			lineVertices.push_back({ WallCoords[i] });
		}

		glVertexPointer(WallCoordsNumber, GL_FLOAT, 0, lineVertices.data());
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
