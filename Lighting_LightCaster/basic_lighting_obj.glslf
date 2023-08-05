#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};
struct Light {
    vec3 position;
    vec3 direction;
    float cutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

in vec3 Normal;  
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;

void main()
{
    vec3 lightDir = normalize(light.position - FragPos);
    float theta = dot(lightDir, normalize(-light.direction));

    // flashlight
    if (theta > light.cutOff) // within cosine
    {
        // ambient
        vec3 ambient = vec3(texture(material.diffuse, TexCoords)) * light.ambient;
        
        // diffuse 
        vec3 norm = normalize(Normal);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(texture(material.diffuse, TexCoords)) * light.diffuse;
        
        // specular
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = spec * light.specular * vec3(texture(material.specular, TexCoords));  
            
        // attenuation
        float distance = length(light.position - FragPos);
        float attenuation = 1.0 / 
        (light.constant + light.linear * distance + light.quadratic * (distance * distance));
        
        // ambient  *= attenuation; 
        diffuse  *= attenuation;
        specular *= attenuation;

        FragColor = vec4(ambient + diffuse + specular, 1.0);
    }
    else
    {
        FragColor = vec4(light.ambient * texture(material.diffuse, TexCoords).rgb, 1.0);
    }

} 