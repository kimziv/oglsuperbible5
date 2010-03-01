#version 150 
// flatBlend.fs
// 

in vec2 vTexCoord;

out vec4 oColor;

void main(void) 
{ 
    const int sampleCount = 8;
    
    // Calculate un-normalized texture coordinates
	vec2 tmp = floor(textureSize2DMS(origImage) * vTexCoord); 

    // Find both the weighted and unweighted colors
	vec4 vColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	for (int i = 0; i <= 8; i++)
	{
	    vColor += texelFetch(origImage, ivec2(tmp), i);
	}

    oColor = vColor / (sampleCount+1);    
    oColor.a = 1.0f;
}