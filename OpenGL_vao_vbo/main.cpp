#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define GLFW_INCLUDE_GLU
#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//ディレクトリの設定ファイル
#include "common.h"

static int WIN_WIDTH = 500;
static int WIN_HEIGHT = 500;
static const char *WIN_TITLE = "OpenGL Course";

static const double Pi = 4.0 * atan(1.0);

//shader file
static std::string VERT_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.vert";
static std::string FRAG_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.frag";

struct Vertex {
	Vertex(const glm::vec3 &position_, const glm::vec3 &color_)
		: position(position_)
		, color(color_) {
	}

	glm::vec3 position;
	glm::vec3 color;
};

static const glm::vec3 positions[12] = {
	glm::vec3(1, sqrt(3), (-3 - sqrt(5)) / 2),
	glm::vec3(-2, 0, (-3 - sqrt(5)) / 2),
	glm::vec3(1, -sqrt(3), (-3 - sqrt(5)) / 2),
	glm::vec3(-(1 + sqrt(5)) / 2,-(1 + sqrt(5))*sqrt(3) / 2,(1 - sqrt(5)) / 2),
	glm::vec3(1 + sqrt(5),0,(1 - sqrt(5)) / 2),
	glm::vec3(-(1 + sqrt(5)) / 2,(1 + sqrt(5))*sqrt(3) / 2,(1 - sqrt(5)) / 2),
	glm::vec3((1 + sqrt(5)) / 2,(1 + sqrt(5))*sqrt(3) / 2,(sqrt(5) - 1) / 2),
	glm::vec3(-1 - sqrt(5),0,(sqrt(5) - 1) / 2),
	glm::vec3((1 + sqrt(5)) / 2,-(1 + sqrt(5))*sqrt(3) / 2,(sqrt(5) - 1) / 2),
	glm::vec3(-1,-sqrt(3),(3 + sqrt(5)) / 2),
	glm::vec3(2,0,(3 + sqrt(5)) / 2),
	glm::vec3(-1,sqrt(3),(3 + sqrt(5)) / 2),
};

static const glm::vec3 colors[20] = {
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(0.5f, 0.0f, 0.0f),

	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 0.5f, 0.0f),

	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 0.0f, 0.5f),

	glm::vec3(1.0f, 1.0f, 0.0f),
	glm::vec3(0.5f, 1.0f, 0.0f),
	glm::vec3(1.0f, 0.5f, 0.0f),
	glm::vec3(0.5f, 0.5f, 0.0f),


	glm::vec3(0.0f, 1.0f, 1.0f),
	glm::vec3(0.0f, 0.5f, 1.0f),
	glm::vec3(0.0f, 1.0f, 0.5f),
	glm::vec3(0.0f, 0.5f, 0.5f),


	glm::vec3(1.0f, 0.0f, 1.0f),
	glm::vec3(0.5f, 0.0f, 1.0f),
	glm::vec3(1.0f, 0.0f, 0.5f),
	glm::vec3(0.5f, 0.0f, 0.5f),

	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(1.0f, 1.0f, 1.0f),
};

static const unsigned int faces[20][3] = {
	{ 0, 1, 2 },
	{ 0, 5, 1 },
	{ 1, 3, 2 },
	{ 2, 4, 0 },
	{ 0, 6, 5 },
	{ 0, 4, 6 },
	{ 2, 8, 4 },
	{ 2, 3, 8 },
	{ 1, 7, 3 },
	{ 1, 5, 7 },
	{ 11, 6, 5 },
	{ 11, 5, 7 },
	{ 9, 7, 3 },
	{ 9, 3, 8 },
	{ 10, 8, 4 },
	{ 10, 4, 6 },
	{ 10, 6, 11 },
	{ 11, 7, 9 },
	{ 9, 8, 10 },
	{ 9, 10, 11 },
};

GLuint vaoId;
GLuint vertexBufferId;
GLuint indexBufferId;

GLuint programId;

static float theta = 0.0f;

void initVAO() {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	int idx = 0;

	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 3; j++) {
			Vertex v(positions[faces[i][j]], colors[i]);
			vertices.push_back(v);
			indices.push_back(idx++);
		}
	}

	glGenVertexArrays(1, &vaoId);
	glBindVertexArray(vaoId);

	glGenBuffers(1, &vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);


	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	glGenBuffers(1, &indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* indices.size(), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}

GLuint compileShader(const std::string &filename, GLuint type) {
	//create shader
	GLuint shaderId = glCreateShader(type);

	//read file
	std::fstream reader;
	size_t codeSize;
	std::string code;

	//open file
	reader.open(filename.c_str(), std::ios::in);
	if (!reader.is_open()) {
		//output error if failed in opening file
		fprintf(stderr, "Failed to load a shader: %s\n", VERT_SHADER_FILE.c_str());
		exit(1);
	}

	//ファイルを全て読んで変数に格納
	reader.seekg(0, std::ios::end);		//ファイル読み取り位置を終端に移動
	codeSize = reader.tellg();			//現在の箇所（＝終端）の位置がファイルサイズ
	code.resize(codeSize);				//コードを格納する変数の大きさを設定
	reader.seekg(0);					//ファイルの読み取り位置を先頭に移動
	reader.read(&code[0], codeSize);	//先頭からファイルサイズ分を読んでコードの変数に格納

										//close file
	reader.close();

	//compile code
	const char *codeChars = code.c_str();
	glShaderSource(shaderId, 1, &codeChars, NULL);
	glCompileShader(shaderId);

	//judge whether compile succeeded
	GLint compileStatus;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE) {
		//コンパイルが失敗したらエラーとソースコードを表示して終了
		fprintf(stderr, "Failed to compile a shader!\n");

		//エラーメッセージの長さを取得する
		GLint logLength;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			//エラーメッセージを取得する
			GLsizei length;
			std::string errMsg;
			errMsg.resize(logLength);
			glGetShaderInfoLog(shaderId, logLength, &length, &errMsg[0]);

			//output of error message and source code
			fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
			fprintf(stderr, "%s\n", code.c_str());
		}
		exit(1);
	}

	return shaderId;
}

GLuint buildShaderProgram(const std::string &vShaderFile, const std::string &fShaderFile) {
	//create shader
	GLuint vertShaderId = compileShader(vShaderFile, GL_VERTEX_SHADER);
	GLuint fragShaderId = compileShader(fShaderFile, GL_FRAGMENT_SHADER);

	//シェーダプログラムのリンク
	GLuint programId = glCreateProgram();
	glAttachShader(programId, vertShaderId);
	glAttachShader(programId, fragShaderId);
	glLinkProgram(programId);

	//judge whether link succeeded
	GLint linkState;
	glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
	if (linkState == GL_FALSE) {
		//if failed outputs error message
		fprintf(stderr, "Failed to link shaders!\n");

		//エラーメッセージの長さを取得する
		GLint logLength;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
			//get error message
			GLsizei length;
			std::string errMsg;
			errMsg.resize(logLength);
			glGetProgramInfoLog(programId, logLength, &length, &errMsg[0]);

			//output error message
			fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
		}
		exit(1);
	}

	//シェーダを無効化した後にIDを返す
	glUseProgram(0);
	return programId;
}

void iniShaders() {
	programId = buildShaderProgram(VERT_SHADER_FILE, FRAG_SHADER_FILE);
}

void initializeGL() {
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.3f, 0.4f, 0.6f, 1.0f);

	//initialize VAO
	initVAO();

	//シェーダの用意
	iniShaders();


}

void paintGL() {
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	glm::mat4 projMat = glm::perspective(45.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

	glm::mat4 viewMat = glm::lookAt(glm::vec3(0.0f, 0.0f, 20.0f),   
						glm::vec3(0.0f, 0.0f, 0.0f),   
						glm::vec3(0.0f, 1.0f, 0.0f));  

	glm::mat4 modelMat = glm::rotate(theta, glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 mvpMat = projMat * viewMat * modelMat;

	glUseProgram(programId);

	GLuint mvpMatLocId = glGetUniformLocation(programId, "u_mvpMat");
	glUniformMatrix4fv(mvpMatLocId, 1, GL_FALSE, glm::value_ptr(mvpMat));

	glBindVertexArray(vaoId);

	glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);

	glUseProgram(0);
}

void resizeGL(GLFWwindow *window, int width, int height) {

	WIN_WIDTH = width;
	WIN_HEIGHT = height;

	glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);

	int renderBufferWidth, renderBufferHeight;
	glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);

	glViewport(0, 0, renderBufferWidth, renderBufferHeight);
}

void animate() {
	theta += 2.0f * Pi / 360.0f;
}


int main(int argc, char **argv) {
	if (glfwInit() == GL_FALSE) {
		fprintf(stderr, "Initialization failed!\n");
		return 1;
	}

	//OpenGLのバージョン設定
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE, NULL, NULL);

	if (window == NULL) {
		fprintf(stderr, "Window creation failed!\n");
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "GLEW initialization failed!");
		return 1;
	}

	initializeGL();

	glfwSetWindowSizeCallback(window, resizeGL);

	resizeGL(window, 1000.0f, 1000.0f);

	while (glfwWindowShouldClose(window) == GL_FALSE) {
		paintGL();

		animate();

		//描画用バッファの切り替え
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}


