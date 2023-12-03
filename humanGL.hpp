#ifndef HUMANGL_HPP
#define HUMANGL_HPP
#include <vector>
#include <GL/glew.h>
#include "ft_vec.hpp"
#include "ft_mat.hpp"
#include <chrono>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <filesystem>
#include "imgui.h"

typedef ft::vector<float> vec;
typedef ft::matrix<float> mat;
typedef std::map<float, std::vector<mat>> animation;

using std::string;
using namespace std::chrono::_V2;

extern std::map<string, animation> animations;
extern animation *current_animation;
extern system_clock::time_point start_time;
extern system_clock::time_point end_time;

class Bone
{

public:
    string name;

protected:
    Bone *parent;
    std::vector<Bone *> children;

    vec color;
    vec dims;

    vec jointPos;
    mat jointRot;

public:
    mat transform;

private:
    uint VAO, VBO, EBO;

public:
    Bone(string name, Bone *parent, vec dims, vec jointPos, mat jointRot);

    ~Bone();

    vec &getColor();
    void setColor(vec color);

    vec &getDims();
    void setDims(vec dims);

    vec getJointRot();
    void setJointRot(vec euler);

    vec getJointPos();
    void setJointPos(vec jointPos);

    void addChild(Bone *child);

    std::vector<Bone *> getChildren();

    void render(GLuint shaderProgram);
    void renderModel(GLuint shaderProgram);

    void clear();

    std::vector<mat> getTransforms();
    void setTransforms(std::vector<mat> transforms);
    void resetTransforms(mat parentTransform);
};

Bone *createHumanModel();

void animationEditor(Bone *root);
std::map<string, animation> loadAnimationsFromDir(string dir_path);
animation loadAnimation(const string name);
void saveAnimation(const string name, const animation &a);
std::vector<string> split_set(string s, string delimiter);
void runAnimation(Bone *root, animation &a, system_clock::time_point start);
void boneEditor(Bone *bone);

#endif