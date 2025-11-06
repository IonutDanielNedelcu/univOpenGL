// Codul sursa este adaptat dupa OpenGLBook.com


#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "SOIL.h"			//	Biblioteca pentru texturare;
//////////////////////////////////////



GLuint
	VaoId,
	VboBackgroundId,
	EboBackgroundId,
	ColorBufferId,
	myMatrixLocation,
	codColLocation,
	ProgramId;

GLfloat
	winWidth = 1400, winHeight = 750;


float xMin = 0.f, xMax = 1920.f, yMin = 0.f, yMax = 1080.f;


glm::mat4
	myMatrix, resizeMatrix;




void CreateVBO(void)
{
	// varfurile 
	GLfloat VerticesBackground[] = {
		// background grass
		0.0f,  0.0f, 0.0f, 1.0f,        0.196f, 0.705f, 0.196f, 1.0f, // culoarea margini - daca gasesti una mai potrivita e super
		1920.0f, 0.0f, 0.0f, 1.0f,      0.196f, 0.705f, 0.196f, 1.0f,
		1920.0f, 1080.0f, 0.0f, 1.0f,   0.196f, 0.705f, 0.196f, 1.0f,
		0.0f, 1080.0f, 0.0f, 1.0f,      0.196f, 0.705f, 0.196f, 1.0f,
		// 4

		// street
		0.0f, 100.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,
		1920.0f, 100.0f, 0.0f, 1.0f,	0.20f, 0.20f, 0.20f, 1.0f,
		1920.0f, 804.0f, 0.0f, 1.0f,	0.20f, 0.20f, 0.20f, 1.0f,
		0.0f, 804.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,
		// 8

		//alveola sus
		150.0f, 804.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,
		250.0f, 804.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,
		900.0f, 804.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,
		1000.0f, 804.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,

		250.0f, 980.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,
		900.0f, 980.0f, 0.0f, 1.0f,		0.20f, 0.20f, 0.20f, 1.0f,
		// 14


		// 5 benzi - 2 pe un sens, 3 pe celalalt (2+alveola autobuz) (176px fiecare banda)

		//		STREET LINES
		
		// delimitatorul dintre sensuri - 3 benzi sus, 2 jos
		0.0f, 452.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,
		1920.0f, 452.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,

		// banda de pe sensul de jos
		0.0f, 276.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,
		1920.0f, 276.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,

		// banda de pe sensul de sus
		0.0f, 628.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,
		1920.0f, 628.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,

		// delimitarea de alveola
		150.0f, 804.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,
		1000.0f, 804.0f, 0.0f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,
		// 22
	};

	GLuint IndicesBackground[] = {
		// background grass
		0, 1, 2, // Primul triunghi
		0, 2, 3, // Al doilea triunghi

		// street
		4, 5, 6,
		4, 6, 7,

		// alveola
		8, 9, 12,
		12, 13, 9,
		13, 9, 10,
		10, 11, 13,

		// street lines
		14, 15,
		16, 17,
		18, 19,
		20, 21
	};



	// culorile, ca atribute ale varfurilor
	/*GLfloat Colors[] = {
	  1.0f, 0.5f, 0.2f, 1.0f,
	  1.0f, 0.5f, 0.2f, 1.0f,
	  1.0f, 0.5f, 0.2f, 1.0f,
	  1.0f, 0.5f, 0.2f, 1.0f,
	  1.0f, 0.5f, 0.2f, 1.0f,
	  1.0f, 0.5f, 0.2f, 1.0f,
	};*/



	// se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);

	// se creeaza un buffer nou
	glGenBuffers(1, &VboBackgroundId);
	// este setat ca buffer curent
	glBindBuffer(GL_ARRAY_BUFFER, VboBackgroundId);
	// varfurile sunt "copiate" in bufferul curent
	glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesBackground), VerticesBackground, GL_STATIC_DRAW);

	//	Se creeaza un buffer pentru INDICI;
	glGenBuffers(1, &EboBackgroundId);														//  Generarea bufferului si indexarea acestuia catre variabila EboId;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboBackgroundId);									//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesBackground), IndicesBackground, GL_STATIC_DRAW);



	//	Se activeaza lucrul cu atribute;
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	// atentie - culorile functioneaza aici cu RGBa, au 4 valori
	

}
void DestroyVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ColorBufferId);
	glDeleteBuffers(1, &VboBackgroundId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
	ProgramId = LoadShaders("example.vert", "example.frag");
	glUseProgram(ProgramId);
}
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

void Initialize(void)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // culoarea de fond a ecranului
	CreateVBO();
	CreateShaders();


	codColLocation = glGetUniformLocation(ProgramId, "codCol");
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");

	resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
}
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);       

	// redimensionarea ferestrei
	myMatrix = resizeMatrix;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);



	//	Desenarea primitivelor;
	//	Functia glDrawElements primeste 4 argumente:
	//	 - arg1 = modul de desenare;
	//	 - arg2 = numarul de varfuri;
	//	 - arg3 = tipul de date al indicilor;
	//	 - arg4 = pointer spre indici (EBO): pozitia de start a indicilor;
	glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);

	glLineWidth(5);
	glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (GLvoid*)(24 * sizeof(GL_UNSIGNED_INT)));

	glEnable(GL_LINE_STIPPLE); // linie punctata
	glLineStipple(4, 0xFF00); 
	glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, (GLvoid*)(26 * sizeof(GL_UNSIGNED_INT)));
	glDisable(GL_LINE_STIPPLE); // dezactivez linia punctata

	glFlush();
}
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(0, 0); // pozitia initiala a ferestrei
	glutInitWindowSize(winWidth, winHeight); //dimensiunile ferestrei
	glutCreateWindow("Grafica pe calculator - Proiect 1"); // titlul ferestrei
	glewInit(); // nu uitati de initializare glew; trebuie initializat inainte de a a initializa desenarea
	Initialize();
	glutDisplayFunc(RenderFunction);
	glutCloseFunc(Cleanup);
	glutMainLoop();
}

