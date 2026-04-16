uniform mat4 Proj;
uniform mat4 Model;
uniform mat4 View;

layout(location = 0) in vec3 Position;
layout(location = 3) in vec2 in_textureCoord;

out vec2 textureCoords;

void main() {
    gl_Position = Proj * View * Model * vec4(Position, 1.0);
    textureCoords = in_textureCoord;
}
