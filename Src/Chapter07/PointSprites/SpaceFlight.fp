// SpaceFlight Shader
// Fragment Shader
// Richard S. Wright Jr.
// OpenGL SuperBible
#version 120

//out vec4 vFragColor;

varying vec4 vStarColor;

uniform sampler2D  starImage;

void main(void)
    { 
    gl_FragColor = texture2D(starImage, gl_PointCoord) * vStarColor;
 //   gl_FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    