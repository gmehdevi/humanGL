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

typedef ft::vector<float> vec;
typedef ft::matrix<float> mat;

typedef std::map<float, std::vector<mat>> animation;


using std::string;

class Bone {

public:
    string name;

protected:
    Bone* parent;
    std::vector<Bone*> children;

    vec color;
    vec dims;

    vec jointPos;
    mat jointRot;

public:
    mat transform;
private:
    uint VAO, VBO, EBO;

public:

    Bone(string name, Bone *parent, vec dims, vec jointPos, mat jointRot)
    : name(name), parent(parent), children({}), color(vec(3, 1)), dims(dims), jointPos(jointPos), jointRot(jointRot), transform(mat(4)) {

        float vertices[] = {
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f
        };

        //this way the cube sits on the origin and the rotation is the cube's orientation wihtout having to translate it
        for (int i = 1; i < 24; i+=3)
            vertices[i] += 0.5;

        uint indices[] = {
            0, 1, 2, 2, 3, 0,
            4, 5, 6, 6, 7, 4,
            0, 3, 7, 7, 4, 0,
            1, 2, 6, 6, 5, 1,
            3, 2, 6, 6, 7, 3,
            0, 1, 5, 5, 4, 0
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindVertexArray(0);
    }

    ~Bone() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    vec &getColor() {
        return color;
    }

    void setColor(vec color) {
        this->color = color;
    }

    vec &getDims() {
        return dims;
    }

    void setDims(vec dims) {
        for (int i = 0; i < 3; i++)
            if (dims[i] < 0.0000000000001)
                dims[i] = 0.0000000000001;
        this->dims = dims;
    }

    vec getJointRot() {
        return rotationToEuler(jointRot);;
    }

    void setJointRot(vec euler) {
        mat rot = eulerToRotation(euler, 4);
        this->jointRot = rot;
    }

    vec getJointPos() {
        return jointPos;
    }

    void setJointPos(vec jointPos) {
        this->jointPos = jointPos;
    }

    void addChild(Bone* child) {
        children.push_back(child);
    }

    std::vector<Bone *> getChildren() {
        return children;
    }

    void render(GLuint shaderProgram) {
        vec buff_model(transform.begin(), transform.end());
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, &buff_model[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "Color"), 1, &color[0]);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void renderModel(GLuint shaderProgram) {
        render(shaderProgram);
        for (Bone* child : children)
            child->renderModel(shaderProgram);
    }

    void clear() {
        for (Bone* child : children)
            child->clear();
        delete this;
    }

    std::vector<mat> getTransforms() {
        std::vector<mat> transforms;
        transforms.push_back(transform);

        for (Bone* child : children) {
            std::vector<mat> childTransforms = child->getTransforms();
            transforms.insert(transforms.end(), childTransforms.begin(), childTransforms.end());
        }
        return transforms;
    }

    void setTransforms(std::vector<mat> transforms) {
        transform = transforms[0];
        transforms.erase(transforms.begin());

        for (Bone* child : children) {
            std::vector<mat> childTransforms(transforms.begin(), transforms.begin() + child->getTransforms().size());
            child->setTransforms(childTransforms);
            transforms.erase(transforms.begin(), transforms.begin() + child->getTransforms().size());
        }
    }

    void resetTransforms(mat parentTransform) {
        mat localTransform;

        if (this == nullptr)
            return;

        if (parent != nullptr)
            localTransform = scale(dims) * jointRot * scale(vec({1/parent->dims[0], 1/parent->dims[1], 1/parent->dims[2]})) * translate(jointPos);
        else
            localTransform = scale(dims) * jointRot * translate(jointPos);


        transform = localTransform * parentTransform;

        for (Bone* child : children)
            if (child != nullptr)
                child->resetTransforms(transform);
    }

};

Bone* createHumanModel() {

    Bone* torso = new Bone("torso", nullptr, vec({1, 2, 0.5}), vec({0, 0, 0}), mat(4));
    torso->setColor(vec({0.8, 0.4, 0.2}));

    Bone* head = new Bone("head", torso, vec({0.5, 0.5, 0.5}), vec({0, 1, 0}), mat(4));
    head->setColor(vec({0.8, 0.8, 0.2}));

    Bone* leftBicep = new Bone("leftBicep", torso, vec({0.3, 2.2, 0.3}), vec({0.5, 0.8, 0}), rotate(M_PI_2, vec({0, 0, 1})));
    leftBicep->setColor(vec({0.2, 0.5, 0.2}));

    Bone* rightBicep = new Bone("rightBicep", torso, vec({0.3, 2.2, 0.3}), vec({-0.5, 0.8, 0}), rotate(-M_PI_2, vec({0, 0, 1})));
    rightBicep->setColor(vec({0.2, 0.5, 0.2}));

    Bone* leftForeArm = new Bone("leftForeArm", leftBicep, vec({0.3, 0.3, 0.3}), vec({0, 1, 0}), mat(4));
    leftForeArm->setColor(vec({0.2, 0.7, 0.2}));

    Bone* rightForeArm = new Bone("rightForeArm", rightBicep, vec({0.3, 0.3, 0.3}), vec({0, 1, 0}), mat(4));
    rightForeArm->setColor(vec({0.2, 0.7, 0.2}));

    Bone* leftThigh = new Bone("leftThigh", torso, vec({0.3, 2.5, 0.3}), vec({-0.4, 0, 0}), rotate(M_PI, vec({1, 0, 0})));
    leftThigh->setColor(vec({0.3, 0.4, 0.6}));

    Bone* rightThigh = new Bone("rightThigh", torso, vec({0.3, 2.5, 0.3}), vec({0.4, 0, 0}), rotate(M_PI, vec({1, 0, 0})));
    rightThigh->setColor(vec({0.3, 0.4, 0.6}));

    Bone* leftCalf = new Bone("leftCalf", leftThigh, vec({0.4, 0.3, 0.8}), vec({0, 1, 0}), mat(4));
    leftCalf->setColor(vec({0.1, 0.3, 0.5}));

    Bone* rightCalf = new Bone("rightCalf", rightThigh, vec({0.4, 0.3, 0.8}), vec({0, 1, 0}), mat(4));
    rightCalf->setColor(vec({0.1, 0.3, 0.5}));

    torso->addChild(leftBicep);
    torso->addChild(rightBicep);
    torso->addChild(leftThigh);
    torso->addChild(rightThigh);
    torso->addChild(head);

    leftBicep->addChild(leftForeArm);
    rightBicep->addChild(rightForeArm);
    leftThigh->addChild(leftCalf);
    rightThigh->addChild(rightCalf);

    return torso;
}

void saveAnimation(const string name, const animation &a) {
    std::ofstream file(name + ".anim");
    if (!file.is_open()) {
        std::cerr << "Animation " << name << " not found" << std::endl;
        return;
    }
    for (const auto& [time, transforms] : a) {
        file << time << "\n";
        for (const mat &m : transforms)
            file << m << "\n/";
        file << "~";
    }
    file.close();
}


std::vector<string> split_set(string s, string delimiter) {
    std::vector<string> ret;
    size_t pos_start = 0, pos_end = 0;
    string token;

    while ((pos_start = s.find_first_not_of(delimiter, pos_end)) != string::npos) {
        pos_start = s.find_first_not_of(delimiter, pos_end);
        pos_end = s.find_first_of(delimiter, pos_start);
        ret.push_back(s.substr(pos_start, pos_end - pos_start));
    }
    return ret;
}

animation loadAnimation(const string name) {
    std::ifstream file(name + ".anim");
    if (!file.is_open()) {
        std::cerr << "Animation " << name << " not found" << std::endl;
        return animation();
    }

    std::stringstream buffer;

    buffer << file.rdbuf();

    animation a;
    std::vector<string> frames = split_set(buffer.str(), "~");
    for (string frame : frames) {
        float time = std::stof(frame.substr(0, frame.find_first_of("\n")));
        frame.erase(0, frame.find_first_of("\n") + 1);
        std::vector<string> transforms = split_set(frame, "/");
        std::vector<mat> mats;
        for (string transform : transforms) {
            mat m(4);
            std::stringstream ss(transform);
            ss >> m;
            mats.push_back(m);
        }
        a[time] = mats;
    }

    file.close();
    return a;
}

std::map<string, animation> loadAnimations(string dir_path) {
    std::map<string, animation> animations;
    for (const auto &entry : std::filesystem::directory_iterator(dir_path)) {
        if (entry.path().extension() != ".anim")
            throw std::runtime_error("Invalid file type");
        string path = entry.path();
        string name = path.substr(path.find_last_of("/") + 1, path.find_last_of(".") - path.find_last_of("/") - 1);
        std::cout << "Loading animation " << name << std::endl;
        path = path.substr(0, path.find_last_of("."));
        animations[name] = loadAnimation(path);
    }
    return animations;
}

void runAnimation(Bone *root, animation &a, std::chrono::_V2::system_clock::time_point start) {
    float t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count() / 1000.0f;
    auto before = a.lower_bound(t);
    auto after = a.upper_bound(t);

    if (before != a.begin()) before--;
    if (after == a.end()) after--;
    t = (t - before->first) / (after->first - before->first);

    std::vector<mat> transforms;
    for (int i = 0; i < before->second.size(); i++)
        transforms.push_back(linear_interpolation(before->second[i], after->second[i], t));

    root->setTransforms(transforms);
}

#endif