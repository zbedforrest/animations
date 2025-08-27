#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture0;

void main()
{
    if (TexCoord.y < 0.01) {
        FragColor = texture(texture0, TexCoord);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
