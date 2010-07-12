#version 150 
// blur.fs
// outputs 1 color using a gaussian blur of the input texture
// 

in vec4 vFragColor; 
in vec2 vTexCoord; 

uniform sampler2D textureUnit0;
uniform vec2 tc_offset[25];
 
out vec4 oColor;

void main(void) 
{ 
	vec4 sample[25];
    for (int i = 0; i < 25; i++)
    {
        sample[i] = texture(textureUnit0, vTexCoord.st + tc_offset[i]);
    }

//   1  4  7  4 1
//   4 16 26 16 4
//   7 26 41 26 7 / 273
//   4 16 26 16 4
//   1  4  7  4 1

    oColor = (
                   (1.0  * (sample[0] + sample[4] + sample[20] + sample[24])) +
                   (4.0  * (sample[1] + sample[3] + sample[5] + sample[9] +
                            sample[15] + sample[19] + sample[21] + sample[23])) +
                   (7.0  * (sample[2] + sample[10] + sample[14] + sample[22])) +
                   (16.0 * (sample[6] + sample[8] + sample[16] + sample[18])) +
                   (26.0 * (sample[7] + sample[11] + sample[13] + sample[17])) +
                   (41.0 * sample[12])
                   ) / 273.0;

}