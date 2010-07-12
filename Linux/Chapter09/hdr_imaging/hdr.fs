#version 150 
// hdr.fs
// output texture value directly.
// 

in vec2 vTexCoord;

uniform sampler2D textureUnit0;

void main(void) 
{ 
	// fetch from HDR texture
	vec4 hdrTexel = texture(textureUnit0, vTexCoord); 
	
	gl_FragColor = hdrTexel;
}
