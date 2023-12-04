#ifndef HUMANGL_HPP
#define HUMANGL_HPP
#include <vector>
#include <GL/glew.h>
#include <chrono>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <filesystem>
#include "ft_vec.hpp"
#include "ft_mat.hpp"
#include "settings.hpp"
#include "Animation.hpp"
#include "imgui.h"

typedef ft::vector<float> vec;
typedef ft::matrix<float> mat;
typedef std::map<float, std::vector<Animation>> Animations;

using std::string;
using namespace std::chrono::_V2;

extern std::map<string, Animations> name_to_animations;
extern Animations *current_animation;
extern system_clock::time_point start_time;
extern system_clock::time_point end_time;

typedef enum ModelType {
    Human = 0,
    Alien
} ModelType;

class Bone
{
public:
    string name;

protected:
    Bone *parent;
    std::vector<Bone *> children;

    vec jointPos;
    vec jointRot;
    vec dims;
    vec color;
    mat transform;

    vec default_jointPos;
    vec default_jointRot;
    vec default_dims;
    vec default_color;

private:
    uint VAO, VBO, EBO;

public:
    Bone(string name, Bone *parent, vec dims, vec jointPos, vec jointRot, vec color);

    ~Bone();

    vec &getColor();
    void setColor(vec color);

    vec &getDims();
    void setDims(vec dims);

    vec getJointRot();
    void setJointRot(vec euler);
    void setJointRot(mat rot);

    vec getJointPos();
    void setJointPos(vec jointPos);

    void addChild(Bone *child);

    std::vector<Bone *> getChildren();

    void render(GLuint shaderProgram);
    void renderModel(GLuint shaderProgram);

    void clear();

    std::vector<mat> getTransforms();
    std::vector<Animation> getAnimations();
    void applyAnimations(std::vector<Animation> animations);
    void setTransforms(std::vector<mat> transforms);
    void resetTransforms();
    void applyTransforms(mat parentTransform);
};

Bone *createModel(ModelType model_type);
void boneEditor(Bone *bone);

void animationEditor(Bone *root);
void animationSelectionEditor(string &current_animation_name, float &time);
void currentAnimationEditor(Bone *root, string &current_animation_name, float &time);
void saveAnimations(const string name, const Animations &a);
void animationCreationEditor(string &current_animation_name, float &time);
void animationLoadEditor();
void animationPlayEditor();
void setTimeToLastKeyframe(float &time, const string &current_animation_name);
std::map<string, Animations> loadAnimationsFromDir(string dir_path);
Animations loadAnimations(const string name);
std::vector<Animation> parseAnimations(std::vector<string> string_animations);
std::vector<string> split_set(string s, string delimiter);
void runAnimations(Bone *root, Animations &a, system_clock::time_point start);
bool are_animations_valid(Animations &a);

#endif