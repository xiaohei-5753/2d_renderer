#ifndef __glad_gl_simple_h_
#define __glad_gl_simple_h_

#ifdef __cplusplus
extern "C" {
#endif

/* OpenGL 3.3 Core Profile Function Declarations */
/* This is a simplified header that assumes functions are loaded at runtime */

/* Basic GL types */
typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef long long GLintptr;
typedef long long GLsizeiptr;

/* Function declarations - these will be loaded at runtime */
void glActiveTexture(GLenum texture);
void glAttachShader(GLuint program, GLuint shader);
void glBindBuffer(GLenum target, GLuint buffer);
void glBindTexture(GLenum target, GLuint texture);
void glBindVertexArray(GLuint array);
void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void glClear(GLbitfield mask);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glCompileShader(GLuint shader);
GLuint glCreateProgram(void);
GLuint glCreateShader(GLenum type);
void glDeleteBuffers(GLsizei n, const GLuint *buffers);
void glDeleteProgram(GLuint program);
void glDeleteShader(GLuint shader);
void glDeleteTextures(GLsizei n, const GLuint *textures);
void glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glEnable(GLenum cap);
void glGenBuffers(GLsizei n, GLuint *buffers);
void glGenTextures(GLsizei n, GLuint *textures);
void glGenVertexArrays(GLsizei n, GLuint *arrays);
GLint glGetAttribLocation(GLuint program, const char *name);
void glGetIntegerv(GLenum pname, GLint *data);
GLint glGetUniformLocation(GLuint program, const char *name);
void glLinkProgram(GLuint program);
void glShaderSource(GLuint shader, GLsizei count, const char *const*string, const GLint *length);
void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
void glUniform1f(GLint location, GLfloat v0);
void glUniform1i(GLint location, GLint v0);
void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUseProgram(GLuint program);
void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
void glEnableVertexAttribArray(GLuint index);
void glDisableVertexAttribArray(GLuint index);
void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
void glGenerateMipmap(GLenum target);
const GLubyte* glGetString(GLenum name);
void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, char *infoLog);
void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, char *infoLog);

/* GLAD loader function */
typedef void* (*GLADglproc)(const char* name);
int gladLoadGL(GLADglproc loader);
void gladUnloadGL(void);

#ifdef __cplusplus
}
#endif

#endif /* __glad_gl_simple_h_ */