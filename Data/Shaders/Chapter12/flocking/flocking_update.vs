// flocking_update.vs
// OpenGL SuperBible
// Example of using transform feedback to implement flocking
// Program by Graham Sellers.

// Flocking update vertex shader
#version 330

// Position and velocity inputs
layout (location = 0) in vec3 flock_position;
layout (location = 1) in vec3 flock_velocity;

// Outputs (via transform feedback)
out vec3 position_out;
out vec3 velocity_out;

// TBOs containing the position and velocity of other flock members
uniform samplerBuffer tex_positions;
uniform samplerBuffer tex_velocities;

// Parameters...
// This has to match the app's view of the world - no default is given here.
uniform int flock_size;
// These all have defaults. In the example application, these aren't changed.
// Just edit these and rerun the application. It's certainly possible to change
// these parameters at run time by hooking the uniforms up in the application.
uniform float rule1_weight = 0.17;
uniform float rule2_weight = 0.01;
uniform float damping_coefficient = 0.99999;
uniform float closest_allowed_dist = 50.0;

// Time varying uniforms
uniform vec3 goal;
uniform float timestep;

// The two per-member rules
vec3 rule1(vec3 my_position, vec3 my_velocity, vec3 their_position, vec3 their_velocity)
{
    vec3 d = my_position - their_position;
    if (dot(d, d) < closest_allowed_dist)
        return d;
    return vec3(0.0);
}

vec3 rule2(vec3 my_position, vec3 my_velocity, vec3 their_position, vec3 their_velocity)
{
     vec3 d = their_position - my_position;
     vec3 dv = their_velocity - my_velocity;
     return dv / (dot(d, d) + 10.0);
}

void main(void)
{
    vec3 accelleration = vec3(0.0);
    vec3 center = vec3(0.0);
    vec3 new_velocity;
    int i;

    // Apply rules 1 and 2 for my member in the flock (based on all other
    // members)
    for (i = 0; i < flock_size; i++) {
        if (i != gl_VertexID) {
            vec3 their_position = texelFetch(tex_positions, i).xyz;
            vec3 their_velocity = texelFetch(tex_velocities, i).xyz;
            accelleration += rule1(flock_position, flock_velocity, their_position, their_velocity) * rule1_weight;
            accelleration += rule2(flock_position, flock_velocity, their_position, their_velocity) * rule2_weight;
            center += their_position;
        }
    }
    // Also accellerate towards the goal (rule 3)
    accelleration += normalize(goal - flock_position) * 0.025;
    // Update position based on prior velocity and timestep
    position_out = flock_position + flock_velocity * timestep;
    // Update velocity based on calculated accelleration
    accelleration = normalize(accelleration) * min(length(accelleration), 10.0);
    new_velocity = flock_velocity * damping_coefficient + accelleration * timestep;
    // Hard clamp speed (mag(velocity) to 10 to prevent insanity
    if (length(new_velocity) > 10.0)
        new_velocity = normalize(new_velocity) * 10.0;
    velocity_out = new_velocity;
    // Write position (not strictly necessary as we're capturing user defined
    // outputs using transform feedback)
    gl_Position = vec4(flock_position * 0.1, 1.0);
}
