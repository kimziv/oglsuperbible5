// tex_replace.fs
// outputs 1 color using texture replace
// 
#version 150 

varying vec2 vTexCoord; 
uniform sampler2D textureUnit0; 
uniform vec4 vColor;
out vec4 oColor;
out vec4 oBright;

void main(void) 
{
	const float bloomLimit = 1.0;
 
    oColor =  vColor*texture2D(textureUnit0, vTexCoord);
    oColor.a = 1.0;
    
    vec3 brightColor = max(vColor.rgb - vec3(bloomLimit), vec3(0.0));
    float bright = dot(brightColor, vec3(1.0));
    bright = smoothstep(0.0, 0.5, bright);
    oBright.rgb = mix(vec3(0.0), vColor.rgb, bright).rgb;
    oBright.a = 1.0;
}