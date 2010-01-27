// hdr.fs
// 
// 
#version 150 
varying vec2 vTex;
uniform sampler2D textureUnit0;
void main(void) 
{ 
	// fetch from HDR texture
	vec4 hdrTexel = texture2D(textureUnit0, vTex); 
	
	gl_FragColor = hdrTexel;
}
