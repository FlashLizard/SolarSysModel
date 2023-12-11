#version 330 core    
in vec2 TexCoord;
in vec3 norm;
in vec3 FragPos;

out vec4 FragColor;

uniform sampler2D ourTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform int background;
uniform bool sun;

void main() {
    //FragColor = vColor;
    if(background==1) {
        FragColor = 0.4 * texture2D(ourTexture, TexCoord);
    } else {
        vec3 objectColor = vec3(texture2D(ourTexture, TexCoord));

    // ambient
        float ambientStrength = sun ? 1.0 : 0.3;
        vec3 ambient = ambientStrength * lightColor;

    // diffuse 
        vec3 norm = normalize(norm);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = 1.0 * diff * lightColor;

    // specular
        float specularStrength = 3;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2);
        vec3 specular = specularStrength * spec * lightColor;

        vec3 result = (ambient + diffuse + specular) * objectColor;
        FragColor = vec4(result, 1.0);
    }
}