#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
in vec2 texcoord;

out vec2 vTex;

void main() {
    vTex = texcoord;
    // This line ensures the shader follows your ofTranslate/ofScale commands
    gl_Position = modelViewProjectionMatrix * position;
}
