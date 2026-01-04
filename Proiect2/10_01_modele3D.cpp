//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul X - 10_01_modele3D.cpp           |
// ================================================
// 
// Program FINAL: Camera + BRAD GENERAT PROCEDURAL (Fara erori de geometrie)

// Biblioteci
#include <windows.h>  
#include <stdio.h>
#include <stdlib.h> 
#include <cstdlib> 
#include <vector>
#include <math.h>
#include <iostream>
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

// --- SETARI GENERALE ---
float PI = 3.141592f;
glm::mat4 myMatrix, view, projection;

// TINTA Camerei (Centrul modelului)
float refX = 0.0f, refY = 0.0f, refZ = 0.0f;
// Pozitia OBSERVATORULUI
float obsX, obsY, obsZ;

// --- SETARI CAMERA ---
float alpha = 0.2f;
float beta = -0.5f;
float dist = 6.0f;

float minAlpha = 0.01f;
float maxAlpha = 1.5f;
float incrAlpha = 0.05f;
float incrBeta = 0.05f;

// Proiectie
float width = 1200, height = 900, dNear = 0.1f, fov = 60.f * PI / 180;

// --- SETARI POZITIE BRAD ---
// Ajustam Y la 0.0f ca sa stea pe podea (conul generat are baza la Y=0)
glm::vec3 treePosition(2.9f, 3.0f, -0.5f);
// Scalare normala (1.0) pentru ca obiectul generat are marimi standard (2 metri inaltime)
float treeScale = 0.5f;

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


// ZOOM (+/-)
void processNormalKeys(unsigned char key, int x, int y)
{
    switch (key) {
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
}

void Initialize(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

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

    UploadMeshToGPU(VaoIdRoom, VboIdRoom, roomVertices, roomNormals, roomUvs);
    TextureRoom = LoadTexture("livingroom.png");

    // 2. GENERARE BRAD (AICI E SCHIMBAREA)
    // Nu mai incarcam din fisier, ci generam noi conul.
    CreateProceduralCone(treeVertices, treeNormals, treeUvs);
    nrVerticesTree = treeVertices.size();

    UploadMeshToGPU(VaoIdTree, VboIdTree, treeVertices, treeNormals, treeUvs);
    TextureTree = LoadTexture("green.png"); // Folosim patratul verde

    ProgramId = LoadShaders("10_01_Shader.vert", "10_01_Shader.frag");
    glUseProgram(ProgramId);

    nrVertLocation = glGetUniformLocation(ProgramId, "nrVertices");
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
    viewPosLocation = glGetUniformLocation(ProgramId, "viewPos");
    viewLocation = glGetUniformLocation(ProgramId, "view");
    projLocation = glGetUniformLocation(ProgramId, "projection");

    glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // CAMERA
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

        // --- ATENTIE: Am scos rotirea! ---
        // Conul generat de noi sta deja in picioare (Y-Up), deci nu il mai rotim.
        // modelTree = glm::rotate(modelTree, -PI / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));

        // Scalam la marime normala (1.0) sau ajustam dupa preferinta (latime, inaltime, latime)
        modelTree = glm::scale(modelTree, glm::vec3(treeScale, treeScale * 1.5f, treeScale));

        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &modelTree[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureTree);
        glBindVertexArray(VaoIdTree);

        // Desenam normal
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)nrVerticesTree);
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