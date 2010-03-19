// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 150

out vec4 gl_FragColor;

uniform vec4    ambientColor;
uniform vec4   diffuseColor;   
uniform vec4    specularColor;

in vec3 vVaryingNormal;
in vec3 vVaryingLightDir;


void main(void)
    { 
    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));

    // Multiply intensity by diffuse color, force alpha to 1.0
    gl_FragColor = diff * diffuseColor;

    // Add in ambient light
    gl_FragColor += ambientColor;


    // Specular Light
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if(diff != 0) {
        float fSpec = pow(spec, 128.0);
        gl_FragColor.rgb += vec3(fSpec, fSpec, fSpec);
        }
    }
    