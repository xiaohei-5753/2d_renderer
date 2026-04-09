#ifndef __glad_gl_h_
#define __glad_gl_h_

/* Minimal OpenGL 3.3 Core Profile header for GLAD */

#ifdef __cplusplus
extern "C" {
#endif

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

/* OpenGL 3.3 Core function pointers */
/* These will be loaded at runtime by GLAD */

extern void (*glActiveTexture)(GLenum texture);
extern void (*glAttachShader)(GLuint program, GLuint shader);
extern void (*glBindBuffer)(GLenum target, GLuint buffer);
extern void (*glBindTexture)(GLenum target, GLuint texture);
extern void (*glBindVertexArray)(GLuint array);
extern void (*glBufferData)(GLenum target, GLsizei size, const void *data, GLenum usage);
extern void (*glClear)(GLbitfield mask);
extern void (*glClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (*glCompileShader)(GLuint shader);
extern GLuint (*glCreateProgram)(void);
extern GLuint (*glCreateShader)(GLenum type);
extern void (*glDeleteBuffers)(GLsizei n, const GLuint *buffers);
extern void (*glDeleteProgram)(GLuint program);
extern void (*glDeleteShader)(GLuint shader);
extern void (*glDeleteTextures)(GLsizei n, const GLuint *textures);
extern void (*glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
extern void (*glDrawArrays)(GLenum mode, GLint first, GLsizei count);
extern void (*glEnable)(GLenum cap);
extern void (*glGenBuffers)(GLsizei n, GLuint *buffers);
extern void (*glGenTextures)(GLsizei n, GLuint *textures);
extern void (*glGenVertexArrays)(GLsizei n, GLuint *arrays);
extern GLint (*glGetAttribLocation)(GLuint program, const char *name);
extern void (*glGetIntegerv)(GLenum pname, GLint *data);
extern GLint (*glGetUniformLocation)(GLuint program, const char *name);
extern void (*glLinkProgram)(GLuint program);
extern void (*glShaderSource)(GLuint shader, GLsizei count, const char *const*string, const GLint *length);
extern void (*glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void (*glTexParameteri)(GLenum target, GLenum pname, GLint param);
extern void (*glUniform1f)(GLint location, GLfloat v0);
extern void (*glUniform1i)(GLint location, GLint v0);
extern void (*glUniform2f)(GLint location, GLfloat v0, GLfloat v1);
extern void (*glUniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void (*glUniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUseProgram)(GLuint program);
extern void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void (*glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
extern void (*glEnableVertexAttribArray)(GLuint index);
extern void (*glDisableVertexAttribArray)(GLuint index);
extern const GLubyte* (*glGetString)(GLenum name);

/* GLAD loader */
typedef void* (*GLADglproc)(const char* name);
int gladLoadGL(GLADglproc loader);
void gladUnloadGL(void);

#ifdef __cplusplus
}
#endif

#endif /* __glad_gl_h_ */