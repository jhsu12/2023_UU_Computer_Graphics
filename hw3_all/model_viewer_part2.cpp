// Model viewer code for the assignments in Computer Graphics 1TD388/1MD150.
//
// Modify this and other source files according to the tasks in the instructions.
//

#include "gltf_io.h"
#include "gltf_scene.h"
#include "gltf_render.h"
#include "cg_utils.h"
#include "cg_trackball.h"

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <iostream>

bool perspective = true;
glm::mat4 projection;
float fov = 45.0f;

void toggleProjection() {
    if (perspective) {
        projection = glm::perspective(glm::radians(fov), 1.0f, 0.1f, 100.0f);
    } else {
        projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
    }
}
// Struct for our application context
struct Context {
    int width = 512;
    int height = 512;
    GLFWwindow *window;
    gltf::GLTFAsset asset;
    gltf::DrawableList drawables;
    cg::Trackball trackball;
    GLuint program;
    GLuint emptyVAO;
    float elapsedTime;
    std::string gltfFilename = "gargo.gltf";
    // Add more variables here...
    float clear_color[4];
    float diffuseColor[3];
    float ambientColor[3];
    float specularColor[3];
    float specularPower;

    float lightPosition[3];
    float lightColor[3];

    bool is_surface_normal;
    bool is_gamma;
    
    GLuint cubemap;
    bool is_cubemap_reflect;
};
// Returns the absolute path to the /assets/cubemaps/ directory
std::string cubemap_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/cubemaps/";
}

// Returns the absolute path to the src/shader directory
std::string shader_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/src/shaders/";
}

// Returns the absolute path to the assets/gltf directory
std::string gltf_dir(void)
{
    std::string rootDir = cg::get_env_var("MODEL_VIEWER_ROOT");
    if (rootDir.empty()) {
        std::cout << "Error: MODEL_VIEWER_ROOT is not set." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return rootDir + "/assets/gltf/";
}

void do_initialization(Context &ctx)
{
    ctx.program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
    //ctx.cubemap = cg::load_cubemap(cubemap_dir() + "/LarnacaCastle/");
    //ctx.cubemap = cg::load_cubemap_prefiltered(cubemap_dir() + "/LarnacaCastle2/");
    
    gltf::load_gltf_asset(ctx.gltfFilename, gltf_dir(), ctx.asset);
    gltf::create_drawables_from_gltf_asset(ctx.drawables, ctx.asset);
}

void load_cubemap(Context &ctx, std::string specular)
{
    ctx.cubemap = cg::load_cubemap(cubemap_dir() + "/LarnacaCastle2/" + specular);
}
/*
void load_cubemap(Context &ctx)
{
    ctx.cubemap = cg::load_cubemap(cubemap_dir() + "/LarnacaCastle/" );
}
*/
void draw_scene(Context &ctx)
{
    
    glm::mat4 view = glm::lookAt(glm::vec3(-3.0f, -3.0f, -3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::mat4(ctx.trackball.orient); // eye, target, up
    glm::mat4 model = glm::scale(glm::mat4(1.0f),glm::vec3(3.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.50f));

    // light
    glm::vec3 lightPosition = glm::vec3(ctx.lightPosition[0], ctx.lightPosition[1], ctx.lightPosition[2]);
    glm::vec3 lightColor = glm::vec3(ctx.lightColor[0], ctx.lightColor[1], ctx.lightColor[2]);

    // diffuse
    glm::vec3 diffuseColor = glm::vec3(ctx.diffuseColor[0], ctx.diffuseColor[1], ctx.diffuseColor[2]);// The diffuse surface color of the model
    
    // ambient
    glm::vec3 ambientColor = glm::vec3(ctx.ambientColor[0], ctx.ambientColor[1], ctx.ambientColor[2]);// The ambient surface color of the model

    // specular
    glm::vec3 specularColor = glm::vec3(ctx.specularColor[0], ctx.specularColor[1], ctx.specularColor[2]);// The specular surface color of the model
    float specularPower = ctx.specularPower;

    // Activate texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ctx.cubemap);

    // Activate shader program
    glUseProgram(ctx.program);

    // Set render state
    glEnable(GL_DEPTH_TEST);  // Enable Z-buffering

    // Define per-scene uniforms
    glUniform1f(glGetUniformLocation(ctx.program, "u_time"), ctx.elapsedTime);
    glUniform1f(glGetUniformLocation(ctx.program, "is_surface_normal"), ctx.is_surface_normal);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(ctx.program, "u_model"), 1, GL_FALSE, &model[0][0]);

    glUniform3f(glGetUniformLocation(ctx.program, "u_diffuseColor"), diffuseColor.x, diffuseColor.y, diffuseColor.z);
    glUniform3f(glGetUniformLocation(ctx.program, "u_ambientColor"), ambientColor.x, ambientColor.y, ambientColor.z);
    glUniform3f(glGetUniformLocation(ctx.program, "u_specularColor"), specularColor.x, specularColor.y, specularColor.z);
    glUniform1f(glGetUniformLocation(ctx.program, "specularPower"), specularPower);
    
    // Light
    glUniform3f(glGetUniformLocation(ctx.program, "u_lightPosition"), lightPosition.x, lightPosition.y, lightPosition.z);
    glUniform3f(glGetUniformLocation(ctx.program, "u_lightColor"), lightColor.x, lightColor.y, lightColor.z);

    // gamma
    glUniform1f(glGetUniformLocation(ctx.program, "is_gamma"), ctx.is_gamma);

    //cubemap
    glUniform1i(glGetUniformLocation(ctx.program, "u_cubemap"), 0);
    glUniform1f(glGetUniformLocation(ctx.program, "is_cubemap_reflect"), ctx.is_cubemap_reflect);

    // Draw scene
    for (unsigned i = 0; i < ctx.asset.nodes.size(); ++i) {
        const gltf::Node &node = ctx.asset.nodes[i];
        const gltf::Drawable &drawable = ctx.drawables[node.mesh];

        // Define per-object uniforms
        // ...

        // Draw object
        glBindVertexArray(drawable.vao);
        glDrawElements(GL_TRIANGLES, drawable.indexCount, drawable.indexType,
                       (GLvoid *)(intptr_t)drawable.indexByteOffset);
        glBindVertexArray(0);
    }

    // Clean up
    cg::reset_gl_render_state();
    glUseProgram(0);
}



void do_rendering(Context &ctx)
{
    
    // Clear render states at the start of each frame
    cg::reset_gl_render_state();


    // Clear color and depth buffers
    glClearColor(ctx.clear_color[0], ctx.clear_color[1], ctx.clear_color[2], ctx.clear_color[3]);
    
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    draw_scene(ctx);
}

void reload_shaders(Context *ctx)
{
    glDeleteProgram(ctx->program);
    ctx->program = cg::load_shader_program(shader_dir() + "mesh.vert", shader_dir() + "mesh.frag");
}

void error_callback(int /*error*/, const char *description)
{
    std::cerr << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (ImGui::GetIO().WantCaptureKeyboard) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_R && action == GLFW_PRESS) { reload_shaders(ctx); }
}

void char_callback(GLFWwindow *window, unsigned int codepoint)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_CharCallback(window, codepoint);
    if (ImGui::GetIO().WantTextInput) return;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Forward event to ImGui
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (ImGui::GetIO().WantCaptureMouse) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ctx->trackball.center = glm::vec2(x, y);
        ctx->trackball.tracking = (action == GLFW_PRESS);
    }
}

void cursor_pos_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    if (ImGui::GetIO().WantCaptureMouse) return;

    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    cg::trackball_move(ctx->trackball, float(x), float(y));
}

void scroll_callback(GLFWwindow *window, double x, double y)
{
    // Forward event to ImGui
    fov -= (float)y;
    if (fov < 0.1f)
        fov = 0.1f;
    if (fov > 100.0f)
        fov = 100.0f; 
    ImGui_ImplGlfw_ScrollCallback(window, x, y);
    if (ImGui::GetIO().WantCaptureMouse) return;
}

void resize_callback(GLFWwindow *window, int width, int height)
{
    // Update window size and viewport rectangle
    Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
    ctx->width = width;
    ctx->height = height;
    glViewport(0, 0, width, height);
}

int main(int argc, char *argv[])
{
    Context ctx = Context();
    if (argc > 1) { ctx.gltfFilename = std::string(argv[1]); }

    // Create a GLFW window
    glfwSetErrorCallback(error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
    glfwMakeContextCurrent(ctx.window);
    glfwSetWindowUserPointer(ctx.window, &ctx);
    glfwSetKeyCallback(ctx.window, key_callback);
    glfwSetCharCallback(ctx.window, char_callback);
    glfwSetMouseButtonCallback(ctx.window, mouse_button_callback);
    glfwSetCursorPosCallback(ctx.window, cursor_pos_callback);
    glfwSetScrollCallback(ctx.window, scroll_callback);
    glfwSetFramebufferSizeCallback(ctx.window, resize_callback);

    // Load OpenGL functions
    if (gl3wInit() || !gl3wIsSupported(3, 3) /*check OpenGL version*/) {
        std::cerr << "Error: failed to initialize OpenGL" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // Initialize ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(ctx.window, false /*do not install callbacks*/);
    ImGui_ImplOpenGL3_Init("#version 330" /*GLSL version*/);

    // Initialize rendering
    glGenVertexArrays(1, &ctx.emptyVAO);
    glBindVertexArray(ctx.emptyVAO);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    do_initialization(ctx);
   


    // Start rendering loop
    float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};
    float diffuseColor[3] = {0.0f, 0.0f, 1.0f};
    float ambientColor[3] = {0.0f, 0.0f, 0.0f};
    float specularColor[3] = {1.0f, 1.0f, 1.0f};
    float specularPower = 500.0;

    // light
    float lightPosition[3] = {2.0f, 2.0f, 2.0f};
    float lightColor[3] = {1.0f, 1.0f, 1.0f};

    // checkboxes
    bool is_diffuse = true, is_specular = true, is_light = true, is_ambient = true, is_gamma = true;
    bool is_surface_normal = false;
    bool is_cubemap_reflect = false;

    // cubemap specular power
    int cubemapIndex = 0;
    const char* cubemapNames[] = { "0.125", "0.5", "2", "8", "32", "128", "512", "2048" };

    while (!glfwWindowShouldClose(ctx.window)) {
        glfwPollEvents();
        ctx.elapsedTime = glfwGetTime();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //ImGui::ShowDemoWindow();
        // ImGui
        {
            
            ImGui::Begin("Welcome!");  
            if (ImGui::BeginTabBar("")) 
            {
                
                if(ImGui::BeginTabItem("View"))
                {
                    ImGui::Checkbox("Perspective", &perspective);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Matarial"))
                {
                    ImGui::ColorEdit3("Diffuse Color", diffuseColor);
                    ImGui::Checkbox("Diffuse enabled", &is_diffuse);

                    ImGui::ColorEdit3("Specular Color", specularColor);
                    ImGui::SliderFloat("Specular Power", &specularPower, 0.0, 300.0);
                    ImGui::Checkbox("Specular enabled", &is_specular);
                    ImGui::Checkbox("Gamma enabled", &is_gamma);

                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Background"))
                {
                    ImGui::ColorEdit4("Background Color", clear_color); // Edit 3 floats representing a color
                    
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Lighting"))
                {
                    ImGui::SliderFloat3("Light position", lightPosition, -100.0, 100.0);
                    ImGui::ColorEdit3("Light Color", lightColor);
                    ImGui::Checkbox("Light enabled", &is_light);

                    ImGui::ColorEdit3("Ambient Color", ambientColor);
                    ImGui::Checkbox("Ambient enabled", &is_ambient);
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Misc"))
                {
                    ImGui::Checkbox("Show Normals", &is_surface_normal);
                    ImGui::Checkbox("Show Cubemap Reflection", &is_cubemap_reflect);
                    if (ImGui::BeginCombo("Cubemap", cubemapNames[cubemapIndex])) 
                    { 
                        for (int i = 0; i < 8; i++) 
                        {
                            bool isSelected = (cubemapIndex == i);
                            if (ImGui::Selectable(cubemapNames[i], isSelected)) 
                            {
                                cubemapIndex = i;
                            }
                            if (isSelected) 
                            {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
            }                       // Create a window called "Hello, world!" and append into it.
            
            
           
            ImGui::End();
        }

        // Deciding the projection matrix
        toggleProjection();

        // Assign is_surface_normal to ctx
        ctx.is_surface_normal = is_surface_normal;

        // Assign new background color to ctx
        ctx.clear_color[0] = clear_color[0];
        ctx.clear_color[1] = clear_color[1];
        ctx.clear_color[2] = clear_color[2];
        ctx.clear_color[3] = clear_color[3];

        // Assign new diffuse color to ctx
        ctx.diffuseColor[0] = diffuseColor[0];
        ctx.diffuseColor[1] = diffuseColor[1];
        ctx.diffuseColor[2] = diffuseColor[2];
        
        if (!is_diffuse)
        {
            ctx.diffuseColor[0] = 0.0;
            ctx.diffuseColor[1] = 0.0;
            ctx.diffuseColor[2] = 0.0;
        }
        
        // Assign new ambient color to ctx
        ctx.ambientColor[0] = ambientColor[0];
        ctx.ambientColor[1] = ambientColor[1];
        ctx.ambientColor[2] = ambientColor[2];

        if (!is_ambient)
        {
            ctx.ambientColor[0] = 0.0;
            ctx.ambientColor[1] = 0.0;
            ctx.ambientColor[2] = 0.0;
        }

        // Assign new specular color to ctx
        ctx.specularColor[0] = specularColor[0];
        ctx.specularColor[1] = specularColor[1];
        ctx.specularColor[2] = specularColor[2];

        if (!is_specular)
        {
            ctx.specularColor[0] = 0.0;
            ctx.specularColor[1] = 0.0;
            ctx.specularColor[2] = 0.0;
        }

        // Assign new specular power to ctx
        ctx.specularPower = specularPower;

        // Assign new light position to ctx
        ctx.lightPosition[0] = lightPosition[0];
        ctx.lightPosition[1] = lightPosition[1];
        ctx.lightPosition[2] = lightPosition[2];

        // Assign new light color to ctx
        ctx.lightColor[0] = lightColor[0];
        ctx.lightColor[1] = lightColor[1];
        ctx.lightColor[2] = lightColor[2];

        if (!is_light)
        {
            ctx.lightColor[0] = 0.0;
            ctx.lightColor[1] = 0.0;
            ctx.lightColor[2] = 0.0;
        }

        ctx.is_gamma = is_gamma;
        ctx.is_cubemap_reflect = is_cubemap_reflect;
        load_cubemap(ctx, cubemapNames[cubemapIndex]);
        
        do_rendering(ctx);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(ctx.window);
    }

    // Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(ctx.window);
    glfwTerminate();
    std::exit(EXIT_SUCCESS);
}
