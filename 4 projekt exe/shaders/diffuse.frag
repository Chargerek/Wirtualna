#version 110

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

varying vec3 fragPos;
varying vec3 normal;
varying vec3 color;

void main()
{
    // Normalizacja normalnej
    vec3 N = normalize(normal);
    
    // Wektor kierunku światła
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Obliczenie światła rozproszonego (diffuse)
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Kolor końcowy
    vec3 result = (diffuse + vec3(0.1, 0.1, 0.1)) * color; // Dodajemy ambient
    
    gl_FragColor = vec4(result, 1.0);
}


