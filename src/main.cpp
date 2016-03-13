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
    "layout (location = 1) in vec3 color;\n"
    "uniform mat4 viewMatrix;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "gl_Position = viewMatrix * vec4(position, 1.0);\n"
    "ourColor = color;\n"
    "}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
    "in vec3 ourColor;\n"
    "out vec4 color;\n"
    "void main()\n"
    "{\n"
    "color = vec4(ourColor, 1.0f);\n"
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

    virtual ~Shader() {
        if (shader_ != 0) {
            glDeleteShader(shader_);
        }
    }

    GLuint gl_ref() const {
        return shader_;
    }

protected:
    Shader(ShaderType shader_type, const GLchar* shader_source_): shader_(0)
    {
        shader_source_ = shader_source_;
        shader_type_ = shader_type;

        std::cout << "creating shader" << shader_type << std::endl;

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
    VertexShader(const GLchar* shader_source)
        :Shader(ShaderType::Vertex, shader_source)
    {
    }
};

class FragmentShader: public Shader {
public:
    FragmentShader(const GLchar* shader_source)
        :Shader(ShaderType::Fragment, shader_source)
    {
    }
};

class Program {
public:
    Program(const std::vector<Shader*> &shaders)
        : shaders_(shaders), program_(0)
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
    const std::vector<Shader*> &shaders_;
    GLuint program_;
};

class MapRenderer {
public:
    MapRenderer(const game::Map &map) {
    }

private:
    std::vector<GLfloat> vertexData_;
};

// The MAIN function, from here we start the application and run the game loop
int main()
{
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFWwindow object that we can use for GLFW's functions
    GLFWwindow* window = glfwCreateWindow(window_state.width, window_state.height, "LearnOpenGL", nullptr, nullptr);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwMakeContextCurrent(window);

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    glewInit();

    // Define the viewport dimensions
    glViewport(0, 0, window_state.width, window_state.height);

    VertexShader vertex_shader(vertexShaderSource);
    FragmentShader fragment_shader(fragmentShaderSource);
    Program shader_program({&vertex_shader, &fragment_shader});

    // Set up vertex data (and buffer(s)) and attribute pointers
    GLfloat vertices[] = {
        // Positions         // Colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom Right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // Bottom Left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // Top
    };
    game::Map map(10);
    MapRenderer renderer(map);
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO


    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glm::mat4x4 ortho = computeOrthoMatrix(window_state.width, window_state.height);
        GLint viewMatrixLocation = glGetUniformLocation(shader_program.gl_ref(), "viewMatrix");
        glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(ortho));
        // Draw the triangle
        glUseProgram(shader_program.gl_ref());
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }
    // Properly de-allocate all resources once they've outlived their purpose
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
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
