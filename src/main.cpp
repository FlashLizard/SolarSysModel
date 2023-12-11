#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include <stb/stb_image.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

class Image
{
public:
    unsigned char *data;
    int width, height, nrChannels;
    Image(std::string path)
    {
        data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    }
    void Print() {
        printf("Tex %d,%d,%d\ndata:",width,height,nrChannels);
        for(int i=0;i<std::min(width*height,50);i++) {
            printf("%d,",data[i]);
        }
        if(width*height>=50) {
            printf("...\n");
        } else {
            printf("\n");
        }
    }
    ~Image() {
        stbi_image_free(data);
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
int ball_size;

// background
GLuint back_vao;
GLuint back_vbo;
GLuint back_ebo;
int back_size;

// 球的数据参数
const int X_SEGMENTS = 50;
const int Y_SEGMENTS = 50;
const GLfloat PI = 3.14159265358979323846f;

// 贴图
Image *earthImg;
Image *moonImg;
Image *sunImg;
Image *backImg;

unsigned int earthTex;
unsigned int sunTex;
unsigned int moonTex;
unsigned int backTex;

void genSphere(float radius, int xSegment, int ySegment, bool uv, std::vector<float> &sphereVertices, std::vector<int> &sphereIndices)
{
    // 进行球体顶点和三角面片的计算
    //  生成球的顶点

    for (int y = 0; y < ySegment; y++)
    {
        for (int x = 0; x < xSegment; x++)
        {
            float xi = (float)x / (float)(xSegment-1);
            float yi = (float)y / (float)(ySegment-1);
            float theta = yi * PI;
            float phi = xi * 2 * PI;
            float xPos = radius * std::sin(theta) * std::cos(phi);
            float yPos = radius * std::cos(theta);
            float zPos = radius * std::sin(theta) * std::sin(phi);

            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
            if (uv)
            {
                float u = xi;
                float v = 1.0 - yi;
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

void genTex(unsigned int *id, Image *img) {

    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    // 为当前绑定的纹理对象设置环绕、过滤方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 img->data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

Shader initial(void)
{
    
    stbi_set_flip_vertically_on_load(true);
    // tell stb_image.h to flip loaded texture's on the y-axis.
    // 图片
    earthImg = new Image("res/earth.jpg");
    sunImg = new Image("res/sun.jpg");
    moonImg = new Image("res/moon.jpg");
    backImg = new Image("res/background.jpg");
    //backImg->Print();
    
    // 图片
    genTex(&earthTex,earthImg);
    genTex(&sunTex,sunImg);
    genTex(&moonTex,moonImg);
    genTex(&backTex,backImg);

    
    // 球
    std::vector<float> sphereVertices;
    std::vector<int> sphereIndices;
    genSphere(1.0, X_SEGMENTS, Y_SEGMENTS, true, sphereVertices, sphereIndices);
    ball_size = sphereIndices.size();
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    // 解绑VAO和VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 背景
    float bWidth,bHeight,bDeep;
    bWidth = 1;
    bHeight = 1;
    bDeep = 0.99;
    std::vector<float> backVertices = {
        -1, 1, bDeep, 0,1,
        1, 1, bDeep, 1,1,
        -1, -1, bDeep, 0,0,
        1, -1, bDeep, 1,0
    };
    std::vector<int> backIndices = {
        0, 2, 1,
        3, 1, 2
    };
    back_size = backIndices.size();
    glGenVertexArrays(1, &back_vao);
    glGenBuffers(1, &back_vbo);

    glBindVertexArray(back_vao);
    glBindBuffer(GL_ARRAY_BUFFER, back_vbo);
    // 将顶点数据绑定至当前默认的缓冲中
    glBufferData(GL_ARRAY_BUFFER, backVertices.size() * sizeof(float), &backVertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &back_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, back_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, backIndices.size() * sizeof(int), &backIndices[0], GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
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

    return shaderProgram;
}

void Draw(Shader shaderProgram)
{
    // 清空颜色缓冲和深度缓冲区
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 view(1.0f);
    glm::mat4 one(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
    view = glm::rotate(view, glm::radians(vYRot), glm::vec3(0.0, 1.0, 0.0));
    view = glm::rotate(view, glm::radians(vXRot), glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspact, 1.0f, 500.0f);

    shaderProgram.setMatrix4fv("view",glm::value_ptr(view));
    shaderProgram.setMatrix4fv("projection",glm::value_ptr(projection));

    shaderProgram.setInt("ourTexture",0);

    // 贴图
    glBindTexture(GL_TEXTURE_2D, sunTex);

    // 绘制第一个红色的球
    shaderProgram.setMatrix4fv("model",glm::value_ptr(one));
    glBindVertexArray(vertex_array_object); // 绑定VAO
    glDrawElements(GL_TRIANGLES, ball_size, GL_UNSIGNED_INT, 0);

    // 绘制第二个黑色的球
    xRot += (float)0.05f;
    glm::mat4 earthTrans(1.0f);
    earthTrans = glm::rotate(earthTrans, glm::radians(xRot), glm::vec3(0.0, 1.0, 0.0));
    earthTrans = glm::translate(earthTrans, glm::vec3(3.0f, 0.0f, 0.0f));
    earthTrans = glm::scale(earthTrans, glm::vec3(0.3f, 0.3f, 0.3f));
    glm::vec4 oriPos(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 earthPos = earthTrans * oriPos;
    
    shaderProgram.setMatrix4fv("model",glm::value_ptr(earthTrans));
    
    // 贴图
    glBindTexture(GL_TEXTURE_2D, earthTex);
    glDrawElements(GL_TRIANGLES, ball_size, GL_UNSIGNED_INT, 0);



    // 绘制第三个蓝色的球
    xRot3 += (float)0.35f;
    glm::mat4 eclipticRot = glm::rotate(glm::mat4(1.0f), glm::radians(23.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 negEclipticRot = glm::rotate(glm::mat4(1.0f), glm::radians(-23.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec4 earthAix4 = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * eclipticRot;
    glm::vec3 earthAix3 = glm::vec3(earthAix4);
    glm::mat4 earthPosTrans = glm::translate(glm::mat4(1.0f), glm::vec3(earthPos));
    glm::mat4 moonTrans = earthPosTrans * glm::rotate(glm::mat4(1.0f), glm::radians(xRot3), earthAix3) * negEclipticRot * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

    shaderProgram.setMatrix4fv("model",glm::value_ptr(moonTrans));
    
    // 贴图
    glBindTexture(GL_TEXTURE_2D, moonTex);
    glDrawElements(GL_TRIANGLES, ball_size, GL_UNSIGNED_INT, 0);
    
    // back

    glBindVertexArray(back_vao); // 绑定VAO

    shaderProgram.setMatrix4fv("model",glm::value_ptr(one));
    shaderProgram.setMatrix4fv("view",glm::value_ptr(one));
    shaderProgram.setMatrix4fv("projection",glm::value_ptr(one));

    // 贴图
    glBindTexture(GL_TEXTURE_2D, backTex);
    glDrawElements(GL_TRIANGLES, back_size, GL_UNSIGNED_INT, 0);
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
    delete earthImg;
    delete sunImg;
    delete moonImg;
    delete backImg;
    return 0;
}
