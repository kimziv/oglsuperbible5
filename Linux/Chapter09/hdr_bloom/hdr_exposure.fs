#version 150 
// hdr_exposure.fs
// Scale floating point texture to 0.0 - 1.0 based 
// on the specified exposure
// 

in vec2 vTexCoord;

uniform sampler2D origImage;
uniform sampler2D brightImage;
uniform sampler2D blur1;
uniform sampler2D blur2;
uniform sampler2D blur3;
uniform sampler2D blur4;

uniform float exposure;
uniform float bloomLevel;

out vec4 oColor;
out vec4 oBright;

void main(void) 
{ 
	// fetch from HDR & blur textures
	vec4 vBaseImage  = texture2D(origImage, vTexCoord); 
	vec4 vBrightPass = texture2D(brightImage, vTexCoord); 
	vec4 vBlurColor1 = texture2D(blur1, vTexCoord); 
	vec4 vBlurColor2 = texture2D(blur2, vTexCoord); 
	vec4 vBlurColor3 = texture2D(blur3, vTexCoord); 
	vec4 vBlurColor4 = texture2D(blur4, vTexCoord); 
	
	vec4 vBloom = vBrightPass + 
				  vBlurColor1 + 
				  vBlurColor2 + 
				  vBlurColor3 + 
				  vBlurColor4;
	
	vec4 vColor = vBaseImage + bloomLevel * vBloom;
	
	// Apply the exposure to this texel
	vColor = 1.0 - exp2(-vColor * exposure);
	oColor = vColor;
    oColor.a = 1.0f;
    
}