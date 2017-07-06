// Bruno Henrique Rasteiro, 9292910
// Hugo Moraes Dzin, 8532186
// Marcos Vinícius Junqueira, 8922393
// Raul Zaninetti Rosa, 8517310
#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <GL/glut.h>

struct Point
{
	GLfloat x, y, z;

	void vertex() const
	{
		glVertex3f(this->x, this->y, this->z);
	}

	void normal() const
	{
		glNormal3f(this->x, this->y, this->z);
	}

	void normalAndVertex() const
	{
		normal();
		vertex();
	}
};

// Window size
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
GLdouble gAspect = (GLdouble) WINDOW_WIDTH / WINDOW_HEIGHT;

// User-controlled parameters
GLdouble gFov = 45.0;
GLfloat gLightXyzh[] = { 0.0, 6.0, 0.0, 1.0 };
GLfloat gObjectSpinX = 0.0;
GLfloat gObjectSpinY = 0.0;

const GLfloat gObjectScale = 1.0;
const GLfloat gObjectY = 3.0;

// Mesh data
// Vertex count
#define SPHERE_N 64
// Mesh vertices/normals
Point gPole[2];
Point gLatitude[SPHERE_N - 2][SPHERE_N];
// OpenGL display list index
GLuint gSphereList = 0;


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
void onReshape(int w, int h);


// Shadow
void calculateShadowTransform(
		GLfloat output[16],
		const GLfloat lightXyzh[4],
		const GLfloat planeEquation[4]);
void calculatePlaneEquation(GLfloat output[4], const GLfloat *vertex[3]);


// Mesh
void renderObject(GLfloat scale, GLfloat objectY, GLfloat spinX, GLfloat spinY);
void createSphereMesh(Point outputPole[2], Point outputLat[][SPHERE_N]);
GLuint sphereDisplayList(Point pole[2], Point latitude[][SPHERE_N]);


int main(int argc, char **argv)
{
	std::cout << "Controles\n"
		"  ESC:   sai do programa\n"
		"  Setas: move fonte de luz\n"
		"  WASD:  rotaciona objeto\n"
		"  QE:    zoom\n" << std::endl;

	// Create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(300, 100);
	glutCreateWindow("Ovo Com Sombra");

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
	glutReshapeFunc(onReshape);

	// Calculate shadow plane
	const GLfloat *vertex[3] =
	{
		// Starts from [i][3] to skip nx ny nyz
		&gFloorQuad[0][3],
		&gFloorQuad[1][3],
		&gFloorQuad[2][3],
	};
	calculatePlaneEquation(gShadowPlane, vertex);

	// Create mesh and display list
	createSphereMesh(gPole, gLatitude);
	gSphereList = sphereDisplayList(gPole, gLatitude);


	glutMainLoop();
	return 0;
}


// Event handlers
void onDisplay()
{
	calculateShadowTransform(gShadowTransform, gLightXyzh, gShadowPlane);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(gFov, gAspect, 0.1, 100.0);

	// Camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(10.0, 10.0, 10.0,   0.0, 0.0, 0.0,   0.0, 1.0, 0.0);

	// Reposition light source. Must happen after camera transform.
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
		glColor3f(1.0, 1.0, 0.25);
		glutSolidSphere(0.1, 8, 8);
	}
	glPopMatrix();

	glutSwapBuffers();
}


void onKeyPress(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;

	// Quit
	if(key == 27) // ESC
	{
		exit(0);
	}
	// Rotate object
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
	// Change zoom
	else if(key == 'q')
	{
		gFov += 5.0;
	}
	else if(key == 'e')
	{
		gFov -= 5.0;
	}

	glutPostRedisplay();
}


void onSpecialPress(int key, int x, int y)
{
	(void) x;
	(void) y;

	// Move light source
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


void onReshape(int w, int h)
{
	if(w == 0 || h == 0)
	{
		return;
	}

	glViewport(0, 0 , w, h);
	gAspect = (GLdouble) w / h;

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

	// output = intersection * identity - (light^T [matrix mult] plane)
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

	// Find 2 vectors from the 3 vertices
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
	glScalef(scale, scale, scale);
	glScalef(1.0, 1.0, 1.5); // Turn sphere into ellipsoid
	glCallList(gSphereList);
}


// Create a sphere of radius 1 centered at (0, 0, 0)
// Since radius == 1, the vertices are also the normals
void createSphereMesh(Point outputPole[2], Point outputLat[][SPHERE_N])
{
	outputPole[0] = {0.0,  1.0, 0.0};
	outputPole[1] = {0.0, -1.0, 0.0};

	// Does not compute poles
	for(int i = 1; i < SPHERE_N-2 + 1; i++)
	{
		// Create a horizontal slice
		for(int j = 0; j < SPHERE_N; j++)
		{
			Point *p = &outputLat[i - 1][j];

			GLfloat theta = (2 * M_PI * j) / SPHERE_N;
			GLfloat phi = (M_PI * i) / SPHERE_N;

			p->x = sin(phi) * cos(theta);
			p->z = sin(phi) * sin(theta);
			p->y = cos(phi);
		}
	}
}


// Create an OpenGL display list that renders the sphere mesh
GLuint sphereDisplayList(Point pole[2], Point latitude[][SPHERE_N])
{
	GLuint index = glGenLists(1);

	glNewList(index, GL_COMPILE);
	{
		// Render caps
		glBegin(GL_TRIANGLES);
		{
			for(int i = 0; i < SPHERE_N; i++)
			{
				// Top cap
				Point *p = &pole[0];
				Point *l1 = &latitude[0][i];
				Point *l2 = &latitude[0][(i+1) % SPHERE_N];
				p->normalAndVertex();
				l1->normalAndVertex();
				l2->normalAndVertex();

				// Bottom cap
				p = &pole[1];
				l1 = &latitude[SPHERE_N-2 - 1][(i+1) % SPHERE_N];
				l2 = &latitude[SPHERE_N-2 - 1][i];

				p->normalAndVertex();
				l1->normalAndVertex();
				l2->normalAndVertex();
			}
		}
		glEnd();

		// Render horizontal slices
		glBegin(GL_QUADS);
		{
			for(int i = 0; i < SPHERE_N-2 - 1; i++)
			{
				for(int j = 0; j < SPHERE_N; j++)
				{
					Point *top1 = &latitude[i][j];
					Point *top2 = &latitude[i][(j+1) % SPHERE_N];

					Point *bottom1 = &latitude[i+1][j];
					Point *bottom2 = &latitude[i+1][(j+1) % SPHERE_N];

					top1->normalAndVertex();
					bottom1->normalAndVertex();
					bottom2->normalAndVertex();
					top2->normalAndVertex();
				}
			}
		}
		glEnd();
	}
	glEndList();

	return index;
}
