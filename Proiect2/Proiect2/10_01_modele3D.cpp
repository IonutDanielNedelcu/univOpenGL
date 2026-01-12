//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Proiect 2 - Grafica 3D                       |
// ================================================
//

// Biblioteci
#include <windows.h>  
#include <stdio.h>
#include <stdlib.h> 
#include <cstdlib> 
#include <vector>
#include <math.h>
#include <iostream>
#include <random>
#include <GL/glew.h> 
#include <GL/freeglut.h> 
#include "SOIL.h"

#include "loadShaders.h"

#include "glm/glm.hpp"  
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "objloader.hpp"  

// --- IDENTIFICATORI OPENGL ---
GLuint ProgramId;
GLuint nrVertLocation, myMatrixLocation, viewPosLocation, viewLocation, projLocation;
GLint colorMulLocation = -1, alphaMulLocation = -1;
GLint fireLightPosLocation = -1, fireLightColorLocation = -1, fireLightIntensityLocation = -1;
// base fire intensity (used for flicker modulation)
float fireBaseIntensity = 2.5f;

// 1. Variabile SUFRAGERIE
GLuint VaoIdRoom, VboIdRoom, TextureRoom;
int nrVerticesRoom;
std::vector<glm::vec3> roomVertices;
std::vector<glm::vec2> roomUvs;
std::vector<glm::vec3> roomNormals;

// 2. Variabile BRAD (Generat manual)
GLuint VaoIdTree, VboIdTree, TextureTree;
int nrVerticesTree;
std::vector<glm::vec3> treeVertices;
std::vector<glm::vec2> treeUvs;
std::vector<glm::vec3> treeNormals;

// --- INSTALATIE (fir + beculete) ---
GLuint VaoIdWire = 0, VboIdWire = 0;
int nrWirePoints = 0;

GLuint VaoIdBulb = 0, VboIdBulb = 0;
int nrVerticesBulb = 0;
std::vector<glm::vec3> bulbVerts;
std::vector<glm::vec3> bulbNormals;
std::vector<glm::vec2> bulbUvs;

std::vector<glm::vec3> bulbLocalPositions; // positions on cone (local model space)
std::vector<glm::vec3> bulbColors;
std::vector<float> bulbIntensities;
int bulbCount = 0;

// shader uniform locations for bulbs
GLint bulbCountLocation = -1;
GLint bulbPosLocation = -1;
GLint bulbColorLocation = -1;
GLint bulbIntensityLocation = -1;

GLuint WhiteTexture = 0;
GLuint BlackTexture = 0;
GLint useSolidColorLocation = -1;
GLint solidColorLocation = -1;

// SETARI INSTALATIE 
// 1 = Static, 2 = Palpait, 3 = Pe sarite, 4 = Oprit
int lightMode = 1;

// SETARI GENERALE 
float PI = 3.141592f;
glm::mat4 myMatrix, view, projection;

// TINTA Camerei (Centrul modelului)
float refX = 0.0f, refY = 0.0f, refZ = 0.0f;
// Pozitia OBSERVATORULUI
float obsX, obsY, obsZ;

// SETARI CAMERA 
float alpha = 0.2f;
float beta = -0.5f;
float dist = 6.0f;

float minAlpha = 0.01f;
float maxAlpha = 1.5f;
float incrAlpha = 0.05f;
float incrBeta = 0.05f;

// Proiectie
float width = 1200, height = 900, dNear = 0.1f, fov = 60.f * PI / 180;

// SETARI POZITIE BRAD 
glm::vec3 treePosition(2.9f, 3.0f, -0.5f);
// scalarea bradului
float treeScale = 0.5f;

// allow black texture uniform location
GLint allowBlackLocation = -1;


// --- FUNCTIE GENERARE CON (BRAD) ---
// Aceasta functie inlocuieste incarcarea din fisier .obj
void CreateProceduralCone(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs)
{
    verts.clear(); norms.clear(); uvs.clear();

    float height = 2.5f;    // Inaltime brad
    float radius = 1.0f;    // Latime baza
    int segments = 32;      // Rotunjime

    glm::vec3 topPoint(0.0f, height, 0.0f);
    glm::vec3 centerBottom(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < segments; i++) {
        float angle1 = (float)i / segments * 2.0f * PI;
        float angle2 = (float)(i + 1) / segments * 2.0f * PI;

        float x1 = cos(angle1) * radius;
        float z1 = sin(angle1) * radius;
        float x2 = cos(angle2) * radius;
        float z2 = sin(angle2) * radius;

        glm::vec3 p1(x1, 0.0f, z1);
        glm::vec3 p2(x2, 0.0f, z2);

        // Fata laterala
        verts.push_back(topPoint);
        verts.push_back(p1);
        verts.push_back(p2);

        // Calcul normala
        glm::vec3 n = glm::normalize(glm::cross(p1 - topPoint, p2 - topPoint));
        norms.push_back(n); norms.push_back(n); norms.push_back(n);

        // Texturare simpla
        uvs.push_back(glm::vec2(0.5f, 1.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));
        uvs.push_back(glm::vec2(1.0f, 0.0f));

        // Baza (ca sa nu se vada gol pe dedesubt)
        verts.push_back(centerBottom);
        verts.push_back(p2);
        verts.push_back(p1);

        norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
        norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
        norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f));

        uvs.push_back(glm::vec2(0.5f, 0.5f));
        uvs.push_back(glm::vec2(1.0f, 0.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));
    }
}


// 3. Variabile STEA (Star)
GLuint VaoIdStar, VboIdStar;
int nrVerticesStar;
std::vector<glm::vec3> starVertices;
std::vector<glm::vec3> starNormals;
std::vector<glm::vec2> starUvs;

// Functie pentru generarea unei stele 3D
void CreateProceduralStar(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs)
{
    verts.clear(); norms.clear(); uvs.clear();

    float outerRadius = 0.25f; // Raza varfurilor
    float innerRadius = 0.10f; // Raza interiorului
    float thickness = 0.05f;   // Grosimea stelei (cat de "grasa" e 3D)
    int numPoints = 5;         // Stea in 5 colturi

    // Centrul fetei si spatelui
    glm::vec3 centerFront(0.0f, 0.0f, thickness);
    glm::vec3 centerBack(0.0f, 0.0f, -thickness);

    for (int i = 0; i < numPoints * 2; ++i) {
        // Calculam unghiurile curent si urmator
        float angle1 = (float)i / (numPoints * 2) * 2.0f * PI;
        float angle2 = (float)(i + 1) / (numPoints * 2) * 2.0f * PI;

        // Raza alterneaza: Varf -> Interior -> Varf -> Interior
        float r1 = (i % 2 == 0) ? outerRadius : innerRadius;
        float r2 = ((i + 1) % 2 == 0) ? outerRadius : innerRadius;

        glm::vec3 p1(r1 * cos(angle1 + PI / 2), r1 * sin(angle1 + PI / 2), 0.0f);
        glm::vec3 p2(r2 * cos(angle2 + PI / 2), r2 * sin(angle2 + PI / 2), 0.0f);

        // --- FATA (Front face) ---
        // Triunghi: CenterFront -> p1 -> p2 (deplasate spre Z+)
        glm::vec3 pf1 = p1; pf1.z = thickness;
        glm::vec3 pf2 = p2; pf2.z = thickness; // (la baza, de fapt le unim cu centrul bombat)
        // Simplificare: Stea piramidala pe ambele parti
        // Unim p1(z=0) si p2(z=0) cu CenterFront

        // Triunghi FATA
        verts.push_back(centerFront); verts.push_back(p1); verts.push_back(p2);
        // Normala aproximativa (fata)
        glm::vec3 nF = glm::normalize(glm::cross(p1 - centerFront, p2 - centerFront));
        norms.push_back(nF); norms.push_back(nF); norms.push_back(nF);

        // Triunghi SPATE (inversam ordinea pentru cull face)
        verts.push_back(centerBack); verts.push_back(p2); verts.push_back(p1);
        // Normala aproximativa (spate)
        glm::vec3 nB = glm::normalize(glm::cross(p2 - centerBack, p1 - centerBack));
        norms.push_back(nB); norms.push_back(nB); norms.push_back(nB);

        // UVs dummy (nu folosim textura, ci culoare solida)
        uvs.push_back(glm::vec2(0.5f, 0.5f)); uvs.push_back(glm::vec2(1, 0)); uvs.push_back(glm::vec2(0, 0));
        uvs.push_back(glm::vec2(0.5f, 0.5f)); uvs.push_back(glm::vec2(1, 0)); uvs.push_back(glm::vec2(0, 0));
    }
}


// VARIABILE SEMINEU
GLuint VaoIdFireplace, VboIdFireplace, TextureFireplace;
int nrVerticesFireplace;
std::vector<glm::vec3> fireplaceVertices;
std::vector<glm::vec2> fireplaceUvs; // texturare
std::vector<glm::vec3> fireplaceNormals;

// VARIABILE BLAT SEMINEU
GLuint VaoIdFireplaceTop, VboIdFireplaceTop, TextureFireplaceTop;
int nrVerticesFireplaceTop;
std::vector<glm::vec3> fireplaceTopVertices;
std::vector<glm::vec2> fireplaceTopUvs; // texturare
std::vector<glm::vec3> fireplaceTopNormals;

// VARIABILE LEMNE
GLuint VaoIdLogs = 0, VboIdLogs = 0; 
int nrVerticesLogs = 0;
std::vector<glm::vec3> logVertices;
std::vector<glm::vec3> logNormals;
std::vector<glm::vec2> logUvs; // texturare
GLuint TextureLogs = 0;

// Structura pentru a genera mai multe instante ale aceluiasi lemn
struct LogInstance {
    glm::vec3 pos; // local to fireplace model
    glm::vec3 size; // dimensiuni (X = lungime, Y = inaltime, Z = grosime)
    float rotY;     // rotatia in jurul lui Y
};
std::vector<LogInstance> logs;

// VARIABILE FOC
GLuint VaoIdFire, VboIdFire;
int nrVerticesFire;
std::vector<glm::vec3> fireVertices;
std::vector<glm::vec3> fireNormals;
std::vector<glm::vec2> fireUvs; // texturare

// PARTICULE FUM
struct SmokeParticle {
    glm::vec3 pos;
    glm::vec3 vel; // velocitatea (viteza) particulei
    float life; // in [0,1]; cat % din "viata" mai are particula
    float size;
};
std::vector<SmokeParticle> smokeParticles;

// VARIABILE FUM
GLuint VaoIdSmoke = 0, VboIdSmoke = 0;
GLuint ParticleProgram = 0;
bool smokeUseFallbackShader = false;
int maxSmokeParticles = 800; // nr maxim de particule existente per total
float smokeSpawnRate = 240.0f; // particule pe secunda
float lastSmokeTime = 0.0f;
float lastFrameTime = 0.0f;
float smokeCeilingFactor = 0.45f; // la cat % pana la tavan dispare fumul

// VARIABILE SEMINEU
glm::vec3 fireplacePosition(-2.0f, 0.0f, -1.0f);
float fireplaceScale = 0.8f; // micsorez semineul initial
// dimensiunile semineului
float fireplaceWidth = 1.2f;
float fireplaceHeight = 1.0f;
float fireplaceDepth = 0.6f;

// offset-uri pentru semineu si foc in functie de pozitia initiala pentru a le muta
glm::vec3 fireplaceOffset(3.0f, -0.19f, -2.9f);
glm::vec3 fireOffset(-0.1f, 0.0f, 0.0f);

// functia pentru crearea semineului
void CreateFireplace(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs)
{
    // ma asigur ca vectorii sunt curatati
    verts.clear(); norms.clear(); uvs.clear();

    // punctele pentru semineu
    glm::vec3 p0(0.0f, 0.0f, 0.0f);
    glm::vec3 p1(fireplaceWidth, 0.0f, 0.0f);
    glm::vec3 p2(fireplaceWidth, 0.0f, fireplaceDepth);
    glm::vec3 p3(0.0f, 0.0f, fireplaceDepth);
    glm::vec3 p4(0.0f, fireplaceHeight, 0.0f);
    glm::vec3 p5(fireplaceWidth, fireplaceHeight, 0.0f);
    glm::vec3 p6(fireplaceWidth, fireplaceHeight, fireplaceDepth);
    glm::vec3 p7(0.0f, fireplaceHeight, fireplaceDepth);

    // Podeaua
    verts.push_back(p0); verts.push_back(p1); verts.push_back(p2);
    verts.push_back(p0); verts.push_back(p2); verts.push_back(p3);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // "Tavanul" semineului
    verts.push_back(p4); verts.push_back(p5); verts.push_back(p6);
    verts.push_back(p4); verts.push_back(p6); verts.push_back(p7);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Spate
    verts.push_back(p1); verts.push_back(p0); verts.push_back(p4);
    verts.push_back(p1); verts.push_back(p4); verts.push_back(p5);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Fata
    float sideWidthOpen = 0.18f; // latimea deschiderii semineului
    float openingHeight = 0.65f; // inaltimea deschiderii semineului

    // Coordonatele panoului din fata
    float fx0 = 0.0f;
    float fx1 = sideWidthOpen;
    float fx2 = fireplaceWidth - sideWidthOpen;
    float fx3 = fireplaceWidth;
    float fy0 = 0.0f;
    float fy1 = openingHeight;
    float fy2 = fireplaceHeight;
    float fz = fireplaceDepth; // put opening on back plane (z = depth)

    // Panoul din stanga
    glm::vec3 L0(fx0, fy0, fz);
    glm::vec3 L1(fx1, fy0, fz);
    glm::vec3 L2(fx1, fy2, fz);
    glm::vec3 L3(fx0, fy2, fz);
    verts.push_back(L1); verts.push_back(L0); verts.push_back(L3);
    verts.push_back(L1); verts.push_back(L3); verts.push_back(L2);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Panoul din dreapta
    glm::vec3 R0(fx2, fy0, fz);
    glm::vec3 R1(fx3, fy0, fz);
    glm::vec3 R2(fx3, fy2, fz);
    glm::vec3 R3(fx2, fy2, fz);
    verts.push_back(R1); verts.push_back(R0); verts.push_back(R3);
    verts.push_back(R1); verts.push_back(R3); verts.push_back(R2);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Panoul de deasupra deschiderii
    glm::vec3 T0(fx1, fy1, fz);
    glm::vec3 T1(fx2, fy1, fz);
    glm::vec3 T2(fx2, fy2, fz);
    glm::vec3 T3(fx1, fy2, fz);
    verts.push_back(T0); verts.push_back(T1); verts.push_back(T2);
    verts.push_back(T0); verts.push_back(T2); verts.push_back(T3);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Peretii din interiorul deschiderii semineului
    float innerOffset = 0.30f; // offset-ul fata de exteriorul semineului
    float innerZ = fireplaceDepth - innerOffset;
    float ix0 = fx1; // inner min x
    float ix1 = fx2; // inner max x
    float iy0 = fy0; // inner min y
    float iy1 = fy1; // inner max y (top of opening)

    // Paretele din spate al cavitatii
    glm::vec3 innerBack0(ix0, iy0, innerZ);
    glm::vec3 innerBack1(ix1, iy0, innerZ);
    glm::vec3 innerBack2(ix1, iy1, innerZ);
    glm::vec3 innerBack3(ix0, iy1, innerZ);
    verts.push_back(innerBack1); verts.push_back(innerBack0); verts.push_back(innerBack3);
    verts.push_back(innerBack1); verts.push_back(innerBack3); verts.push_back(innerBack2);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Peretele din stanga al cavitatii
    glm::vec3 innerLeft0(ix0, iy0, innerZ);
    glm::vec3 innerLeft1(ix0, iy0, fireplaceDepth);
    glm::vec3 innerLeft2(ix0, iy1, fireplaceDepth);
    glm::vec3 innerLeft3(ix0, iy1, innerZ);
    verts.push_back(innerLeft1); verts.push_back(innerLeft0); verts.push_back(innerLeft3);
    verts.push_back(innerLeft1); verts.push_back(innerLeft3); verts.push_back(innerLeft2);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Peretele din dreapta al cavitatii
    glm::vec3 innerRight0(ix1, iy0, innerZ);
    glm::vec3 innerRight1(ix1, iy0, fireplaceDepth);
    glm::vec3 innerRight2(ix1, iy1, fireplaceDepth);
    glm::vec3 innerRight3(ix1, iy1, innerZ);
    verts.push_back(innerRight0); verts.push_back(innerRight1); verts.push_back(innerRight3);
    verts.push_back(innerRight1); verts.push_back(innerRight2); verts.push_back(innerRight3);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Tavanul cavitatii
    glm::vec3 innerTop0(ix0, iy1, innerZ);
    glm::vec3 innerTop1(ix1, iy1, innerZ);
    glm::vec3 innerTop2(ix1, iy1, fireplaceDepth);
    glm::vec3 innerTop3(ix0, iy1, fireplaceDepth);
    verts.push_back(innerTop1); verts.push_back(innerTop0); verts.push_back(innerTop3);
    verts.push_back(innerTop1); verts.push_back(innerTop3); verts.push_back(innerTop2);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Stanga semineului
    verts.push_back(p0); verts.push_back(p3); verts.push_back(p7);
    verts.push_back(p0); verts.push_back(p7); verts.push_back(p4);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(-1.0f, 0.0f, 0.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));

    // Dreapta semineului
    verts.push_back(p2); verts.push_back(p1); verts.push_back(p5);
    verts.push_back(p2); verts.push_back(p5); verts.push_back(p6);
    for (int i = 0; i < 6; ++i) norms.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f));
    uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 1.0f)); uvs.push_back(glm::vec2(0.0f, 1.0f));
}

// Crearea blatului de pe semineu
void CreateFireplaceTop(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs)
{
    // Ma asigur ca sunt curatati vectorii
    verts.clear(); norms.clear(); uvs.clear();

    // Dimensiunile blatului si marginile (blatul iese putin in afara)
    float marginX = 0.08f; // marginea pe stanga si dreapta semineului
    float marginFrontZ = 0.08f; // marginea spre sufragerie
    float topHeight = 0.12f; // grosimea blatului

    // Coordonatele finale pentru colturile semineului
    float minX = -marginX;
    float maxX = fireplaceWidth + marginX;
    float minZ = 0.0f;
    float maxZ = fireplaceDepth + marginFrontZ; // extend at back as well
    float baseY = fireplaceHeight; // sits on top of fireplace

    // Fata de jos a blatului
    glm::vec3 a(minX, baseY, minZ);
    glm::vec3 b(maxX, baseY, minZ);
    glm::vec3 c(maxX, baseY, maxZ);
    glm::vec3 d(minX, baseY, maxZ);
    // Fata de sus a blatului
    glm::vec3 e = a + glm::vec3(0.0f, topHeight, 0.0f);
    glm::vec3 f = b + glm::vec3(0.0f, topHeight, 0.0f);
    glm::vec3 g = c + glm::vec3(0.0f, topHeight, 0.0f);
    glm::vec3 h = d + glm::vec3(0.0f, topHeight, 0.0f);

    // baza blatului
    verts.push_back(a); verts.push_back(b); verts.push_back(c);
    verts.push_back(a); verts.push_back(c); verts.push_back(d);
    for(int i=0;i<6;++i) norms.push_back(glm::vec3(0.0f,-1.0f,0.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f)); uvs.push_back(glm::vec2(0.0f,1.0f));

    // "tavanul" blatului
    verts.push_back(e); verts.push_back(f); verts.push_back(g);
    verts.push_back(e); verts.push_back(g); verts.push_back(h);
    for(int i=0;i<6;++i) norms.push_back(glm::vec3(0.0f,1.0f,0.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f)); uvs.push_back(glm::vec2(0.0f,1.0f));

    // fata blatului
    verts.push_back(b); verts.push_back(a); verts.push_back(e);
    verts.push_back(b); verts.push_back(e); verts.push_back(f);
    for(int i=0;i<6;++i) norms.push_back(glm::vec3(0.0f,0.0f,-1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f)); uvs.push_back(glm::vec2(0.0f,1.0f));

    // spatele blatului
    verts.push_back(d); verts.push_back(c); verts.push_back(g);
    verts.push_back(d); verts.push_back(g); verts.push_back(h);
    for(int i=0;i<6;++i) norms.push_back(glm::vec3(0.0f,0.0f,1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f)); uvs.push_back(glm::vec2(0.0f,1.0f));

    // stanga blatului
    verts.push_back(a); verts.push_back(d); verts.push_back(h);
    verts.push_back(a); verts.push_back(h); verts.push_back(e);
    for(int i=0;i<6;++i) norms.push_back(glm::vec3(-1.0f,0.0f,0.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f)); uvs.push_back(glm::vec2(0.0f,1.0f));

    // dreapta blatului
    verts.push_back(c); verts.push_back(b); verts.push_back(f);
    verts.push_back(c); verts.push_back(f); verts.push_back(g);
    for(int i=0;i<6;++i) norms.push_back(glm::vec3(1.0f,0.0f,0.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f));
    uvs.push_back(glm::vec2(0.0f,0.0f)); uvs.push_back(glm::vec2(1.0f,1.0f)); uvs.push_back(glm::vec2(0.0f,1.0f));
}


void CreateFirecone(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs)
{
    // ma asigur ca sunt curatati vectorii
    verts.clear(); norms.clear(); uvs.clear();
    const int segments = 24;
    const float height = 0.45f;
    const float radius = 0.16f;

    // construit in jurul originii sistemului de axe (in sus)
    glm::vec3 top(0.0f, height, 0.0f);
    glm::vec3 centerBottom(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < segments; ++i) {
        float a1 = (float)i / segments * 2.0f * PI;
        float a2 = (float)(i + 1) / segments * 2.0f * PI;
        glm::vec3 p1(radius * cos(a1), 0.0f, radius * sin(a1));
        glm::vec3 p2(radius * cos(a2), 0.0f, radius * sin(a2));

        // triunghi lateral
        verts.push_back(top); verts.push_back(p2); verts.push_back(p1);
        glm::vec3 n = glm::normalize(glm::cross(p2 - top, p1 - top));
        norms.push_back(n); norms.push_back(n); norms.push_back(n);
        uvs.push_back(glm::vec2(0.5f, 1.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f)); uvs.push_back(glm::vec2(0.0f, 0.0f));

        // triunghi la baza
        verts.push_back(centerBottom); verts.push_back(p1); verts.push_back(p2);
        norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f)); norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f)); norms.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
        uvs.push_back(glm::vec2(0.5f, 0.5f)); uvs.push_back(glm::vec2(0.0f, 0.0f)); uvs.push_back(glm::vec2(1.0f, 0.0f));
    }
}

// Create a unit sphere (triangle list) - stacks x slices
void CreateUnitSphereTriangles(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs, float radius = 1.0f, int stacks = 12, int slices = 24)
{
    verts.clear(); norms.clear(); uvs.clear();
    for (int i = 0; i < stacks; ++i) {
        float v0 = (float)i / stacks;
        float v1 = (float)(i + 1) / stacks;
        float phi0 = (v0 - 0.5f) * PI; // -pi/2 .. pi/2
        float phi1 = (v1 - 0.5f) * PI;
        for (int j = 0; j < slices; ++j) {
            float u0 = (float)j / slices;
            float u1 = (float)(j + 1) / slices;
            float theta0 = u0 * 2.0f * PI;
            float theta1 = u1 * 2.0f * PI;

            glm::vec3 p00(radius * cosf(phi0) * cosf(theta0), radius * cosf(phi0) * sinf(theta0), radius * sinf(phi0));
            glm::vec3 p10(radius * cosf(phi1) * cosf(theta0), radius * cosf(phi1) * sinf(theta0), radius * sinf(phi1));
            glm::vec3 p11(radius * cosf(phi1) * cosf(theta1), radius * cosf(phi1) * sinf(theta1), radius * sinf(phi1));
            glm::vec3 p01(radius * cosf(phi0) * cosf(theta1), radius * cosf(phi0) * sinf(theta1), radius * sinf(phi0));

            // triangle 1
            verts.push_back(p00); verts.push_back(p10); verts.push_back(p11);
            // triangle 2
            verts.push_back(p00); verts.push_back(p11); verts.push_back(p01);

            // normals (approximate by position normalized)
            for (int k = 0; k < 6; ++k) {
                norms.push_back(glm::normalize(verts[verts.size() - 6 + k]));
            }
            // uvs (simple)
            uvs.push_back(glm::vec2(u0, v0)); uvs.push_back(glm::vec2(u0, v1)); uvs.push_back(glm::vec2(u1, v1));
            uvs.push_back(glm::vec2(u0, v0)); uvs.push_back(glm::vec2(u1, v1)); uvs.push_back(glm::vec2(u1, v0));
        }
    }
}

// Generate spiral points on a cone (DISTANTA EGALA + ROTIRE SPRE SPATE)
void GenerateSpiralOnCone(std::vector<glm::vec3>& outPoints, int totalBulbs, float height, float baseRadius, int turns)
{
    outPoints.clear();
    outPoints.reserve(totalBulbs);

    // --- MODIFICARE AICI: ROTIRE ---
    // Adaugam PI (3.14) ca sa rotim startul cu 180 de grade (sa fie in spate)
    float startOffset = 3.14159f;
    // -----------------------------

    int samples = 2000;
    float totalWireLength = 0.0f;
    std::vector<float> distAtStep;
    distAtStep.push_back(0.0f);

    // Functie helper locala pentru pozitie
    auto getPos = [&](float t) {
        float y = t * height;
        float radius = baseRadius * (1.0f - t);
        // Adaugam startOffset la unghi
        float angle = turns * 2.0f * PI * t + startOffset;
        return glm::vec3(cosf(angle) * radius, y, sinf(angle) * radius);
        };

    glm::vec3 prevP = getPos(0.0f);

    for (int i = 1; i <= samples; ++i) {
        float t = (float)i / samples;
        glm::vec3 currP = getPos(t);
        float stepDist = glm::length(currP - prevP);
        totalWireLength += stepDist;
        distAtStep.push_back(totalWireLength);
        prevP = currP;
    }

    float spacing = totalWireLength / (float)(totalBulbs - 1);

    for (int i = 0; i < totalBulbs; ++i) {
        float targetDist = i * spacing;

        int foundIndex = 0;
        for (int k = 0; k < samples; ++k) {
            if (distAtStep[k + 1] >= targetDist) {
                foundIndex = k;
                break;
            }
        }

        float dist1 = distAtStep[foundIndex];
        float dist2 = distAtStep[foundIndex + 1];
        float alpha = (targetDist - dist1) / (dist2 - dist1);
        if (dist2 - dist1 < 0.0001f) alpha = 0;

        float t1 = (float)foundIndex / samples;
        float t2 = (float)(foundIndex + 1) / samples;
        float exactT = t1 + (t2 - t1) * alpha;

        outPoints.push_back(getPos(exactT));
    }
}

// Upload a polylines vertex buffer (positions only)
void UploadLineToGPU(GLuint& vao, GLuint& vbo, const std::vector<glm::vec3>& points)
{
    if (vao == 0) glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    if (vbo == 0) glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
    nrWirePoints = (int)points.size();
}

// Create a 1x1 white texture for untextured bulbs
GLuint CreateWhiteTexture()
{
    GLuint tid;
    glGenTextures(1, &tid);
    glBindTexture(GL_TEXTURE_2D, tid);
    unsigned char data[4] = { 255,255,255,255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tid;
}

GLuint CreateBlackTexture()
{
    GLuint tid;
    glGenTextures(1, &tid);
    glBindTexture(GL_TEXTURE_2D, tid);
    unsigned char data[4] = { 0,0,0,255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
    return tid;
}


// ZOOM (+/-)
void processNormalKeys(unsigned char key, int x, int y)
{
    switch (key) {
    case '1':
        lightMode = 1; // Default (Static)
        break;
    case '2':
        lightMode = 2; // Palpait (Blink)
        break;
    case '3':
        lightMode = 3; // Pe sarite (Chasing)
        break;
    case '4':
        lightMode = 4; // Stins
        break;
    case '+':
        dist -= 0.5f;
        if (dist < 1.0f) dist = 1.0f;
        break;
    case '-':
        dist += 0.5f;
        break;
    }
    if (key == 27) exit(0);
}

// ROTIRE (Sageti)
void processSpecialKeys(int key, int xx, int yy)
{
    switch (key)
    {
    case GLUT_KEY_LEFT:
        beta -= incrBeta;
        break;
    case GLUT_KEY_RIGHT:
        beta += incrBeta;
        break;
    case GLUT_KEY_UP:
        alpha += incrAlpha;
        if (alpha > maxAlpha) alpha = maxAlpha;
        break;
    case GLUT_KEY_DOWN:
        alpha -= incrAlpha;
        if (alpha < minAlpha) alpha = minAlpha;
        break;
    }
}

GLuint LoadTexture(const char* texturePath)
{
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, channels;

    // Folosim SOIL_LOAD_RGBA (incarca si transparenta)
    unsigned char* image = SOIL_load_image(texturePath, &width, &height, &channels, SOIL_LOAD_RGBA);

    if (image)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);
        std::cout << "Textura incarcata: " << texturePath << std::endl;
    }
    else
    {
        std::cout << "Eroare textura: " << texturePath << std::endl;
        return 0;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}

void UploadMeshToGPU(GLuint& vao, GLuint& vbo,
    std::vector<glm::vec3>& verts,
    std::vector<glm::vec3>& norms,
    std::vector<glm::vec2>& texCoords)
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    size_t vertsBytes = verts.size() * sizeof(glm::vec3);
    size_t normsBytes = norms.size() * sizeof(glm::vec3);
    size_t uvsBytes = texCoords.size() * sizeof(glm::vec2);

    if (verts.size() > 0) {
        glBufferData(GL_ARRAY_BUFFER, vertsBytes + normsBytes + uvsBytes, NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertsBytes, &verts[0]);
        if (norms.size() > 0) glBufferSubData(GL_ARRAY_BUFFER, vertsBytes, normsBytes, &norms[0]);
        if (texCoords.size() > 0) glBufferSubData(GL_ARRAY_BUFFER, vertsBytes + normsBytes, uvsBytes, &texCoords[0]);

        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
        if (norms.size() > 0) { glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertsBytes)); }
        if (texCoords.size() > 0) { glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertsBytes + normsBytes)); }
    }
}

void Cleanup(void) {
    glDeleteProgram(ProgramId);
    glDeleteVertexArrays(1, &VaoIdRoom); glDeleteBuffers(1, &VboIdRoom);
    glDeleteVertexArrays(1, &VaoIdTree); glDeleteBuffers(1, &VboIdTree);
    glDeleteVertexArrays(1, &VaoIdStar); glDeleteBuffers(1, &VboIdStar);
    glDeleteVertexArrays(1, &VaoIdFireplace); glDeleteBuffers(1, &VboIdFireplace);
    glDeleteVertexArrays(1, &VaoIdFireplaceTop); glDeleteBuffers(1, &VboIdFireplaceTop);
    glDeleteVertexArrays(1, &VaoIdFire); glDeleteBuffers(1, &VboIdFire);
    if (VaoIdWire) { glDeleteVertexArrays(1, &VaoIdWire); glDeleteBuffers(1, &VboIdWire); }
    if (VaoIdBulb) { glDeleteVertexArrays(1, &VaoIdBulb); glDeleteBuffers(1, &VboIdBulb); }
    if (WhiteTexture) glDeleteTextures(1, &WhiteTexture);
    if (BlackTexture) glDeleteTextures(1, &BlackTexture);
}


void Initialize(void)
{
    //glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    //glClearColor(0.35f, 0.35f, 0.35f, 1.0f);

    // 1. INCARCARE SUFRAGERIE
    std::string roomPath = "livingroom.obj";
    loadOBJ(roomPath.c_str(), roomVertices, roomUvs, roomNormals);
    nrVerticesRoom = roomVertices.size();

    if (nrVerticesRoom > 0) {
        glm::vec3 center(0.0f);
        for (const auto& v : roomVertices) center += v;
        center /= (float)nrVerticesRoom;
        refX = center.x; refY = center.y; refZ = center.z;
    }

    // pun baza semineului la baza sufrageriei
    fireplacePosition.y = refY;

    UploadMeshToGPU(VaoIdRoom, VboIdRoom, roomVertices, roomNormals, roomUvs);
    TextureRoom = LoadTexture("livingroom.png");

    // 2. GENERARE BRAD (AICI E SCHIMBAREA)
    // Nu mai incarcam din fisier, ci generam noi conul.
    CreateProceduralCone(treeVertices, treeNormals, treeUvs);
    nrVerticesTree = treeVertices.size();

    UploadMeshToGPU(VaoIdTree, VboIdTree, treeVertices, treeNormals, treeUvs);
    TextureTree = LoadTexture("green.png"); // Folosim patratul verde

    // --- GENERARE INSTALATIE (fir + beculete) ---
    // number of bulbs and spiral params (local cone coordinates)
    bulbCount = 70; // changeable, reduced to lower total light
    float coneHeight = 2.5f; // must match CreateProceduralCone
    float coneBaseRadius = 1.0f;
    int turns = 8;
    GenerateSpiralOnCone(bulbLocalPositions, bulbCount, coneHeight, coneBaseRadius, turns);

    // offset bulbs slightly outward along radial direction so they render over the wire
    float bulbOffset = 0.02f; // small offset in local cone units
    for (int i = 0; i < (int)bulbLocalPositions.size(); ++i) {
        glm::vec3 &p = bulbLocalPositions[i];
        glm::vec3 radial = glm::vec3(p.x, 0.0f, p.z);
        float len = glm::length(radial);
        if (len > 1e-5f) radial = glm::normalize(radial);
        else radial = glm::vec3(1.0f, 0.0f, 0.0f);
        p += radial * bulbOffset;
    }

    // create a small sphere mesh for bulb geometry
    CreateUnitSphereTriangles(bulbVerts, bulbNormals, bulbUvs, 1.0f, 10, 14);
    nrVerticesBulb = (int)bulbVerts.size();
    UploadMeshToGPU(VaoIdBulb, VboIdBulb, bulbVerts, bulbNormals, bulbUvs);

    // upload wire points (same spiral, denser)
    std::vector<glm::vec3> wirePoints;

    // Pastram densitatea mare ca sa fie linia fina (50 puncte per segment de bec)
    int wireSamples = bulbCount * 50;

    // --- MODIFICARE IMPORTANTA ---
    // In loc sa calculam manual (si gresit), apelam aceeasi functie ca la becuri!
    // Astfel, firul va avea EXACT aceeasi rotatie si distantare ca becurile.
    GenerateSpiralOnCone(wirePoints, wireSamples, coneHeight, coneBaseRadius, turns);
    // -----------------------------

    UploadLineToGPU(VaoIdWire, VboIdWire, wirePoints);

    // bulb colors and intensities
    bulbColors.resize(bulbCount);
    bulbIntensities.resize(bulbCount);

    // NU mai folosim random, vrem totul constant
    for (int i = 0; i < bulbCount; ++i) {
        int colorType = i % 4;

        // 1. Setam Culorile (Rosu, Albastru, Verde, Portocaliu)
        if (colorType == 0) {
            bulbColors[i] = glm::vec3(1.0f, 0.0f, 0.0f); // Rosu
        }
        else if (colorType == 1) {
            bulbColors[i] = glm::vec3(0.0f, 0.5f, 1.0f); // Albastru
        }
        else if (colorType == 2) {
            bulbColors[i] = glm::vec3(0.1f, 1.0f, 0.1f); // Verde
        }
        else {
            bulbColors[i] = glm::vec3(1.0f, 0.5f, 0.0f); // Portocaliu
        }

        // 2. MODIFICARE: Intensitate CONSTANTA pentru toate
        // Setam o valoare fixa (ex: 2.0f) ca sa fie toate la fel de luminoase si mereu aprinse
        //bulbIntensities[i] = 2.0f;
        bulbIntensities[i] = 2.0f;
    }

    // white texture for bulbs (simple shader sampling)
    WhiteTexture = CreateWhiteTexture();
    // load user-provided black texture for the wire if available
    BlackTexture = LoadTexture("black.png");
    if (BlackTexture == 0) {
        // fallback to procedural black
        BlackTexture = CreateBlackTexture();
    }
    // 3. GENERARE STEA
    CreateProceduralStar(starVertices, starNormals, starUvs);
    nrVerticesStar = starVertices.size();
    UploadMeshToGPU(VaoIdStar, VboIdStar, starVertices, starNormals, starUvs);

    // Generare semineu
    CreateFireplace(fireplaceVertices, fireplaceNormals, fireplaceUvs);
    nrVerticesFireplace = fireplaceVertices.size();
    UploadMeshToGPU(VaoIdFireplace, VboIdFireplace, fireplaceVertices, fireplaceNormals, fireplaceUvs);
    TextureFireplace = LoadTexture("fireplace.png");

    // Generare blat semineu
    CreateFireplaceTop(fireplaceTopVertices, fireplaceTopNormals, fireplaceTopUvs);
    nrVerticesFireplaceTop = fireplaceTopVertices.size();
    UploadMeshToGPU(VaoIdFireplaceTop, VboIdFireplaceTop, fireplaceTopVertices, fireplaceTopNormals, fireplaceTopUvs);
    TextureFireplaceTop = LoadTexture("wood.png");

    // Generare foc
    CreateFirecone(fireVertices, fireNormals, fireUvs);
    nrVerticesFire = fireVertices.size();
    UploadMeshToGPU(VaoIdFire, VboIdFire, fireVertices, fireNormals, fireUvs);

    // Generare lemne foc
    // De fapt generez un cub pe care apoi il mut vizual pentru mai multe lemne
    logVertices.clear(); logNormals.clear(); logUvs.clear();
    glm::vec3 p000(-0.5f, -0.5f, -0.5f);
    glm::vec3 p100(0.5f, -0.5f, -0.5f);
    glm::vec3 p110(0.5f, 0.5f, -0.5f);
    glm::vec3 p010(-0.5f, 0.5f, -0.5f);
    glm::vec3 p001(-0.5f, -0.5f, 0.5f);
    glm::vec3 p101(0.5f, -0.5f, 0.5f);
    glm::vec3 p111(0.5f, 0.5f, 0.5f);
    glm::vec3 p011(-0.5f, 0.5f, 0.5f);

    // fata cubului
    logVertices.push_back(p101); logVertices.push_back(p001); logVertices.push_back(p011);
    logVertices.push_back(p101); logVertices.push_back(p011); logVertices.push_back(p111);
    for(int i=0;i<6;++i) logNormals.push_back(glm::vec3(0.0f,0.0f,1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f)); logUvs.push_back(glm::vec2(0.0f,1.0f));

    // spatele cubului
    logVertices.push_back(p000); logVertices.push_back(p100); logVertices.push_back(p110);
    logVertices.push_back(p000); logVertices.push_back(p110); logVertices.push_back(p010);
    for(int i=0;i<6;++i) logNormals.push_back(glm::vec3(0.0f,0.0f,-1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f)); logUvs.push_back(glm::vec2(0.0f,1.0f));

    // stanga cubului
    logVertices.push_back(p001); logVertices.push_back(p000); logVertices.push_back(p010);
    logVertices.push_back(p001); logVertices.push_back(p010); logVertices.push_back(p011);
    for(int i=0;i<6;++i) logNormals.push_back(glm::vec3(-1.0f,0.0f,0.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f)); logUvs.push_back(glm::vec2(0.0f,1.0f));

    // dreapta cubului
    logVertices.push_back(p100); logVertices.push_back(p101); logVertices.push_back(p111);
    logVertices.push_back(p100); logVertices.push_back(p111); logVertices.push_back(p110);
    for(int i=0;i<6;++i) logNormals.push_back(glm::vec3(1.0f,0.0f,0.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f)); logUvs.push_back(glm::vec2(0.0f,1.0f));

    // baza de sus a cubului
    logVertices.push_back(p011); logVertices.push_back(p010); logVertices.push_back(p110);
    logVertices.push_back(p011); logVertices.push_back(p110); logVertices.push_back(p111);
    for(int i=0;i<6;++i) logNormals.push_back(glm::vec3(0.0f,1.0f,0.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f)); logUvs.push_back(glm::vec2(0.0f,1.0f));

    // baza de jos a cubului
    logVertices.push_back(p000); logVertices.push_back(p001); logVertices.push_back(p101);
    logVertices.push_back(p000); logVertices.push_back(p101); logVertices.push_back(p100);
    for(int i=0;i<6;++i) logNormals.push_back(glm::vec3(0.0f,-1.0f,0.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f));
    logUvs.push_back(glm::vec2(0.0f,0.0f)); logUvs.push_back(glm::vec2(1.0f,1.0f)); logUvs.push_back(glm::vec2(0.0f,1.0f));

    nrVerticesLogs = logVertices.size();
    UploadMeshToGPU(VaoIdLogs, VboIdLogs, logVertices, logNormals, logUvs);
    // pentru lemne folosesc textura de la blatul semineului, care e tot lemn
    TextureLogs = TextureFireplaceTop;

    // creez cateva instante pentru lemne
    logs.clear();
    // pun lemnele in foc
    const float fireInset = 0.28f;
    float zBase = fireplaceDepth - 0.32f; // putin mai in fata
    float baseY = 0.03f; // le ridic putin
    logs.push_back({ glm::vec3(fireplaceWidth*0.5f - 0.12f, baseY + 0.04f, zBase), 
                     glm::vec3(0.60f, 0.08f, 0.12f), 
                     -0.25f }); // primul lemn
    logs.push_back({ glm::vec3(fireplaceWidth*0.5f + 0.08f, baseY + 0.02f, zBase + 0.02f),
                     glm::vec3(0.50f, 0.07f, 0.10f), 
                     0.15f }); // al doilea lemn
    logs.push_back({ glm::vec3(fireplaceWidth*0.5f - 0.05f, baseY + 0.10f, zBase - 0.05f), 
                     glm::vec3(0.40f, 0.06f, 0.10f), 
                     0.60f }); // al treilea lemn
    logs.push_back({ glm::vec3(fireplaceWidth*0.5f + 0.20f, baseY + 0.05f, zBase - 0.02f), 
                     glm::vec3(0.45f, 0.07f, 0.11f), 
                     -0.40f }); // al patrulea lemn

    ProgramId = LoadShaders("10_01_Shader.vert", "10_01_Shader.frag");
    glUseProgram(ProgramId);

    nrVertLocation = glGetUniformLocation(ProgramId, "nrVertices");
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
    viewPosLocation = glGetUniformLocation(ProgramId, "viewPos");
    viewLocation = glGetUniformLocation(ProgramId, "view");
    projLocation = glGetUniformLocation(ProgramId, "projection");
    colorMulLocation = glGetUniformLocation(ProgramId, "globalColorMul");
    alphaMulLocation = glGetUniformLocation(ProgramId, "globalAlphaMul");
    fireLightPosLocation = glGetUniformLocation(ProgramId, "fireLightPos");
    fireLightColorLocation = glGetUniformLocation(ProgramId, "fireLightColor");
    fireLightIntensityLocation = glGetUniformLocation(ProgramId, "fireLightIntensity");
    

    if (colorMulLocation >= 0) glUniform1f(colorMulLocation, 1.0f);
    if (alphaMulLocation >= 0) glUniform1f(alphaMulLocation, 1.0f);
    if (fireLightColorLocation >= 0) glUniform3f(fireLightColorLocation, 1.0f, 0.6f, 0.15f);
    if (fireLightIntensityLocation >= 0) glUniform1f(fireLightIntensityLocation, 2.5f);


    glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

    // bulb light uniform locations
    bulbCountLocation = glGetUniformLocation(ProgramId, "bulbCount");
    bulbPosLocation = glGetUniformLocation(ProgramId, "bulbPos");
    bulbColorLocation = glGetUniformLocation(ProgramId, "bulbColor");
    bulbIntensityLocation = glGetUniformLocation(ProgramId, "bulbIntensity");
    allowBlackLocation = glGetUniformLocation(ProgramId, "allowBlack");
    if (allowBlackLocation >= 0) glUniform1i(allowBlackLocation, 0);
    useSolidColorLocation = glGetUniformLocation(ProgramId, "useSolidColor");
    solidColorLocation = glGetUniformLocation(ProgramId, "solidColor");
    if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 0);

    // Shader separat pentru particule
    ParticleProgram = LoadShaders("particle.vert", "particle.frag");

    // VAO si VBO pentru particule
    glGenVertexArrays(1, &VaoIdSmoke);
    glBindVertexArray(VaoIdSmoke);
    glGenBuffers(1, &VboIdSmoke);
    glBindBuffer(GL_ARRAY_BUFFER, VboIdSmoke);
    // 5 float-uri fiecare particula (vec3 + life + size)
    glBufferData(GL_ARRAY_BUFFER, maxSmokeParticles * 5 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    // pozitia (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
    // durata de viata (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    // dimensiunea (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(4 * sizeof(float)));

    // initializez particulele
    smokeParticles.resize(maxSmokeParticles);
    for (int i = 0; i < maxSmokeParticles; ++i) 
    {
        smokeParticles[i].life = 0.0f; // toate sunt moarte
        smokeParticles[i].pos = glm::vec3(0.0f);
        smokeParticles[i].vel = glm::vec3(0.0f);
        smokeParticles[i].size = 0.0f;
    }

    lastFrameTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(ProgramId);

    // --- CAMERA ---
    obsY = refY + dist * sin(alpha);
    float radiusXZ = dist * cos(alpha);
    obsX = refX + radiusXZ * sin(beta);
    obsZ = refZ + radiusXZ * cos(beta);

    glm::vec3 obs = glm::vec3(obsX, obsY, obsZ);
    glm::vec3 pctRef = glm::vec3(refX, refY, refZ);
    glm::vec3 vert = glm::vec3(0.0f, 1.0f, 0.0f);

    glUniform3f(viewPosLocation, obsX, obsY, obsZ);
    view = glm::lookAt(obs, pctRef, vert);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    projection = glm::infinitePerspective(GLfloat(fov), GLfloat(width) / GLfloat(height), dNear);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

    // 1. DESENARE SUFRAGERIE
    if (VaoIdRoom != 0 && nrVerticesRoom > 0) {
        glm::mat4 modelRoom = glm::mat4(1.0f);
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelRoom[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureRoom);
        glBindVertexArray(VaoIdRoom);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesRoom);
    }

    // 2. DESENARE BRAD GENERAT
    if (VaoIdTree != 0 && nrVerticesTree > 0) {
        glm::mat4 modelTree = glm::mat4(1.0f);

        modelTree = glm::translate(modelTree, treePosition);
        // Scalam la marime normala
        modelTree = glm::scale(modelTree, glm::vec3(treeScale, treeScale * 1.5f, treeScale));

        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTree[0][0]);

        // Draw tree geometry
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureTree);
        glBindVertexArray(VaoIdTree);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesTree);

        // Draw wire (firul negru)
        if (VaoIdWire != 0 && nrWirePoints > 0) {
            if (allowBlackLocation >= 0) glUniform1i(allowBlackLocation, 1);

            glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTree[0][0]);
            glBindVertexArray(VaoIdWire);

            // Setari de netezire
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glLineWidth(2.5f);

            // Polygon Offset pentru a evita suprapunerea cu bradul
            glEnable(GL_POLYGON_OFFSET_LINE);
            glPolygonOffset(-1.0f, -1.0f);

            glDrawArrays(GL_LINE_STRIP, 0, nrWirePoints);

            glDisable(GL_POLYGON_OFFSET_LINE);
            glLineWidth(1.0f);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);

            if (allowBlackLocation >= 0) glUniform1i(allowBlackLocation, 0);
        }

        // Draw bulbs (beculete)
        if (VaoIdBulb != 0 && nrVerticesBulb > 0) {

            // --- ACTUALIZARE MODURI ILUMINARE ---
            float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // Timpul in secunde

            for (int i = 0; i < bulbCount; ++i) {
                switch (lightMode) {
                case 1:
                    // MOD 1: DEFAULT (Static Puternic)
                    bulbIntensities[i] = 10.0f;
                    break;

                case 2:
                    // MOD 2: PALPAIT (Toate odata, lent)
                    bulbIntensities[i] = 6.0f + 4.0f * sin(time * 3.0f);
                    break;

                case 3:
                    // MOD 3: PE SARITE (Chasing)
                {
                    int speed = int(time * 5.0f);
                    if ((i + speed) % 3 == 0) {
                        bulbIntensities[i] = 10.0f; // Aprins
                    }
                    else {
                        bulbIntensities[i] = 0.0f;  // Stins
                    }
                }
                break;

                case 4:
                    // MOD 4: STINSE
                    bulbIntensities[i] = 0.0f;
                    break;
                }
            }

            // 1. Trimitem pozitiile si culorile luminilor catre shader (pentru iluminarea camerei)
            if (bulbPosLocation >= 0 && bulbCountLocation >= 0) {
                std::vector<glm::vec3> bulbWorldPositions;
                bulbWorldPositions.resize(bulbCount);
                for (int i = 0; i < bulbCount; ++i) {
                    glm::vec4 wp = modelTree * glm::vec4(bulbLocalPositions[i], 1.0f);
                    bulbWorldPositions[i] = glm::vec3(wp.x, wp.y, wp.z);
                }
                glUniform1i(bulbCountLocation, bulbCount);
                glUniform3fv(bulbPosLocation, bulbCount, (const GLfloat*)(&bulbWorldPositions[0]));
                if (bulbColorLocation >= 0) glUniform3fv(bulbColorLocation, bulbCount, (const GLfloat*)(&bulbColors[0]));
                if (bulbIntensityLocation >= 0) glUniform1fv(bulbIntensityLocation, bulbCount, (const GLfloat*)(&bulbIntensities[0]));
            }

            // 2. Desenam sferele fizice ale becurilor
            float bulbScale = 0.03f;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, WhiteTexture ? WhiteTexture : TextureTree);

            if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 1);

            for (int i = 0; i < bulbCount; ++i) {
                glm::mat4 m = modelTree * glm::translate(glm::mat4(1.0f), bulbLocalPositions[i]) * glm::scale(glm::mat4(1.0f), glm::vec3(bulbScale));
                glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &m[0][0]);

                // --- CALCUL CULOARE VIZUALA ---
                glm::vec3 finalColor = bulbColors[i];

                // Daca intensitatea e 0 (stins), becul devine gri inchis
                if (bulbIntensities[i] < 0.5f) {
                    finalColor = glm::vec3(0.05f, 0.05f, 0.05f);
                }
                else {
                    // Calculam luminozitatea vizuala (clamp manual ca sa evitam erori)
                    float brightness = bulbIntensities[i] / 10.0f;
                    if (brightness < 0.5f) brightness = 0.5f;
                    if (brightness > 1.0f) brightness = 1.0f;

                    finalColor *= brightness;
                }

                if (solidColorLocation >= 0) {
                    glUniform3f(solidColorLocation, finalColor.x, finalColor.y, finalColor.z);
                }

                glBindVertexArray(VaoIdBulb);
                glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesBulb);
            }

            if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 0);

            // Reset matrix logic
            glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTree[0][0]);
            if (bulbCountLocation >= 0) glUniform1i(bulbCountLocation, 0);
        }
    }

    // 3. DESENARE STEA (In varful bradului)
    if (VaoIdStar != 0 && nrVerticesStar > 0) {
        glm::mat4 modelStar = glm::mat4(1.0f);

        float tipHeight = 1.875f;

        // 1. Pozitionare (Varf)
        modelStar = glm::translate(modelStar, treePosition + glm::vec3(0.0f, tipHeight, 0.0f));

        // 2. Rotire FIXA (fara 'time')
        // O rotim cu -90 grade (-PI/2) ca sa priveasca spre Spate (X-), unde e firul
        modelStar = glm::rotate(modelStar, -PI / 2.0f + 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));

        // 3. Scalare (Mai mica)
        // Era 1.0 (implicita), acum o facem 0.6
        modelStar = glm::scale(modelStar, glm::vec3(0.6f));

        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelStar[0][0]);

        // Culoare Galben-Auriu
        if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 1);
        if (solidColorLocation >= 0) glUniform3f(solidColorLocation, 0.7f, 0.5f, 0.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, WhiteTexture);
        glBindVertexArray(VaoIdStar);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesStar);

        if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 0);
    }

    // Actualizarea mediului in functie de foc
    if (VaoIdFire != 0 && nrVerticesFire > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, VboIdFire);
        glBufferSubData(GL_ARRAY_BUFFER, 0, fireVertices.size() * sizeof(glm::vec3), &fireVertices[0]);

        // mut si scalez focul
        glm::mat4 modelFire = glm::mat4(1.0f);
        modelFire = glm::translate(modelFire, fireplacePosition + fireplaceOffset + 
            glm::vec3(fireplaceWidth * 0.5f, 0.0f, fireplaceDepth - 0.28f) + fireOffset);
        modelFire = glm::scale(modelFire, glm::vec3(fireplaceScale));
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelFire[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureTree);
        glBindVertexArray(VaoIdFire);
        if (fireLightPosLocation >= 0) {
            glm::vec3 fireLightWorld = fireplacePosition + fireplaceOffset + glm::vec3(fireplaceWidth * 0.5f, 0.18f, fireplaceDepth - 0.28f) + fireOffset;
            glUniform3f(fireLightPosLocation, fireLightWorld.x, fireLightWorld.y, fireLightWorld.z);
        }
    }

    // Desenez semineul
    if (VaoIdFireplace != 0 && nrVerticesFireplace > 0) {
        glm::mat4 modelFireplace = glm::mat4(1.0f);
        modelFireplace = glm::translate(modelFireplace, fireplacePosition + fireplaceOffset);
        modelFireplace = glm::scale(modelFireplace, glm::vec3(fireplaceScale));
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelFireplace[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureFireplace);
        glBindVertexArray(VaoIdFireplace);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        if (alphaMulLocation >= 0) glUniform1f(alphaMulLocation, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesFireplace);
    }

    // Desenez blatul semineului
    if (VaoIdFireplaceTop != 0 && nrVerticesFireplaceTop > 0) {
        glm::mat4 modelTop = glm::mat4(1.0f);
        modelTop = glm::translate(modelTop, fireplacePosition + fireplaceOffset);
        modelTop = glm::scale(modelTop, glm::vec3(fireplaceScale));
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTop[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureFireplaceTop);
        glBindVertexArray(VaoIdFireplaceTop);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesFireplaceTop);
    }

    // Desenez lemnele
    if (VaoIdLogs != 0 && nrVerticesLogs > 0 && !logs.empty()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureLogs);
        glBindVertexArray(VaoIdLogs);
        for (size_t li = 0; li < logs.size(); ++li) {
            const LogInstance &L = logs[li];
            glm::mat4 modelLog = glm::mat4(1.0f);
            modelLog = glm::translate(modelLog, fireplacePosition + fireplaceOffset + L.pos);
            modelLog = glm::rotate(modelLog, L.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
            modelLog = glm::scale(modelLog, glm::vec3(L.size.x * fireplaceScale, L.size.y * fireplaceScale, L.size.z * fireplaceScale));
            glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelLog[0][0]);
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesLogs);
        }
    }

    // Desenez focul la final
    if (VaoIdFire != 0 && nrVerticesFire > 0) {
        glm::mat4 modelFire = glm::translate(glm::mat4(1.0f),
            fireplacePosition + fireplaceOffset + glm::vec3(fireplaceWidth * 0.5f, 0.0f, fireplaceDepth - 0.28f) + fireOffset);
        modelFire = glm::scale(modelFire, glm::vec3(fireplaceScale));
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelFire[0][0]);

        if (useSolidColorLocation >= 0 && solidColorLocation >= 0) {
            glUniform1i(useSolidColorLocation, 1);
            glUniform3f(solidColorLocation, 1.0f, 0.5f, 0.0f);
        }

        float ft = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float flicker = glm::clamp(0.90f + 0.18f * sin(ft * 2.2f) + 0.08f * sin(ft * 0.9f + 1.3f), 0.6f, 1.4f);
        if (fireLightIntensityLocation >= 0) glUniform1f(fireLightIntensityLocation, fireBaseIntensity * flicker);

        glBindVertexArray(VaoIdFire);
        glBindTexture(GL_TEXTURE_2D, WhiteTexture);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesFire);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 0);
    }

    // Fumul
    {
        float curTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float dt = curTime - lastFrameTime;
        if (dt <= 0.0f) dt = 0.0f;
        if (dt > 0.1f) dt = 0.1f;
        lastFrameTime = curTime;

        // engine pentru generarea random a particulelor
        static std::default_random_engine rng(123456);
        static std::uniform_real_distribution<float> dOffset(-0.03f, 0.03f);
        static std::uniform_real_distribution<float> dVelY(0.6f, 1.3f);
        static std::uniform_real_distribution<float> dSize(3.0f, 6.0f);
        static int nextSmokeIndex = 0; // suprascriu cea mai veche particula

        // calculez centrul fumului
        const float fireInset = 0.28f;
        glm::vec3 emitCenter = fireplacePosition + fireplaceOffset + 
                glm::vec3(fireplaceWidth * 0.5f, 0.18f, fireplaceDepth - fireInset) + 
                fireOffset;

        // folosesc generarile random pentru particule
        int toSpawn = int(smokeSpawnRate * dt + 0.5f);
        for (int s = 0; s < toSpawn; ++s) {
            SmokeParticle &p = smokeParticles[nextSmokeIndex];
            p.pos = emitCenter + glm::vec3(dOffset(rng), 0.0f, dOffset(rng));
            p.vel = glm::vec3(dOffset(rng) * 0.7f, dVelY(rng), dOffset(rng) * 0.7f);
            p.life = 1.0f;
            p.size = dSize(rng);
            nextSmokeIndex = (nextSmokeIndex + 1) % maxSmokeParticles;
        }

        // umplu bufferul
        float maxSmokeY = fireplacePosition.y + fireplaceHeight * fireplaceScale * smokeCeilingFactor;
        std::vector<float> buf;
        buf.resize(maxSmokeParticles * 5);
        for (int i = 0; i < maxSmokeParticles; ++i) {
            SmokeParticle& p = smokeParticles[i];
            if (p.life > 0.0f) {
                p.pos += p.vel * dt;
                p.vel.y += 1.2f * dt; // deplasare pe verticala
                p.vel *= 0.99f;
                p.life -= dt * 0.12f; // pierdere din viata
                // opresc particulele la limita maxima superioara
                if (p.pos.y >= maxSmokeY) { p.pos.y = maxSmokeY; p.vel = glm::vec3(0.0f); }
                if (p.life < 0.0f) p.life = 0.0f;
            }
            int idx = i * 5;
            buf[idx + 0] = p.pos.x;
            buf[idx + 1] = p.pos.y;
            buf[idx + 2] = p.pos.z;
            buf[idx + 3] = p.life;
            buf[idx + 4] = p.size;
        }

        glBindBuffer(GL_ARRAY_BUFFER, VboIdSmoke);
        glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(buf.size() * sizeof(float)), buf.data());

        glUseProgram(ParticleProgram);
        GLint locView = glGetUniformLocation(ParticleProgram, "view");
        GLint locProj = glGetUniformLocation(ParticleProgram, "projection");
        if (locView >= 0) glUniformMatrix4fv(locView, 1, GL_FALSE, &view[0][0]);
        if (locProj >= 0) glUniformMatrix4fv(locProj, 1, GL_FALSE, &projection[0][0]);
        glEnable(GL_PROGRAM_POINT_SIZE);

        glBindVertexArray(VaoIdSmoke);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        glDrawArrays(GL_POINTS, 0, (GLsizei)maxSmokeParticles);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glUseProgram(ProgramId);
    }

    glutSwapBuffers();
    glFlush();
}

void ChangeSize(int w, int h)
{
    width = (float)w;
    height = (float)h;
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(1200, 900);
    glutCreateWindow("Camera + Brad Generat");
    glewInit();
    Initialize();
    glutReshapeFunc(ChangeSize);
    glutIdleFunc(RenderFunction);
    glutDisplayFunc(RenderFunction);
    glutKeyboardFunc(processNormalKeys);
    glutSpecialFunc(processSpecialKeys);
    glutCloseFunc(Cleanup);
    glutMainLoop();
}
