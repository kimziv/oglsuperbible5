// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 130

out vec4 gl_FragColor;

uniform vec4      ambientColor;
uniform vec4      diffuseColor;   
uniform vec4      specularColor;
uniform sampler2D colorMap;

in vec3 vVaryingNormal;
in vec3 vVaryingLightDir;
in vec2 vTexCoords;

void main(void)
    { 
    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(normalize(vVaryingNormal), normalize(vVaryingLightDir)));

    // Multiply intensity by diffuse color, force alpha to 1.0
    gl_FragColor = diff * diffuseColor;

    // Add in ambient light
    gl_FragColor += ambientColor;

    // Modulate in the texture
    gl_FragColor *= texture2D(colorMap, vTexCoords);

    // Specular Light
    vec3 vReflection = normalize(reflect(-normalize(vVaryingLightDir), normalize(vVaryingNormal)));
    float spec = max(0.0, dot(normalize(vVaryingNormal), vReflection));
    if(diff != 0) {
        float fSpec = pow(spec, 128.0);
        gl_FragColor.rgb += vec3(fSpec, fSpec, fSpec);
        }
    }
    