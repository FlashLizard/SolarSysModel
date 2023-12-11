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

struct DotLight
{
    glm::vec3 pos;
    glm::vec3 color;
};

DotLight sunLight = {
    glm::vec3(0, 0, 0),
    glm::vec3(1, 1, 1)};

class Image
{
public:
    unsigned char *data;
    int width, height, nrChannels;
    Image(std::string path)
    {
        data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    }
    void Print()
    {
        printf("Tex %d,%d,%d\ndata:", width, height, nrChannels);
        for (int i = 0; i < std::min(width * height, 50); i++)
        {
            printf("%d,", data[i]);
        }
        if (width * height >= 50)
        {
            printf("...\n");
        }
        else
        {
            printf("\n");
        }
    }
    ~Image()
    {
        stbi_image_free(data);
    }
};

// 窗口大小参数
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int fullWidth;
int fullHeight;
bool fullWin = false;
float aspect = (float)4.0 / (float)3.0;

// 旋转参数
static GLfloat earthRot = 20.0f;
static GLfloat moonRot = 20.0f;
static GLfloat selfRot = 0.0f;

// 句柄参数
GLuint ballVAO; // == VAO句柄
GLuint ballVBO; // == VBO句柄
GLuint ballEBO; //==EBO句柄
int ballSize;

// background
GLuint backVAO;
GLuint backVBO;
GLuint backEBO;
int backSize;

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

// 相机控制相关
bool firstMouse = true;
float lastX, lastY;
float deltaTime = 0;
float speed = 0.3;
float yaw = 90, pitch = 0;
glm::vec3 viewPos(0.0f, 0.0f, -5.0f);
glm::vec3 cameraFront(0, 0, 1.0);
glm::vec3 cameraRight(-1, 0, 0);
glm::vec3 cameraUp(0, 1, 0);

bool pause = false;

void genSphere(float radius, int xSegment, int ySegment, bool uv, std::vector<float> &sphereVertices, std::vector<int> &sphereIndices)
{
    // 进行球体顶点和三角面片的计算
    //  生成球的顶点

    for (int y = 0; y < ySegment; y++)
    {
        for (int x = 0; x < xSegment; x++)
        {
            float xi = (float)x / (float)(xSegment - 1);
            float yi = (float)y / (float)(ySegment - 1);
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

void genTex(unsigned int *id, Image *img)
{

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
    // backImg->Print();

    // 图片
    genTex(&earthTex, earthImg);
    genTex(&sunTex, sunImg);
    genTex(&moonTex, moonImg);
    genTex(&backTex, backImg);

    // 球vao设置
    std::vector<float> sphereVertices;
    std::vector<int> sphereIndices;
    genSphere(1.0, X_SEGMENTS, Y_SEGMENTS, true, sphereVertices, sphereIndices);
    ballSize = sphereIndices.size();
    glGenVertexArrays(1, &ballVAO);
    glGenBuffers(1, &ballVBO);
    // 生成并绑定球体的VAO和VBO
    glBindVertexArray(ballVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ballVBO);
    // 将顶点数据绑定至当前默认的缓冲中
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &ballEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    // 解绑VAO和VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 背景vao设置
    float bWidth, bHeight, bDeep;
    bWidth = 1;
    bHeight = 1;
    bDeep = 0.99;
    std::vector<float> backVertices = {
        -1, 1, bDeep, 0, 1,
        1, 1, bDeep, 1, 1,
        -1, -1, bDeep, 0, 0,
        1, -1, bDeep, 1, 0};
    std::vector<int> backIndices = {
        0, 2, 1,
        3, 1, 2};
    backSize = backIndices.size();
    glGenVertexArrays(1, &backVAO);
    glGenBuffers(1, &backVBO);

    glBindVertexArray(backVAO);
    glBindBuffer(GL_ARRAY_BUFFER, backVBO);
    // 将顶点数据绑定至当前默认的缓冲中
    glBufferData(GL_ARRAY_BUFFER, backVertices.size() * sizeof(float), &backVertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &backEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backEBO);
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
    // 改变球体角度
    if (!pause)
    {
        earthRot += (float)0.004f;
        selfRot += (float)0.3f;
        moonRot += (float)0.1f;
    }

    // 清空颜色缓冲和深度缓冲区
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 view(1.0f);
    glm::mat4 one(1.0f);
    view = glm::lookAt(viewPos, viewPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 500.0f);

    shaderProgram.setMatrix4fv("view", glm::value_ptr(view));
    shaderProgram.setMatrix4fv("projection", glm::value_ptr(projection));
    shaderProgram.setInt("ourTexture", 0);
    shaderProgram.setVec3("viewPos", viewPos);

    // 太阳光源
    shaderProgram.setVec3("lightPos", sunLight.pos);
    shaderProgram.setVec3("lightColor", sunLight.color);

    // 贴图
    glBindTexture(GL_TEXTURE_2D, sunTex);

    // 当前不是背景
    shaderProgram.setInt("background", 0);
    // 当前是太阳
    shaderProgram.setBool("sun", true);

    // 绘制sun
    shaderProgram.setMatrix4fv("model", glm::value_ptr(one));
    glBindVertexArray(ballVAO); // 绑定VAO
    glDrawElements(GL_TRIANGLES, ballSize, GL_UNSIGNED_INT, 0);

    shaderProgram.setBool("sun", false);

    // 绘制earth
    glm::mat4 earthTrans(1.0f);
    earthTrans = glm::rotate(earthTrans, glm::radians(earthRot), glm::vec3(0.0, 1.0, 0.0));
    earthTrans = glm::translate(earthTrans, glm::vec3(3.0f, 0.0f, 0.0f));
    earthTrans = glm::scale(earthTrans, glm::vec3(0.3f, 0.3f, 0.3f));
    earthTrans = glm::rotate(earthTrans, glm::radians(selfRot), glm::vec3(0.0, 1.0, 0.0));
    glm::vec4 oriPos(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 earthPos = earthTrans * oriPos;

    shaderProgram.setMatrix4fv("model", glm::value_ptr(earthTrans));

    // 贴图
    glBindTexture(GL_TEXTURE_2D, earthTex);
    glDrawElements(GL_TRIANGLES, ballSize, GL_UNSIGNED_INT, 0);

    // 绘制moon
    glm::mat4 eclipticRot = glm::rotate(glm::mat4(1.0f), glm::radians(23.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 negEclipticRot = glm::rotate(glm::mat4(1.0f), glm::radians(-23.5f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::vec4 earthAix4 = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f) * eclipticRot;
    glm::vec3 earthAix3 = glm::vec3(earthAix4);
    glm::mat4 earthPosTrans = glm::translate(glm::mat4(1.0f), glm::vec3(earthPos));
    glm::mat4 moonTrans = earthPosTrans * glm::rotate(glm::mat4(1.0f), glm::radians(moonRot), earthAix3) * negEclipticRot * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));

    shaderProgram.setMatrix4fv("model", glm::value_ptr(moonTrans));

    // 贴图
    glBindTexture(GL_TEXTURE_2D, moonTex);
    glDrawElements(GL_TRIANGLES, ballSize, GL_UNSIGNED_INT, 0);

    // 绘制背景
    glBindVertexArray(backVAO); // 绑定VAO

    shaderProgram.setInt("background", 1);

    shaderProgram.setMatrix4fv("model", glm::value_ptr(one));
    shaderProgram.setMatrix4fv("view", glm::value_ptr(one));
    shaderProgram.setMatrix4fv("projection", glm::value_ptr(one));

    // 贴图
    glBindTexture(GL_TEXTURE_2D, backTex);
    glDrawElements(GL_TRIANGLES, backSize, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void reshaper(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    if (height == 0)
    {
        aspect = (float)width;
    }
    else
    {
        aspect = (float)width / (float)height;
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        glfwSetCursorPos(window, 400, 300);
        xpos = 400;
        ypos = 300;
        lastX = 400;
        lastY = 300;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    float sensitivity = 0.01;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
    cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0, 1, 0))); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    // 重设鼠标位置
    glfwSetCursorPos(window, 400, 300);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        viewPos += speed * cameraFront * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        viewPos -= speed * cameraFront * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        viewPos -= speed * cameraRight * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        viewPos += speed * cameraRight * deltaTime;
    // 重置镜头
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    {
        yaw = 90;
        pitch = 0;
        viewPos = glm::vec3(0.0f, 0.0f, -5.0f);
        cameraFront = glm::vec3(0, 0, 1.0);
        cameraRight = glm::vec3(1, 0, 0);
        cameraUp = glm::vec3(0, 1, 0);
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        fullWin = !fullWin;
        if (fullWin)
        {

            glfwSetWindowSize(window, fullWidth, fullHeight);
        }
        else
        {
            glfwSetWindowSize(window, SCR_WIDTH, SCR_WIDTH);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        pause = !pause;
    }
}

void run(GLFWwindow *window, float fps)
{
    Shader shaderProgram = initial(); // 初始化
    float interval = 1000 / fps;
    auto startTime = std::chrono::high_resolution_clock::now();
    float lastFrame = static_cast<float>(glfwGetTime());
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    // 窗口大小改变时调用reshaper函数
    glfwSetFramebufferSizeCallback(window, reshaper);

    // 获取主监视器的分辨率
    GLFWmonitor *primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primaryMonitor);
    fullHeight = mode->height;
    fullWidth = mode->width;

    while (!glfwWindowShouldClose(window))
    {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto accTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        // if (accTime.count() < interval)
        //     continue;
        processInput(window);
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        Draw(shaderProgram);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // 解绑和删除VAO和VBO
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteVertexArrays(1, &ballVAO);
    glDeleteBuffers(1, &ballVBO);
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
