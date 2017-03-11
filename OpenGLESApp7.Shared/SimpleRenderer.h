#pragma once

#include <string.h>
#include <vector>

#ifdef __APPLE__
#include <unistd.h>
#include <sys/resource.h>

#include <OpenGLES/ES2/gl.h>
#else // __ANDROID__ or _WIN32
#include <GLES2/gl2.h>
#endif

class SimpleRenderer
{
public:
    SimpleRenderer();
    ~SimpleRenderer();
    void Draw();
    void UpdateWindowSize(GLsizei width, GLsizei height);
	void sendTextureToOpenGL(unsigned char* pdata, GLuint& textureId);
	std::vector<GLuint> textureVec;

private:
	
    GLuint mProgram;
    GLsizei mWindowWidth;
    GLsizei mWindowHeight;

    GLint mPositionAttribLocation;
    GLint mColorAttribLocation;
	GLint mTextureAttribLocation;

    GLint mModelUniformLocation;
    GLint mViewUniformLocation;
    GLint mProjUniformLocation;
	GLint lightPositionUniformLocation;
	GLint eyePositionUniformLocation;
	GLint ambientUniformLocation;

    GLuint mVertexPositionBuffer;
    GLuint mVertexColorBuffer;
	GLuint mTextureCoordsBuffer;
    GLuint mIndexBuffer;

    int mDrawCount;

	
};
