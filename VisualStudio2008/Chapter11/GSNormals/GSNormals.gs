// Normal Visualizer
// Geometry Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

layout (triangles) in;
layout (line_strip, max_vertices = 8) out;

in Vertex
{
    vec3 normal;
} vertex[];

out vec4 color;

uniform mat4	mvpMatrix;
uniform mat4	mvMatrix;

void main(void)
{
    int n;

    for (n = 0; n < gl_in.length(); n++) {
        color = vec4(1.0, 0.3, 0.3, 1.0);
        gl_Position = mvpMatrix * gl_in[n].gl_Position;
        EmitVertex();
        color = vec4(0.0);
        gl_Position = mvpMatrix * (gl_in[n].gl_Position + vec4(vertex[n].normal * 0.05, 0.0));
        EmitVertex();
        EndPrimitive();
    }

    vec4 cent = (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position) / 3.0;
    vec3 face_normal = normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz,
                                       gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz));

    gl_Position = mvpMatrix * cent;
    color = vec4(0.3, 1.0, 0.3, 1.0);
    EmitVertex();
    gl_Position = mvpMatrix * (cent + vec4(face_normal * 0.1, 0.0));
    color = vec4(0.0);
    EmitVertex();
    EndPrimitive();
}