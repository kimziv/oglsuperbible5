// multibuffer_frag_location.fs
// outputs to 3 buffers: normal color, greyscale, 
// and luminance adjusted color
#version 150 

varying vec4 vFragColor; 
varying vec2 vTexCoord; 
uniform sampler2D textureUnit0; 
uniform int bUseTexture;
uniform samplerBuffer lumCurveSampler; 

out vec4 oStraightColor;
out vec4 oGreyscale;
out vec4 oLumAdjColor;

void main(void) { 
    vec4 vColor; 
    vec4 lumFactor; 
    
    if (bUseTexture != 0) 
         vColor =  texture2D(textureUnit0, vTexCoord);
    else 
         vColor = vFragColor;
         
    // Untouched output goes to first buffer
    oStraightColor = vColor;

     // Black and white to second buffer
    float grey = dot(vColor.rgb, vec3(0.3, 0.59, 0.11));
    oGreyscale = vec4(grey, grey, grey, 1.0f);
    
    // clamp input color to make sure it is between 0.0 and 1.0
    vColor = clamp(vColor, 0.0f, 1.0f);
     
    int offset = int(vColor.r * 1024); 
    oLumAdjColor.r = texelFetchBuffer(lumCurveSampler, offset ).r;
    
    offset = int(vColor.g * 1024); 
    oLumAdjColor.g = texelFetchBuffer(lumCurveSampler, offset ).r;
    
    offset = int(vColor.b * 1024); 
    oLumAdjColor.b = texelFetchBuffer(lumCurveSampler, offset ).r;
    
    oLumAdjColor.a = 1.0f;
}