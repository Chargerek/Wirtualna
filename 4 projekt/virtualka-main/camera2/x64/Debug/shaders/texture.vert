#version 110

uniform mat4 MVP;

attribute vec3 vPos;
attribute vec2 vTexCoord;

varying vec2 texCoord;

void main()
{
    texCoord = vTexCoord;
    gl_Position = MVP * vec4(vPos, 1.0);
}


