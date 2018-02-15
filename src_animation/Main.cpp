/////////////////////////////////////////////
//
// Skeletal Animation Tutorial
//
// (C) by Sven Forstmann in 2014
//
// License : MIT
// http://opensource.org/licenses/MIT
/////////////////////////////////////////////
// Mathlib included from 
// http://sourceforge.net/projects/nebuladevice/
/////////////////////////////////////////////
#include <fstream>
#include <iostream> 
#include <vector> 
#include <string> 
#include <stdio.h>
#include <glew.h>
#include <wglew.h>
#include <windows.h>
#include <mmsystem.h>
#include <GL/glut.h>
using namespace std;
#pragma comment(lib,"winmm.lib")

#include <sstream>
#include <iterator>
///////////////////////////////////////////
#include "core.h"
#include "Bmp.h"
#include "ogl.h"
#include "glsl.h"
///////////////////////////////////////////
vec4f lightvec(8, 6, 0, 0);
#include "Mesh.h"
///////////////////////////////////////////
#define WINDOWS_WIDTH 1280
#define WINDOWS_HEIGHT 800

#define FUNC_JOINT(joint, x, y, z) {\
	rot.set(1,0,0,\
			0,1,0,\
			0,0,1);\
	index = halo.animation.GetBoneIndexOf(joint);\
	m = halo.animation.bones[index].matrix;;\
	rot.rotate_z(z * PI / 180.0);\
	rot.rotate_y(y * PI / 180.0);\
	rot.rotate_x(x * PI / 180.0);\
	m.x_component() = rot.x_component();\
	m.y_component() = rot.y_component();\
	m.z_component() = rot.z_component();\
	halo.animation.bones[index].matrix = m;\
	loopi(0, halo.animation.bones[index].childs.size())\
	{\
		halo.animation.EvalSubtree(\
			halo.animation.bones[index].childs[i],\
			halo.animation.animations[0],\
			-1);\
	}\
};

#define FUNC_WRITE2D(joint) {\
	jndex = halo.animation.GetBoneIndexOf(joint);\
	j_pos = halo.animation.bones[jndex].matrix.m;\
	GetCoordinate(j_pos[3][0], j_pos[3][1], j_pos[3][2], x, y, z, 1);\
	outputFile << x << " " << y << "\n";\
};

void str2angle(std::string input, vec3f & output)
{
	std::vector<double> to_double;
	std::istringstream iss(input);
	copy(std::istream_iterator<double>(iss),
		std::istream_iterator<double>(),
		std::back_inserter(to_double));
	output.set(to_double[0], to_double[1], to_double[2]);
}

void GetCurrentDir(char* szPathTemp)
{
	GetModuleFileName(NULL, szPathTemp, 512);
	//find out the executing direction
	for (int i = strlen(szPathTemp); i >= 0; i--)
	{
		if (szPathTemp[i] == '\\')
		{
			szPathTemp[i] = '\0';
			break;
		}
	}
	return;
}

void GetCoordinate(float x, float y, float z, float& resultx, float& resulty, float& resultz, int intMode)
{
	GLint viewport[4];   //   
	GLdouble modelview[16]; //  
	GLdouble projection[16]; //  
	GLfloat winX, winY, winZ;  //  
	GLdouble posX, posY, posZ;  //  
	GLdouble Point3f[3];

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	if (intMode == 0)
	{
		winX = (float)x;
		winY = (float)viewport[3] - (float)y;
		glReadPixels(int(winX), int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
		resulty = (float)posY;
	}
	else if (intMode == 1)
	{
		winX = x;
		winY = y;
		winZ = z;
		gluProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
		resulty = (float)viewport[3] - (float)posY;
	}

	resultx = (float)posX;
	resultz = (float)posZ;
}

void DrawScene()
{
	clock_t tt = clock();
	bool isFreshed = false;

	static int frameCnt = 0;
	static bool fileFlag = false;

	static std::ifstream inputFile;
	static std::ofstream outputFile;
	static clock_t startTime;

	//for FUNC_JOINT
	int index;
	matrix44 m;
	matrix33 rot;

	if (GetAsyncKeyState(VK_ESCAPE))  exit(0);

	// mouse pointer position
	POINT cursor;
	GetCursorPos(&cursor);

	//// camera orientation
	//float	viewangle_x = float(cursor.x-1920/2)/4.0;
	//float	viewangle_y = float(cursor.y-1200/2)/4.0;

	//time
	static uint t0 = timeGetTime();
	double time_elapsed = double(timeGetTime() - t0) / 1000;

	// clear and basic
	glClearDepth(1);
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// projection
	int vp[4];
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glGetIntegerv(GL_VIEWPORT, vp);
	gluPerspective(110.0, float(vp[2]) / float(vp[3]), 0.01, 100);

	// modelview
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(0, 1, 0, 0);		// set rotation
	glRotatef(0, 0, 1, 0);	// set rotation
	glRotatef(0, 0, 0, 1);		// set rotation

	gluLookAt(8, 8, 0,
		0, 8, 0,
		0, 1, 0);

	// select level of detail for rendering
	// (lods are generated automatically by the ogre xml converter )

	int lod = 0;
	// Test 1 : Halo character (animated mesh)

	static Mesh halo("../data/halo/halo.material",		//	required material file)
		"../data/halo/halo.mesh.xml",		//	required mesh file
		"../data/halo/halo.skeleton.xml");	//	optional skeleton file

											//show one masterchief only
											// Set the skeleton to an animation at a given time
	std::string line;

	if (GetAsyncKeyState(VK_F6)) /// & 0x8000
	{
		FUNC_JOINT("joint2", 0, -45, -45);
	}
	if (GetAsyncKeyState(VK_F1)) /// & 0x8000
	{
		if (fileFlag == false)
		{
			fileFlag = true;
			std::string angluarName;
			std::cout << "waiting for angluar input\n";
			//std::cin >> angluarName;
			char currDir[MAX_PATH];
			GetCurrentDir(currDir);
			strcat(currDir, "\\S001C001P001R001A001.angle");
			inputFile.open(currDir);

			if (inputFile.is_open())
				std::cout << "file opened\n";
			else
				std::cout << "file opened failed\n";

			getline(inputFile, line);
			std::cout << line << "\n";
			frameCnt = stoi(line);
			std::cout << "Input " << frameCnt << " lines.\n";
		}
		std::cout << "====start====\n";
	}
	if (GetAsyncKeyState(VK_F2)) /// & 0x8000
	{
		if (fileFlag == false)
		{
			fileFlag = true;
			std::string angluarName;
			std::cout << "waiting for angluar input\n";
			char currDir[MAX_PATH];
			GetCurrentDir(currDir);
			strcat(currDir, "\\S001C001P001R001A031.angle");
			inputFile.open(currDir);

			if (inputFile.is_open())
				std::cout << "file opened\n";
			else
				std::cout << "file opened failed\n";

			getline(inputFile, line);
			std::cout << line << "\n";
			frameCnt = stoi(line);
			std::cout << "Input " << frameCnt << " lines.\n";
		}
		std::cout << "====start====\n";
	}
	if (GetAsyncKeyState(VK_F3)) /// & 0x8000
	{
		if (fileFlag == false)
		{
			fileFlag = true;
			std::string angluarName;
			std::cout << "waiting for angluar input\n";
			char currDir[MAX_PATH];
			GetCurrentDir(currDir);
			strcat(currDir, "\\S001C001P001R001A038.angle");
			inputFile.open(currDir);

			GetCurrentDir(currDir);
			strcat(currDir, "\\S001C001P001R001A038.2d");
			outputFile.open(currDir);

			getline(inputFile, line);
			std::cout << line << "\n";
			outputFile << line << "\n";
			frameCnt = stoi(line);
			std::cout << "Input " << frameCnt << " lines.\n";
		}
		std::cout << "====start====\n";
	}


	if ((frameCnt > 0) && (fileFlag == true))
	{
		int jndex;
		float(*j_pos)[4];
		float x, y, z;
		//replay motion
		std::cout << "current at " << frameCnt << "\n";

		getline(inputFile, line);//'7'
		outputFile << "7\n";
		getline(inputFile, line);//head, skip
		FUNC_WRITE2D("joint17");

		vec3f angle;

		getline(inputFile, line);//left shoulder
		str2angle(line, angle);
		FUNC_JOINT("joint2", angle[0], angle[1], angle[2]);
		FUNC_WRITE2D("joint2");
		////get 2D information
		//jndex = halo.animation.GetBoneIndexOf("joint2");
		//j_pos = halo.animation.bones[jndex].matrix.m;
		//GetCoordinate(j_pos[3][0], j_pos[3][1], j_pos[3][2], x, y, z, 1);
		////z is about 1
		//outputFile << x << " " << y << "\n";


		getline(inputFile, line);//left elbow
		str2angle(line, angle);
		FUNC_JOINT("joint3", angle[0], angle[1], angle[2]);
		FUNC_WRITE2D("joint3");

		getline(inputFile, line);//left hand
		FUNC_WRITE2D("joint4");

		getline(inputFile, line);//right shoulder
		str2angle(line, angle);
		FUNC_JOINT("joint5", angle[0], angle[1], angle[2]);
		FUNC_WRITE2D("joint5");

		getline(inputFile, line);//right elbow
		str2angle(line, angle);
		FUNC_JOINT("joint6", angle[0], angle[1], angle[2]);
		FUNC_WRITE2D("joint6");

		getline(inputFile, line);//right hand
		FUNC_WRITE2D("joint7");

		isFreshed = true;
		frameCnt--;
	}
	if ((frameCnt <= 0) && (fileFlag == true))
	{
		inputFile.close();
		outputFile.close();
		std::cout << "=====\nplayback done!\n";
		fileFlag = false;
	}

	// Draw the model
	halo.Draw(
		vec3f(0, 0, 0),				// position
		vec3f(0, 0, 0),				// rotation
		lod,						// LOD level
		0);						// 0: model; 1: skeleton

								// Swap
	glutSwapBuffers();

	clock_t ttt = clock();
	//std::cout << "Time is      : " << ttt - startTime << endl;
	//if (isFreshed)
	//{
	//	//int realTimeSleep = timeSleep - (ttt - startTime);
	//	int realTimeSleep = 100 - (ttt - startTime);
	//	std::cout << "Real sleep time: " << realTimeSleep << "\n";
	//	if (realTimeSleep > 0)
	//		Sleep(realTimeSleep / 2);
	//	else
	//		Sleep(3);
	//}

	Sleep(50);
	startTime = ttt;
}
///////////////////////////////////////////
int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
	glutInitWindowSize(WINDOWS_WIDTH, WINDOWS_HEIGHT);
	glutInitWindowPosition(320, 156);
	glutCreateWindow("Skinned Skeletal Animation Sample (c) Sven Forstmann in 2014");
	glutDisplayFunc(DrawScene);
	glutIdleFunc(DrawScene);
	glewInit();
	wglSwapIntervalEXT(0);
	glutMainLoop();
	getchar();
	return 0;
}
///////////////////////////////////////////
