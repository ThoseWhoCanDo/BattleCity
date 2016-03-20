#include <iostream>
#include <vector>
#include <memory>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include <game/map.h>


// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void window_size_callback(GLFWwindow* window, int width, int height);

struct WindowState {
    GLuint width = 800;
    GLuint height = 600;
} window_state;

// Shaders
const GLchar* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 position;\n"
    "uniform mat4 viewMatrix;\n"
    "void main()\n"
    "{\n"
    "gl_Position = viewMatrix * vec4(position, 1.0);\n"
    "}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
    "}\n\0";

/**
 * @brief computeOrthoMatrix creates an orthographic projection matrix for 2D rendering
 *
 * The function takes the aspect ratio into account, such that content fits all window
 * configurations, such as narrow and tall, or wide and thin
 * @param width the width of the window
 * @param height the height of the window, must be greater than 1
 * @return a glm::mat4x4 matrix object that describes the orthographic projection transformation
 */
glm::mat4x4 computeOrthoMatrix(int width, int height)
{
    assert(height > 0);
    float ratio = static_cast<float>(width) / height;
    if (ratio > 1.0) {
        return glm::ortho(-ratio, ratio, -1.0f, 1.0f);
    }
    return glm::ortho(-1.0f, 1.0f, -1/ratio, 1/ratio);
}


class Shader {
public:
    enum ShaderType {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER
    };

    virtual ~Shader()
    {
        if (shader_ != 0) {
            glDeleteShader(shader_);
        }
    }

    GLuint gl_ref() const
    {
        return shader_;
    }

protected:
    Shader(ShaderType shaderType, const GLchar* shaderSource): shader_(0)
    {
        shader_source_ = shaderSource;
        shader_type_ = shaderType;

        std::cout << "creating shader" << shaderType << std::endl;

        shader_ = glCreateShader(shader_type_);
        glShaderSource(shader_, 1, &shader_source_, NULL);
        glCompileShader(shader_);
        GLint success;
        GLchar info_log[512];
        glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader_, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::" << shader_type_ << "::COMPILATION_FAILED\n" << info_log << std::endl;
        }
    }

private:
    ShaderType shader_type_;
    const GLchar *shader_source_;
    GLuint shader_;
};


class VertexShader: public Shader {
public:
    VertexShader(const GLchar* shaderSource)
        :Shader(ShaderType::Vertex, shaderSource)
    {
    }
};


class FragmentShader: public Shader {
public:
    FragmentShader(const GLchar* shaderSource)
        :Shader(ShaderType::Fragment, shaderSource)
    {
    }
};


class Program {
public:
    Program(const std::vector<Shader*> &shaders)
        : shaders_(shaders),
          program_(0)
    {
        // Link shaders
        program_ = glCreateProgram();
        for (auto it=shaders.begin(); it != shaders.end(); it++) {
            glAttachShader(program_, (*it)->gl_ref());
        }
        glLinkProgram(program_);
        // Check for linking errors
        GLint success;
        GLchar info_log[512];
        glGetProgramiv(program_, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program_, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
        }
    }

    GLuint gl_ref()
    {
        return program_;
    }

    virtual ~Program()
    {
        if (program_ != 0) {
            glDeleteProgram(program_);
        }
    }

private:
    std::vector<Shader*> shaders_;
    GLuint program_;
};


class MapRenderer {
public:
    MapRenderer(const game::Map &map, glm::mat4x4 &ortho)
        : map_(map),
          ortho_(ortho),
          vertexShader_(vertexShaderSource),
          fragmentShader_(fragmentShaderSource),
          program_({&vertexShader_, &fragmentShader_})
    {
        glGenVertexArrays(1, &cellVao_);
        glGenBuffers(1, &cellVbo_);
        glBindVertexArray(cellVao_);

        glBindBuffer(GL_ARRAY_BUFFER, cellVbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void render()
    {
        GLint viewMatrixLocation = glGetUniformLocation(program_.gl_ref(), "viewMatrix");
        glUseProgram(program_.gl_ref());
        glBindVertexArray(cellVao_);
        for (int i=0; i<map_.row_count(); ++i) {
            for (int j=0; j<map_.col_count(); ++j) {
                glm::mat4 cellTranslate = glm::translate(glm::mat4(1.0f),
                                                         glm::vec3((j - (map_.col_count()-1)/2.0)*2,
                                                                   (i - (map_.row_count()-1)/2.0)*2,
                                                                   0));
                float scaleFactor = 1.0f / map_.row_count();
                glm::mat4 gridScale = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
                glm::mat4 viewMatrix = ortho_ * gridScale * cellTranslate;
                glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
        }
        glBindVertexArray(0);
    }

    ~MapRenderer()
    {
        glDeleteVertexArrays(1, &cellVao_);
        glDeleteBuffers(1, &cellVbo_);
    }

private:
    GLfloat vertices[18] = {
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -0.5f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f
    };

    VertexShader vertexShader_;
    FragmentShader fragmentShader_;
    Program program_;
    GLuint cellVbo_;
    GLuint cellVao_;
    glm::mat4x4 &ortho_;

    const game::Map map_;
};


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_state.width, window_state.height, "LearnOpenGL", nullptr, nullptr);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    glewInit();

    glViewport(0, 0, window_state.width, window_state.height);

    game::Map map(10);
    glm::mat4x4 ortho;
    MapRenderer renderer(map, ortho);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ortho = computeOrthoMatrix(window_state.width, window_state.height);
        renderer.render();

        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    window_state.width = width;
    window_state.height = height > 0 ? height : 1;
    glViewport(0, 0, window_state.width, window_state.height);
}
