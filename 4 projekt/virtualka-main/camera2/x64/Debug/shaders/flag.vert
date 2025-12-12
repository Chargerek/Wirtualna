#version 110

uniform mat4 MVP;
uniform mat4 M;
uniform float time;

attribute vec3 vPos;
attribute vec3 vNormal;
attribute vec2 vTexCoord;

varying vec3 fragPos;
varying vec3 normal;
varying vec2 texCoord;

void main()
{
    // Efekt falowania flagi
    vec3 pos = vPos;
    float wave = sin(pos.x * 3.0 + time * 2.0) * 0.1;
    pos.y += wave;
    
    fragPos = vec3(M * vec4(pos, 1.0));
    // Transformacja normalnej przez macierz 4x4 (GLSL 110 nie wspiera mat3(M))
    normal = normalize(vec3(M * vec4(vNormal, 0.0)));
    texCoord = vTexCoord;
    gl_Position = MVP * vec4(pos, 1.0);
}

