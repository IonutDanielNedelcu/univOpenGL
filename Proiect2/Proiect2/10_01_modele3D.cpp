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

// General
float PI = 3.141592f;
glm::mat4 myMatrix, view, projection;
float refX = 0.0f, refY = 0.0f, refZ = 0.0f;
// Pozitia observatorului
float obsX, obsY, obsZ;
// Variabile pentru controlul camerei
float alpha = 0.2f;
float beta = -0.5f;
float dist = 6.0f;
float minAlpha = 0.01f;
float maxAlpha = 1.5f;
float incrAlpha = 0.05f;
float incrBeta = 0.05f;
// Proiectie
float width = 1200, height = 900, dNear = 0.1f, fov = 60.f * PI / 180;


// 1. Variabile SUFRAGERIE
GLuint VaoRoom, VboRoom, TextureRoom;
int nrVerticesRoom;
std::vector<glm::vec3> roomVertices, roomNormals;
std::vector<glm::vec2> roomUvs;



// 2. Variabile BRAD 
GLuint VaoTree, VboTree, TextureTree;
int nrVerticesTree;
std::vector<glm::vec3> treeVertices, treeNormals;
std::vector<glm::vec2> treeUvs;
glm::vec3 treePosition(2.9f, 3.0f, -0.5f);
float treeScale = 0.5f;
GLint allowBlackLocation = -1;

// Generare brad ca un con
void CreateTree(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs)
{
    // Resetam vectorii de iesire
    verts.clear();
    norms.clear();
    uvs.clear();

    const float height =2.5f;   // inaltimea bradului
    const float radius = 1.0f;   // raza bazei
    const int segments = 32;     // cate segmente in jurul bazei

    glm::vec3 top(0.0f, height, 0.0f);
    glm::vec3 bottomCenter(0.0f, 0.0f, 0.0f);

    // Pentru fiecare segment generam doua triunghiuri: unul pentru fata laterala
    // si unul pentru baza (pentru a acoperi partea inferioara).
    for (int i = 0; i < segments; ++i) {
        float a1 = (float)i / segments * 2.0f * PI;
        float a2 = (float)(i + 1) / segments * 2.0f * PI;

        // Pozitiile punctelor de pe circumferinta bazei
        glm::vec3 p1(radius * cos(a1), 0.0f, radius * sin(a1));
        glm::vec3 p2(radius * cos(a2), 0.0f, radius * sin(a2));

        // 1) Triunghi lateral: (top, p1, p2)
        verts.push_back(top);
        verts.push_back(p1);
        verts.push_back(p2);

        // Normala fetei laterale (aproximata folosind produsul vectorial)
        glm::vec3 nSide = glm::normalize(glm::cross(p1 - top, p2 - top));
        norms.push_back(nSide);
        norms.push_back(nSide);
        norms.push_back(nSide);

        // UV-uri simple: varful la centru sus, cele doua varfuri la baza
        uvs.push_back(glm::vec2(0.5f, 1.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));
        uvs.push_back(glm::vec2(1.0f, 0.0f));

        // 2) Triunghi baza: (bottomCenter, p2, p1)
        verts.push_back(bottomCenter);
        verts.push_back(p2);
        verts.push_back(p1);

        // Normaala bazei -- orientata in jos
        glm::vec3 nDown(0.0f, -1.0f, 0.0f);
        norms.push_back(nDown);
        norms.push_back(nDown);
        norms.push_back(nDown);

        // UV-uri pentru baza (placeholder)
        uvs.push_back(glm::vec2(0.5f, 0.5f));
        uvs.push_back(glm::vec2(1.0f, 0.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));
    }
}


// 3. Variabile INSTALATIE 
GLuint VaoWire = 0, VboWire = 0, VaoBulb = 0, VboBulb = 0;
int nrWirePoints = 0, nrVerticesBulb = 0, bulbCount = 0;
std::vector<glm::vec3> bulbVerts;
std::vector<glm::vec3> bulbNormals;
std::vector<glm::vec2> bulbUvs;
std::vector<glm::vec3> bulbLocalPositions; // Pozitiile beculetelor
std::vector<glm::vec3> bulbColors;
std::vector<float> bulbIntensities;
// Locatiile uniformelor din shader pentru beculete
GLint bulbCountLocation = -1;
GLint bulbPosLocation = -1;
GLint bulbColorLocation = -1;
GLint bulbIntensityLocation = -1;

GLuint WhiteTexture = 0;
GLuint BlackTexture = 0;
GLint useSolidColorLocation = -1;
GLint solidColorLocation = -1;
int lightMode = 1; 



// 4. Variabile STEA
GLuint VaoStar = 0, VboStar = 0;
int nrVerticesStar;
std::vector<glm::vec3> starVertices;
std::vector<glm::vec3> starNormals;
std::vector<glm::vec2> starUvs;

// Generare stea
void CreateProceduralStar(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs)
{
    // Resetam vectorii de iesire
    verts.clear();
    norms.clear();
    uvs.clear();

    const float outerRadius = 0.25f; // distanta de la centru la varfurile exterioare
    const float innerRadius = 0.10f; // distanta de la centru la punctele interioare dintre varfuri
    const float thickness = 0.05f;   // grosimea stelei pe axa Z
    const int numPoints = 5;         // numarul de varfuri

    // Cele doua centre: unul pentru fata (+Z) si unul pentru spate (-Z)
    // pentru a forma triunghiuri piramidale mici pe fiecare parte
    glm::vec3 centerFront(0.0f, 0.0f, thickness);
    glm::vec3 centerBack(0.0f, 0.0f, -thickness);

    // Vom parcurge numPoints*2 pozitii pe cerc: varf si interior
    for (int i = 0; i < numPoints * 2; i++) {
        // Calculam unghiurile pentru punctul curent si urmatorul
        // Normalizam indexul in [0,1] si inmultim cu 2*PI
        float angle1 = (float)i / (numPoints * 2) * 2.0f * PI;
        float angle2 = (float)(i + 1) / (numPoints * 2) * 2.0f * PI;

        // Alternam raza: daca indexul e par => varf exterior, altfel punct interior
        float r1 = (i % 2 == 0) ? outerRadius : innerRadius;
        float r2 = ((i + 1) % 2 == 0) ? outerRadius : innerRadius;

        // Convertim polare in carteziene
        glm::vec3 p1(r1 * cosf(angle1 + PI / 2.0f), r1 * sinf(angle1 + PI / 2.0f), 0.0f);
        glm::vec3 p2(r2 * cosf(angle2 + PI / 2.0f), r2 * sinf(angle2 + PI / 2.0f), 0.0f);

        // Fata stelei +Z
        verts.push_back(centerFront);
        verts.push_back(p1);
        verts.push_back(p2);

        // Normala triunghiului de pe fata
        glm::vec3 nF = glm::normalize(glm::cross(p1 - centerFront, p2 - centerFront));
        norms.push_back(nF); norms.push_back(nF); norms.push_back(nF);

        // UV-uri simple (placeholder)
        uvs.push_back(glm::vec2(0.0f, 0.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));

        // Spatele stelei -Z
        verts.push_back(centerBack);
        verts.push_back(p2);
        verts.push_back(p1);

        glm::vec3 nB = glm::normalize(glm::cross(p2 - centerBack, p1 - centerBack));
        norms.push_back(nB); norms.push_back(nB); norms.push_back(nB);

        uvs.push_back(glm::vec2(0.0f, 0.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));
        uvs.push_back(glm::vec2(0.0f, 0.0f));
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

// Creare sfera pentru globuri
void CreateSphere(std::vector<glm::vec3>& verts, std::vector<glm::vec3>& norms, std::vector<glm::vec2>& uvs, float radius = 1.0f, int stacks = 12, int slices = 24)
{
    // Resetam vectorii de iesire
    verts.clear();
    norms.clear();
    uvs.clear();

    // Construim sfera ca o retea de 'stacks' (latitudini) si 'slices' (longitudini)
    for (int stack = 0; stack < stacks; ++stack) {
        // v0 si v1 sunt parametri in [0,1] pentru latitudine: 0 = jos, 1 = sus
        float v0 = (float)stack / stacks;
        float v1 = (float)(stack + 1) / stacks;

        // phi = latitudine in radiani (-pi/2 .. +pi/2)
        float phi0 = (v0 - 0.5f) * PI;
        float phi1 = (v1 - 0.5f) * PI;

        for (int slice = 0; slice < slices; ++slice) {
            // u0 si u1 sunt parametri in [0,1] pentru longitudine
            float u0 = (float)slice / slices;
            float u1 = (float)(slice + 1) / slices;

            // theta = longitudine in radiani (0 .. 2*PI)
            float theta0 = u0 * 2.0f * PI;
            float theta1 = u1 * 2.0f * PI;

            // Calculam cele patru puncte ale patrulaterului pe suprafata sferei
            glm::vec3 p00(radius * cosf(phi0) * cosf(theta0),
                          radius * cosf(phi0) * sinf(theta0),
                          radius * sinf(phi0));

            glm::vec3 p10(radius * cosf(phi1) * cosf(theta0),
                          radius * cosf(phi1) * sinf(theta0),
                          radius * sinf(phi1));

            glm::vec3 p11(radius * cosf(phi1) * cosf(theta1),
                          radius * cosf(phi1) * sinf(theta1),
                          radius * sinf(phi1));

            glm::vec3 p01(radius * cosf(phi0) * cosf(theta1),
                          radius * cosf(phi0) * sinf(theta1),
                          radius * sinf(phi0));

            // Triunghi 1: p00, p10, p11
            verts.push_back(p00);
            verts.push_back(p10);
            verts.push_back(p11);

            // Triunghi 2: p00, p11, p01
            verts.push_back(p00);
            verts.push_back(p11);
            verts.push_back(p01);

            // Adaugam normalele 
            norms.push_back(glm::normalize(p00));
            norms.push_back(glm::normalize(p10));
            norms.push_back(glm::normalize(p11));
            norms.push_back(glm::normalize(p00));
            norms.push_back(glm::normalize(p11));
            norms.push_back(glm::normalize(p01));

            // UV-uri simple
            uvs.push_back(glm::vec2(u0, v0));
            uvs.push_back(glm::vec2(u0, v1));
            uvs.push_back(glm::vec2(u1, v1));
            uvs.push_back(glm::vec2(u0, v0));
            uvs.push_back(glm::vec2(u1, v1));
            uvs.push_back(glm::vec2(u1, v0));
        }
    }
}

// Genereaza spirala pentru instalatie
void GenerateSpiralOnTree(std::vector<glm::vec3>& outPoints, int totalBulbs, float height, float baseRadius, int turns)
{
    outPoints.clear();

    // Configurare parametri spirala
    float maxHeightPercent = 0.95f;     // oprim la 95% din inaltime
    float startAngle = PI;              // unghi de start
    int resolution = 3000;              // generam 3000 de puncte pentru fir

    // Liste temporare pentru puncte si distante
    std::vector<glm::vec3> spiralPoints;
    std::vector<float> spiralDistances;

    // Generam firul complet, pas cu pas
    float totalLength = 0.0f;           // lungime cumulata
    glm::vec3 prevPoint;                // punctul anterior

    // Calculam primul punct
    {
        float r = baseRadius;
        float y = 0.0f;
        float a = startAngle;
        prevPoint = glm::vec3(cos(a) * r, y, sin(a) * r);

        // Adaugam punctul initial in liste
        spiralPoints.push_back(prevPoint);
        spiralDistances.push_back(0.0f);
    }

    for (int i = 1; i <= resolution; i++) {
        // t creste de la 0 la 0.95
        float t = ((float)i / resolution) * maxHeightPercent;

        // Matematica spiralei
        float y = t * height;
        float r = baseRadius * (1.0f - t);
        float angle = turns * 2.0f * 3.14159f * t + startAngle;

        glm::vec3 currPoint = glm::vec3(cos(angle) * r, y, sin(angle) * r);

        // Calculam lungimea segmentului si actualizam lungimea totala
        float segmentLen = glm::length(currPoint - prevPoint);
        totalLength += segmentLen;

        // Salvam punctul si distanta cumulata pana la el
        spiralPoints.push_back(currPoint);
        spiralDistances.push_back(totalLength);

        prevPoint = currPoint;
    }

    // Alegem becurile la distante aproximativ egale
    float idealStep = totalLength / (float)(totalBulbs - 1);

    for (int i = 0; i < totalBulbs; i++) {
        float targetDist = i * idealStep;

        // Gasim punctul cu distanta cea mai apropiata de tinta
        int bestIndex = 0;
        float bestDiff = 999999.0f;

        // Parcurgem distantele pentru a gasi cea mai buna potrivire
        for (int k = 0; k < (int)spiralDistances.size(); k++) {
            float diff = abs(spiralDistances[k] - targetDist);
            if (diff < bestDiff) {
                bestDiff = diff;
                bestIndex = k;
            }
        }

        // Adaugam punctul gasit in lista finala
        outPoints.push_back(spiralPoints[bestIndex]);
    }
}

// Creare fir instalatie
void CreateWire(GLuint& vao, GLuint& vbo, const std::vector<glm::vec3>& points)
{
    // Verificam daca avem deja un VAO si cream unul nou daca nu
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);

    // Verificam daca avem deja un VBO si cream unul nou daca nu    
    if (vbo == 0) {
        glGenBuffers(1, &vbo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Trimitem punctele catre placa video
    if (points.size() > 0) {
        // Calculam cata memorie ocupa punctele
        size_t bufferSize = points.size() * sizeof(glm::vec3);

        glBufferData(GL_ARRAY_BUFFER, bufferSize, points.data(), GL_STATIC_DRAW);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    // Actualizam variabila globala nrWirePoints
    nrWirePoints = (int)points.size();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


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

// Trimitem un obiect complet (mesh) catre placa video
void UploadMeshToGPU(GLuint& vao, GLuint& vbo,
    std::vector<glm::vec3>& verts,
    std::vector<glm::vec3>& norms,
    std::vector<glm::vec2>& texCoords)
{
    // Generam si activam VAO si VBO
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
    }
    glBindVertexArray(vao);
    if (vbo == 0) {
        glGenBuffers(1, &vbo);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Calculam cat spatiu ocupa tipurile de date in bytes
    size_t vertsSize = verts.size() * sizeof(glm::vec3);
    size_t normsSize = norms.size() * sizeof(glm::vec3);
    size_t uvsSize = texCoords.size() * sizeof(glm::vec2);
    size_t totalSize = vertsSize + normsSize + uvsSize;

    // Daca avem date de trimis
    if (totalSize > 0) {
        // Rezervam memoria
        glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);

        // Umplem spatiul rezervat pe bucati 
        // Punem varfurile la inceput (offset 0) ...
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertsSize, verts.data());
        if (normsSize > 0) {
            glBufferSubData(GL_ARRAY_BUFFER, vertsSize, normsSize, norms.data());
        }
        if (uvsSize > 0) {
            glBufferSubData(GL_ARRAY_BUFFER, vertsSize + normsSize, uvsSize, texCoords.data());
        }

        // Configuram atributele pentru shader
        // Atributul 0: pozitia
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Atributul 1: normala
        if (normsSize > 0) {
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)vertsSize);
        }
        // Atributul 2: textura
        if (uvsSize > 0) {
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vertsSize + normsSize));
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Cleanup(void) {
    glDeleteProgram(ProgramId);
    glDeleteVertexArrays(1, &VaoRoom); glDeleteBuffers(1, &VboRoom);
    glDeleteVertexArrays(1, &VaoTree); glDeleteBuffers(1, &VboTree);
    glDeleteVertexArrays(1, &VaoStar); glDeleteBuffers(1, &VboStar);
    glDeleteVertexArrays(1, &VaoIdFireplace); glDeleteBuffers(1, &VboIdFireplace);
    glDeleteVertexArrays(1, &VaoIdFireplaceTop); glDeleteBuffers(1, &VboIdFireplaceTop);
    glDeleteVertexArrays(1, &VaoIdFire); glDeleteBuffers(1, &VboIdFire);
    if (VaoWire) { glDeleteVertexArrays(1, &VaoWire); glDeleteBuffers(1, &VboWire); }
    if (VaoBulb) { glDeleteVertexArrays(1, &VaoBulb); glDeleteBuffers(1, &VboBulb); }
    if (WhiteTexture) glDeleteTextures(1, &WhiteTexture);
    if (BlackTexture) glDeleteTextures(1, &BlackTexture);
}


void Initialize(void)
{
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    // Incarcare sufragerie
    std::string roomPath = "livingroom.obj";
    loadOBJ(roomPath.c_str(), roomVertices, roomUvs, roomNormals);
    nrVerticesRoom = roomVertices.size();

    // Calculam centrul sufrageriei pentru a centra camera
    if (nrVerticesRoom > 0) {
        glm::vec3 sum(0.0f);
        // Adunam toate pozitiile punctelor din camera
        for (const auto& vertex : roomVertices) {
            sum += vertex;
        }
        // Calculam media
        glm::vec3 roomCenter = sum / (float)nrVerticesRoom;
        // Actualizam punctul de referinta
        refX = roomCenter.x;
        refY = roomCenter.y;
        refZ = roomCenter.z;
    }
    UploadMeshToGPU(VaoRoom, VboRoom, roomVertices, roomNormals, roomUvs);
    TextureRoom = LoadTexture("livingroom.png");

    // pun baza semineului la baza sufrageriei
    fireplacePosition.y = refY;

    // Creare brad 
    CreateTree(treeVertices, treeNormals, treeUvs);
    nrVerticesTree = treeVertices.size();
    UploadMeshToGPU(VaoTree, VboTree, treeVertices, treeNormals, treeUvs);
    TextureTree = LoadTexture("green.png"); 

    // Generare instalatie
    bulbCount = 70;
    float coneHeight = 2.5f;
    float coneBaseRadius = 1.0f;
    int turns = 8;
    GenerateSpiralOnTree(bulbLocalPositions, bulbCount, coneHeight, coneBaseRadius, turns);

    // Mutam usor fiecare bec deasupra firului
    float pushDistance = 0.02f; 
    for (int i = 0; i < bulbLocalPositions.size(); ++i) {
        glm::vec3 currentPos = bulbLocalPositions[i];
        // Calculam directia spre exterior
        glm::vec3 directionOut(currentPos.x, 0.0f, currentPos.z);
        if (glm::length(directionOut) > 0.0f) {
            // Normalizam directia 
            directionOut = glm::normalize(directionOut);
            bulbLocalPositions[i] += directionOut * pushDistance;
        }
    }

    // Creare mesh pentru bec 
    CreateSphere(bulbVerts, bulbNormals, bulbUvs, 1.0f, 10, 14);
    nrVerticesBulb = (int)bulbVerts.size();
    UploadMeshToGPU(VaoBulb, VboBulb, bulbVerts, bulbNormals, bulbUvs);

    // Generare fir instalatie
    std::vector<glm::vec3> wirePoints;
    int densityFactor = 50; // densitate mare pentru linie fina
    int totalWirePoints = bulbCount * densityFactor; 
    GenerateSpiralOnTree(wirePoints, totalWirePoints, coneHeight, coneBaseRadius, turns);
    CreateWire(VaoWire, VboWire, wirePoints);

    bulbColors.resize(bulbCount);
    bulbIntensities.resize(bulbCount);
    for (int i = 0; i < bulbCount; ++i) {
        int colorType = i % 4;
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
        bulbIntensities[i] = 2.0f;
    }

    WhiteTexture = LoadTexture("white.png");
    BlackTexture = LoadTexture("black.png");

    // Generare stea
    CreateProceduralStar(starVertices, starNormals, starUvs);
    nrVerticesStar = starVertices.size();
    UploadMeshToGPU(VaoStar, VboStar, starVertices, starNormals, starUvs);

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

    // Camera
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

    // Desenare sufragerie
    if (VaoRoom != 0 && nrVerticesRoom > 0) {
        glm::mat4 modelRoom = glm::mat4(1.0f);
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelRoom[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureRoom);
        glBindVertexArray(VaoRoom);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesRoom);
    }

    // Desenare brad cu instalatie si beculete
    if (VaoTree != 0 && nrVerticesTree > 0) {
        // Desenare brad
        glm::mat4 modelTree = glm::mat4(1.0f);
        // Pozitionare si Scalare
        modelTree = glm::translate(modelTree, treePosition);
        modelTree = glm::scale(modelTree, glm::vec3(treeScale, treeScale * 1.5f, treeScale));
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTree[0][0]);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureTree);
        glBindVertexArray(VaoTree);
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesTree);

        // Desenare fir instalatie
        if (VaoWire != 0 && nrWirePoints > 0) {
            // Fortam culoarea neagra in shader
            if (allowBlackLocation >= 0) glUniform1i(allowBlackLocation, 1);
            glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTree[0][0]);
            glBindVertexArray(VaoWire);
            // Setari pentru linie (netezire + transparenta)
            glEnable(GL_LINE_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glLineWidth(2.5f);
            glDrawArrays(GL_LINE_STRIP, 0, nrWirePoints);
            // Resetam setarile grafice
            glLineWidth(1.0f);
            glDisable(GL_LINE_SMOOTH);
            glDisable(GL_BLEND);
            if (allowBlackLocation >= 0) glUniform1i(allowBlackLocation, 0);
        }

        // Desenare beculete
        if (VaoBulb != 0 && nrVerticesBulb > 0) {
            float timeSeconds = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
            for (int i = 0; i < bulbCount; ++i) {
                switch (lightMode) {
                    case 1: // Toate aprinse
                        bulbIntensities[i] = 10.0f;
                        break;
                    case 2: // Palpait
                        bulbIntensities[i] = 6.0f + 4.0f * sin(timeSeconds * 3.0f);
                        break;
                    case 3: // Pe sarite
                    {
                        int speed = int(timeSeconds * 5.0f);
                        if ((i + speed) % 3 == 0) {
                            bulbIntensities[i] = 10.0f;
                        } else {
                            bulbIntensities[i] = 0.0f;
                        }
                    }
                    break;
                    case 4: // Stins 
                        bulbIntensities[i] = 0.0f;
                        break;
                }
            }

            // Trimitem datele becurilor catre shader
            if (bulbPosLocation >= 0 && bulbCountLocation >= 0) {
                std::vector<glm::vec3> worldPositions(bulbCount);
                for (int i = 0; i < bulbCount; ++i) {
                    // Calculam pozitia globala a fiecarui bec
                    glm::vec4 pos4 = modelTree * glm::vec4(bulbLocalPositions[i], 1.0f);
                    worldPositions[i] = glm::vec3(pos4.x, pos4.y, pos4.z);
                }
                glUniform1i(bulbCountLocation, bulbCount);
                glUniform3fv(bulbPosLocation, bulbCount, (const GLfloat*)worldPositions.data());
                if (bulbColorLocation >= 0) 
                    glUniform3fv(bulbColorLocation, bulbCount, (const GLfloat*)bulbColors.data());
                if (bulbIntensityLocation >= 0) 
                    glUniform1fv(bulbIntensityLocation, bulbCount, (const GLfloat*)bulbIntensities.data());
            }
            // Desenam sferele becurilor
            float bulbSize = 0.03f;
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, WhiteTexture ? WhiteTexture : TextureTree);
            if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 1);
            glBindVertexArray(VaoBulb);
            for (int i = 0; i < bulbCount; ++i) {
                glm::mat4 modelBulb = modelTree; 
                modelBulb = glm::translate(modelBulb, bulbLocalPositions[i]);
                modelBulb = glm::scale(modelBulb, glm::vec3(bulbSize));
                glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelBulb[0][0]);
                // Calculam culoarea
                glm::vec3 drawColor = bulbColors[i];
                if (bulbIntensities[i] < 0.5f) {
                    drawColor = glm::vec3(0.05f, 0.05f, 0.05f); // Gri inchis
                } else {
                    // Calculam luminozitatea
                    float brightness = bulbIntensities[i] / 10.0f;
                    if (brightness < 0.5f) brightness = 0.5f;
                    if (brightness > 1.0f) brightness = 1.0f;
                    drawColor *= brightness;
                }
                // Trimitem culoarea finala la shader
                if (solidColorLocation >= 0) {
                    glUniform3f(solidColorLocation, drawColor.x, drawColor.y, drawColor.z);
                }
                glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesBulb);
            }

            // Dezactivam modul Solid Color
            if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 0);
            glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTree[0][0]);
            if (bulbCountLocation >= 0) glUniform1i(bulbCountLocation, 0);
        }
    }

    // Desenare stea
    // if (VaoStar != 0 && nrVerticesStar > 0) {
    //     glm::mat4 modelStar = glm::mat4(1.0f);
    //     float tipHeight = 1.875f;

    //     // 1. Pozitionare (Varf)
    //     modelStar = glm::translate(modelStar, treePosition + glm::vec3(0.0f, tipHeight, 0.0f));

    //     // 2. Rotire FIXA (fara 'time')
    //     // O rotim cu -90 grade (-PI/2) ca sa priveasca spre spate (X-), unde e firul
    //     // Am eliminat offset-ul +0.5 care devia pozitia stelei.
    //     modelStar = glm::rotate(modelStar, -PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));

    //     // 3. Scalare (Mai mica)
    //     // Era 1.0 (implicita), acum o facem 0.6
    //     modelStar = glm::scale(modelStar, glm::vec3(0.6f));

    //     glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelStar[0][0]);

    //     // Culoare Galben-Auriu
    //     if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 1);
    //     if (solidColorLocation >= 0) glUniform3f(solidColorLocation, 0.7f, 0.5f, 0.0f);

    //     glActiveTexture(GL_TEXTURE0);
    //     glBindTexture(GL_TEXTURE_2D, WhiteTexture);
    //     glBindVertexArray(VaoStar);

    //     glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesStar);

    //     if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 0);
    // }
    if (VaoStar != 0 && nrVerticesStar > 0) {
        glm::mat4 modelStar = glm::mat4(1.0f);
        // Inaltimea la care punem steaua 
        float starHeightOffset = 1.875f;
        // O mutam deasupra pozitiei bradului
        modelStar = glm::translate(modelStar, treePosition + glm::vec3(0.0f, starHeightOffset, 0.0f));
        modelStar = glm::rotate(modelStar, -PI / 2.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        modelStar = glm::scale(modelStar, glm::vec3(0.6f));
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelStar[0][0]);

        if (useSolidColorLocation >= 0) glUniform1i(useSolidColorLocation, 1);
        // Setam culoarea aurie (R=0.7, G=0.5, B=0.0)
        if (solidColorLocation >= 0) glUniform3f(solidColorLocation, 0.7f, 0.5f, 0.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, WhiteTexture);
        glBindVertexArray(VaoStar);
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
    glutCreateWindow("Scena de Craciun");
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
