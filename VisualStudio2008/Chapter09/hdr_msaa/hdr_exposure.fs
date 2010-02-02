#version 150 
// hdr_exposure.fs
// Scale floating point texture to 0.0 - 1.0 based 
// on the specified exposure
// Resolve multisample buffer based on input sample count
// 

in vec2 vTexCoord;

uniform sampler2DMS origImage;
uniform samplerBuffer sampleWeightSampler; 
uniform int sampleCount; // 0-based, 0=1sample, 1=2samples, etc
uniform int useWeightedResolve; // 0-false, 1-true
uniform float exposure;

out vec4 oColor;

// do all tone mapping in a separate function
vec4 toneMap(vec4 vHdrColor)
{
	vec4 vLdrColor;
	vLdrColor = 1.0 - exp2(-vHdrColor * exposure);
	vLdrColor.a = 1.0f;
	return vLdrColor;
}

void main(void) 
{ 
    // Calculate un-normalized texture coordinates
	vec2 tmp = floor(textureSize2DMS(origImage) * vTexCoord); 

    // Find both the weighted and unweighted colors
	vec4 vColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 vWeightedColor = vec4(0.0, 0.0, 0.0, 1.0);
	
	for (int i = 0; i <= sampleCount; i++)
	{
	    // Get the weight for this sample from the texBo, this changes
	    // based on the number of samples
	    float weight = texelFetch(sampleWeightSampler, i).r;
	    
	    // tone-map the HDR texel before it is weighted
	    vec4 sample = toneMap(texelFetch(origImage, ivec2(tmp), i));
	    
		vWeightedColor += sample * weight;
		vColor += sample;
	}

    // now, decide on the type of resolve to perform
    oColor = vWeightedColor;
	
	// if the user selected the unweighed resolve, output the 
	// equally weighted value
	if (useWeightedResolve != 0)
	{
	    oColor = vColor / (sampleCount+1);    
	}

    oColor.a = 1.0f;
    
}