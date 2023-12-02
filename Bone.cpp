#include "humanGL.hpp"
#include "imgui.h"

Bone::Bone(string name, Bone *parent, vec dims, vec jointPos, mat jointRot, vec color)
	: name(name), parent(parent), children({}), jointPos(jointPos), jointRot(jointRot), dims(dims), color(color), transform(mat(4))
{
	default_jointPos = jointPos;
	default_jointRot = jointRot;
	default_dims = dims;
	default_color = color;

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,

		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f};

	// this way the cube sits on the origin and the rotation is the cube's orientation wihtout having to translate it
	for (int i = 1; i < 24; i += 3)
		vertices[i] += 0.5;

	uint indices[] = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
		0, 3, 7, 7, 4, 0,
		1, 2, 6, 6, 5, 1,
		3, 2, 6, 6, 7, 3,
		0, 1, 5, 5, 4, 0};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

Bone::~Bone()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

vec &Bone::getColor()
{
	return color;
}

void Bone::setColor(vec color)
{
	this->color = color;
}

vec &Bone::getDims()
{
	return dims;
}

void Bone::setDims(vec dims)
{
	for (int i = 0; i < 3; i++)
		if (dims[i] < 0.0000000000001)
			dims[i] = 0.0000000000001;
	this->dims = dims;
}

vec Bone::getJointRot()
{
	return rotationToEuler(jointRot);
}

void Bone::setJointRot(vec euler)
{
	mat rot = eulerToRotation(euler, 4);
	this->jointRot = rot;
}

void Bone::setJointRot(mat rot)
{
	this->jointRot = rot;
}

vec Bone::getJointPos()
{
	return jointPos;
}

void Bone::setJointPos(vec jointPos)
{
	this->jointPos = jointPos;
}

void Bone::addChild(Bone *child)
{
	children.push_back(child);
}

std::vector<Bone *> Bone::getChildren()
{
	return children;
}

void Bone::renderModel(GLuint shaderProgram)
{
	render(shaderProgram);

	for (Bone *child : children)
		child->renderModel(shaderProgram);
}

void Bone::render(GLuint shaderProgram)
{
	vec buff_model(transform.begin(), transform.end());
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uModel"), 1, GL_FALSE, &buff_model[0]);
	glUniform3fv(glGetUniformLocation(shaderProgram, "Color"), 1, &color[0]);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Bone::clear()
{
	for (Bone *child : children)
		child->clear();

	delete this;
}

std::vector<mat> Bone::getTransforms()
{
	std::vector<mat> transforms;
	transforms.push_back(transform);

	for (Bone *child : children)
	{
		std::vector<mat> childTransforms = child->getTransforms();
		transforms.insert(transforms.end(), childTransforms.begin(), childTransforms.end());
	}
	return transforms;
}

std::vector<Animation> Bone::getAnimations()
{
	std::vector<Animation> animations;
	animations.push_back(Animation(jointPos, rotationToEuler(jointRot), dims, color));

	for (Bone *child : children)
	{
		std::vector<Animation> childAnimations = child->getAnimations();
		animations.insert(animations.end(), childAnimations.begin(), childAnimations.end());
	}

	return animations;
}

void Bone::applyAnimations(std::vector<Animation> animations)
{
	setJointPos(animations[0].getTranslation());
	setJointRot(animations[0].getRotation());
	setDims(animations[0].getScale());
	setColor(animations[0].getColor());
	animations.erase(animations.begin());

	if (parent != nullptr)
		applyTransforms(parent->transform);
	else
		applyTransforms(mat(4));

	for (Bone *child : children)
	{
		std::vector<Animation> childAnimations(animations.begin(), animations.begin() + child->getAnimations().size());
		child->applyAnimations(childAnimations);
		animations.erase(animations.begin(), animations.begin() + child->getAnimations().size());
	}
}

void Bone::setTransforms(std::vector<mat> transforms)
{
	transform = transforms[0];
	transforms.erase(transforms.begin());

	for (Bone *child : children)
	{
		std::vector<mat> childTransforms(transforms.begin(), transforms.begin() + child->getTransforms().size());
		child->setTransforms(childTransforms);
		transforms.erase(transforms.begin(), transforms.begin() + child->getTransforms().size());
	}
}

void Bone::resetTransforms()
{
	setJointPos(default_jointPos);
	setJointRot(default_jointRot);
	setDims(default_dims);
	setColor(default_color);
	applyTransforms(parent == nullptr ? mat(4) : parent->transform);

	for (Bone *child : children)
		child->resetTransforms();
}

void Bone::applyTransforms(mat parentTransform)
{
	mat localTransform;

	if (parent != nullptr)
		localTransform = scale(dims) * jointRot * scale(vec({1 / parent->dims[0], 1 / parent->dims[1], 1 / parent->dims[2]})) * translate(jointPos);
	else
		localTransform = scale(dims) * jointRot * translate(jointPos);

	transform = localTransform * parentTransform;

	for (Bone *child : children)
		if (child != nullptr)
			child->applyTransforms(transform);
}

Bone *createHumanModel()
{
	Bone *torso = new Bone("torso", nullptr, vec({1, 2, 0.5}), vec({0, 0, 0}), mat(4), TORSO_COLOR);
	Bone *head = new Bone("head", torso, vec({0.5, 0.5, 0.5}), vec({0, 1, 0}), mat(4), HEAD_COLOR);
	Bone *leftBicep = new Bone("leftBicep", torso, vec({0.3, 2.2, 0.3}), vec({0.5, 0.8, 0}), rotate(M_PI_2, vec({0, 0, 1})), LEFT_ARM_COLOR);
	Bone *rightBicep = new Bone("rightBicep", torso, vec({0.3, 2.2, 0.3}), vec({-0.5, 0.8, 0}), rotate(-M_PI_2, vec({0, 0, 1})), RIGHT_ARM_COLOR);
	Bone *leftForeArm = new Bone("leftForeArm", leftBicep, vec({0.3, 0.3, 0.3}), vec({0, 1, 0}), mat(4), LEFT_FOREARM_COLOR);
	Bone *rightForeArm = new Bone("rightForeArm", rightBicep, vec({0.3, 0.3, 0.3}), vec({0, 1, 0}), mat(4), RIGHT_FOREARM_COLOR);
	Bone *leftThigh = new Bone("leftThigh", torso, vec({0.3, 2.5, 0.3}), vec({-0.4, 0, 0}), rotate(M_PI, vec({1, 0, 0})), LEFT_THIGH_COLOR);
	Bone *rightThigh = new Bone("rightThigh", torso, vec({0.3, 2.5, 0.3}), vec({0.4, 0, 0}), rotate(M_PI, vec({1, 0, 0})), RIGHT_THIGH_COLOR);
	Bone *leftCalf = new Bone("leftCalf", leftThigh, vec({0.4, 0.3, 0.8}), vec({0, 1, 0}), mat(4), LEFT_CALF_COLOR);
	Bone *rightCalf = new Bone("rightCalf", rightThigh, vec({0.4, 0.3, 0.8}), vec({0, 1, 0}), mat(4), RIGHT_CALF_COLOR);

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
