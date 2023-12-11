#version 330 core
layout(location = 0) in vec3 vPos;           // 位置变量的属性位置值为0
layout(location = 1) in vec2 aTexCoord;
out vec4 vColor;           // 位置变量的属性位置值为0
out vec2 TexCoord;
uniform mat4 transform;
uniform vec4 color;
void main() {
    gl_Position = transform * vec4(vPos, 1.0);
    vColor = color;
    TexCoord = aTexCoord;
}