// flocking_render.fs
// OpenGL SuperBible
// Example of using transform feedback to implement flocking
// Program by Graham Sellers.

// Flocking render fragment shader
#version 330

// Super-duper simple flat shading FS
flat in vec4 object_color;

layout (location = 0) out vec4 color;

void main(void)
{
    color = object_color;
}
