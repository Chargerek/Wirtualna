#version 110

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

varying vec3 fragPos;
varying vec3 normal;
varying vec3 color;

void main()
{
    // Normalizacja
    vec3 N = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, N);
    
    // Światło rozproszone
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Światło odbite (specular)
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;
    
    // Kolor końcowy
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    vec3 result = (ambient + diffuse + specular) * color;
    
    gl_FragColor = vec4(result, 1.0);
}


