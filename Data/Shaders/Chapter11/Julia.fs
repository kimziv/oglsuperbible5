// Julia set renderer
// Fragment Shader
// Graham Sellers
// OpenGL SuperBible
#version 150
precision highp float;

out vec4 color;

in vec2 initial_z;

uniform sampler1D tex_gradient;
uniform vec2 C;
uniform int max_iterations = 100;

void main(void)
{
    vec2 Z = initial_z;
    int iterations = 0;
    const float threshold_squared = 16.0;
    while (iterations < max_iterations && dot(Z, Z) < threshold_squared) {
        vec2 Z_squared;
        Z_squared.x = Z.x * Z.x - Z.y * Z.y;
        Z_squared.y = 2.0 * Z.x * Z.y;
        Z = Z_squared + C;
        iterations++;
    }
    if (iterations == max_iterations)
        color = vec4(0.0, 0.0, 0.0, 1.0);
    else
        color = texture(tex_gradient, float(iterations) / float(max_iterations));
}
