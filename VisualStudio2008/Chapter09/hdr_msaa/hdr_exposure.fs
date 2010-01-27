#version 150 
// hdr_exposure.fs
// Scale floating point texture to 0.0 - 1.0 based 
// on the specified exposure
// 

varying vec2 vTex;
uniform sampler2DMS origImage;
uniform sampler2DMS brightImage;
uniform sampler2DMS blur1;
uniform sampler2DMS blur2;
uniform sampler2DMS blur3;
uniform sampler2DMS blur4;

uniform float exposure;
uniform float bloomLevel;

// TODO remove these
uniform ivec2 origImageSize;
uniform ivec2 brightImageSize;
uniform ivec2 blur1Size;
uniform ivec2 blur2Size;
uniform ivec2 blur3Size;
uniform ivec2 blur4Size;

out vec4 oColor;
out vec4 oBright;

void main(void) 
{ 
	vec2 tmp; 
//	ivec2 iTmp = textureSize2DMS(origImage);
	ivec2 iTmp = origImageSize;
	tmp = floor(iTmp * vTex); 
    // fetch from HDR & blur textures
	vec4 vBaseImage  = texelFetch2DMS(origImage, ivec2(tmp), 0); 
	
	//iTmp = textureSize2DMS(brightImage);
	iTmp = brightImageSize;
	tmp = floor(iTmp * vTex); 
	vec4 vBrightPass = texelFetch(brightImage, ivec2(tmp), 0); 
	
	//iTmp = textureSize2DMS(blur1);
	iTmp = blur1Size;
	tmp = floor(iTmp * vTex); 
	vec4 vBlurColor1 = texelFetch(blur1, ivec2(tmp), 0); 
	
	//iTmp = textureSize2DMS(blur2);
	iTmp = blur2Size;
	tmp = floor(iTmp * vTex); 
	vec4 vBlurColor2 = texelFetch(blur2, ivec2(tmp), 0); 
	
	//iTmp = textureSize2DMS(blur3);
	iTmp = blur3Size;
	tmp = floor(iTmp * vTex); 
	vec4 vBlurColor3 = texelFetch(blur3, ivec2(tmp), 0); 
	
	//iTmp = textureSize2DMS(blur4);
	iTmp = blur4Size;
	tmp = floor(iTmp * vTex); 
	vec4 vBlurColor4 = texelFetch(blur4, ivec2(tmp), 0); 
	
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXx
	// Tone map each of these first.
	
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