#include "glad/glad.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Definicja wierzchołków bryły - każdy wierzchołek ma pozycję (x,y,z) i kolor (r,g,b)
static const struct {
    float x, y, z;
    float r, g, b;
} vertices[] = {

    { 0.0f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f }, // czerwony
    { 0.433f, 0.25f, -0.5f,  1.0f, 0.0f, 0.0f },
    { -0.433f, 0.25f, -0.5f, 1.0f, 0.0f, 0.0f },

    { 0.0f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f }, // zielony
    { 0.433f, 0.25f, 0.5f,   0.0f, 1.0f, 0.0f },
    { -0.433f, 0.25f, 0.5f,  0.0f, 1.0f, 0.0f },

    { 0.0f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f }, // niebieski
    { 0.433f, 0.25f, -0.5f,  0.0f, 0.0f, 1.0f },
    { 0.433f, 0.25f, 0.5f,   0.0f, 0.0f, 1.0f },

    { 0.0f, -0.5f, -0.5f,    0.0f, 0.0f, 1.0f }, // niebieski 2 
    { 0.433f, 0.25f, 0.5f,   0.0f, 0.0f, 1.0f },
    { 0.0f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f },

    { 0.433f, 0.25f, -0.5f,  1.0f, 1.0f, 0.0f }, // żółty
    { -0.433f, 0.25f, -0.5f, 1.0f, 1.0f, 0.0f }, 
    { -0.433f, 0.25f, 0.5f,  1.0f, 1.0f, 0.0f },

    { 0.433f, 0.25f, -0.5f,  1.0f, 1.0f, 0.0f }, // żółty 2
    { -0.433f, 0.25f, 0.5f,  1.0f, 1.0f, 0.0f },
    { 0.433f, 0.25f, 0.5f,   1.0f, 1.0f, 0.0f },

    { -0.433f, 0.25f, -0.5f,     0.5f, 0.0f, 0.5f }, // fioletowy
    { 0.0f, -0.5f, -0.5f,        0.5f,0.0f, 0.5f },
    { 0.0f, -0.5f, 0.5f,         0.5f,0.0f, 0.5f },

    { -0.433f, 0.25f, -0.5f,        0.5f, 0.0f, 0.5f }, // fioletowy 2
    { 0.0f, -0.5f, 0.5f,            0.5f,0.0f, 0.5f },
    { -0.433f, 0.25f, 0.5f,         0.5f,0.0f, 0.5f }, 
};

// Shadery - programy które mówią karcie graficznej jak rysować
static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec3 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

// Struktury do przechowywania stanu kamery i aplikacji
typedef struct {
    vec3 position;
    float yaw;    // obrót w lewo/prawo
    float pitch;  // obrót góra/dół
} Camera;

typedef struct {
    Camera camera;
    float fov;
    float moveSpeed;
    float mouseSensitivity;
    int keyW, keyS, keyA, keyD;  // które klawisze są wciśnięte
    vec3 objectPositions[15];
    int numObjects;
} AppState;

// Funkcje do obliczania macierzy widoku i kierunków kamery
void calculateViewMatrix(mat4x4 V, const Camera* camera) {
    mat4x4 Wc, T, Ry, Rx;
    
    mat4x4_translate(T, camera->position[0], camera->position[1], camera->position[2]);
    mat4x4_identity(Ry);
    mat4x4_rotate_Y(Ry, Ry, camera->yaw);
    mat4x4_identity(Rx);
    mat4x4_rotate_X(Rx, Rx, camera->pitch);
    
    mat4x4_mul(Wc, Ry, Rx);
    mat4x4_mul(Wc, T, Wc);
    mat4x4_invert(V, Wc);  // macierz widoku to odwrotność macierzy kamery
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

// Obsługa błędów
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// Obsługa klawiatury - zapisuje które klawisze są wciśnięte
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    AppState* app = (AppState*)glfwGetWindowUserPointer(window);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    
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
    
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            app->fov += 5.0f;
            if (app->fov > 120.0f) app->fov = 120.0f;
        }
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            app->fov -= 5.0f;
            if (app->fov < 10.0f) app->fov = 10.0f;
        }
    }
}

// Obsługa myszy - obraca kamerę gdy ruszasz myszką
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    AppState* app = (AppState*)glfwGetWindowUserPointer(window);
    static double lastX = 0.0, lastY = 0.0;
    static int firstMouse = 1;
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = 0;
        return;
    }
    
    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;
    
    lastX = xpos;
    lastY = ypos;
    
    xoffset *= app->mouseSensitivity;
    yoffset *= app->mouseSensitivity;
    
    app->camera.yaw -= (float)xoffset;
    app->camera.pitch += (float)yoffset;
    
    // ograniczenie żeby nie można było obrócić się do góry nogami
    if (app->camera.pitch > 89.0f * (float)M_PI / 180.0f)
        app->camera.pitch = 89.0f * (float)M_PI / 180.0f;
    if (app->camera.pitch < -89.0f * (float)M_PI / 180.0f)
        app->camera.pitch = -89.0f * (float)M_PI / 180.0f;
}

// Ustawienie początkowych wartości - kamera, prędkość, pozycje brył
void initAppState(AppState* app) {
    app->camera.position[0] = 0.0f;
    app->camera.position[1] = 0.0f;
    app->camera.position[2] = 8.0f;
    app->camera.yaw = 0.0f; //lewo/prawo
    app->camera.pitch = 0.0f; //góra/dół
    
    app->fov = 60.0f;
    app->moveSpeed = 5.0f;
    app->mouseSensitivity = 0.001f;
    
    app->keyW = app->keyS = app->keyA = app->keyD = 0;
    // randomizuje pozycje brył
    srand((unsigned int)time(NULL));
    app->numObjects = 15;
    for (int i = 0; i < app->numObjects; i++) {
        app->objectPositions[i][0] = ((float)rand() / RAND_MAX) * 20.0f - 10.0f;
        app->objectPositions[i][1] = ((float)rand() / RAND_MAX) * 10.0f - 5.0f;
        app->objectPositions[i][2] = ((float)rand() / RAND_MAX) * 20.0f - 10.0f;
    }
}

// Główna funkcja - inicjalizuje okno, shadery i uruchamia pętlę renderowania
int main(void)
{
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
    
    AppState app;
    initAppState(&app);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1024, 768, "Kamera pierwszoosobowa (FPS)", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowUserPointer(window, &app);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(window);
    gladLoadGL(); // inicjalizacja OpenGL
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST); // włączenie testu głębi - obiekty bliżej zasłaniają dalsze 
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);

    // tworzenie bufora z danymi wierzchołków bryły (pozycje i kolory)
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // tworzenie shaderów
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    // połączenie shaderów w jeden program renderujący
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)(sizeof(float) * 3));

    // Główna pętla renderowania - rysuje obraz 60 razy na sekundę
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window))
    {
        // obliczanie czasu między klatkami do płynnego ruchu
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // czyszczenie ekranu

        // Poruszanie kamerą na podstawie wciśniętych klawiszy
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

        // Obliczanie macierzy rzutowania (perspektywa) - przekształca 3D na 2D ekran
        mat4x4 P;
        float fov_rad = app.fov * (float)M_PI / 180.0f;
        float near = 0.1f;
        float far = 100.0f;
        mat4x4_perspective(P, fov_rad, ratio, near, far);

        // Obliczanie macierzy widoku - uwzględnia pozycję i kierunek kamery
        mat4x4 V;
        calculateViewMatrix(V, &app.camera);

        // Rysowanie wszystkich brył - dla każdej obliczamy jak wygląda z perspektywy kamery
        glUseProgram(program);
        
        for (int i = 0; i < app.numObjects; i++) {
            mat4x4 M, MVP;
            
            // przesunięcie bryły do jej pozycji w świecie 3D
            mat4x4_identity(M);
            mat4x4_translate_in_place(M, app.objectPositions[i][0],
                                          app.objectPositions[i][1],
                                          app.objectPositions[i][2]);
            
            // MVP = macierz która przekształca wierzchołki 3D na pozycje na ekranie 2D
            // (Model * View * Projection)
            mat4x4_mul(MVP, V, M);
            mat4x4_mul(MVP, P, MVP);
            
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)MVP);
            glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(vertices[0])); // rysowanie bryły
        }

        glfwSwapBuffers(window); // wyświetlenie narysowanej klatki
        glfwPollEvents(); // sprawdzenie zdarzeń (klawisze, mysz)
    }

    glDeleteBuffers(1, &vertex_buffer);
    glDeleteProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
