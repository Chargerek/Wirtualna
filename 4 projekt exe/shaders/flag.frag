#version 110

uniform sampler2D textureSampler;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

varying vec3 fragPos;
varying vec3 normal;
varying vec2 texCoord;

void main()
{
    // Normalizacja normalnej
    vec3 N = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 viewDir = normalize(viewPos - fragPos);
    
    // Światło rozproszone
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(N, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;
    
    // Tekstura z cieniowaniem
    vec4 texColor = texture2D(textureSampler, texCoord);
    vec3 ambient = vec3(0.1, 0.1, 0.1);
    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    
    gl_FragColor = vec4(result, texColor.a);
}

