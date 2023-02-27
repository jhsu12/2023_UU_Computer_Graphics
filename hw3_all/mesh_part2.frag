#version 330
#extension GL_ARB_explicit_attrib_location : require


// Uniform constants
uniform float u_time;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_model;
uniform bool is_surface_normal;

// Light
uniform vec3 u_lightPosition; // The position of your light source
uniform vec3 u_lightColor;

// Ambient
uniform vec3 u_ambientColor;

// Diffuse
uniform vec3 u_diffuseColor; // The diffuse surface color of the model

// Specular
uniform vec3 u_specularColor;
uniform float specularPower;

// Gamma
uniform bool is_gamma;

//cubemap
uniform samplerCube u_cubemap;
uniform bool is_cubemap_reflect;

// ...

// Fragment shader inputs
in vec3 v_color;
in vec3 N;
in vec3 L;
in vec3 V;

// ...

// Fragment shader outputs
out vec4 frag_color;

void main()
{
    vec3 final_v_color = v_color;
    vec3 R = reflect(-V, N);

    // Calculate the diffuse (Lambertian) reflection term
    float diffuse = max(0.0, dot(N, L));
    float specular = 0.0;
    
    if (diffuse > 0.0) 
    {
        // this is blinn phong
        vec3 halfDir = normalize(L + V);
        float specAngle = max(dot(halfDir, N), 0.0);
        // normalized 
        specular = (specularPower+8.0)/8.0*pow(specAngle, specularPower);
    }
    
    if(!is_surface_normal)
    {
        // Multiply the diffuse reflection term with the base surface color
        final_v_color = u_ambientColor + diffuse * u_diffuseColor * u_lightColor + specular * u_specularColor * u_lightColor;
        
    }

    if(is_gamma)
    {
        final_v_color = pow(final_v_color, vec3(1 / 2.2));
        
    }

    if(is_cubemap_reflect)
    {
        final_v_color = texture(u_cubemap, R).rgb;
    }
    
    frag_color = vec4(final_v_color, 1.0);
    
    
    
}
