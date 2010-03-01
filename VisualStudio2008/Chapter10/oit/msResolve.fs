#version 150 
// msResolve.fs
// 

in vec2 vTexCoord;

uniform sampler2DMS origImage;
uniform sampler2DMS origDepth;

out vec4 oColor;

void main(void) 
{ 
    const int sampleCount = 4;
    
    // Calculate un-normalized texture coordinates
	vec2 tmp = floor(textureSize2DMS(origImage) * vTexCoord); 

    // Find both the weighted and unweighted colors
	vec4 vColor[8];
	float vDepth[8];
	int   vSurfOrder[8];
	int i = 0;
	
	// First, get sample data
	for (i = 0; i < 8; i++)
	{
	    vSurfOrder[i] = i;
	    if (i < sampleCount)
	    {
	        vColor[i] = texelFetch(origImage, ivec2(tmp), i);
	        vDepth[i] = texelFetch(origDepth, ivec2(tmp), i).r;
	    }
	    else
	    {
	        //vColor[i] = vec4(0.0, 0.0, 0.0, 1.0);
	        vDepth[i] = 1.0;
	    }
	}
    float val = vDepth[1];
    oColor = vec4(val, val, val, 1.0);	
	for (i = 0; i < sampleCount; i++)
	{
	    oColor += vColor[i];
	}
    oColor = oColor / (sampleCount); 
    //oColor = vColor[1];
    oColor.a = 1.0f;
}