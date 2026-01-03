//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul X - 10_01_modele3D.cpp |
// =============================================
// 
// Program care deseneaza un model 3D importat  

// Biblioteci
#include <windows.h>  // biblioteci care urmeaza sa fie incluse
#include <stdio.h>
#include <stdlib.h> // necesare pentru citirea shader-elor
#include <cstdlib> 
#include <vector>
#include <math.h>
#include <iostream>
#include <GL/glew.h> // glew apare inainte de freeglut
#include <GL/freeglut.h> // nu trebuie uitat freeglut.h

#include "loadShaders.h"

#include "glm/glm.hpp"  
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "objloader.hpp"  


//  Identificatorii obiectelor de tip OpenGL;
GLuint
	VaoId,
	VboId,
	ProgramId,
	nrVertLocation,
	myMatrixLocation,
	viewPosLocation,
	viewLocation,
	projLocation;
// Sfera
GLuint SphereVao = 0;
GLuint SphereVbo = 0;
GLuint SphereEbo = 0;
int sphereIndexCount = 0;

// Valoarea lui pi
float PI = 3.141592;

// Variabila pentru numarul de varfuri
int nrVertices;

// Vectori pentru varfuri, coordonate de texturare, normale
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;  

// Matrice utilizate
glm::mat4 myMatrix; 
glm::mat4 view;
glm::mat4 projection;

//	Elemente pentru matricea de vizualizare;
float refX = 0.0f, refY = 0.0f, refZ = 0.0f,
obsX, obsY, obsZ,
vX = 0.0f, vY = 0.0f, vZ = 1.0f;
//	Elemente pentru deplasarea pe sfera;
float alpha = 0.0f, beta = 0.0f, dist = 6.0f,
incrAlpha1 = 0.01, incrAlpha2 = 0.01;
//	Elemente pentru matricea de proiectie;
float width = 800, height = 600, dNear = 4.f, fov = 60.f * PI / 180;


void processNormalKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '+':
		dist -= 0.25;	//	apasarea tastelor `-` si `+` schimba pozitia observatorului (se departeaza / aproprie);
		break;
	case '-':
		dist += 0.25;
		break;
	}
	if (key == 27)
		exit(0);
}

void processSpecialKeys(int key, int xx, int yy)
{
	switch (key)				//	Procesarea tastelor 'LEFT', 'RIGHT', 'UP', 'DOWN';
	{							//	duce la deplasarea observatorului pe suprafata sferica in jurul cubului;
	case GLUT_KEY_LEFT:
		beta -= 0.01;
		break;
	case GLUT_KEY_RIGHT:
		beta += 0.01;
		break;
	case GLUT_KEY_UP:
		alpha += incrAlpha1;
		if (abs(alpha - PI / 2) < 0.05)
		{
			incrAlpha1 = 0.f;
		}
		else
		{
			incrAlpha1 = 0.01f;
		}
		break;
	case GLUT_KEY_DOWN:
		alpha -= incrAlpha2;
		if (abs(alpha + PI / 2) < 0.05)
		{
			incrAlpha2 = 0.f;
		}
		else
		{
			incrAlpha2 = 0.01f;
		}
		break;
	}
}

// Se initializeaza un vertex Buffer Object(VBO) pentru transferul datelor spre memoria placii grafice(spre shadere);
// In acesta se stocheaza date despre varfuri;
void CreateVBO(void)
{

// Generare VAO;
  glGenVertexArrays(1, &VaoId);
  glBindVertexArray(VaoId);

 // Generare VBO - varfurile si normalele sunt memorate in sub-buffere;
  glGenBuffers(1, &VboId);
  glBindBuffer(GL_ARRAY_BUFFER, VboId);
	size_t vertsBytes = vertices.size() * sizeof(glm::vec3);
	size_t normsBytes = normals.size() * sizeof(glm::vec3);
	if (vertices.size() > 0) {
		glBufferData(GL_ARRAY_BUFFER, vertsBytes + normsBytes, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertsBytes, &vertices[0]);
		if (normals.size() > 0)
			glBufferSubData(GL_ARRAY_BUFFER, vertsBytes, normsBytes, &normals[0]);

		// Atributele; 
		glEnableVertexAttribArray(0); // atributul 0 = pozitie
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
		if (normals.size() > 0) {
			glEnableVertexAttribArray(1); // atributul 1 = normale
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertsBytes));
		}
	} else {
		// daca nu exista varfuri, eliberam buffer si setam VaoId la 0 pentru a nu incerca desenarea
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &VboId);
		VaoId = 0;
	}

}

// Creeaza o sfera (positions + normals + indices) si VAO/VBO/EBO
void CreateSphere(float radius = 1.0f, int sectors = 40, int stacks = 20)
{
	std::vector<glm::vec3> sphVerts;
	std::vector<glm::vec3> sphNormals;
	std::vector<GLuint> sphIndices;

	const float PI_local = 3.14159265359f;
	for (int i = 0; i <= stacks; ++i) {
		float V = (float)i / (float)stacks;
		float phi = (V - 0.5f) * PI_local; // -pi/2 .. pi/2
		float cosPhi = cosf(phi);
		float sinPhi = sinf(phi);
		for (int j = 0; j <= sectors; ++j) {
			float U = (float)j / (float)sectors;
			float theta = U * 2.0f * PI_local; // 0..2pi
			float cosTheta = cosf(theta);
			float sinTheta = sinf(theta);

			float x = cosPhi * cosTheta;
			float y = cosPhi * sinTheta;
			float z = sinPhi;

			glm::vec3 pos = glm::vec3(x * radius, y * radius, z * radius);
			glm::vec3 norm = glm::normalize(glm::vec3(x, y, z));
			sphVerts.push_back(pos);
			sphNormals.push_back(norm);
		}
	}

	for (int i = 0; i < stacks; ++i) {
		for (int j = 0; j < sectors; ++j) {
			int first = i * (sectors + 1) + j;
			int second = first + sectors + 1;
			// two triangles per sector
			sphIndices.push_back(first);
			sphIndices.push_back(second);
			sphIndices.push_back(first + 1);

			sphIndices.push_back(first + 1);
			sphIndices.push_back(second);
			sphIndices.push_back(second + 1);
		}
	}

	sphereIndexCount = (int)sphIndices.size();

	// upload to GPU
	glGenVertexArrays(1, &SphereVao);
	glBindVertexArray(SphereVao);

	glGenBuffers(1, &SphereVbo);
	glBindBuffer(GL_ARRAY_BUFFER, SphereVbo);

	size_t vertsSize = sphVerts.size() * sizeof(glm::vec3);
	size_t normsSize = sphNormals.size() * sizeof(glm::vec3);
	glBufferData(GL_ARRAY_BUFFER, vertsSize + normsSize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertsSize, &sphVerts[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vertsSize, normsSize, &sphNormals[0]);

	glGenBuffers(1, &SphereEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SphereEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphIndices.size() * sizeof(GLuint), &sphIndices[0], GL_STATIC_DRAW);

	// attribute 0 = position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	// attribute 1 = normal (offset = vertsSize)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertsSize));

	// unbind VAO
	glBindVertexArray(0);
}


// CreateVAO5 removed — not used

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &VaoId);
  
	// VboId5/EboId5 removed with CreateVAO5 elimination

		// stergere resurse sfera
		if (SphereVao) glDeleteVertexArrays(1, &SphereVao);
		if (SphereVbo) glDeleteBuffers(1, &SphereVbo);
		if (SphereEbo) glDeleteBuffers(1, &SphereEbo);
}

//  Crearea si compilarea obiectelor de tip shader;
void CreateShaders(void)
{
  ProgramId=LoadShaders("10_01_Shader.vert", "10_01_Shader.frag");
  glUseProgram(ProgramId);
}

// Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
  glDeleteProgram(ProgramId);
} 

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
};

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(0.95f, 0.82f, 0.4f, 1.0f); // culoarea de fond a ecranului
 
	// Incarcarea modelului 3D in format OBJ: incarcare directa din locatia proiectului
	// Construim o cale relativa pornind de la fisierul sursa compilat (__FILE__).
	std::string srcFile = __FILE__;
	std::string srcDir;
	size_t pos = srcFile.find_last_of("\\/");
	if (pos != std::string::npos) srcDir = srcFile.substr(0, pos);
	// din Lb10/Lb10 -> ../src/tor.obj
	std::string torPath = srcDir + std::string("\\..\\src\\tor.obj");
	bool model = loadOBJ(torPath.c_str(), vertices, uvs, normals);
	if (model) {
		std::cerr << "Loaded OBJ from: " << torPath << std::endl;
	} else {
		// Ultima incercare: incercam exact "tor.obj" in working directory (optional)
		model = loadOBJ("tor.obj", vertices, uvs, normals);
		if (model) std::cerr << "Loaded OBJ from working directory: tor.obj" << std::endl;
		else std::cerr << "Warning: failed to load tor.obj from '"<<torPath<<"' and working directory." << std::endl;
	}
	nrVertices = vertices.size();

	// Crearea VBO / shadere-lor
	CreateVBO();
	CreateShaders();

	// Locatii ptr shader
	nrVertLocation = glGetUniformLocation(ProgramId, "nrVertices");
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	viewPosLocation = glGetUniformLocation(ProgramId, "viewPos");
	viewLocation = glGetUniformLocation(ProgramId, "view");
	projLocation = glGetUniformLocation(ProgramId, "projection");

	// Variabile ce pot fi transmise catre shader
	glUniform1i(nrVertLocation, nrVertices);

	// creeaza sfera (radius, sectors, stacks)
	CreateSphere(1.0f, 40, 24);
}

//	Functia de desenare a graficii pe ecran;
void RenderFunction(void)
{
   // Initializare ecran + test de adancime;
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glEnable(GL_DEPTH_TEST);
   
   // Matricea de modelare 
   myMatrix = glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(0.0, 1.0, 0.0))
	   * glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(0.0, 0.0, 1.0));
   glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

   //	Vizualizare;
   //	Pozitia observatorului - se deplaseaza pe sfera;
   obsX = refX + dist * cos(alpha) * cos(beta);
   obsY = refY + dist * cos(alpha) * sin(beta);
   obsZ = refZ + dist * sin(alpha);
   //	Vectori pentru matricea de vizualizare;
   glm::vec3 obs = glm::vec3(obsX, obsY, obsZ);		//	Pozitia observatorului;	
   glm::vec3 pctRef = glm::vec3(refX, refY, refZ); 	//	Pozitia punctului de referinta;
   glm::vec3 vert = glm::vec3(vX, vY, vZ);			//	Verticala din planul de vizualizare; 
   // Pozitia observatorului, transmitere catre shader
   glUniform3f(viewPosLocation, obsX, obsY, obsZ);
   // Matricea de vizualizare, transmitere catre shader
   view = glm::lookAt(obs, pctRef, vert);
   glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

   //	Proiectie;
   projection = glm::infinitePerspective(GLfloat(fov), GLfloat(width) / GLfloat(height), dNear);
   glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

   // "Legarea"VAO, desenare;
   if (VaoId != 0 && vertices.size() > 0) {
	   glBindVertexArray(VaoId);
	   glEnableVertexAttribArray(0); // atributul 0 = pozitie
	   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	   glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
   }

   // Desenare sfera
   if (SphereVao && sphereIndexCount > 0) {
	   // setam matricea de model pentru sfera (mutata usor deasupra originii ca sa nu acopere torul)
	   glm::mat4 sphereModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.8f, 0.0f))
		   * glm::scale(glm::mat4(1.0f), glm::vec3(0.8f));
	   glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &sphereModel[0][0]);

	   glBindVertexArray(SphereVao);
	   glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

	   // restauram matricea model initiala
	   glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
   }

   glutSwapBuffers();
   glFlush ( );
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
  glutInitWindowPosition (100,100); 
  glutInitWindowSize(1200,900); 
  glutCreateWindow("Utilizarea unui model predefinit in format OBJ");
  glewInit(); 
  Initialize( );
  glutIdleFunc(RenderFunction);
  glutDisplayFunc(RenderFunction);
  glutKeyboardFunc(processNormalKeys);
  glutSpecialFunc(processSpecialKeys);
  glutCloseFunc(Cleanup);
  glutMainLoop();
}

