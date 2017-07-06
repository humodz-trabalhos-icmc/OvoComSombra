#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <GL/glut.h>


// Globals
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

const GLdouble ASPECT = (GLdouble) WINDOW_WIDTH / WINDOW_HEIGHT;

// User-controlled parameters
GLdouble gFov = 45.0;
GLfloat gLightXyzh[] = { 0.0, 5.0, 0.0, 1.0 };
GLfloat gObjectSpinX = 0.0;
GLfloat gObjectSpinY = 0.0;

const GLfloat gObjectScale = 1.0;
const GLfloat gObjectY = 3.0;


// Used for shadow casting
GLfloat gShadowTransform[16] = { 0 };
GLfloat gShadowPlane[4] = { 0 };


// Floor
const GLfloat gFloorQuad[][6] =
{
	// nx   ny   nz     x    y     z
	{ 0.0, 1.0, 0.0, -5.0, 0.0, -5.0 },
	{ 0.0, 1.0, 0.0, -5.0, 0.0, +5.0 },
	{ 0.0, 1.0, 0.0, +5.0, 0.0, +5.0 },
	{ 0.0, 1.0, 0.0, +5.0, 0.0, -5.0 },
};


// Event handlers
void onDisplay();
void onKeyPress(unsigned char key, int x, int y);
void onSpecialPress(int key, int x, int y);


// Shadow
void calculateShadowTransform(
		GLfloat output[16],
		const GLfloat lightXyzh[4],
		const GLfloat planeEquation[4]);
void calculatePlaneEquation(GLfloat output[4], const GLfloat *vertex[3]);


// Mesh
void renderObject(GLfloat scale, GLfloat objectY, GLfloat spinX, GLfloat spinY);


int main(int argc, char **argv)
{
	// Create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(300, 100);
	glutCreateWindow("Ovo Com Sombra");

	glMatrixMode(GL_PROJECTION);
	gluPerspective(gFov, ASPECT, 0.1, 100.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);

	// Lighting
	GLfloat ambient[]  = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat specular[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Event handlers
	glutDisplayFunc(onDisplay);
	glutKeyboardFunc(onKeyPress);
	glutSpecialFunc(onSpecialPress);

	// Calculate shadow plane
	const GLfloat *vertex[3] =
	{
		// Starts from [i][3] to skip nx ny nyz
		&gFloorQuad[0][3],
		&gFloorQuad[1][3],
		&gFloorQuad[2][3],
	};
	calculatePlaneEquation(gShadowPlane, vertex);

	glutMainLoop();
	return 0;
}


// Event handlers
void onDisplay()
{
	calculateShadowTransform(gShadowTransform, gLightXyzh, gShadowPlane);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(10.0, 10.0, 10.0,   0.0, 0.0, 0.0,   0.0, 1.0, 0.0);

	// Reposition light source. Must happen after camera transform
	glLightfv(GL_LIGHT0, GL_POSITION, gLightXyzh);


	// Render ground
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	{
		glInterleavedArrays(GL_N3F_V3F, 0, gFloorQuad);
		glDrawArrays(GL_QUADS, 0, 4);
	}
	glPopMatrix();

	// Render shadow
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	{
		glColor3f(0.1, 0.1, 0.1);
		glMultMatrixf(gShadowTransform);
		renderObject(gObjectScale, gObjectY, gObjectSpinX, gObjectSpinY);
	}
	glPopMatrix();

	// Render object
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glPushMatrix();
	{
		renderObject(gObjectScale, gObjectY, gObjectSpinX, gObjectSpinY);
	}
	glPopMatrix();

	// Render light source
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPushMatrix();
	{
		glTranslatef(gLightXyzh[0], gLightXyzh[1], gLightXyzh[2]);
		glColor3f(1.0, 1.0, 0.5);
		glutSolidSphere(0.1, 8, 8);
	}
	glPopMatrix();

	glutSwapBuffers();
}


void onKeyPress(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	std::cout << "Pressed key: " << (int) key << std::endl;

	if(key == 27) // ESC
	{
		exit(0);
	}
	else if(key == 'w')
	{
		gObjectSpinX += 10.0;
	}
	else if(key == 's')
	{
		gObjectSpinX -= 10.0;

	}
	else if(key == 'a')
	{

		gObjectSpinY += 10.0;
	}
	else if(key == 'd')
	{
		gObjectSpinY -= 10.0;

	}

	glutPostRedisplay();
}


void onSpecialPress(int key, int x, int y)
{
	(void) x;
	(void) y;

	if(key == GLUT_KEY_UP)
	{
		gLightXyzh[0] -= 0.25;
	}
	else if(key == GLUT_KEY_DOWN)
	{
		gLightXyzh[0] += 0.25;
	}
	else if(key == GLUT_KEY_LEFT)
	{
		gLightXyzh[2] += 0.25;
	}
	else if(key == GLUT_KEY_RIGHT)
	{
		gLightXyzh[2] -= 0.25;
	}

	glutPostRedisplay();
}




// Shadow
void calculateShadowTransform(
		GLfloat output[16],
		const GLfloat lightXyzh[4],
		const GLfloat planeEquation[4])
{
	// Intersection between light source and plane
	GLfloat intersection = 0.0;

	// intersection = lightXyzh [dot product] planeEquation
	for(int i = 0; i < 4; i++)
	{
		intersection += lightXyzh[i] * planeEquation[i];
	}

	// output = intersection * I_4by4 - (light^T [matrix mult] plane)
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			output[4*i + j] = -lightXyzh[j] * planeEquation[i];
		}
		output[5*i] += intersection;
	}
}


void calculatePlaneEquation(GLfloat output[4], const GLfloat *vertex[3])
{
	GLfloat vec[2][3];

	// Find 2 vectors from the 2 vertices
	for(int i = 0; i < 2; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			vec[i][j] = vertex[i + 1][j] - vertex[0][j];
		}
	}

	// Calculate plane equation
	output[0] = vec[0][1]*vec[1][2] - vec[0][2]*vec[1][1];
	output[1] = vec[0][2]*vec[1][0] - vec[0][0]*vec[1][2];
	output[2] = vec[0][0]*vec[1][1] - vec[0][1]*vec[1][0];

	output[3] = 0;
	for(int i = 0; i < 3; i++)
	{
		output[3] -= output[i] * vertex[0][i];
	}
}




// Mesh
void renderObject(GLfloat scale, GLfloat objectY, GLfloat spinX, GLfloat spinY)
{
	glTranslatef(0.0, objectY, 0.0);
	glRotatef(-spinY, 0.0, 1.0, 0.0);
	glRotatef(-spinX, 1.0, 0.0, 0.0);
	glScalef(1.0, 1.0, 1.5);
	glScalef(scale, scale, scale);
	glutSolidSphere(1, 16, 16);
}
