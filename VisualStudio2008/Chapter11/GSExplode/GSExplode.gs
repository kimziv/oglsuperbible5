// Normal Visualizer
// Geometry Shader
// Graham Sellers
// OpenGL SuperBible
#version 150

precision highp float;

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in Vertex
{
    vec3 normal;
    vec4 color;
} vertex[];

out vec4 color;

uniform vec3 vLightPosition;
uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat3 normalMatrix;

uniform float push_out;

void main(void)
{
    int n;

    vec3 face_normal = normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz,
                                       gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz));

    for (n = 0; n < gl_in.length(); n++) {
        color = vertex[n].color;
        gl_Position = mvpMatrix * vec4(gl_in[n].gl_Position.xyz + face_normal * push_out, gl_in[n].gl_Position.w);
        EmitVertex();
    }
    EndPrimitive();
}