cmake_minimum_required(VERSION 3.20)
project(Pong)

set(CMAKE_CXX_STANDARD 14)

include_directories(GL)
include_directories(Pong)

target_link_libraries(OpenGL GL/Glaux.lib GL/GLU32.LIB GL/glui32.lib GL/glut32.lib GL/OPENGL32.LIB)

add_executable(Pong
        Pong/AABB.h
        Pong/AABBImpl.cpp
        Pong/AABBImpl.h
        Pong/AABBTree.cpp
        Pong/AABBTree.h
        Pong/BulletManager.cpp
        Pong/BulletManager.h
        Pong/Header.h
        Pong/IAABB.h
        Pong/Pong.cpp)
