/*

	Copyright 2016 Chen Xi

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <assert.h>

#include <math.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "texture.h"

using namespace std;

#define WIDTH 800
#define HEIGHT 600
#define OUT_SIZE 1920*1080*4
#define OUT_IMAGE "../Image/out.png"

GLuint VBO;
GLuint IBO;
GLuint FBO;

GLuint bfShaderProgram;
GLuint edgeShaderProgram;
GLuint nmsShaderProgram;
GLuint edgeSampler;
GLuint nmsSampler;
GLuint bfSampler0;
GLuint bfSampler1;
Texture* pTexture = NULL;
Texture* pEdgeTexture = NULL;
GLfloat size[2];
GLuint sketchSize;

GLfloat out[OUT_SIZE];

const char* pVsFileName = "../Shader/shader.vs";
const char* pBfFileName = "../Shader/bfilter.fs";
const char* pNmsFileName = "../Shader/nms.fs";
const char* pEdgeFileName = "../Shader/edge.fs";

bool ReadFile(const char* pFileName, string& outFile)
{
    ifstream f(pFileName);
    
    bool ret = false;
    
    if (f.is_open()) {
        string line;
        while (getline(f, line)) {
            outFile.append(line);
            outFile.append("\n");
        }
        
        f.close();
        
        ret = true;
    }
    else {
        fprintf(stderr, "File Not Found!\n");
    }
    
    return ret;
}

static void Save()
{
	glReadPixels((WIDTH-size[0])/2,(HEIGHT-size[1])/2, size[0], size[1], GL_RGBA, GL_FLOAT, out); 	
	
	Magick::Image m = Magick::Image(size[0], size[1], "RGBA", MagickCore::FloatPixel, out);
	m.flip();
	m.write(OUT_IMAGE);
}

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,size[0],0,size[1]);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport((WIDTH-size[0])/2,(HEIGHT-size[1])/2,size[0],size[1]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, (GLvoid*)12);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	pTexture->Bind(GL_TEXTURE0);
	pEdgeTexture->Bind(GL_TEXTURE1);
	
	// Sober Edge Filter 
	glUseProgram(edgeShaderProgram);
	glUniform1i(edgeSampler, 0);
	glUniform2fv(sketchSize, 1, size);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// NMS Filter
	glUseProgram(nmsShaderProgram);
	glUniform1i(nmsSampler, 1);
	glUniform2fv(sketchSize, 1, size);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// Bilateral Filter
	glUseProgram(bfShaderProgram);
	glUniform1i(bfSampler0, 0);
	glUniform1i(bfSampler1, 1);
	glUniform2fv(sketchSize, 1, size);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glutSwapBuffers();
	Save();
}

static void InitializeGlutCallbacks()
{
	glutDisplayFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{
	GLfloat Vertices[] = {
	//  color				position     		
		-1.0f,  1.0f, 0.0f,  0.0f,  1.0f,
		1.0f,  1.0f, 0.0f,  1.0f,  1.0f,
		1.0f, -1.0f, 0.0f,  1.0f,  0.0f,
		-1.0f, -1.0f, 0.0f,  0.0f,  0.0f,
	};

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
	GLuint Indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

static void CreateFramebuffer()
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pEdgeTexture->getHandler(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
		exit(1);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0]= strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);
	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		exit(1);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

static GLuint CompileShaders(const char *pVSFileName, const char *pFSFileName)
{
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
		exit(1);
	}

	string vs, fs;

	if (!ReadFile(pVSFileName, vs)) {
		exit(1);
	};

	if (!ReadFile(pFSFileName, fs)) {
		exit(1);
	};

	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	return ShaderProgram;
}

static void InitShaders()
{
	bfShaderProgram = CompileShaders(pVsFileName, pBfFileName);
	bfSampler0 = glGetUniformLocation(bfShaderProgram, "bfSampler0");
	bfSampler1 = glGetUniformLocation(bfShaderProgram, "bfSampler1");
	sketchSize = glGetUniformLocation(bfShaderProgram, "sketchSize");
	assert(bfSampler0 != 0xFFFFFFFF);
	assert(bfSampler1 != 0xFFFFFFFF);
	assert(sketchSize != 0xFFFFFFFF);


	edgeShaderProgram = CompileShaders(pVsFileName, pEdgeFileName);
	edgeSampler = glGetUniformLocation(edgeShaderProgram, "edgeSampler");
	sketchSize = glGetUniformLocation(edgeShaderProgram, "sketchSize");
	assert(edgeSampler != 0xFFFFFFFF);
	assert(sketchSize != 0xFFFFFFFF);


	nmsShaderProgram = CompileShaders(pVsFileName, pNmsFileName);
	nmsSampler = glGetUniformLocation(nmsShaderProgram, "nmsSampler");
	sketchSize = glGetUniformLocation(nmsShaderProgram, "sketchSize");
	assert(nmsSampler != 0xFFFFFFFF);
	assert(sketchSize != 0xFFFFFFFF);
}

static bool InitTextures(const char *textureName)
{
	pTexture = new Texture(GL_TEXTURE_2D, textureName);

	if (!pTexture->Load()) {
		return -1;
	}

	size[0] = pTexture->Width();
	size[1] = pTexture->Height();

	pEdgeTexture = new Texture(GL_TEXTURE_2D, "");
	pEdgeTexture->Init(size[0], size[1]);
	return 0;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Error: Please input the image path\n");
		return -1;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Opengl Filter");
	InitializeGlutCallbacks();

	// Must be done after glut is initialized!
	GLenum res = glewInit();
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	printf("GL version: %s\n", glGetString(GL_VERSION));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	if (InitTextures(argv[1]) != 0)
		fprintf(stderr, "Error: Texture initialization failed\n");

	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateFramebuffer();

	InitShaders();
	glutMainLoop();

	return 0;
}
