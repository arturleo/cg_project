#version 330 core

uniform vec3 lcolor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(lcolor,1.0);
}