//
// This file is used by the template to render a basic scene using GL.
//
#include "pch.h"

#include "SimpleRenderer.h"
#include "MathHelper.h"

// These are used by the shader compilation methods.
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, (GLsizei)infoLog.size(), NULL, infoLog.data());

        std::string errorMessage = std::string("Shader compilation failed: ");
        errorMessage += std::string(infoLog.begin(), infoLog.end()); 

        throw std::runtime_error(errorMessage.c_str());
    }

    return shader;
}

GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
    GLuint program = glCreateProgram();

    if (program == 0)
    {
        throw std::runtime_error("Program creation failed");
    }

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, (GLsizei)infoLog.size(), NULL, infoLog.data());

        std::string errorMessage = std::string("Program link failed: ");
        errorMessage += std::string(infoLog.begin(), infoLog.end()); 

        throw std::runtime_error(errorMessage.c_str());
    }

    return program;
}

SimpleRenderer::SimpleRenderer() :
    mWindowWidth(0),
    mWindowHeight(0),
    mDrawCount(0)
{
	
    // Vertex Shader source
	const std::string vs = R"(
        uniform mat4 uModelMatrix;
        uniform mat4 uViewMatrix;
        uniform mat4 uProjMatrix;
        attribute vec4 aPosition;
        attribute vec4 aColor;
		attribute vec2 aTexCoord;
		varying vec2 vTexCoord;
        varying vec4 vColor;
		varying vec3 positionWorld;
        varying vec3 normalWorld;
        void main()
        {
            gl_Position = uProjMatrix * uViewMatrix * uModelMatrix * aPosition;
			vec4 normal = vec4(aColor.x, aColor.y, aColor.z, 0.0);
			normalWorld = normalize(vec3(uModelMatrix * normal)); //normal = normalModel
			positionWorld = vec3(uModelMatrix * aPosition); //aPositon = positionModel
            vColor = aColor;
			vTexCoord = aTexCoord;
        }
    )";

    // Fragment Shader source
	const std::string fs = R"(
        precision mediump float;
		uniform sampler2D OurTexture;
		uniform sampler2D normalMap;
        uniform vec3 lightPosition;
		uniform vec3 eyePosition;
		uniform float ambient;
        varying vec4 vColor;
		varying vec2 vTexCoord;
		varying vec3 positionWorld;
        varying vec3 normalWorld;
        void main()
        {
			vec4 normalColor = texture2D(normalMap, vTexCoord);
			vec3 normal = vec3(normalColor.r*2.0-1.0,0.0,normalColor.g*2.0-1.0);

			vec3 r = lightPosition - positionWorld;
			vec3 lightVector = normalize(r);

			//specular:
			vec3 reflectedLightVector = -reflect(lightVector,normalize(normalWorld+normal));
			vec3 eyeDirection = normalize(eyePosition - positionWorld);
			float product = dot(reflectedLightVector, eyeDirection);
			float s = clamp(product,0.0,1.0);
			float len = length(r);
			float ss = pow(s,20.0)/len;
			vec3 light = vec3(ss, ss, ss) + vec3(ambient,ambient,ambient);
			//
            //gl_FragColor = vColor;
            gl_FragColor = texture2D(OurTexture, vTexCoord) * vec4(light.x,light.y,light.z, 1.0);
        }
    )";

    // Set up the shader and its uniform/attribute locations.
    mProgram = CompileProgram(vs, fs);
    mPositionAttribLocation = glGetAttribLocation(mProgram, "aPosition");
    mColorAttribLocation = glGetAttribLocation(mProgram, "aColor");
	mTextureAttribLocation = glGetAttribLocation(mProgram, "aTexCoord");
    mModelUniformLocation = glGetUniformLocation(mProgram, "uModelMatrix");
    mViewUniformLocation = glGetUniformLocation(mProgram, "uViewMatrix");
    mProjUniformLocation = glGetUniformLocation(mProgram, "uProjMatrix");
	lightPositionUniformLocation = glGetUniformLocation(mProgram, "lightPosition");
	eyePositionUniformLocation = glGetUniformLocation(mProgram, "eyePosition");
	ambientUniformLocation = glGetUniformLocation(mProgram, "ambient");

    // Then set up the cube geometry.
    GLfloat vertexPositions[] =
    {
		-1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		+1.0f, +1.0f, -1.0f,
		+1.0f, +1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, +1.0f, +1.0f,
		-1.0f, +1.0f, +1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, +1.0f,
		+1.0f, -1.0f, +1.0f,
    };

	glGenBuffers(1, &mVertexPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexPositions), vertexPositions, GL_STATIC_DRAW);

    GLfloat vertexColors[] =
    {
		+0.0f, +1.0f, +0.0f,
		+0.0f, +1.0f, +0.0f,
		+0.0f, +1.0f, +0.0f,
		+0.0f, +1.0f, +0.0f,
		+0.0f, +0.0f, -1.0f,
		+0.0f, +0.0f, -1.0f,
		+0.0f, +0.0f, -1.0f,
		+0.0f, +0.0f, -1.0f,
		+1.0f, +0.0f, +0.0f,
		+1.0f, +0.0f, +0.0f,
		+1.0f, +0.0f, +0.0f,
		+1.0f, +0.0f, +0.0f,
		-1.0f, +0.0f, +0.0f,
		-1.0f, +0.0f, +0.0f,
		-1.0f, +0.0f, +0.0f,
		-1.0f, +0.0f, +0.0f,
		+0.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f,
		+0.0f, +0.0f, +1.0f,
		+0.0f, -1.0f, +0.0f,
		+0.0f, -1.0f, +0.0f,
		+0.0f, -1.0f, +0.0f,
		+0.0f, -1.0f, +0.0f,
    };

    glGenBuffers(1, &mVertexColorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexColorBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);

	GLfloat textureCoords[] =
	{
		0.0f,1.0f,
		1.0f,1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};

	glGenBuffers(1, &mTextureCoordsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordsBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    short indices[] =
    {
		0, 1, 2, 0, 2, 3, // Top
		4, 5, 6, 4, 6, 7, // Front
		8, 9, 10, 8, 10, 11, // Right
		12, 13, 14, 12, 14, 15, // Left
		16, 17, 18, 16, 18, 19, // Back
		20, 22, 21, 20, 23, 22, // Bottom
    };

    glGenBuffers(1, &mIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

SimpleRenderer::~SimpleRenderer()
{
    if (mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }

    if (mVertexPositionBuffer != 0)
    {
        glDeleteBuffers(1, &mVertexPositionBuffer);
        mVertexPositionBuffer = 0;
    }

    if (mVertexColorBuffer != 0)
    {
        glDeleteBuffers(1, &mVertexColorBuffer);
        mVertexColorBuffer = 0;
    }

	if (mTextureCoordsBuffer != 0)
	{
		glDeleteBuffers(1, &mTextureCoordsBuffer);
		mTextureCoordsBuffer = 0;
	}

    if (mIndexBuffer != 0)
    {
        glDeleteBuffers(1, &mIndexBuffer);
        mIndexBuffer = 0;
    }
}

void SimpleRenderer::Draw()
{
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mProgram == 0)
        return;

    glUseProgram(mProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureVec[0]);
	glUniform1i(glGetUniformLocation(mProgram, "OurTexture"), 0);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, textureVec[1]);
	glUniform1i(glGetUniformLocation(mProgram, "normalMap"), 1);
	
    glBindBuffer(GL_ARRAY_BUFFER, mVertexPositionBuffer);
    glEnableVertexAttribArray(mPositionAttribLocation);
    glVertexAttribPointer(mPositionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexColorBuffer);
    glEnableVertexAttribArray(mColorAttribLocation);
    glVertexAttribPointer(mColorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, mTextureCoordsBuffer);
	glEnableVertexAttribArray(mTextureAttribLocation);
	glVertexAttribPointer(mTextureAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);

    MathHelper::Matrix4 modelMatrix = MathHelper::SimpleModelMatrix((float)mDrawCount / 50.0f);
    glUniformMatrix4fv(mModelUniformLocation, 1, GL_FALSE, &(modelMatrix.m[0][0]));

    MathHelper::Matrix4 viewMatrix = MathHelper::SimpleViewMatrix();
    glUniformMatrix4fv(mViewUniformLocation, 1, GL_FALSE, &(viewMatrix.m[0][0]));

    MathHelper::Matrix4 projectionMatrix = MathHelper::SimpleProjectionMatrix(float(mWindowWidth) / float(mWindowHeight));
    glUniformMatrix4fv(mProjUniformLocation, 1, GL_FALSE, &(projectionMatrix.m[0][0]));

	MathHelper::vec3 lightPosition = MathHelper::vector3(0.0f, 2.0f, 0.0f);
	glUniform3fv(lightPositionUniformLocation, 1, &(lightPosition.m[0]));

	MathHelper::vec3 eyePosition = MathHelper::vector3(0.0f, 4.33f, 2.5f);
	glUniform3fv(eyePositionUniformLocation, 1, &(eyePosition.m[0]));

	GLint ambientUniformLocation = glGetUniformLocation(mProgram, "ambient");
	float ambient = 0.3f;
	glUniform1fv(ambientUniformLocation, 1, &ambient);

    // Draw 36 indices: six faces, two triangles per face, 3 indices per triangle
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glDrawElements(GL_TRIANGLES, (6 * 2) * 3, GL_UNSIGNED_SHORT, 0);

    mDrawCount += 1;
}

void SimpleRenderer::UpdateWindowSize(GLsizei width, GLsizei height)
{
    glViewport(0, 0, width, height);
    mWindowWidth = width;
    mWindowHeight = height;
}

void SimpleRenderer::sendTextureToOpenGL(unsigned char* pdata, GLuint& textureId)
{
	//texture0:
	//int width, height;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//load texture from file:
	//unsigned char* image = SOIL_load_image("tiles3.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, pdata);
	//glGenerateMipmap(GL_TEXTURE_2D);
	//SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


}
