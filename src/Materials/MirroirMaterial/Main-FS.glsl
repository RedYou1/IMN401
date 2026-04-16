in vec2 textureCoords;
layout(location = 0) out vec4 Color;
uniform sampler2D mirroirTextureSampler;
uniform float mirrorRendering;

void main() {
    Color = 0.9 * texture(mirroirTextureSampler, textureCoords);
}
