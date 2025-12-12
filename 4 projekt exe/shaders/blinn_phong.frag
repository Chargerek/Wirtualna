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
    
    // Ambient component
    vec3 ambient = vec3(0.1, 0.1, 0.1) * color;
    
    // Diffuse component
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * color;
    
    // Specular component (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(N, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;
    
    // 3-komponentowy model Blinna-Phonga
    vec3 result = ambient + diffuse + specular;
    
    gl_FragColor = vec4(result, 1.0);
}


