#include "humanGL.hpp"
#include "Camera.hpp"
#include "GL_Prog.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <map>

uint wWidth = 1000;
uint wHeight = 1000;
bool keys[GLFW_KEY_LAST] = {false};
vec backGroundColor = {0.4f, 0.4f, 0.5f, 1.0f};

animation *currentAnimation;
auto startTime = std::chrono::high_resolution_clock::time_point();
auto endTime = std::chrono::high_resolution_clock::time_point();

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

std::map<string, animation> animations;
void animationEditor(Bone *root)
{
    ImGui::Begin("Animation Editor");
    static string currAnimationName = "";
    static char newAnimationName[100] = "";
    static char loadAnimationName[100] = "";
    static float getT = 0.0f;

    if (ImGui::BeginCombo("Animations", currAnimationName.c_str()))
    {
        for (auto &anim : animations)
        {
            bool is_selected = (currAnimationName == anim.first);
            if (ImGui::Selectable(anim.first.c_str(), is_selected))
                currAnimationName = anim.first;
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
                getT = animations[currAnimationName].rbegin()->first;
            }
        }
        ImGui::EndCombo();
    }

    if (currAnimationName != "" && animations[currAnimationName].size() > 0)
        ImGui::SliderFloat("Time", &getT, animations[currAnimationName].rbegin()->first, 10.0f);

    if (currAnimationName != "" && ImGui::Button("Save Keyframe"))
        animations[currAnimationName].insert(std::make_pair(getT, root->getTransforms()));

    if (currAnimationName != "" && animations[currAnimationName].size() > 0 && ImGui::Button("Delete Keyframe"))
        animations[currAnimationName].erase(animations[currAnimationName].rbegin()->first);

    if (currAnimationName != "" && ImGui::Button("Delete Animation"))
    {
        animations.erase(currAnimationName);
        currAnimationName = "";
    }

    if (currAnimationName != "" && animations[currAnimationName].size() > 1 && ImGui::Button("Save to file"))
        saveAnimation(currAnimationName, animations[currAnimationName]);

    ImGui::Separator();

    ImGui::InputText("New", newAnimationName, 100);

    if (newAnimationName[0] != '\0')
    {
        if (ImGui::Button("Create Animation"))
        {
            animations[newAnimationName] = animation();
            newAnimationName[0] = '\0';
            std::cout << "Created new animation " << newAnimationName << std::endl;
        }
    }

    ImGui::InputText("Load", loadAnimationName, 100);

    if (loadAnimationName[0] != '\0')
    {
        if (ImGui::Button("Load Animation"))
        {
            animation a = loadAnimation(loadAnimationName);
            if (a.size() > 0)
                animations[loadAnimationName] = a;
            else
                std::cerr << "Animation " << loadAnimationName << " not found" << std::endl;
            loadAnimationName[0] = '\0';
        }
    }

    ImGui::Separator();

    ImGui::Text("Play Animation");
    for (auto &anim : animations)
    {
        if (anim.second.size() > 1 && ImGui::Button(anim.first.c_str()))
        {
            currentAnimation = &anim.second;
            startTime = std::chrono::high_resolution_clock::now();
            endTime = startTime + std::chrono::milliseconds((int)(anim.second.rbegin()->first * 1000));
            std::cout << "animation selected" << std::endl;
        }
    }

    ImGui::End();
}

void boneEditor(Bone *bone)
{
    ImGui::Begin("Bone Editor");

    if (ImGui::TreeNode(bone, "%s", bone->name.c_str()))
    {

        vec color = bone->getColor();
        vec dims = bone->getDims();
        vec jointRot = bone->getJointRot();

        if (bone->name == "torso")
        {
            vec jointPos = bone->getJointPos();
            if (ImGui::SliderFloat3("Position", &jointPos[0], -3.0f, 3.0f))
                bone->setJointPos(jointPos);
        }

        if (ImGui::ColorEdit3("Color", &color[0], ImGuiColorEditFlags_NoOptions))
            bone->setColor(color);

        if (ImGui::SliderFloat3("Dimensions", &dims[0], 0.0f, 3.0f))
            bone->setDims(dims);

        if (ImGui::SliderFloat3("Rotation", &jointRot[0], -M_PI, M_PI))
            bone->setJointRot(jointRot);

        for (Bone *child : bone->getChildren())
            boneEditor(child);

        ImGui::TreePop();
    }

    ImGui::End();
}

int main()
{
    Camera cam({0.0f, 0.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 0.01f, 0.1f, keys);
    GL_Prog prog("shaders/vs.glsl", "shaders/fs.glsl", key_callback, mouse_callback, wWidth, wHeight);

    auto window = prog.getWindow();
    auto shaderProgram = prog.getShaderProgram();

    animations = loadAnimations("anim");

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

        if (currentAnimation != nullptr && std::chrono::high_resolution_clock::now() > endTime)
            currentAnimation = nullptr;

        if (currentAnimation == nullptr)
            root->resetTransforms(mat(4));
        else
            runAnimation(root, *currentAnimation, startTime);

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
