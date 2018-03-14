#include<glad/glad.h>
#include<glfw/glfw3.h>
#include<string.h>
#include<iostream>
unsigned char p[256];
int temp[256];
void init() {
	int A = 777;
	int B = 3337;
	int M = 8192;
	p[0] = 0;
	temp[0] = 0;
	for (int i = 1; i < 256; i++) {
		temp[i] = ((temp[i - 1] * A + B+i) % M);
		
		p[i] = temp[i] % 256;
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

}

static void windowsize_callback(GLFWwindow* window, int width, int height) {
	glViewport(0,0,width, height);
}
int main() {
	GLFWwindow* window;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(512,512, "white noise", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSetKeyCallback(window, key_callback);

	glfwSetWindowSizeCallback(window, windowsize_callback);
	glfwSwapInterval(1);
	glClearColor(0.5, 0.5, 0.5, 1);

	GLfloat vertexData[16] = {
		-1.0,-1.0,0.0,0.0,
		1.0,-1.0,1.0,0.0,
		1.0,1.0,1.0,1.0,
		-1.0,1.0,0.0,1.0
	};
	GLuint vao, vbo, shader, vs, fs;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1,&vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_READ);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	shader = glCreateProgram();
	vs = glCreateShader(GL_VERTEX_SHADER);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	const char* vss = "#version 430\n"
		"in vec4 vVertex;\n"
		"out vec2 vTexCoord;\n"
		"void main(void){\n"
		"gl_Position = vec4(vVertex.xy,vec2(0.0,1.0));\n"
		"vTexCoord = vVertex.zw;\n"
		"}";
	int l = strlen(vss);
	glShaderSource(vs, 1, &vss, &l);
	glCompileShader(vs);

	const char* fss = "#version 430\n"
		"in vec2 vTexCoord;\n"
		"layout(std430, binding = 0)buffer ssbo\n"
		"{\n"
		"uint data_SSBO[256];\n"
		"};\n"
		"vec2 originColor(vec2 coord){\n"
		"int i=int(coord.x*255.0)%256;\n"
		"int y = int(coord.y*255.0)%256;"
		"float red,green;\n"
		"red  = ((data_SSBO[(data_SSBO[i]+y)%256])%256)/256.0;\n"
		"green  = ((data_SSBO[(data_SSBO[y]+i)%256])%256)/256.0;\n"
		"return vec2(red,green);\n"
		"}\n"
		"void main(void){\n"
		"gl_FragColor = vec4(originColor(vTexCoord),0,1);\n"
		"}";
	l = strlen(fss);
	glShaderSource(fs, 1, &fss, &l);
	glCompileShader(fs);

	glAttachShader(shader, vs);
	glAttachShader(shader, fs);
	glLinkProgram(shader);
	glUseProgram(shader);
	
	init();


	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(temp), temp, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

	
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glEnableVertexAttribArray(0);
		glDisable(GL_CULL_FACE);
		glUniform1i(glGetUniformLocation(shader, "s"), 0);

		glDrawArrays(GL_TRIANGLE_FAN,0,4);
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	glfwTerminate();
}
