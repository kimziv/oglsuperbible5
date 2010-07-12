//#version 150 
// oitResolve.fs
// 

in vec2 vTexCoord;

uniform sampler2DMS origImage;
uniform sampler2DMS origDepth;

out vec4 oColor;

void main(void) 
{ 
    const int sampleCount = 8;
    	
    vec4  vColor[sampleCount];
	float vDepth[sampleCount];
	int   vSurfOrder[sampleCount];
	int   i = 0;
	
    // Calculate un-normalized texture coordinates
	vec2 tmp = floor(textureSize2DMS(origDepth) * vTexCoord); 
		
    // First, get sample data and init the surface order
	for (i = 0; i < sampleCount; i++)
	{
		vSurfOrder[i] = i;
        vColor[i] = texelFetch(origImage, ivec2(tmp), i);
	    vDepth[i] = texelFetch(origDepth, ivec2(tmp), i).r;
	}
	
	// Sort depth values, largest to front and smallest to back
	// Must run through array (size^2-size) times, or early-exit
	// if any pass shows all samples to be in order
	for (int j = 0; j < sampleCount; j++)
    {
		bool bFinished = true;
        for (i = 0; i < (sampleCount-1); i++)
	    {
	        float temp1 = vDepth[vSurfOrder[i]];
	        float temp2 = vDepth[vSurfOrder[i+1]];
    	    
	        if (temp2 < temp1)
	        {
	            // swap values
	            int tempIndex   = vSurfOrder[i];
	            vSurfOrder[i]   = vSurfOrder[i+1];
	            vSurfOrder[i+1] = tempIndex;
	            bFinished = false;
	        }
	    }
	    
	    if (bFinished)
            j = 8;
	}
	
	// Now, sum all colors in order from front to back. Apply alpha.
	bool bFoundFirstColor = false;
	vec4 summedColor = vec4(0.0, 0.0, 0.0, 0.0);
	for (i = (sampleCount-1); i >= 0; i--)
    {
		int surfIndex = vSurfOrder[i];
		if(vColor[surfIndex].a > 0.001)
		{
			if (bFoundFirstColor == false)
			{
				// apply 100% of the first color
				summedColor = vColor[surfIndex];
				bFoundFirstColor = true;
			}
			else
			{
				// apply color with alpha
				summedColor.rgb = (summedColor.rgb * (1 - vColor[surfIndex].a))     +
				                  (vColor[surfIndex].rgb * vColor[surfIndex].a);
			}
		}
    }
   
   oColor = summedColor;
   
    int surfIndex = 2;
    float val = vDepth[vSurfOrder[surfIndex]];
    //oColor = vec4(val, val, val, 1.0);
    //oColor = vec4(vColor[vSurfOrder[surfIndex]].rgb, 1.0);
    //oColor = vec4(vColor[0].rgb, 1.0);
    oColor.a = 1.0f;
}