// flocking_render.gs
// OpenGL SuperBible
// Example of using transform feedback to implement flocking
// Program by Graham Sellers.

// Flocking rendering geometry shader
#version 330

// This is a really simple geometry shader that simply calculates flat shading.
// There are better ways to do this, but this is easy.
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

// Color from the VS
flat in vec3 vs_color[3];

// Output to the FS
flat out vec4 object_color;

void main(void)
{
    int n;
    // Calculate face normal
    vec3 ab = normalize(gl_PositionIn[1].xyz - gl_PositionIn[0].xyz);
    vec3 ac = normalize(gl_PositionIn[2].xyz - gl_PositionIn[0].xyz);
    vec3 face_normal = cross(ac, ab);
    // Simple lighting calculation
    vec3 base3 = vs_color[0];
    vec4 base4 = vec4(base3, 1.0);
    vec4 color = vec4(vec3(0.2, 0.2, 0.2) + vec3(0.8, 0.8, 0.8) * abs(face_normal).z, 1.0) * base4;

    // Output three vertices
    for (n = 0; n < 3; n++) {
        object_color = color;
        gl_Position = gl_PositionIn[n];
        EmitVertex();
    }
    EndPrimitive();
}
