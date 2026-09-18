#version 300 es
in vec2 aPosition;
in vec2 aTextureCoord;
out vec2 vTextureCoord;
void main() {
	vTextureCoord = aTextureCoord;
	gl_Position = vec4(aPosition, 0.0, 1.0);
}