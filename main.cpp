#include "humanGL.hpp"
#include "Camera.hpp"
#include "GL_Prog.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <map>

using namespace std::chrono::_V2;

uint wWidth = 1000;
uint wHeight = 1000;
bool keys[GLFW_KEY_LAST] = {false};
vec backGroundColor = {0.4f, 0.4f, 0.5f, 1.0f};

std::map<string, animation> animations;
animation *current_animation;
system_clock::time_point start_time = std::chrono::high_resolution_clock::time_point();
system_clock::time_point end_time = start_time;

Bone *root;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    keys[key] = action != GLFW_RELEASE;

    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (keys[GLFW_KEY_R])
        root = createHumanModel();
}

static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
}

int main()
{
    Camera cam({0.0f, 0.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.01f, 0.1f, keys);
    GL_Prog prog("shaders/vs.glsl", "shaders/fs.glsl", key_callback, mouse_callback, wWidth, wHeight);

    auto window = prog.getWindow();
    auto shaderProgram = prog.getShaderProgram();

    animations = loadAnimationsFromDir("anim");

    root = createHumanModel();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glClearColor(backGroundColor[0], backGroundColor[1], backGroundColor[2], backGroundColor[3]);

        mat view = cam.getViewMatrix();
        mat projection = perspective(M_PI / 4, prog.getWidth() / prog.getHeight(), 0.1f, 100.0f);

        vec buff_view(view.begin(), view.end());
        vec buff_projection(projection.begin(), projection.end());

        GLint viewLoc = glGetUniformLocation(shaderProgram, "uView");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &(buff_view[0]));

        GLint projectionLoc = glGetUniformLocation(shaderProgram, "uProjection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &(buff_projection[0]));

        glUniform3f(glGetUniformLocation(shaderProgram, "Color"), 1.0f, 0.0f, 0.0f);

        if (current_animation != nullptr && std::chrono::high_resolution_clock::now() > end_time)
            current_animation = nullptr;

        if (current_animation == nullptr)
            root->resetTransforms(mat(4));
        else
            runAnimation(root, *current_animation, start_time);

        root->renderModel(shaderProgram);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        boneEditor(root);
        animationEditor(root);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!ImGui::GetIO().WantCaptureMouse)
            cam.update(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();

    root->clear();

    return 0;
}
