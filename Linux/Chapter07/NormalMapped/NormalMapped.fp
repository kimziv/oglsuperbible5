// Normal mapping light shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 330

uniform vec4      ambientColor;
uniform vec4      diffuseColor;   

uniform sampler2D colorMap;
uniform sampler2D normalMap;

smooth in vec3 vVaryingLightDir;
smooth in vec2 vTexCoords;

out vec4 vFragColor;

void main(void)
    { 
    vec3 vTextureNormal = texture2D(normalMap, vTexCoords).xyz;
    vTextureNormal = (vTextureNormal - 0.5) * 2.0;
    
    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(normalize(vTextureNormal), normalize(vVaryingLightDir)));

    // Multiply intensity by diffuse color, force alpha to 1.0
    vFragColor = diff * diffuseColor;

    // Add in ambient light
    vFragColor += ambientColor;
    vFragColor.rgb = min(vec3(1.0,1.0,1.0), vFragColor.rgb);

    // Modulate in the texture
    vFragColor *= texture(colorMap, vTexCoords);
    }
    