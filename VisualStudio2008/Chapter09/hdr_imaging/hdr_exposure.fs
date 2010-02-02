#version 150 
// hdr_exposure.fs
// Scale floating point texture to 0.0 - 1.0 based 
// on the specified exposure
// 

in vec2 vTexCoord;

uniform sampler2D textureUnit0;
uniform float exposure;

out vec4 oColor;

void main(void) 
{ 
	// fetch from HDR texture
	vec4 vColor = texture(textureUnit0, vTexCoord); 
	
	// Apply the exposure to this texel
	oColor = 1.0 - exp2(-vColor * exposure);
    oColor.a = 1.0f;
}