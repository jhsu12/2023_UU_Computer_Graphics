#version 330
#extension GL_ARB_explicit_attrib_location : require

// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;


// Light
uniform vec3 u_lightPosition; // The position of your light source

// Vertex inputs (attributes from vertex buffers)
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_color;
layout(location = 2) in vec3 a_normal;
// ...

// Vertex shader outputs
out vec3 v_color;
out vec3 N;
out vec3 L;
out vec3 V;


// ...

void main()
{
    v_color = 0.5 * a_normal + 0.5; // maps the normal direction to an RGB color

    gl_Position = u_projection * u_view * u_model * a_position;

    // model view matrix
    mat4 mv = u_view * u_model;
    
   // Transform the vertex position to view space (eye coordinates)
    vec3 positionEye = vec3(mv * a_position);

	// Transform lightPosition to view coordinates
    vec3 LightPosView = vec3(u_view * vec4(u_lightPosition, 1.0));

    // Calculate the view-space normal
    N = normalize(mat3(mv) * a_normal);

    // Calculate the view-space light direction
    L = normalize(LightPosView  - positionEye);

    // view vector V
    V = normalize(-positionEye);

    
    
	
	
}
