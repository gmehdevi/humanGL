#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GL_Prog {
public:
    GL_Prog(const std::string& vertexShaderPath, const std::string& fragmentShaderPath,
            GLFWkeyfun keyCallback, GLFWcursorposfun mouseCallback,
            int width, int height, std::string title = "OpenGL program")
        : window(nullptr), shader_program(0), key_callback(keyCallback), mouse_callback(mouseCallback),
          screenWidth(width), screenHeight(height) {

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW." << std::endl;
            exit(-1);
        }

        glfwSetErrorCallback(error_callback);

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        screenWidth = width <= 0 || width > glfwGetVideoMode(glfwGetPrimaryMonitor())->width ? glfwGetVideoMode(glfwGetPrimaryMonitor())->width : width;
        screenHeight = height <= 0 || height > glfwGetVideoMode(glfwGetPrimaryMonitor())->height ? glfwGetVideoMode(glfwGetPrimaryMonitor())->height : height;

        window = glfwCreateWindow(screenWidth, screenHeight, title.data(), nullptr, nullptr);
        if (!window) {
            std::cerr << "Failed to create GLFW window." << std::endl;
            glfwTerminate();
            exit(-1);
        }

        if (screenWidth <= 0 || screenHeight <= 0) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        } else {
            glfwSetWindowMonitor(window, nullptr, 0, 0, screenWidth, screenHeight, GLFW_DONT_CARE);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (glewInit() != GLEW_OK) {
            std::cerr << "Failed to initialize GLEW." << std::endl;
            exit(-1);
        }

        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);

        loadShaders(vertexShaderPath, fragmentShaderPath);
    }

    ~GL_Prog() {
        glDeleteProgram(shader_program);
        glfwTerminate();
    }

    void changeShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
        glDeleteProgram(shader_program);
        loadShaders(vertexShaderPath, fragmentShaderPath);
    }

    GLFWwindow* getWindow() const {
        return window;
    }

    GLuint getShaderProgram() const {
        return shader_program;
    }

    int getWidth() const {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return width;
    }

    int getHeight() const {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        return height;
    }


private:
    GLFWwindow *window;
    GLuint shader_program;
    GLFWkeyfun key_callback;
    GLFWcursorposfun mouse_callback;
    int screenWidth;
    int screenHeight;

    static void error_callback(int error, const char *description) {
        std::cerr << "Error: " << description << std::endl;
    }

    void loadShaders(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
        std::string vertexShaderSource = loadShaderSource(vertexShaderPath);
        std::string fragmentShaderSource = loadShaderSource(fragmentShaderPath);

        GLuint vertex_shader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragment_shader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        checkShaderError(shader_program, GL_LINK_STATUS, true, "Error: Failed to link shader program.");

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    std::string loadShaderSource(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file) {
            std::cerr << "Failed to open shader file: " << filePath << std::endl;
            exit(-1);
        }

        std::stringstream stream;
        stream << file.rdbuf();
        return stream.str();
    }

    GLuint createShader(GLenum shaderType, const std::string& shaderSource) {
        GLuint shader = glCreateShader(shaderType);
        const char* shaderSourcePtr = shaderSource.c_str();
        glShaderSource(shader, 1, &shaderSourcePtr, nullptr);
        glCompileShader(shader);
        checkShaderError(shader, GL_COMPILE_STATUS, false, "Error: Failed to compile shader.");
        return shader;
    }

    void checkShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage) {
        GLint success = 0;
        GLchar errorLog[1024] = {0};

        if (isProgram)
            glGetProgramiv(shader, flag, &success);
        else
            glGetShaderiv(shader, flag, &success);

        if (success == GL_FALSE) {
            if (isProgram)
                glGetProgramInfoLog(shader, sizeof(errorLog), nullptr, errorLog);
            else
                glGetShaderInfoLog(shader, sizeof(errorLog), nullptr, errorLog);

            std::cerr << errorMessage << ": " << errorLog << std::endl;
            exit(-1);
        }
    }

};