// flocking_render.vs
// OpenGL SuperBible
// Example of using transform feedback to implement flocking
// Program by Graham Sellers.

// Flocking render vertex shader
#version 330

// Model-view-projection matrix
uniform mat4 mvp;

// Instanced attributes (position and velocity of the flock member)
layout (location = 0) in vec3 instance_position;
layout (location = 1) in vec3 instance_velocity;
// Geometry input
layout (location = 2) in vec3 position;

// Output color from the vertex shader
flat out vec3 vs_color;

// This is essentially a 'lookat' matrix to make the airplane fly fowards
// and hopefully, stay upright
mat4 calculate_rotation_matrix(vec3 direction)
{
    vec3 forwards = normalize(direction);
    vec3 outwards;
    outwards = cross(forwards, vec3(1.0, 0.0, 0.0)) * forwards.x * forwards.x +
               cross(forwards, vec3(0.0, 1.0, 0.0)) * forwards.y * forwards.y +
               cross(forwards, vec3(0.0, 0.0, 1.0)) * forwards.z * forwards.z;
//     outwards /= (abs(forwards.x) + abs(forwards.y) + abs(forwards.z));
    vec3 up = cross(forwards, outwards);
    outwards = cross(forwards, up);

    vec4 row3 = vec4(forwards, 0.0);
    vec4 row2 = vec4(normalize(outwards), 0.0);
    vec4 row1 = vec4(normalize(up), 0.0);

    return mat4(row1, row2, row3, vec4(0.0));
}

void main(void)
{
    // Get the forwards matrix
    mat4 rotation = calculate_rotation_matrix(instance_velocity);
    // Rotate the airplane geometry into the correct orientation and then
    // offset by position within the flock
    vec4 pos = rotation * vec4(position, 0.0) + vec4(instance_position, 1.0);
    // Calculate a color based in instance index (evey plane gets a new color)
    float r = float((gl_InstanceID * 37 & 0xF) + 5) / 20.0;
    float g = float(((gl_InstanceID * 73 >> 4) & 0xF) + 5) / 20.0;
    float b = float(((gl_InstanceID * 327 >> 8) & 0xF) + 5) / 20.0;
    vs_color = vec3(r, g, b);
    // Output position
    gl_Position = mvp * pos;
}
