#version 110
varying vec3 fragNormal;
varying vec3 fragPosition;

uniform vec4 fillColor;
uniform vec3 lightPosition;
uniform vec3 lightDir;
uniform vec3 viewPosition;
uniform vec4 lightColor;
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;
uniform int lightMode;

void main()
{
    switch(lightMode){
    case 0:
        gl_FragColor = fillColor;
        break;
    case 1:
        {
            float diff = max(dot(normalize(fragNormal), lightDir), 0.0);

            vec3 lighting = (ambientStrength + (1.0 - ambientStrength) * diff) * lightColor.rgb;

            vec3 result = lighting * fillColor.rgb;
            gl_FragColor = vec4(result, fillColor.a);
        }
        break;
    case 2:
        {
            vec3 ambient = ambientStrength * lightColor.rgb;

            vec3 norm = normalize(fragNormal);

            vec3 lightVec = lightPosition - fragPosition;
            float lightDist = length(lightVec);

            if (lightDist < 0.0001) {
                lightDist = 0.0001;
            }

            vec3 lightDir = lightVec / lightDist;
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor.rgb;

            vec3 viewVec = viewPosition - fragPosition;
            float viewDist = length(viewVec);

            if (viewDist < 0.0001) {
                viewDist = 0.0001;
            }

            vec3 viewDir = viewVec / viewDist;
            vec3 reflectDir = reflect(-lightDir, norm);

            float specDot = max(dot(viewDir, reflectDir), 0.0);
            float spec = pow(specDot, shininess);
            vec3 specular = specularStrength * spec * lightColor.rgb;

            vec3 result = (ambient + diffuse + specular) * fillColor.rgb;
            gl_FragColor = vec4(result, fillColor.a);
        }
        break;
    };
}
