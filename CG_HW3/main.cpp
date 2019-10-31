/*
CG Homework3 - Bump Mapping
Objective - learning GLSL, glm model datatype(for .obj file) and bump mapping
In this homework, you should load "Ball.obj" and normal map "NormalMap.ppm" with glm.c(done by TA)
and render the object with color texure and normal mapping with Phong shading(and Phong lighting model of course).
Please focus on the part with comment like "you may need to do somting here".
If you don't know how to access vertices information of the model,
I suggest you refer to glm.c for _GLMmodel structure and glm.h for glmReadOBJ() and glmDraw() function.
And the infomation printed by print_model_info(model); of glm_helper.h helps as well!
Finally, please pay attention to the datatype of the variable you use(might be a ID list or value array)
Good luck!
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
#include "../GL/glew.h"
#include "../GL/glut.h""
#include "../shader_lib/shader.h"
#include "glm/glm.h"
#include "../glm/glm/gtc/matrix_transform.hpp"

// my inclusion
#include "iostream"
#include <sstream>
#include <fstream>
#include <vector>
#include "../glm/glm/glm.hpp"

extern "C"
{
#include "glm_helper.h"
}


/*you may need to do somting here
you may use the following struct type to perform your single VBO method,
or you can define/declare multiple VBOs for VAO method.
Please feel free to modify it*/
struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
	GLfloat tangent[3];
	GLfloat bitangent[3];
};
typedef struct Vertex Vertex;

//no need to modify the following function declarations and gloabal variables
void init(void);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void idle(void);
void draw_light_bulb(void);
void camera_light_ball_move();
GLuint load_normal_map(char* name);

// my function
void initShader(void);
void shaderReader(std::string path);
void drawBigBall(void);

int bumpCount = 0;

namespace
{
	char *obj_file_dir = "../Resources/Ball.obj";
	char *normal_map_dir = "../Resources/NormalMap.ppm";
	char *texture_dir = "../Resources/Tile.ppm";
	GLfloat light_rad = 0.05;//radius of the light bulb
	float eyet = 0.0;//theta in degree
	float eyep = 90.0;//phi in degree
	bool mleft = false;
	bool mright = false;
	bool mmiddle = false;
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool lforward = false;
	bool lbackward = false;
	bool lleft = false;
	bool lright = false;
	bool lup = false;
	bool ldown = false;
	bool bforward = false;
	bool bbackward = false;
	bool bleft = false;
	bool bright = false;
	bool bup = false;
	bool bdown = false;
	bool bx = false;
	bool by = false;
	bool bz = false;
	bool brx = false;
	bool bry = false;
	bool brz = false;
	int mousex = 0;
	int mousey = 0;

	// my variables
	std::string shaderSource;
	const GLchar* glvss;

	const GLchar* glfss;

	GLuint shaderProgram;
	GLuint VAO;
	GLuint VBO[3];


	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> texCoord;
	std::vector<glm::vec3> normals;

	std::vector<glm::vec3> tangent;
	std::vector<glm::vec3> bitangent;

	glm::mat4 V, P;
	glm::mat4 M(1.0f);

	GLfloat ka[3];
	GLfloat kd[3];
	GLfloat ks[3];

	GLuint textureID;
	GLfloat textureHight, textureWidth;

	int bump = 0;

	int g_width = 512;
	int g_height = 512;

}

//you can modify the moving/rotating speed if it's too fast/slow for you
const float speed = 0.03;//camera/light/ball moving speed
const float rotation_speed = 0.05;//ball rotation speed
//you may need to use some of the following variables in your program 
GLuint normalTextureID;//TA has already loaded the normal texture for you
GLMmodel *model;//TA has already loaded the model for you(!but you still need to convert it to VBO(s)!)
float eyex = 0.0;
float eyey = 0.0;
float eyez = 3.0;
GLfloat light_pos[] = { 1, 1, 1 };
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };
GLfloat normalWid, normalHei;

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	//remember to replace "YourStudentID" with your own student ID
	glutCreateWindow("CG_HW3_0656632");
	glutReshapeWindow(512, 512);

	glewInit();

	init();


	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);



	glutMainLoop();

	glmDelete(model);
	return 0;
}

void shaderReader(std::string path)
{
	std::ifstream file;

	file = std::ifstream(path);

	std::string lineBuffer;
	std::string buffer;

	if (!file.good())
	{
		std::cout << path << " dose not exists." << std::endl;
		return;
	}
	else
	{
		std::cout << path << " is OK." << std::endl;
	}

	while (std::getline(file, buffer))
	{
		lineBuffer += buffer + "\n";
	}

	shaderSource = lineBuffer;
}

void initShader(void)
{
	glewInit();

	// init vertexShader
	shaderReader("../GLSL_example/Shaders/vertShader.txt");

	glvss = shaderSource.c_str();


	GLint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &glvss, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	else
	{
		std::cout << "vertexShader compiled successfully" << std::endl;
	}

	// init fragmentShader
	shaderReader("../GLSL_example/Shaders/fragShader.txt");

	glfss = shaderSource.c_str();

	GLint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &glfss, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	else
	{
		std::cout << "fragmentShader compiled successfully" << std::endl;
	}

	// shaderProgram
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::LINK::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	else
	{
		std::cout << "shaderProgram linked successfully" << std::endl;
	}

	// deleteShader
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// genBuffer
	glGenVertexArrays(1, &VAO);
	glGenBuffers(5, VBO);


	// VAO needs to be binded first
	glBindVertexArray(VAO);


	// init vertices, texCoord, normals 

	for (int i = 0; i < model->numtriangles; i++)
	{
		int vIndex[3] = { model->triangles[i].vindices[0], model->triangles[i].vindices[1] ,model->triangles[i].vindices[2] };
		int tIndex[3] = { model->triangles[i].tindices[0], model->triangles[i].tindices[1] ,model->triangles[i].tindices[2] };
		int nIndex[3] = { model->triangles[i].nindices[0], model->triangles[i].nindices[1], model->triangles[i].nindices[2] };

		glm::vec3 PQ[3];
		glm::vec2 PQP[3];


		for (int j = 0; j < 3; j++)
		{
			glm::vec3 vTemp;
			vTemp.x = model->vertices[vIndex[j] * 3];
			vTemp.y = model->vertices[vIndex[j] * 3 + 1];
			vTemp.z = model->vertices[vIndex[j] * 3 + 2];
			vertices.push_back(vTemp);

			glm::vec2 tTemp;
			tTemp.x = model->texcoords[tIndex[j] * 2];
			tTemp.y = model->texcoords[tIndex[j] * 2 + 1];
			texCoord.push_back(tTemp);

			glm::vec3 nTemp;
			nTemp.x = model->normals[nIndex[j] * 3];
			nTemp.y = model->normals[nIndex[j] * 3 + 1];
			nTemp.z = model->normals[nIndex[j] * 3 + 2];
			normals.push_back(nTemp);

			PQ[j] = vTemp;
			PQP[j] = tTemp;
			;

		}
		float r = 1 / ((PQP[1] - PQP[0]).x * (PQP[2] - PQP[0]).y - (PQP[1] - PQP[0]).y * (PQP[2] - PQP[0]).x);
		glm::vec3 tangentTemp = ((PQ[1] - PQ[0]) * (PQP[2] - PQP[0]).y - (PQ[2] - PQ[0]) * (PQP[1] - PQP[0]).y)*r;
		glm::vec3 bitangentTemp = ((PQ[2] - PQ[0]) * (PQP[1] - PQP[0]).x - (PQ[1] - PQ[0])*(PQP[2] - PQP[0]).x)*r;

		for (int i = 0; i < 3; i++)
		{
			tangent.push_back(tangentTemp);
			bitangent.push_back(bitangentTemp);
		}
	}

	// VBO0(vertices)

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	// VBO1(texCoord)	

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*texCoord.size(), &texCoord[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	// VBO2(normals)

	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	// VBO3(tangent)

	glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*tangent.size(), &tangent[0], GL_STATIC_DRAW);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	// VBO4(bitangent)

	glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*bitangent.size(), &bitangent[0], GL_STATIC_DRAW);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(4);


	// Unbind Buffer
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0); // Unbind VAO

}

void init(void) {

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);
	model = glmReadOBJ(obj_file_dir);
	normalTextureID = load_normal_map(normal_map_dir);
	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(model);

	//you may need to do somting here(create shaders/program(s) and create vbo(s)/vao from GLMmodel model)

	textureID = glmLoadTexture(texture_dir, false, true, true, true, &textureWidth, &textureHight);

	initShader();
}

void drawBigBall(void)
{
	ka[0] = model->materials[1].ambient[0];
	ka[1] = model->materials[1].ambient[1];
	ka[2] = model->materials[1].ambient[2];

	kd[0] = model->materials[1].diffuse[0];
	kd[1] = model->materials[1].diffuse[1];
	kd[2] = model->materials[1].diffuse[2];

	ks[0] = model->materials[1].specular[0];
	ks[1] = model->materials[1].specular[1];
	ks[2] = model->materials[1].specular[2];

	glUseProgram(shaderProgram);

	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "V"), 1, GL_FALSE, &V[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "P"), 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "M"), 1, GL_FALSE, &M[0][0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "ka"), 1, ka);
	glUniform3fv(glGetUniformLocation(shaderProgram, "kd"), 1, kd);
	glUniform3fv(glGetUniformLocation(shaderProgram, "ks"), 1, ks);
	glUniform1i(glGetUniformLocation(shaderProgram, "bump"), bump);

	// texture
	glEnable(GL_TEXTURE_2D);

	GLint loc = glGetUniformLocation(shaderProgram, "MyTexture_1");

	glActiveTexture(GL_TEXTURE0 + 0);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glUniform1i(loc, 0);

	// bumpmap

	GLint bumpLoc = glGetUniformLocation(shaderProgram, "MyTexture_2");
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);

	glUniform1i(bumpLoc, 1);


	GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
	glUniform3fv(lightPosLoc, 1, light_pos);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, model->numtriangles * 3);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//you may need to do somting here(declare some local variables you need and maybe load inverse Model matrix here...)
	glUseProgram(0);
	//please try not to modify the following block of code(you can but you are not supposed to)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyex, eyey, eyez,
		eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180), eyey + sin(eyet*M_PI / 180), eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180),
		0.0, 1.0, 0.0);
	draw_light_bulb();

	glm::vec3 ballTran = glm::vec3(ball_pos[0], ball_pos[1], ball_pos[2]);
	glm::vec3 ballRot = glm::vec3(ball_rot[0], ball_rot[1], ball_rot[2]);

	M = glm::mat4(1.0f);
	M = glm::translate(M, ballTran);
	M = glm::rotate(M, ball_rot[0], glm::vec3(1, 0, 0));
	M = glm::rotate(M, ball_rot[1], glm::vec3(0, 1, 0));
	M = glm::rotate(M, ball_rot[2], glm::vec3(0, 0, 1));


	//please try not to modify the previous block of code

	//you may need to do something here(pass the uniform variable to shader and render the model)
	//glmDraw(model,GLM_TEXTURE);//please delete this line in your final code! It's just a preview of rendered object

	// MVP
	glm::vec3 camPos = glm::vec3(eyex, eyey, eyez);
	glm::vec3 center = glm::vec3(eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180), eyey + sin(eyet*M_PI / 180), eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180));
	glm::vec3 camUp = glm::vec3(0, 1, 0);

	V = glm::lookAt(camPos, center, camUp);
	P = glm::perspective((float)glm::radians(45.0), (float)g_width / (float)g_height, (float) 0.001f, (float)1000);

	drawBigBall();

	glutSwapBuffers();

	camera_light_ball_move();
}

//please implement bump mapping toggle(enable/disable bump mapping) in case 'b'(lowercase)
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
	{	//ESC
		break;
	}
	case 'b'://toggle bump mapping
	{
		//you may need to do somting here
		if (bumpCount % 2 == 0)
		{
			bump = 1;
			bumpCount++;

		}
		else
		{
			bump = 0;
			bumpCount++;
		}


		break;
	}
	case 'd':
	{
		right = true;
		break;
	}
	case 'a':
	{
		left = true;
		break;
	}
	case 'w':
	{
		forward = true;
		break;
	}
	case 's':
	{
		backward = true;
		break;
	}
	case 'q':
	{
		up = true;
		break;
	}
	case 'e':
	{
		down = true;
		break;
	}
	case 't':
	{
		lforward = true;
		break;
	}
	case 'g':
	{
		lbackward = true;
		break;
	}
	case 'h':
	{
		lright = true;
		break;
	}
	case 'f':
	{
		lleft = true;
		break;
	}
	case 'r':
	{
		lup = true;
		break;
	}
	case 'y':
	{
		ldown = true;
		break;
	}
	case 'i':
	{
		bforward = true;
		break;
	}
	case 'k':
	{
		bbackward = true;
		break;
	}
	case 'l':
	{
		bright = true;
		break;
	}
	case 'j':
	{
		bleft = true;
		break;
	}
	case 'u':
	{
		bup = true;
		break;
	}
	case 'o':
	{
		bdown = true;
		break;
	}
	case '7':
	{
		bx = true;
		break;
	}
	case '8':
	{
		by = true;
		break;
	}
	case '9':
	{
		bz = true;
		break;
	}
	case '4':
	{
		brx = true;
		break;
	}
	case '5':
	{
		bry = true;
		break;
	}
	case '6':
	{
		brz = true;
		break;
	}

	//special function key
	case 'z'://move light source to front of camera
	{
		light_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180);
		light_pos[1] = eyey + sin(eyet*M_PI / 180);
		light_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180);
		break;
	}
	case 'x'://move ball to front of camera
	{
		ball_pos[0] = eyex + cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) * 3;
		ball_pos[1] = eyey + sin(eyet*M_PI / 180) * 5;
		ball_pos[2] = eyez - cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180) * 3;
		break;
	}
	case 'c'://reset all pose
	{
		light_pos[0] = 1;
		light_pos[1] = 1;
		light_pos[2] = 1;
		ball_pos[0] = 0;
		ball_pos[1] = 0;
		ball_pos[2] = 0;
		ball_rot[0] = 0;
		ball_rot[1] = 0;
		ball_rot[2] = 0;
		eyex = 0;
		eyey = 0;
		eyez = 3;
		eyet = 0;
		eyep = 90;
		break;
	}
	default:
	{
		break;
	}
	}
}

//no need to modify the following functions
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
	g_width = width;
	g_height = height;
	glMatrixMode(GL_MODELVIEW);
}
void motion(int x, int y) {
	if (mleft)
	{
		eyep -= (x - mousex)*0.1;
		eyet -= (y - mousey)*0.12;
		if (eyet > 89.9)
			eyet = 89.9;
		else if (eyet < -89.9)
			eyet = -89.9;
		if (eyep > 360)
			eyep -= 360;
		else if (eyep < 0)
			eyep += 360;
	}
	mousex = x;
	mousey = y;
}
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN && !mright && !mmiddle)
		{
			mleft = true;
			mousex = x;
			mousey = y;
		}
		else
			mleft = false;
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mmiddle)
		{
			mright = true;
			mousex = x;
			mousey = y;
		}
		else
			mright = false;
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mright)
		{
			mmiddle = true;
			mousex = x;
			mousey = y;
		}
		else
			mmiddle = false;
	}
}
void camera_light_ball_move()
{
	GLfloat dx = 0, dy = 0, dz = 0;
	if (left || right || forward || backward || up || down)
	{
		if (left)
			dx = -speed;
		else if (right)
			dx = speed;
		if (forward)
			dy = speed;
		else if (backward)
			dy = -speed;
		eyex += dy * cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx * sin(eyep*M_PI / 180);
		eyey += dy * sin(eyet*M_PI / 180);
		eyez += dy * (-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx * cos(eyep*M_PI / 180);
		if (up)
			eyey += speed;
		else if (down)
			eyey -= speed;
	}
	if (lleft || lright || lforward || lbackward || lup || ldown)
	{
		dx = 0;
		dy = 0;
		if (lleft)
			dx = -speed;
		else if (lright)
			dx = speed;
		if (lforward)
			dy = speed;
		else if (lbackward)
			dy = -speed;
		light_pos[0] += dy * cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx * sin(eyep*M_PI / 180);
		light_pos[1] += dy * sin(eyet*M_PI / 180);
		light_pos[2] += dy * (-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx * cos(eyep*M_PI / 180);
		if (lup)
			light_pos[1] += speed;
		else if (ldown)
			light_pos[1] -= speed;
	}
	if (bleft || bright || bforward || bbackward || bup || bdown)
	{
		dx = 0;
		dy = 0;
		if (bleft)
			dx = -speed;
		else if (bright)
			dx = speed;
		if (bforward)
			dy = speed;
		else if (bbackward)
			dy = -speed;
		ball_pos[0] += dy * cos(eyet*M_PI / 180)*cos(eyep*M_PI / 180) + dx * sin(eyep*M_PI / 180);
		ball_pos[1] += dy * sin(eyet*M_PI / 180);
		ball_pos[2] += dy * (-cos(eyet*M_PI / 180)*sin(eyep*M_PI / 180)) + dx * cos(eyep*M_PI / 180);
		if (bup)
			ball_pos[1] += speed;
		else if (bdown)
			ball_pos[1] -= speed;
	}
	if (bx || by || bz || brx || bry || brz)
	{
		dx = 0;
		dy = 0;
		dz = 0;
		if (bx)
			dx = -rotation_speed;
		else if (brx)
			dx = rotation_speed;
		if (by)
			dy = rotation_speed;
		else if (bry)
			dy = -rotation_speed;
		if (bz)
			dz = rotation_speed;
		else if (brz)
			dz = -rotation_speed;
		ball_rot[0] += dx;
		ball_rot[1] += dy;
		ball_rot[2] += dz;
	}
}
void draw_light_bulb()
{
	GLUquadric *quad;
	quad = gluNewQuadric();
	glPushMatrix();
	glColor3f(0.4, 0.5, 0);
	glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
	gluSphere(quad, light_rad, 40, 20);
	glPopMatrix();
}
void keyboardup(unsigned char key, int x, int y) {
	switch (key) {
	case 'd':
	{
		right = false;
		break;
	}
	case 'a':
	{
		left = false;
		break;
	}
	case 'w':
	{
		forward = false;
		break;
	}
	case 's':
	{
		backward = false;
		break;
	}
	case 'q':
	{
		up = false;
		break;
	}
	case 'e':
	{
		down = false;
		break;
	}
	case 't':
	{
		lforward = false;
		break;
	}
	case 'g':
	{
		lbackward = false;
		break;
	}
	case 'h':
	{
		lright = false;
		break;
	}
	case 'f':
	{
		lleft = false;
		break;
	}
	case 'r':
	{
		lup = false;
		break;
	}
	case 'y':
	{
		ldown = false;
		break;
	}
	case 'i':
	{
		bforward = false;
		break;
	}
	case 'k':
	{
		bbackward = false;
		break;
	}
	case 'l':
	{
		bright = false;
		break;
	}
	case 'j':
	{
		bleft = false;
		break;
	}
	case 'u':
	{
		bup = false;
		break;
	}
	case 'o':
	{
		bdown = false;
		break;
	}
	case '7':
	{
		bx = false;
		break;
	}
	case '8':
	{
		by = false;
		break;
	}
	case '9':
	{
		bz = false;
		break;
	}
	case '4':
	{
		brx = false;
		break;
	}
	case '5':
	{
		bry = false;
		break;
	}
	case '6':
	{
		brz = false;
		break;
	}

	default:
	{
		break;
	}
	}
}
void idle(void)
{
	glutPostRedisplay();
}
GLuint load_normal_map(char* name)
{
	return glmLoadTexture(name, false, true, true, true, &normalWid, &normalHei);
}