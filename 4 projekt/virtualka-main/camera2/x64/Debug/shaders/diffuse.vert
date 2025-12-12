#version 110

uniform mat4 MVP;
uniform mat4 M;  // Macierz modelu
uniform mat4 V;  // Macierz widoku
uniform vec3 objectColor;  // Kolor obiektu

attribute vec3 vPos;
attribute vec3 vNormal;
attribute vec3 vCol;

varying vec3 fragPos;
varying vec3 normal;
varying vec3 color;

void main()
{
    fragPos = vec3(M * vec4(vPos, 1.0));
    // Transformacja normalnej przez macierz 4x4 (GLSL 110 nie wspiera mat3(M))
    normal = normalize(vec3(M * vec4(vNormal, 0.0)));
    // Użyj uniformu objectColor (domyślnie będzie ustawiony dla każdego obiektu)
    color = objectColor;
    gl_Position = MVP * vec4(vPos, 1.0);
}


