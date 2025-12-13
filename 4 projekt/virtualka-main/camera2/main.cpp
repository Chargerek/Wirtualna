#include "glad/glad.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#pragma warning(push)
#pragma warning(disable: 4244)
#include "linmath.h"
#pragma warning(pop)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Funkcje do ładowania shaderów z plików
// Shadery są w oddzielnych plikach zgodnie z wymaganiami
char* loadShaderFile(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Nie można otworzyć pliku: %s\n", filename);
        return NULL;
    }
    
    // Sprawdzamy rozmiar pliku
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Alokujemy pamięć i wczytujemy zawartość
    char* buffer = (char*)malloc(length + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    fread(buffer, 1, length, file);
    buffer[length] = '\0'; // Dodajemy null terminator
    fclose(file);
    
    return buffer;
}

// Kompilacja shadera z pliku
GLuint loadShader(GLenum type, const char* filename) {
    char* source = loadShaderFile(filename);
    if (!source) {
        return 0;
    }
    
    // Tworzymy shader i kompilujemy
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char**)&source, NULL);
    glCompileShader(shader);
    
    // Sprawdzamy czy kompilacja się powiodła
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Błąd kompilacji shadera %s:\n%s\n", filename, infoLog);
        free(source);
        glDeleteShader(shader);
        return 0;
    }
    
    free(source);
    return shader;
}

// Tworzenie programu shaderowego z vertex i fragment shadera
GLuint createShaderProgram(const char* vertFile, const char* fragFile) {
    GLuint vertShader = loadShader(GL_VERTEX_SHADER, vertFile);
    GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, fragFile);
    
    if (!vertShader || !fragShader) {
        if (vertShader) glDeleteShader(vertShader);
        if (fragShader) glDeleteShader(fragShader);
        return 0;
    }
    
    // Łączymy shadery w program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    
    // Sprawdzamy czy linkowanie się powiodło
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Błąd linkowania programu:\n%s\n", infoLog);
        glDeleteProgram(program);
        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
        return 0;
    }
    
    // Usuwamy shadery bo są już w programie
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return program;
}

// Struktura wierzchołka - pozycja, normalna, kolor, współrzędne tekstury
struct Vertex {
    float x, y, z;      // pozycja
    float nx, ny, nz;   // normalna (do oświetlenia)
    float r, g, b;      // kolor
    float u, v;         // współrzędne tekstury
};

// Sześcian z normalnymi i współrzędnymi tekstury
static const Vertex cubeVertices[] = {
    // Front face
    {-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f},
    { 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f},
    // Back face
    { 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    {-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f},
    {-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    { 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    {-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    { 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f},
    // Top face
    {-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f},
    { 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f},
    // Bottom face
    {-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f},
    { 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f},
    // Right face
    { 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f},
    { 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    { 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    { 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    { 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f},
    // Left face
    {-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    {-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f},
    {-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f},
    {-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f},
    {-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f},
};

// Płaszczyzna dla flagi (z normalnymi i teksturą)
static const Vertex planeVertices[] = {
    {-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f},
    { 1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f},
    { 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f},
    {-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f},
    { 1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f},
    {-1.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f},
};

// Funkcje do tworzenia tekstur proceduralnych
// Różne wzory dla różnych obiektów
GLuint createProceduralTexture(int width, int height, int patternType) {
    unsigned char* data = (unsigned char*)malloc(width * height * 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            
            switch (patternType) {
                case 0: // Szachownica czarno-biała
                {
                    int check = ((x / 32) + (y / 32)) % 2;
                    data[index + 0] = check * 255;
                    data[index + 1] = check * 255;
                    data[index + 2] = check * 255;
                    break;
                }
                case 1: // Czerwono-niebieska szachownica
                {
                    int check = ((x / 32) + (y / 32)) % 2;
                    data[index + 0] = check * 255;
                    data[index + 1] = 0;
                    data[index + 2] = (1 - check) * 255;
                    break;
                }
                case 2: // Zielono-żółta szachownica
                {
                    int check = ((x / 32) + (y / 32)) % 2;
                    data[index + 0] = check * 255;
                    data[index + 1] = 255;
                    data[index + 2] = (1 - check) * 128;
                    break;
                }
                case 3: // Niebiesko-fioletowa szachownica
                {
                    int check = ((x / 32) + (y / 32)) % 2;
                    data[index + 0] = check * 128;
                    data[index + 1] = 0;
                    data[index + 2] = 255;
                    break;
                }
                case 4: // Pomarańczowo-różowa szachownica
                {
                    int check = ((x / 32) + (y / 32)) % 2;
                    data[index + 0] = 255;
                    data[index + 1] = check * 128;
                    data[index + 2] = (1 - check) * 200;
                    break;
                }
                default: // Domyślna szachownica
                {
                    int check = ((x / 32) + (y / 32)) % 2;
                    data[index + 0] = check * 255;
                    data[index + 1] = check * 255;
                    data[index + 2] = check * 255;
                    break;
                }
            }
        }
    }
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    free(data);
    return texture;
}

// Funkcja do tworzenia żółtej tekstury (dla słońca)
GLuint createYellowTexture(int width, int height) {
    unsigned char* data = (unsigned char*)malloc(width * height * 3);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 3;
            // Żółty kolor
            data[index + 0] = 255; // R
            data[index + 1] = 255; // G
            data[index + 2] = 0;    // B
        }
    }
    
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    free(data);
    return texture;
}

// SEKCJA 4: STRUKTURY KAMERY I APLIKACJ
typedef struct {
    vec3 position;
    float yaw;
    float pitch;
} Camera;

typedef struct {
    vec3 position;
    vec3 color;
} Light;

typedef struct {
    vec3 position;
    int materialType; // 0=diffuse, 1=specular, 2=blinn-phong, 3=texture, 4=flag
    int textureIndex; // Indeks tekstury dla tego obiektu
    vec3 color; // Kolor obiektu
} SceneObject;

typedef struct {
    Camera camera;
    Light light;
    float fov;
    float moveSpeed;
    float mouseSensitivity;
    
    int keyW, keyS, keyA, keyD;
    int keySpace, keyC;
    int keyL;
    
    int controlMode; // 0 = camera, 1 = light
    
    SceneObject objects[5];
    int numObjects;
} AppState;


// SEKCJA 5: FUNKCJE POMOCNICZE KAMERY


void calculateViewMatrix(mat4x4 V, const Camera* camera) {
    mat4x4 Wc, T, Ry, Rx;
    mat4x4_translate(T, camera->position[0], camera->position[1], camera->position[2]);
    mat4x4_identity(Ry);
    mat4x4_rotate_Y(Ry, Ry, camera->yaw);
    mat4x4_identity(Rx);
    mat4x4_rotate_X(Rx, Rx, camera->pitch);
    mat4x4_mul(Wc, Ry, Rx);
    mat4x4_mul(Wc, T, Wc);
    mat4x4_invert(V, Wc);
}

void getCameraForward(vec3 forward, const Camera* camera) {
    forward[0] = -sinf(camera->yaw) * cosf(camera->pitch);
    forward[1] = sinf(camera->pitch);
    forward[2] = -cosf(camera->yaw) * cosf(camera->pitch);
}

void getCameraRight(vec3 right, const Camera* camera) {
    right[0] = cosf(camera->yaw);
    right[1] = 0.0f;
    right[2] = -sinf(camera->yaw);
}

// SEKCJA 6: OBSŁUGA WEJŚCIA

static void error_callback(int error, const char* description) {
    (void)error;
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    (void)scancode;
    (void)mods;
    AppState* app = (AppState*)glfwGetWindowUserPointer(window);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    // Klawisz L zmienia tryb sterowania (wymaganie zadania)
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        app->controlMode = 1 - app->controlMode; // 0=kamera, 1=światło
    }
    
    if (key == GLFW_KEY_W) {
        app->keyW = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_S) {
        app->keyS = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_A) {
        app->keyA = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_D) {
        app->keyD = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_SPACE) {
        app->keySpace = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_C) {
        app->keyC = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    AppState* app = (AppState*)glfwGetWindowUserPointer(window);
    static double lastX = 0.0, lastY = 0.0;
    static int firstMouse = 1;
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = 0;
        return;
    }
    
    if (app->controlMode == 0) { // Tylko w trybie kamery
        double xoffset = xpos - lastX;
        double yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;
        
        xoffset *= app->mouseSensitivity;
        yoffset *= app->mouseSensitivity;
        
        app->camera.yaw -= (float)xoffset;
        app->camera.pitch += (float)yoffset;
        
        if (app->camera.pitch > 89.0f * (float)M_PI / 180.0f)
            app->camera.pitch = 89.0f * (float)M_PI / 180.0f;
        if (app->camera.pitch < -89.0f * (float)M_PI / 180.0f)
            app->camera.pitch = -89.0f * (float)M_PI / 180.0f;
    } else {
        lastX = xpos;
        lastY = ypos;
    }
}

// SEKCJA 7: INICJALIZACJA

void initAppState(AppState* app) {
    app->camera.position[0] = 0.0f;
    app->camera.position[1] = 0.0f;
    app->camera.position[2] = 8.0f;
    app->camera.yaw = 0.0f;
    app->camera.pitch = 0.0f;
    
    app->light.position[0] = 2.0f;
    app->light.position[1] = 2.0f;
    app->light.position[2] = 2.0f;
    app->light.color[0] = 1.0f;
    app->light.color[1] = 1.0f;
    app->light.color[2] = 1.0f;
    
    app->fov = 60.0f;
    app->moveSpeed = 5.0f;
    app->mouseSensitivity = 0.001f;
    app->controlMode = 0; // Tryb kamery
    
    app->keyW = app->keyS = app->keyA = app->keyD = 0;
    app->keySpace = app->keyC = 0;
    app->keyL = 0;
    
    // Inicjalizacja 5 obiektów - każdy z innym materiałem zgodnie z wymaganiami
    // 0=diffuse, 1=specular, 2=blinn-phong, 3=texture, 4=flag
    app->numObjects = 5;
    
    // Obiekt 1: Model światła rozproszonego (diffuse) - niebieski
    app->objects[0].position[0] = -4.0f; app->objects[0].position[1] = 0.0f; app->objects[0].position[2] = 0.0f;
    app->objects[0].materialType = 0;
    app->objects[0].textureIndex = 0;
    app->objects[0].color[0] = 0.0f; app->objects[0].color[1] = 0.0f; app->objects[0].color[2] = 1.0f;
    
    // Obiekt 2: Model światła odbitego (specular) - czerwony
    app->objects[1].position[0] = -2.0f; app->objects[1].position[1] = 0.0f; app->objects[1].position[2] = 0.0f;
    app->objects[1].materialType = 1;
    app->objects[1].textureIndex = 0;
    app->objects[1].color[0] = 1.0f; app->objects[1].color[1] = 0.0f; app->objects[1].color[2] = 0.0f;
    
    // Obiekt 3: Model Blinna-Phonga - zielony
    app->objects[2].position[0] = 0.0f; app->objects[2].position[1] = 0.0f; app->objects[2].position[2] = 0.0f;
    app->objects[2].materialType = 2;
    app->objects[2].textureIndex = 0;
    app->objects[2].color[0] = 0.0f; app->objects[2].color[1] = 1.0f; app->objects[2].color[2] = 0.0f;
    
    // Obiekt 4: Teksturowanie bez oświetlenia - żółty
    app->objects[3].position[0] = 2.0f; app->objects[3].position[1] = 0.0f; app->objects[3].position[2] = 0.0f;
    app->objects[3].materialType = 3;
    app->objects[3].textureIndex = 1; // Różna tekstura
    app->objects[3].color[0] = 1.0f; app->objects[3].color[1] = 1.0f; app->objects[3].color[2] = 0.0f;
    
    // Obiekt 5: Efekt falowania flagi - fioletowy
    app->objects[4].position[0] = 4.0f; app->objects[4].position[1] = 0.0f; app->objects[4].position[2] = 0.0f;
    app->objects[4].materialType = 4;
    app->objects[4].textureIndex = 2; // Różna tekstura
    app->objects[4].color[0] = 1.0f; app->objects[4].color[1] = 0.0f; app->objects[4].color[2] = 1.0f;
}


// SEKCJA 8: GŁÓWNA FUNKCJA

int main(void) {
    AppState app;
    initAppState(&app);
    
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Oswietlenie i Teksturowanie", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Ładowanie shaderów z plików - zgodnie z wymaganiami (5 shaderów vertex + 5 fragment)
    GLuint programs[5];
    programs[0] = createShaderProgram("shaders/diffuse.vert", "shaders/diffuse.frag");      // Diffuse
    programs[1] = createShaderProgram("shaders/specular.vert", "shaders/specular.frag");    // Specular
    programs[2] = createShaderProgram("shaders/blinn_phong.vert", "shaders/blinn_phong.frag"); // Blinn-Phong
    programs[3] = createShaderProgram("shaders/texture.vert", "shaders/texture.frag");      // Texture
    programs[4] = createShaderProgram("shaders/flag.vert", "shaders/flag.frag");             // Flag
    
    for (int i = 0; i < 5; i++) {
        if (!programs[i]) {
            fprintf(stderr, "Błąd ładowania shaderów!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Tworzenie różnych tekstur dla różnych obiektów
    GLuint textures[5];
    textures[0] = createProceduralTexture(256, 256, 0); // Szachownica czarno-biała
    textures[1] = createProceduralTexture(256, 256, 1); // Czerwono-niebieska
    textures[2] = createProceduralTexture(256, 256, 2); // Zielono-żółta
    textures[3] = createProceduralTexture(256, 256, 3); // Niebiesko-fioletowa
    textures[4] = createProceduralTexture(256, 256, 4); // Pomarańczowo-różowa
    
    // Żółta tekstura dla słońca
    GLuint yellowTexture = createYellowTexture(256, 256);
    
    // Bufor wierzchołków dla sześcianu
    GLuint cubeVBO;
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    
    // Bufor wierzchołków dla płaszczyzny (flaga)
    GLuint planeVBO;
    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;
        
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Aktualizacja pozycji kamery lub światła
        if (app.controlMode == 0) { // Tryb kamery
            vec3 forward, right;
            getCameraForward(forward, &app.camera);
            getCameraRight(right, &app.camera);
            
            if (app.keyW) {
                app.camera.position[0] += forward[0] * app.moveSpeed * deltaTime;
                app.camera.position[1] += forward[1] * app.moveSpeed * deltaTime;
                app.camera.position[2] += forward[2] * app.moveSpeed * deltaTime;
            }
            if (app.keyS) {
                app.camera.position[0] -= forward[0] * app.moveSpeed * deltaTime;
                app.camera.position[1] -= forward[1] * app.moveSpeed * deltaTime;
                app.camera.position[2] -= forward[2] * app.moveSpeed * deltaTime;
            }
            if (app.keyA) {
                app.camera.position[0] -= right[0] * app.moveSpeed * deltaTime;
                app.camera.position[2] -= right[2] * app.moveSpeed * deltaTime;
            }
            if (app.keyD) {
                app.camera.position[0] += right[0] * app.moveSpeed * deltaTime;
                app.camera.position[2] += right[2] * app.moveSpeed * deltaTime;
            }
        } else { 
            // Tryb sterowania światłem - zgodnie z wymaganiami:
            // W/S -> -Z/+Z, A/D -> -X/+X, SPACE/C -> +Y/-Y
            if (app.keyW) app.light.position[2] -= app.moveSpeed * deltaTime; // -Z
            if (app.keyS) app.light.position[2] += app.moveSpeed * deltaTime; // +Z
            if (app.keyA) app.light.position[0] -= app.moveSpeed * deltaTime; // -X
            if (app.keyD) app.light.position[0] += app.moveSpeed * deltaTime; // +X
            if (app.keySpace) app.light.position[1] += app.moveSpeed * deltaTime; // +Y
            if (app.keyC) app.light.position[1] -= app.moveSpeed * deltaTime; // -Y
        }
        
        // Macierze
        mat4x4 P, V, M, MVP;
        float fov_rad = app.fov * (float)M_PI / 180.0f;
        mat4x4_perspective(P, fov_rad, ratio, 0.1f, 100.0f);
        calculateViewMatrix(V, &app.camera);
        
        // Renderowanie 5 obiektów - każdy z innym materiałem
        for (int i = 0; i < app.numObjects; i++) {
            // Wybieramy odpowiedni shader w zależności od typu materiału
            GLuint program = programs[app.objects[i].materialType];
            glUseProgram(program);
            
            mat4x4_identity(M);
            mat4x4_translate_in_place(M, app.objects[i].position[0], 
                                         app.objects[i].position[1], 
                                         app.objects[i].position[2]);
            mat4x4_mul(MVP, V, M);
            mat4x4_mul(MVP, P, MVP);
            
            // Pobieramy lokalizacje uniformów i przekazujemy dane do shaderów
            GLint mvpLoc = glGetUniformLocation(program, "MVP");
            GLint mLoc = glGetUniformLocation(program, "M");
            GLint vLoc = glGetUniformLocation(program, "V");
            GLint lightPosLoc = glGetUniformLocation(program, "lightPos");      // Pozycja światła punktowego
            GLint lightColorLoc = glGetUniformLocation(program, "lightColor");
            GLint viewPosLoc = glGetUniformLocation(program, "viewPos");
            GLint timeLoc = glGetUniformLocation(program, "time");              // Dla animacji flagi
            GLint texLoc = glGetUniformLocation(program, "textureSampler");
            GLint objectColorLoc = glGetUniformLocation(program, "objectColor");  // Kolor obiektu
            
            // Przekazujemy macierze i parametry do shaderów
            if (mvpLoc >= 0) glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, (const GLfloat*)MVP);
            if (mLoc >= 0) glUniformMatrix4fv(mLoc, 1, GL_FALSE, (const GLfloat*)M);
            if (vLoc >= 0) glUniformMatrix4fv(vLoc, 1, GL_FALSE, (const GLfloat*)V);
            if (lightPosLoc >= 0) glUniform3fv(lightPosLoc, 1, app.light.position);
            if (lightColorLoc >= 0) glUniform3fv(lightColorLoc, 1, app.light.color);
            if (viewPosLoc >= 0) glUniform3fv(viewPosLoc, 1, app.camera.position);
            if (timeLoc >= 0) glUniform1f(timeLoc, (float)glfwGetTime());
            if (objectColorLoc >= 0) glUniform3fv(objectColorLoc, 1, app.objects[i].color);
            
            // Dla obiektów z teksturami (texture i flag) ustawiamy odpowiednią teksturę
            if (app.objects[i].materialType == 3 || app.objects[i].materialType == 4) {
                glActiveTexture(GL_TEXTURE0);
                int texIdx = app.objects[i].textureIndex;
                if (texIdx >= 0 && texIdx < 5) {
                    glBindTexture(GL_TEXTURE_2D, textures[texIdx]);
                } else {
                    glBindTexture(GL_TEXTURE_2D, textures[0]);
                }
                if (texLoc >= 0) glUniform1i(texLoc, 0);
            }
            
            GLint vposLoc = glGetAttribLocation(program, "vPos");
            GLint vnormalLoc = glGetAttribLocation(program, "vNormal");
            GLint vcolLoc = glGetAttribLocation(program, "vCol");
            GLint vtexLoc = glGetAttribLocation(program, "vTexCoord");
            
            if (app.objects[i].materialType == 4) { // Flaga - użyj płaszczyzny
                glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
            } else {
                glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
            }
            
            if (vposLoc >= 0) {
                glEnableVertexAttribArray(vposLoc);
                glVertexAttribPointer(vposLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            }
            if (vnormalLoc >= 0) {
                glEnableVertexAttribArray(vnormalLoc);
                glVertexAttribPointer(vnormalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
            }
            if (vcolLoc >= 0) {
                glEnableVertexAttribArray(vcolLoc);
                glVertexAttribPointer(vcolLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
            }
            if (vtexLoc >= 0) {
                glEnableVertexAttribArray(vtexLoc);
                glVertexAttribPointer(vtexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9));
            }
            
            if (app.objects[i].materialType == 4) {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
        
        // Wizualizacja światła punktowego jako kostki 
        // Używamy żółtej tekstury żeby wyglądało jak słońce
        glUseProgram(programs[3]);
        mat4x4_identity(M);
        mat4x4_translate_in_place(M, app.light.position[0], app.light.position[1], app.light.position[2]);
        mat4x4_scale_aniso(M, M, 0.2f, 0.2f, 0.2f); // Mała kostka
        mat4x4_mul(MVP, V, M);
        mat4x4_mul(MVP, P, MVP);
        
        GLint mvpLoc = glGetUniformLocation(programs[3], "MVP");
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, (const GLfloat*)MVP);
        
        // Żółta tekstura dla światła
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, yellowTexture);
        GLint texLoc = glGetUniformLocation(programs[3], "textureSampler");
        if (texLoc >= 0) glUniform1i(texLoc, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        GLint vposLoc = glGetAttribLocation(programs[3], "vPos");
        GLint vtexLoc = glGetAttribLocation(programs[3], "vTexCoord");
        
        if (vposLoc >= 0) {
            glEnableVertexAttribArray(vposLoc);
            glVertexAttribPointer(vposLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        }
        if (vtexLoc >= 0) {
            glEnableVertexAttribArray(vtexLoc);
            glVertexAttribPointer(vtexLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 9));
        }
        
        // Wyłączamy depth test żeby światło było zawsze widoczne
        glDisable(GL_DEPTH_TEST);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glEnable(GL_DEPTH_TEST);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Czyszczenie
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &planeVBO);
    for (int i = 0; i < 5; i++) {
        glDeleteTextures(1, &textures[i]);
        glDeleteProgram(programs[i]);
    }
    glDeleteTextures(1, &yellowTexture);
    
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


