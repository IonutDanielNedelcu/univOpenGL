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
	VaoCarId,
	VboCarId,
	EboCarId,
	VaoBusId,
	VboBusId,
	EboBusId,
	ColorBufferId,
	myMatrixLocation,
	codColLocation,
	ProgramId;

GLuint
	matrTranslBusUpLocation, matrRotBusUpLocation, matrTranslCarUpLocation, matrRotCarUpLocation;
	

GLfloat
	winWidth = 1400, winHeight = 750;


float xMin = 0.f, xMax = 1920.f, yMin = 0.f, yMax = 1080.f;
float xBus=1920.0f, stepBus=0.1f, xCarUp=2500.0f, stepCarUp=0.13f, angleBus=0.0f, angleCarUp=0.0f;
float yBus = 0.0f, yCarUp = 0.0f;
float xBusRotationPoint = 1100.0f, yBusRotationPoint = 600.0f;
float xCarUpRotationPoint = 600.0f , yCarUpRotationPoint = 400.0f;
float xCarDown1 = -100.0f, yCarDown1 = 188.0f, xCarDown2 = -700.0f, yCarDown2 = 188.0f, stepCarDown1 = 0.15f, stepCarDown2 = 0.2f;
bool station = false;


glm::mat4
	myMatrix, resizeMatrix;

glm::mat4
	matrTranslBusUpX, matrTranslBusUpY, matrRotBusUp, matrTranslCarUp, matrRotCarUp,
	matrTranslBusToOrigin, matrTranslBusFromOrigin;

glm::mat4
    matrTranslCarDown1, matrTranslCarDown2, matrRotCarDownFull, matrTranslCarDownFull;

glm::vec3 busRotationPoint1(xBusRotationPoint, yBusRotationPoint, 0.0f),
			upCarRotationPoint1(xCarUpRotationPoint, yCarUpRotationPoint, 0.0f);


void MoveThings(void)
{

	if (xBus > 600)
	{
		xBus = xBus - 0.15;
	}
	else if (angleBus >= -0.13 && yBus < 100)
	{
		angleBus = angleBus - 0.0001;
		xBus = xBus - 0.18;
		yBus = yBus + 0.1;
		station = true;
	}
	else if (angleBus <= 0 && xBus > 200 && station == true)
	{
		xBusRotationPoint = 100.0f;
		yBusRotationPoint = 1000.0f;
		angleBus = angleBus + 0.00005;
		xBus = xBus - (stepBus);
		yBus = yBus + 0.05;
	}

	if (xCarUp > 900)
	{
		xCarUp = xCarUp - 0.2;
	}
	else
	{
		xCarUp = xCarUp - 0.4;
		if (yCarUp > -160)
			yCarUp = yCarUp - 0.1;
	}
	if (xCarUp < -700.0)
	{
		xBus = 1920.0f;
		yBus = 0.0f;
		xCarUp = 2500.0f;
		yCarUp = 0.0f;
		station = false;
		angleBus = 0.0f;
	}


	xCarDown1 = xCarDown1 + stepCarDown1;
    xCarDown2 = xCarDown2 + stepCarDown2;

    // cand masina 2 se apropie de masina 1, incepe depasirea 
    if (xCarDown2 >= xCarDown1 - 400.0f && xCarDown2 <= xCarDown1 + 300.0f) {
        if (yCarDown2 < 364.0f) // centrul benzii a doua
            yCarDown2 = yCarDown2 + 0.05f; 
    }

    if (yCarDown2 > 364.0f) {
        yCarDown2 = 364.0f; 
    }

    // resetez pozitiile cand masinile de jos ies din ecran
    if (xCarDown1 > xMax + 500.0) {
        xCarDown1 = -100.0;
        xCarDown2 = -700.0;
        yCarDown2 = 188.0f; 
    }

	glutPostRedisplay();
}


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

void CreateCarVBO(void)
{
	// varfurile pentru Mini Cooper
	GLfloat VerticesCar[] = {
		// corpul principal
		500.0f, 680.0f, 0.0f, 1.0f,		0.85f, 0.15f, 0.15f, 1.0f,
		620.0f, 680.0f, 0.0f, 1.0f,		0.85f, 0.15f, 0.15f, 1.0f,
		620.0f, 750.0f, 0.0f, 1.0f,		0.85f, 0.15f, 0.15f, 1.0f,
		500.0f, 750.0f, 0.0f, 1.0f,		0.85f, 0.15f, 0.15f, 1.0f,

		// parbrizul
		535.0f, 692.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,
		555.0f, 692.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,
		555.0f, 738.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,
		535.0f, 738.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,

		// plafonul alb
		555.0f, 687.0f, 0.0f, 1.0f,		0.95f, 0.95f, 0.95f, 1.0f,
		605.0f, 687.0f, 0.0f, 1.0f,		0.95f, 0.95f, 0.95f, 1.0f,
		605.0f, 743.0f, 0.0f, 1.0f,		0.95f, 0.95f, 0.95f, 1.0f,
		555.0f, 743.0f, 0.0f, 1.0f,		0.95f, 0.95f, 0.95f, 1.0f,

		// luneta 
		605.0f, 692.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,
		615.0f, 692.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,
		615.0f, 738.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,
		605.0f, 738.0f, 0.0f, 1.0f,		0.40f, 0.70f, 0.90f, 1.0f,

		// roata fata stanga
		510.0f, 675.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		540.0f, 675.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		540.0f, 680.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		510.0f, 680.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// roata fata dreapta
		510.0f, 750.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		540.0f, 750.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		540.0f, 755.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		510.0f, 755.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// roata spate stanga
		580.0f, 675.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		610.0f, 675.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		610.0f, 680.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		580.0f, 680.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// roata spate dreapta
		580.0f, 750.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		610.0f, 750.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		610.0f, 755.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		580.0f, 755.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// farurile galbene 
		496.0f, 695.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		502.0f, 695.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		502.0f, 703.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		496.0f, 703.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,

		496.0f, 727.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		502.0f, 727.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		502.0f, 735.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		496.0f, 735.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,

		// stopurile rosii 
		618.0f, 695.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		624.0f, 695.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		624.0f, 708.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		618.0f, 708.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,

		618.0f, 722.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		624.0f, 722.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		624.0f, 735.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		618.0f, 735.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,

		// lumina far 1
		496.0f, 695.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,
		410.0f, 660.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,
		410.0f, 740.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,
		496.0f, 703.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,

		// lumina far 2
		496.0f, 727.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,
		410.0f, 690.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,
		410.0f, 772.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,
		496.0f, 735.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 0.2f,
	};

	GLuint IndicesCar[] = {
		// corpul 
		0, 1, 2,
		0, 2, 3,

		// parbrizul
		4, 5, 6,
		4, 6, 7,

		// plafonul alb
		8, 9, 10,
		8, 10, 11,

		// luneta
		12, 13, 14,
		12, 14, 15,

		// roata fata stanga
		16, 17, 18,
		16, 18, 19,

		// roata fata dreapta
		20, 21, 22,
		20, 22, 23,

		// roata spate stanga
		24, 25, 26,
		24, 26, 27,

		// roata spate dreapta
		28, 29, 30,
		28, 30, 31,

		// farurile
		32, 33, 34,
		32, 34, 35,
		36, 37, 38,
		36, 38, 39,

		// stopurile
		40, 41, 42,
		40, 42, 43,
		44, 45, 46,
		44, 46, 47,

		// lumina farurilor
		48, 49, 50,
		48, 50, 51,
		52, 53, 54,
		52, 54, 55,
	};

	// se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO
	glGenVertexArrays(1, &VaoCarId);
	glBindVertexArray(VaoCarId);

	// se creeaza un buffer nou pentru masina
	glGenBuffers(1, &VboCarId);
	// este setat ca buffer curent
	glBindBuffer(GL_ARRAY_BUFFER, VboCarId);
	// varfurile sunt "copiate" in bufferul curent
	glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesCar), VerticesCar, GL_STATIC_DRAW);

	//	Se creeaza un buffer pentru indici
	glGenBuffers(1, &EboCarId);														//  Generarea bufferului si indexarea acestuia catre variabila EboCarId
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboCarId);									//  Setarea tipului de buffer - atributele varfurilor
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesCar), IndicesCar, GL_STATIC_DRAW);

	//	Se activeaza lucrul cu atribute
	//  Se asociaza atributul (0 = coordonate) pentru shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	//  Se asociaza atributul (1 =  culoare) pentru shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
	// atentie - culorile functioneaza aici cu RGBa, au 4 valori
}

void CreateBusVBO(void)
{
	// varfurile pentru autobuz
	GLfloat VerticesBus[] = {
		// corpul principal 
		150.0f, 660.0f, 0.0f, 1.0f,		1.0f, 0.85f, 0.0f, 1.0f,
		400.0f, 660.0f, 0.0f, 1.0f,		1.0f, 0.85f, 0.0f, 1.0f,
		400.0f, 770.0f, 0.0f, 1.0f,		1.0f, 0.85f, 0.0f, 1.0f,
		150.0f, 770.0f, 0.0f, 1.0f,		1.0f, 0.85f, 0.0f, 1.0f,

		// parbrizul
		170.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		190.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		190.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		170.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// plafonul
		190.0f, 670.0f, 0.0f, 1.0f,		1.0f, 0.90f, 0.2f, 1.0f,
		390.0f, 670.0f, 0.0f, 1.0f,		1.0f, 0.90f, 0.2f, 1.0f,
		390.0f, 760.0f, 0.0f, 1.0f,		1.0f, 0.90f, 0.2f, 1.0f,
		190.0f, 760.0f, 0.0f, 1.0f,		1.0f, 0.90f, 0.2f, 1.0f,

		// roata fata stanga
		180.0f, 655.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		220.0f, 655.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		220.0f, 660.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		180.0f, 660.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// roata fata dreapta
		180.0f, 770.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		220.0f, 770.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		220.0f, 775.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		180.0f, 775.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// roata spate stanga
		330.0f, 655.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		370.0f, 655.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		370.0f, 660.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		330.0f, 660.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// roata spate dreapta
		330.0f, 770.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		370.0f, 770.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		370.0f, 775.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,
		330.0f, 775.0f, 0.0f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,

		// far stanga
		145.0f, 670.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		150.0f, 670.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		150.0f, 680.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		145.0f, 680.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,

		// far dreapta
		145.0f, 750.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		150.0f, 750.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		150.0f, 760.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,
		145.0f, 760.0f, 0.0f, 1.0f,		1.0f, 1.0f, 0.0f, 1.0f,

		// stop stanga
		400.0f, 665.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		406.0f, 665.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		406.0f, 678.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		400.0f, 678.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,

		// stop dreapta
		400.0f, 752.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		406.0f, 752.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		406.0f, 765.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,
		400.0f, 765.0f, 0.0f, 1.0f,		0.90f, 0.10f, 0.10f, 1.0f,

		// geam 1 stanga 
		195.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		225.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		225.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		195.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// geam 2 stanga
		270.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		300.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		300.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		270.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// geam 3 stanga
		310.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		340.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		340.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		310.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// geam 4 stanga
		350.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		380.0f, 665.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		380.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		350.0f, 670.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// geam 1 dreapta 
		195.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		225.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		225.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		195.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// geam 2 dreapta
		270.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		300.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		300.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		270.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// geam 3 dreapta
		310.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		340.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		340.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		310.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// geam 4 dreapta
		350.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		380.0f, 760.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		380.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,
		350.0f, 765.0f, 0.0f, 1.0f,		0.5f, 0.7f, 0.9f, 1.0f,

		// lumina far 1
		145.0f, 670.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,
		50.0f, 610.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,
		50.0f, 740.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,
		145.0f, 680.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,

		// lumina far 2
		145.0f, 750.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,
		50.0f, 690.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,
		50.0f, 820.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,
		145.0f, 760.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.2f,
	};

	GLuint IndicesBus[] = {
		// corpul principal 
		0, 1, 2,
		0, 2, 3,

		// parbrizul
		4, 5, 6,
		4, 6, 7,

		// plafonul
		8, 9, 10,
		8, 10, 11,

		// roata fata stanga
		12, 13, 14,
		12, 14, 15,

		// roata fata dreapta
		16, 17, 18,
		16, 18, 19,

		// roata spate stanga
		20, 21, 22,
		20, 22, 23,

		// roata spate dreapta
		24, 25, 26,
		24, 26, 27,

		28, 29, 30,
		28, 30, 31,
		32, 33, 34,
		32, 34, 35,

		36, 37, 38,
		36, 38, 39,
		40, 41, 42,
		40, 42, 43,

		// geam 1 stanga 
		44, 45, 46,
		44, 46, 47,

		// geam 2 stanga 
		48, 49, 50,
		48, 50, 51,

		// geam 3 stanga 
		52, 53, 54,
		52, 54, 55,

		// geam 4 stanga 
		56, 57, 58,
		56, 58, 59,

		// geam 1 dreapta
		60, 61, 62,
		60, 62, 63,

		// geam 2 dreapta
		64, 65, 66,
		64, 66, 67,

		// geam 3 dreapta
		68, 69, 70,
		68, 70, 71,

		// geam 4 dreapta
		72, 73, 74,
		72, 74, 75,

		// lumina faruri
		76, 77, 78,
		76, 78, 79,
		80, 81, 82,
		80, 82, 83,
	};

	// se creeaza / se leaga un VAO (Vertex Array Object)
	glGenVertexArrays(1, &VaoBusId);
	glBindVertexArray(VaoBusId);

	// se creeaza un buffer nou pentru autobuz
	glGenBuffers(1, &VboBusId);
	// este setat ca buffer curent
	glBindBuffer(GL_ARRAY_BUFFER, VboBusId);
	// varfurile sunt "copiate" in bufferul curent
	glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesBus), VerticesBus, GL_STATIC_DRAW);

	//	Se creeaza un buffer pentru indici
	glGenBuffers(1, &EboBusId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboBusId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesBus), IndicesBus, GL_STATIC_DRAW);

	//	Se activeaza lucrul cu atribute
	//  Se asociaza atributul (0 = coordonate) pentru shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	//  Se asociaza atributul (1 =  culoare) pentru shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(4 * sizeof(GLfloat)));
}

void DestroyBusVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &EboBusId);
	glDeleteBuffers(1, &VboBusId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoBusId);
}

void DestroyCarVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &EboCarId);
	glDeleteBuffers(1, &VboCarId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoCarId);
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
	CreateCarVBO();
	CreateBusVBO();
	CreateShaders();

	codColLocation = glGetUniformLocation(ProgramId, "codCol");
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");

	resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);

	glutIdleFunc(MoveThings);
}
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);       

	matrTranslBusUpX = glm::translate(glm::mat4(1.0f), glm::vec3(xBus, yBus, 0.0f));
	matrRotBusUp = glm::rotate(glm::mat4(1.0f), angleBus, glm::vec3(0.0f, 0.0f, 1.0f));
	matrTranslBusToOrigin = glm::translate(glm::mat4(1.0f), -busRotationPoint1);
	matrTranslBusFromOrigin = glm::translate(glm::mat4(1.0f), busRotationPoint1);
	matrTranslCarUp = glm::translate(glm::mat4(1.0f), glm::vec3(xCarUp, yCarUp, 0.0));

	matrRotCarDownFull = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0));
    matrTranslCarDownFull = glm::translate(glm::mat4(1.0f), glm::vec3(-510.0f, -715.0f, 0.0));
    matrTranslCarDown1 = glm::translate(glm::mat4(1.0f), glm::vec3(xCarDown1, yCarDown1, 0.0));
    matrTranslCarDown2 = glm::translate(glm::mat4(1.0f), glm::vec3(xCarDown2, yCarDown2, 0.0));

	// redimensionarea ferestrei
	myMatrix = resizeMatrix;

	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	// ===== DESENARE BACKGROUND =====
	glBindVertexArray(VaoId);  // Activam VAO-ul pentru fundal

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

	// activez blending pentru a desena farurile cu transparenta
		glEnable(GL_BLEND);
	// setez functia de blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);




	// matricea animatiei autobuzului de sus

	myMatrix = resizeMatrix * matrTranslBusFromOrigin * matrRotBusUp * matrTranslBusToOrigin * matrTranslBusUpX;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	// ===== DESENARE AUTOBUZ SCOLAR =====
	glBindVertexArray(VaoBusId);
	
	// desenez toate componentele autobuzului
	glDrawElements(GL_TRIANGLES, 126, GL_UNSIGNED_INT, 0);



	// matricea animatiei masinii de sus

	myMatrix = resizeMatrix * matrTranslCarUp;
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

	// ===== DESENARE MINI COOPER =====
	glBindVertexArray(VaoCarId);
	
	// desenez toate componentele masinii 
	glDrawElements(GL_TRIANGLES, 90, GL_UNSIGNED_INT, 0); 


	
	// animatia masinii de jos care merge mai incet
    myMatrix = resizeMatrix * matrTranslCarDown1 * matrRotCarDownFull * matrTranslCarDownFull;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    // desenez toate componentele masinii 
    glDrawElements(GL_TRIANGLES, 90, GL_UNSIGNED_INT, 0); 



    // animatia masinii de jos care merge mai repede
    myMatrix = resizeMatrix * matrTranslCarDown2 * matrRotCarDownFull * matrTranslCarDownFull;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    // desenez toate componentele masinii 
    glDrawElements(GL_TRIANGLES, 90, GL_UNSIGNED_INT, 0); 


    // inlocuiesc imaginea deseneata in fereastra cu cea randata 
    glutSwapBuffers();
	glFlush();
}

void Cleanup(void)
{
	DestroyShaders();
	DestroyBusVBO();
	DestroyCarVBO();  
	DestroyVBO();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(0, 0); // pozitia initiala a ferestrei
	glutInitWindowSize(winWidth, winHeight); //dimensiunile ferestrei
	glutCreateWindow("Grafica pe calculator - Proiect 1"); // titlul ferestrei
	glewInit(); // nu uitati de initializare glew; trebuie initializat inainte de a a initializa desenarea
	Initialize();
	glutDisplayFunc(RenderFunction);
	glutCloseFunc(Cleanup);
	glutMainLoop();
}

