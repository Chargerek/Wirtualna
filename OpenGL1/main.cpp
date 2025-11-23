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

// ============================================================================
// SEKCJA 1: DEFINICJE WIERZCHOŁKÓW BRYŁY
// ============================================================================
// Definicja wierzchołków graniastosłupa trójkątnego (2 podstawy + 3 ściany boczne)
// Każdy wierzchołek: x, y, z, r, g, b
static const struct {
    float x, y, z;
    float r, g, b;
} vertices[] = {
    // Dolna podstawa (trójkąt) - czerwony
    { 0.0f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f },
    { 0.433f, 0.25f, -0.5f, 1.0f, 0.0f, 0.0f },
    { -0.433f, 0.25f, -0.5f, 1.0f, 0.0f, 0.0f },

    // Górna podstawa (trójkąt) - zielony
    { 0.0f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f },
    { 0.433f, 0.25f, 0.5f, 0.0f, 1.0f, 0.0f },
    { -0.433f, 0.25f, 0.5f, 0.0f, 1.0f, 0.0f },

    // Ściana 1 (A-B-B'-A') - niebieski
    { 0.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f },
    { 0.433f, 0.25f, -0.5f, 0.0f, 0.0f, 1.0f },
    { 0.433f, 0.25f, 0.5f, 0.0f, 0.0f, 1.0f },
    { 0.0f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f },
    { 0.433f, 0.25f, 0.5f, 0.0f, 0.0f, 1.0f },
    { 0.0f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f },

    // Ściana 2 (B-C-C'-B') - żółty
    { 0.433f, 0.25f, -0.5f, 1.0f, 1.0f, 0.0f },
    { -0.433f, 0.25f, -0.5f, 1.0f, 1.0f, 0.0f },
    { -0.433f, 0.25f, 0.5f, 1.0f, 1.0f, 0.0f },
    { 0.433f, 0.25f, -0.5f, 1.0f, 1.0f, 0.0f },
    { -0.433f, 0.25f, 0.5f, 1.0f, 1.0f, 0.0f },
    { 0.433f, 0.25f, 0.5f, 1.0f, 1.0f, 0.0f },

    // Ściana 3 (C-A-A'-C') - fioletowy
    { -0.433f, 0.25f, -0.5f, 0.5f, 0.0f, 0.5f },
    { 0.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.5f },
    { 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.5f },
    { -0.433f, 0.25f, -0.5f, 0.5f, 0.0f, 0.5f },
    { 0.0f, -0.5f, 0.5f, 0.5f, 0.0f, 0.5f },
    { -0.433f, 0.25f, 0.5f, 0.5f, 0.0f, 0.5f },
};

// ============================================================================
// SEKCJA 2: SHADERY
// ============================================================================
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

// ============================================================================
// SEKCJA 3: STRUKTURA KAMERY I STAN APLIKACJI
// ============================================================================
// Struktura przechowująca stan kamery wirtualnej
typedef struct {
    vec3 position;      // Pozycja kamery w przestrzeni światowej
    float yaw;          // Kąt obrotu wokół osi Y (rozglądanie się na boki)
    float pitch;        // Kąt pochylenia (góra/dół) - ograniczony do [-89, 89] stopni
} Camera;

// Stan aplikacji
typedef struct {
    Camera camera;
    float fov;          // Kąt pola widzenia w stopniach (10-120)
    float moveSpeed;    // Prędkość poruszania się kamery
    float mouseSensitivity; // Czułość myszy
    
    // Flagi stanu klawiszy (ruch trwa do puszczenia klawisza)
    int keyW, keyS, keyA, keyD;
    
    // Pozycje brył w przestrzeni 3D
    vec3 objectPositions[15];
    int numObjects;
} AppState;

// ============================================================================
// SEKCJA 4: FUNKCJE POMOCNICZE KAMERY
// ============================================================================
// Oblicza macierz światową kamery Wc na podstawie pozycji i kątów
// Następnie oblicza macierz widoku V = Wc^-1
// Macierz Wc reprezentuje transformację z układu kamery do układu świata
// W OpenGL mnożenie macierzy jest od prawej do lewej, więc:
// Wc = T * Ry * Rx oznacza: najpierw Rx, potem Ry, potem T
void calculateViewMatrix(mat4x4 V, const Camera* camera) {
    mat4x4 Wc;  // Macierz światowa kamery
    mat4x4 T, Ry, Rx;  // Macierze transformacji: przesunięcie, obrót Y, obrót X
    
    // Krok 1: Utwórz macierz przesunięcia do pozycji kamery
    mat4x4_translate(T, camera->position[0], camera->position[1], camera->position[2]);
    
    // Krok 2: Utwórz macierz obrotu wokół osi Y (yaw) - rozglądanie się na boki
    mat4x4_identity(Ry);
    mat4x4_rotate_Y(Ry, Ry, camera->yaw);
    
    // Krok 3: Utwórz macierz obrotu wokół osi X (pitch) - pochylanie się góra/dół
    mat4x4_identity(Rx);
    mat4x4_rotate_X(Rx, Rx, camera->pitch);
    
    // Krok 4: Składamy macierze: Wc = T * Ry * Rx
    // (w OpenGL mnożenie jest od prawej do lewej, więc najpierw Rx, potem Ry, potem T)
    mat4x4_mul(Wc, Ry, Rx);  // Wc = Ry * Rx
    mat4x4_mul(Wc, T, Wc);   // Wc = T * (Ry * Rx) = T * Ry * Rx
    
    // Krok 5: Oblicz macierz widoku jako odwrotność macierzy światowej kamery
    // V = Wc^-1
    // Macierz widoku transformuje punkty ze świata do układu kamery
    mat4x4_invert(V, Wc);
}

// Oblicza wektor kierunku patrzenia kamery (forward vector)
void getCameraForward(vec3 forward, const Camera* camera) {
    forward[0] = -sinf(camera->yaw) * cosf(camera->pitch);
    forward[1] = sinf(camera->pitch);
    forward[2] = -cosf(camera->yaw) * cosf(camera->pitch);
}

// Oblicza wektor prawej strony kamery (right vector)
void getCameraRight(vec3 right, const Camera* camera) {
    right[0] = cosf(camera->yaw);
    right[1] = 0.0f;
    right[2] = -sinf(camera->yaw);
}

// ============================================================================
// SEKCJA 5: OBSŁUGA WEJŚCIA (KLAWISZE I MYSZ)
// ============================================================================
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

// Callback dla klawiszy - obsługuje W, S, A, D, PLUS, MINUS, ESCAPE
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    AppState* app = (AppState*)glfwGetWindowUserPointer(window);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    // Obsługa klawiszy ruchu (stan trwa do puszczenia klawisza)
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
    
    // Obsługa zmiany FOV (PLUS i MINUS)
    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) { // PLUS
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            app->fov += 5.0f;
            if (app->fov > 120.0f) app->fov = 120.0f;
        }
    }
    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) { // MINUS
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            app->fov -= 5.0f;
            if (app->fov < 10.0f) app->fov = 10.0f;
        }
    }
}

// Callback dla ruchu myszy - obsługuje obrót kamery
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
    
    // Oblicz różnicę pozycji myszy
    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos; // Odwrócona oś Y (inverted Y axis)
    
    lastX = xpos;
    lastY = ypos;
    
    // Zastosuj czułość myszy
    xoffset *= app->mouseSensitivity;
    yoffset *= app->mouseSensitivity;
    
    // Zaktualizuj kąty kamery
    // Zmieniamy znak xoffset, żeby ruch myszy w prawo obracał kamerę w prawo
    app->camera.yaw -= (float)xoffset;
    app->camera.pitch += (float)yoffset;
    
    // Ogranicz pitch do zakresu [-89, 89] stopni (żeby nie można było obrócić się do góry nogami)
    if (app->camera.pitch > 89.0f * (float)M_PI / 180.0f)
        app->camera.pitch = 89.0f * (float)M_PI / 180.0f;
    if (app->camera.pitch < -89.0f * (float)M_PI / 180.0f)
        app->camera.pitch = -89.0f * (float)M_PI / 180.0f;
}

// ============================================================================
// SEKCJA 6: INICJALIZACJA APLIKACJI
// ============================================================================
// Inicjalizuje stan aplikacji z losowymi pozycjami brył
void initAppState(AppState* app) {
    // Inicjalizacja kamery - startujemy przed sceną, patrzymy w kierunku -Z
    app->camera.position[0] = 0.0f;
    app->camera.position[1] = 0.0f;
    app->camera.position[2] = 8.0f; // Startujemy 8 jednostek przed sceną (bryły są w zakresie -10 do 10)
    app->camera.yaw = 0.0f;          // Patrzymy w kierunku -Z (w głąb sceny, gdzie są bryły)
    app->camera.pitch = 0.0f;       // Poziomo
    
    // Inicjalizacja parametrów
    app->fov = 60.0f;                 // Domyślny FOV 60 stopni
    app->moveSpeed = 5.0f;            // Prędkość ruchu
    app->mouseSensitivity = 0.001f;    // Czułość myszy
    
    // Inicjalizacja flag klawiszy
    app->keyW = app->keyS = app->keyA = app->keyD = 0;
    
    // Generuj losowe pozycje dla brył
    srand((unsigned int)time(NULL));
    app->numObjects = 15;
    for (int i = 0; i < app->numObjects; i++) {
        app->objectPositions[i][0] = ((float)rand() / RAND_MAX) * 20.0f - 10.0f; // X: -10 do 10
        app->objectPositions[i][1] = ((float)rand() / RAND_MAX) * 10.0f - 5.0f;  // Y: -5 do 5
        app->objectPositions[i][2] = ((float)rand() / RAND_MAX) * 20.0f - 10.0f; // Z: -10 do 10
    }
}

// ============================================================================
// SEKCJA 7: GŁÓWNA FUNKCJA
// ============================================================================
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

    // Ustaw wskaźnik do struktury aplikacji
    glfwSetWindowUserPointer(window, &app);
    
    // Ustaw callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    
    // Ukryj kursor i przechwytuj go (tryb FPS)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    // Włącz test głębi (depth testing)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Wyłącz backface culling - żeby wszystkie ściany były widoczne
    glDisable(GL_CULL_FACE);

    // Utwórz bufor wierzchołków
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Utwórz shader wierzchołków
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    // Utwórz shader fragmentów
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    // Utwórz program i połącz shadery
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // Pobierz lokalizacje uniformów i atrybutów
    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    // Włącz atrybuty wierzchołków
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)(sizeof(float) * 3));

    // ============================================================================
    // SEKCJA 8: GŁÓWNA PĘTLA RENDEROWANIA
    // ============================================================================
    double lastTime = glfwGetTime();
    
    while (!glfwWindowShouldClose(window))
    {
        // Oblicz czas delta (do płynnego ruchu niezależnego od FPS)
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;
        
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ========================================================================
        // SEKCJA 8.1: AKTUALIZACJA POZYCJI KAMERY NA PODSTAWIE WEJŚCIA
        // ========================================================================
        vec3 forward, right;
        getCameraForward(forward, &app.camera);
        getCameraRight(right, &app.camera);
        
        // Ruch do przodu (W) i do tyłu (S)
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
        
        // Ruch w lewo (A) i w prawo (D)
        if (app.keyA) {
            app.camera.position[0] -= right[0] * app.moveSpeed * deltaTime;
            app.camera.position[2] -= right[2] * app.moveSpeed * deltaTime;
        }
        if (app.keyD) {
            app.camera.position[0] += right[0] * app.moveSpeed * deltaTime;
            app.camera.position[2] += right[2] * app.moveSpeed * deltaTime;
        }

        // ========================================================================
        // SEKCJA 8.2: OBLICZENIE MACIERZY RZUTOWANIA PERSPEKTYWICZNEGO
        // ========================================================================
        mat4x4 P;
        float fov_rad = app.fov * (float)M_PI / 180.0f; // Konwersja stopni na radiany
        float near = 0.1f;
        float far = 100.0f;
        mat4x4_perspective(P, fov_rad, ratio, near, far);

        // ========================================================================
        // SEKCJA 8.3: OBLICZENIE MACIERZY WIDOKU V = Wc^-1
        // ========================================================================
        mat4x4 V;
        calculateViewMatrix(V, &app.camera);

        // ========================================================================
        // SEKCJA 8.4: RENDEROWANIE WSZYSTKICH BRYŁ
        // ========================================================================
        glUseProgram(program);
        
        // Dla każdej bryły oblicz macierz MVP i narysuj ją
        for (int i = 0; i < app.numObjects; i++) {
            mat4x4 M, MVP;
            
            // Macierz modelu: przesunięcie do pozycji bryły
            mat4x4_identity(M);
            mat4x4_translate_in_place(M, app.objectPositions[i][0],
                                          app.objectPositions[i][1],
                                          app.objectPositions[i][2]);
            
            // Oblicz MVP = P * V * M
            mat4x4_mul(MVP, V, M);
            mat4x4_mul(MVP, P, MVP);
            
            // Ustaw uniform i narysuj bryłę
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)MVP);
            glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(vertices[0]));
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ============================================================================
    // SEKCJA 9: CZYSZCZENIE ZASOBÓW
    // ============================================================================
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
