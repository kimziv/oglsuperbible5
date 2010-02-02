#version 150 
// hdr_adaptive.fs
// perform adaptive tone mapping based on relative pixel locations
// 

in vec2 vTexCoord;

uniform sampler2D textureUnit0;
uniform sampler1D textureUnit1;
uniform vec2 tc_offset[25];

out vec4 oColor;

void main(void) 
{ 
    vec4 hdrSample[25];
    for (int i = 0; i < 25; i++)
    {   // Perform 25 lookups around the current texel 
		hdrSample[i] = texture(textureUnit0, vTexCoord.st + tc_offset[i]); 
	}
	
	// Calculate weighted color of region
    vec4 vColor = hdrSample[12];
    vec4 kernelcolor = (
                   (1.0  * (hdrSample[0] + hdrSample[4] + hdrSample[20] + hdrSample[24])) +
                   (4.0  * (hdrSample[1] + hdrSample[3] + hdrSample[5] + hdrSample[9] +
                            hdrSample[15] + hdrSample[19] + hdrSample[21] + hdrSample[23])) +
                   (7.0  * (hdrSample[2] + hdrSample[10] + hdrSample[14] + hdrSample[22])) +
                   (16.0 * (hdrSample[6] + hdrSample[8] + hdrSample[16] + hdrSample[18])) +
                   (26.0 * (hdrSample[7] + hdrSample[11] + hdrSample[13] + hdrSample[17])) +
                   (41.0 * hdrSample[12])
                   ) / 273.0;
                   
    // Calculate luminance for the whole filter kernel
	float kernelLuminance = dot(kernelcolor.rgb, vec3(0.3, 0.59, 0.11));

	// look up the corresponding exposure
	float exposure = texture1D(textureUnit1, kernelLuminance/2.0).r;
	exposure = clamp(exposure, 0.02f, 20.0f);
	
	// Apply the exposure to this texel
	oColor = 1.0 - exp2(-vColor * exposure);
    oColor.a = 1.0f;
}
