#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include<string.h>
#include<iostream>

#include<stdlib.h>

GLuint p[65536];
void init() {
	srand(GetTickCount());
	for (int i = 0; i < 65536; i++) {
		p[i] = rand();
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

}

static void windowsize_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}
int main() {
	GLFWwindow* window;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	window = glfwCreateWindow(512, 512, "fractal noise", NULL, NULL);
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
	glGenBuffers(1, &vbo);
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
		"uniform int t;\n"
		"layout(std430, binding = 0)buffer ssbo\n"
		"{\n"
		"uint data_SSBO[256];\n"
		"};\n"

		"float originColor(ivec3 coord){\n"
		"int x = coord.x;\n"
		"int y = coord.y;\n"
		"int z = coord.z;\n"
		"float color;\n"
		"color  = ( data_SSBO[(data_SSBO[(x*256 + y)%65536] + z)%65536]%256)/256.0;\n"
		"return color;\n"
		"}\n"

		"float sectionColor(int x,int y,int t1) {\n"
		"	float color = 0;"
		"int l = 32;\n"
		"float f = 0.5;\n"
		"while (l > 1) {\n"
		"	int x1 = x - (x%l);\n"
		"	int y1 = y - (y%l);\n"
		"	int x2 = (x1 + l)%256;\n"
		"	int y2 = (y1 + l)%256;\n"
		"	float yl = (y%l)/float(l)\n;"
		"	float xl = (x%l)/float(l)\n;"
		"	float temp1 = originColor(ivec3(x1, y1, t1));\n"
		"	float temp2 = originColor(ivec3(x1, y2, t1));\n"
		"	float temp3 = originColor(ivec3(x2, y1, t1));\n"
		"	float temp4 = originColor(ivec3(x2, y2, t1));\n"
		"	float temp5 = temp1*(1-yl) + temp2*yl;\n"
		"	float temp6 = temp3*(1-yl) + temp4*yl;\n"
		"	color += (temp5*(1-xl) + temp6*xl)*f;\n"
		"	l = l/2;\n"
		"	f = f/2.0;\n"
		"}\n"
		"	return color;\n"
		"}\n"

		"vec4 fractalColor(vec2 coord){\n"
		"int x = int(coord.x*256.0)%256;\n"
		"int y = int(coord.y*256.0)%256;\n"
		"int speed = 2048;\n"
		"float color = 0;"
		"float f = 0.5;\n"
		"while (speed > 128) {\n"
		"	float tl = (t%speed)/float(speed);\n"
			"color += ((1-tl) * sectionColor(x,y,t/speed) + tl*sectionColor(x,y,t/speed+1))*f;\n"
			"speed = speed/2;\n"
			"f = f/2;\n"
		"}\n"	

		"return vec4(color,color,color,1);\n"
		"}\n"
		"void main(void){\n"
		"gl_FragColor = fractalColor(vTexCoord);\n"
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
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(p), p, GL_STATIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);


	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glEnableVertexAttribArray(0);
		glDisable(GL_CULL_FACE);
		glUniform1i(glGetUniformLocation(shader, "s"), 0);
		glUniform1i(glGetUniformLocation(shader, "t"), GetTickCount());
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		glfwSwapBuffers(window);
		glfwPollEvents();

	}
	glfwTerminate();
}
