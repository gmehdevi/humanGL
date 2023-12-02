#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <GLFW/glfw3.h>
#include <vector>

using namespace ft;
typedef vector<float> vec;
typedef matrix<float> mat;

class Camera
{
public:
    vector<vec> init;
    vec eye;
    vec center;
    vec up;
    float cameraRSpeed;
    float cameraTSpeed;
    bool *keys;
    double lastMouseX;
    double lastMouseY;
    bool firstMouse;

    Camera(vec eyePos, vec centerPos, vec upVec, float rotateSpeed, float translateSpeed, bool *keyStates)
        : eye(eyePos), center(centerPos), up(upVec), cameraRSpeed(rotateSpeed), cameraTSpeed(translateSpeed), keys(keyStates), lastMouseX(0.0), lastMouseY(0.0), firstMouse(true)
    {
        init.push_back(eyePos);
        init.push_back(centerPos);
        init.push_back(upVec);
    }

    void reset()
    {
        eye = init[0];
        center = init[1];
        up = init[2];
    }

    void rotateCamera(GLFWwindow *window)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if (firstMouse)
        {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        }

        float xOffset = lastMouseX - mouseX;
        float yOffset = lastMouseY - mouseY;

        lastMouseX = mouseX;
        lastMouseY = mouseY;

        xOffset *= cameraRSpeed;
        yOffset *= cameraRSpeed;

        float maxPitch = 89.0f * (M_PI / 180.0f);  // Converting degrees to radians
        float minPitch = -89.0f * (M_PI / 180.0f); // Converting degrees to radians

        vec forward = (center - eye).normalize();
        vec right = cross(forward, up).normalize();
        vec actualUp = cross(right, forward).normalize();

        mat rotationAroundUp = rotate(xOffset, actualUp);

        vec tmp_forward({forward[0], forward[1], forward[2], 0.0f});

        tmp_forward = (rotationAroundUp * tmp_forward);

        mat rotationAroundRight = rotate(yOffset, right);
        tmp_forward = (rotationAroundRight * tmp_forward);

        forward = {tmp_forward[0], tmp_forward[1], tmp_forward[2]};
        // Preventing flipping by constraining pitch
        float pitch = asin(-forward[1]);
        if (pitch > maxPitch)
        {
            forward[1] = -sin(maxPitch);
            forward = forward.normalize();
        }
        else if (pitch < minPitch)
        {
            forward[1] = -sin(minPitch);
            forward = forward.normalize();
        }

        center = eye + forward;
        up = cross(right, forward).normalize();
    }

    void rotateCameraArrows()
    {
        vec forward = (center - eye).normalize();
        vec right = (cross(forward, up)).normalize();
        vec rotation(3);

        rotation += (keys[GLFW_KEY_UP] ? right : vec(3));
        rotation -= (keys[GLFW_KEY_DOWN] ? right : vec(3));
        rotation += (keys[GLFW_KEY_LEFT] ? up : vec(3));
        rotation -= (keys[GLFW_KEY_RIGHT] ? up : vec(3));

        if (rotation == vec(3))
            return;

        mat rotationMatrix = rotate(cameraRSpeed, rotation);
        vec forward4 = forward;
        forward4.push_back(1.0f);

        vec newDirection = rotationMatrix * forward4;
        newDirection.pop_back();

        center = eye + newDirection;
        up = (cross(right, newDirection)).normalize();
    }

    void translateCamera()
    {
        vec forward = (center - eye).normalize();
        vec right = cross(forward, up).normalize();

        vec translation(3);
        translation += keys[GLFW_KEY_W] ? forward : vec(3);
        translation -= keys[GLFW_KEY_S] ? forward : vec(3);
        translation += keys[GLFW_KEY_D] ? right : vec(3);
        translation -= keys[GLFW_KEY_A] ? right : vec(3);
        translation += keys[GLFW_KEY_SPACE] ? up : vec(3);
        translation -= keys[GLFW_KEY_LEFT_SHIFT] ? up : vec(3);

        translation *= cameraTSpeed;

        eye += translation;
        center += translation;
    }

    void update([[maybe_unused]] GLFWwindow *window)
    {
        // rotateCamera(window);
        rotateCameraArrows();
        translateCamera();

        if (keys[KEY_RESET_CAMERA])
            reset();
    }

    mat getViewMatrix() const
    {
        return camera(eye, center, up);
    }
};

#endif
