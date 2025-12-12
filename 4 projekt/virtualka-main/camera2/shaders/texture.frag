#version 110

uniform sampler2D textureSampler;

varying vec2 texCoord;

void main()
{
    // Teksturowanie bez oświetlenia
    gl_FragColor = texture2D(textureSampler, texCoord);
}


