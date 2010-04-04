// ADS Point lighting Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 120

uniform vec4      ambientColor;
uniform vec4      diffuseColor;   

uniform sampler2D colorMap;
uniform sampler2D normalMap;

varying vec3 vVaryingLightDir;
varying vec2 vTexCoords;
varying vec3 vTrueLightDir;

void main(void)
    { 
    vec3 vTextureNormal = texture2D(normalMap, vTexCoords).xyz;
    vTextureNormal = (vTextureNormal - 0.5) * 2.0;
    
    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(normalize(vTextureNormal), normalize(vVaryingLightDir)));

    // Multiply intensity by diffuse color, force alpha to 1.0
    gl_FragColor = diff * diffuseColor;

    // Add in ambient light
    gl_FragColor += ambientColor;
    gl_FragColor.rgb = min(vec3(1.0,1.0,1.0), gl_FragColor.rgb);

    // Modulate in the texture
    gl_FragColor *= texture2D(colorMap, vTexCoords);
    }
    