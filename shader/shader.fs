#version 330 core    
in vec2 TexCoord;
out vec4 FragColor;   

uniform sampler2D ourTexture;

void main()
{
    //FragColor = vColor;
    FragColor = texture2D(ourTexture, TexCoord);
    //FragColor = vec4(TexCoord.xyy,1.0);
}