#version 150 
// msResolve.fs
// 

in vec2 vTexCoord;

uniform sampler2DMS origImage;
uniform int sampleCount;
out vec4 oColor;

void main(void) 
{ 
    // Calculate un-normalized texture coordinates
	vec2 tmp = floor(textureSize2DMS(origImage) * vTexCoord); 

    // Find both the weighted and unweighted colors
	vec4 vColor = vec4(0.0, 0.0, 0.0, 0.0);
	
	// First, get sample data
	for (int i = 0; i < sampleCount; i++)
	{
	    vColor += texelFetch(origImage, ivec2(tmp), i);
	}

    oColor = vColor / (sampleCount); 
    oColor.a = 1.0f;
}
