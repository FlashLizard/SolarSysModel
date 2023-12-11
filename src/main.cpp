#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include <stb/stb_image.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <vmath.h>


class Image {
public:
    unsigned char *data;
    int width, height, nrChannels;
    Image(std::string path) {
        data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    }
};

// 窗口大小参数
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float aspact = (float)4.0 / (float)3.0;

// 旋转参数
static GLfloat xRot = 20.0f;
static GLfloat xRot3 = 20.0f;
static GLfloat yRot = 20.0f;

static GLfloat vXRot = 0.0f;
static GLfloat vYRot = 0.0f;

// 句柄参数
GLuint vertex_array_object;   // == VAO句柄
GLuint vertex_buffer_object;  // == VBO句柄
GLuint element_buffer_object; //==EBO句柄

// 球的数据参数
const int X_SEGMENTS = 20;
const int Y_SEGMENTS = 20;
const GLfloat PI = 3.14159265358979323846f;

// 贴图
Image *earthImg;
Image *moonImg;
Image *sunImg;

void genSphere(float radio, int xSegment, int ySegment, bool uv, std::vector<float> &sphereVertices, std::vector<int> &sphereIndices)
{
    // 进行球体顶点和三角面片的计算
    //  生成球的顶点
    for (int y = 0; y < ySegment; y++)
    {
        for (int x = 0; x < xSegment; x++)
        {
            float xi = (float)x / (float)xSegment;
            float yi = (float)y / (float)ySegment;
            float theta = yi * PI;
            float phi = xi * 2 * PI;
            float xPos = radio * std::sin(theta) * std::cos(phi);
            float yPos = radio * std::cos(theta);
            float zPos = radio * std::sin(theta) * std::sin(phi);

            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
            if (uv)
            {
                float u = phi / (2 * PI);
                float v = theta / PI;
                sphereVertices.push_back(u);
                sphereVertices.push_back(v);
            }
        }
    }

    // 生成球的顶点
    for (int i = 0; i < ySegment; i++)
    {
        for (int j = 0; j < xSegment; j++)
        {

            sphereIndices.push_back(i * (xSegment) + j);
            sphereIndices.push_back((i + 1) * (xSegment) + j);
            sphereIndices.push_back((i + 1) * (xSegment) + j + 1);

            sphereIndices.push_back(i * (xSegment) + j);
            sphereIndices.push_back((i + 1) * (xSegment) + j + 1);
            sphereIndices.push_back(i * (xSegment) + j + 1);
        }
    }
}

Shader initial(void)
{
    std::vector<float> sphereVertices;
    std::vector<int> sphereIndices;
    genSphere(1.0, X_SEGMENTS, Y_SEGMENTS, true, sphereVertices, sphereIndices);
    // 球
    glGenVertexArrays(1, &vertex_array_object);
    glGenBuffers(1, &vertex_buffer_object);
    // 生成并绑定球体的VAO和VBO
    glBindVertexArray(vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    // 将顶点数据绑定至当前默认的缓冲中
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &element_buffer_object);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // 解绑VAO和VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 顶点着色器和片段着色器源码
    const char *vertexShader = "shader/shader.vs";
    const char *fragmentShader = "shader/shader.fs";

    // 生成并编译着色器
    Shader shaderProgram(vertexShader, fragmentShader);
    shaderProgram.use();
    // 设定点线面的属性
    glPointSize(15); // 设置点的大小
    glLineWidth(5);  // 设置线宽
    // 启动剔除操作
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // 开启深度测试
    glEnable(GL_DEPTH_TEST);

    // 图片
    earthImg = new Image("res/earth.jpg");
    sunImg = new Image("res/sun.jpg");
    moonImg = new Image("res/moon.jpg");

    return shaderProgram;
}

void Draw(Shader shaderProgram)
{
    // 清空颜色缓冲和深度缓冲区
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vmath::mat4 vTrans = vmath::rotate(vXRot, vmath::vec3(1.0, 0.0, 0.0)) *
                         vmath::rotate(vYRot, vmath::vec3(0.0, 1.0, 0.0)) * vmath::translate(0.0f, 0.0f, -5.0f);
    // 处理图形的旋转
    vmath::mat4 trans = vmath::perspective(60, aspact, 1.0f, 500.0f) * vTrans;
    unsigned int transformLoc = glGetUniformLocation(shaderProgram.ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans);

    // 处理图形的颜色
    GLfloat vColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    unsigned int colorLoc = glGetUniformLocation(shaderProgram.ID, "color");
    glUniform4fv(colorLoc, 1, vColor);

    // 绘制第一个红色的球
    glBindVertexArray(vertex_array_object); // 绑定VAO
    glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

    // 绘制第二个黑色的球
    xRot += (float)0.05f;
    vmath::mat4 earthTrans = vmath::rotate(xRot, vmath::vec3(0.0, 1.0, 0.0)) *
                             vmath::translate(3.0f, 0.0f, 0.0f) * vmath::scale(0.3f);
    // vmath::vec4 earthPos = vmath::vec4(0.0f, 0.0f, 0.0f, 1.0f)* earthTrans;
    vmath::vec4 oriPos(0.0f, 0.0f, 0.0f, 1.0f);
    vmath::vec4 earthPos(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            earthPos[j] += earthTrans[i][j] * oriPos[i];
        }
    }
    vmath::mat4 trans2 = vmath::perspective(60, aspact, 1.0f, 500.0f) * vTrans * earthTrans;
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans2);
    GLfloat vColor2[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    glUniform4fv(colorLoc, 1, vColor2);
    glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

    // 绘制第三个蓝色的球
    xRot3 += (float)0.35f;
    vmath::mat4 eclipticRot = vmath::rotate(23.5f, vmath::vec3(0.0, 0.0, 1.0));
    vmath::mat4 negEclipticRot = vmath::rotate(-23.5f, vmath::vec3(0.0, 0.0, 1.0));
    vmath::vec4 earthAix4 = vmath::vec4(0.0, 1.0, 0.0, 0.0) * eclipticRot;
    vmath::vec3 earthAix3 = vmath::vec3(earthAix4[0], earthAix4[1], earthAix4[2]);
    vmath::mat4 earthPosTrans = vmath::translate(earthPos[0], earthPos[1], earthPos[2]);

    vmath::mat4 trans3 = vmath::perspective(60, aspact, 1.0f, 500.0f) * vTrans * earthPosTrans * vmath::rotate(xRot3, earthAix3) * negEclipticRot * vmath::translate(0.5f, 0.0f, 0.0f) * vmath::scale(0.1f);
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans3);
    GLfloat vColor3[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    glUniform4fv(colorLoc, 1, vColor3);
    glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void reshaper(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    if (height == 0)
    {
        aspact = (float)width;
    }
    else
    {
        aspact = (float)width / (float)height;
    }
}

void run(GLFWwindow *window, float fps)
{
    Shader shaderProgram = initial(); // 初始化
    float interval = 1000 / fps;
    auto startTime = std::chrono::high_resolution_clock::now();
    // 窗口大小改变时调用reshaper函数
    glfwSetFramebufferSizeCallback(window, reshaper);
    while (!glfwWindowShouldClose(window))
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto accTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        if (accTime.count() < interval)
            continue;
        Draw(shaderProgram);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 解绑和删除VAO和VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteVertexArrays(1, &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer_object);
}

int main()
{
    glfwInit(); // 初始化GLFW
    // OpenGL版本为3.3，主次版本号均设为3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    // 使用核心模式(无需向后兼容性)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 创建窗口(宽、高、窗口名称)
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sphere", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to Create OpenGL Context" << std::endl;
        glfwTerminate();
        return -1;
    }
    // 将窗口的上下文设置为当前线程的主上下文
    glfwMakeContextCurrent(window);
    // 初始化GLAD，加载OpenGL函数指针地址的函数
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    run(window, 30);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
