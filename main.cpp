#include <map>
#include "humanGL.hpp"
#include "Camera.hpp"
#include "GL_Prog.hpp"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std::chrono::_V2;

bool keys[GLFW_KEY_LAST] = {false};
ModelType model_type = Human;
vec background_color = {BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, BACKGROUND_COLOR_A};
Bone *root;
std::map<string, Animations> name_to_animations;
Animations *current_animation;
system_clock::time_point start_time = std::chrono::high_resolution_clock::time_point();
system_clock::time_point end_time = start_time;

static void key_callback(GLFWwindow *window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods)
{
    keys[key] = action != GLFW_RELEASE;

    if (keys[KEY_EXIT])
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (keys[KEY_RECREATE_MODEL])
    {
        root->clear();
        root = createModel(model_type);
    }
}

static void mouse_callback([[maybe_unused]] GLFWwindow *window, [[maybe_unused]] double xpos, [[maybe_unused]] double ypos) {}

ModelType getModelType(int argc, char **argv)
{
    if (argc < 2)
    {
        return Human;
    }

    if (strcmp(argv[1], "human") == 0)
    {
        return Human;
    }
    else if (strcmp(argv[1], "alien") == 0)
    {
        return Alien;
    }
    else
    {
        std::cerr << "Invalid model type: " << argv[1] << std::endl;

        exit(-1);
    }
}

int main(int argc, char **argv)
{
    if (argc > 2 && string(argv[1]).compare("-h"))
    {
        std::cerr << "Usage: " << argv[0] << " [human|alien (defualt: human)]" << std::endl;
        return 0;
    }

    model_type = getModelType(argc, argv);

    Camera cam(CAMERA_EYE_POSITION, CAMERA_CENTER_POSITION, CAMERA_UP_VECTOR, CAMERA_ROTATE_SPEED, CAMERA_TRANSLATE_SPEED, keys);
    GL_Prog prog("shaders/vs.glsl", "shaders/fs.glsl", key_callback, mouse_callback, WINDOW_WIDTH, WINDOW_HEIGHT);

    auto window = prog.getWindow();
    auto shaderProgram = prog.getShaderProgram();

    name_to_animations = loadAnimationsFromDir(DEFAULT_ANIMATIONS_DIRECTORY);

    root = createModel(model_type);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glClearColor(background_color[0], background_color[1], background_color[2], background_color[3]);

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
            root->applyTransforms(mat(4));
        else
            runAnimations(root, *current_animation, start_time);

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
