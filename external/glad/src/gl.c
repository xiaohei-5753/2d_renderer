#include "glad/gl.h"
#include <string.h>

/* Global function pointers - initially NULL */
/* These will be populated by gladLoadGL */

void (*glActiveTexture)(GLenum) = NULL;
void (*glAttachShader)(GLuint, GLuint) = NULL;
void (*glBindBuffer)(GLenum, GLuint) = NULL;
void (*glBindTexture)(GLenum, GLuint) = NULL;
void (*glBindVertexArray)(GLuint) = NULL;
void (*glBufferData)(GLenum, GLsizei, const void*, GLenum) = NULL;
void (*glClear)(GLbitfield) = NULL;
void (*glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (*glCompileShader)(GLuint) = NULL;
GLuint (*glCreateProgram)(void) = NULL;
GLuint (*glCreateShader)(GLenum) = NULL;
void (*glDeleteBuffers)(GLsizei, const GLuint*) = NULL;
void (*glDeleteProgram)(GLuint) = NULL;
void (*glDeleteShader)(GLuint) = NULL;
void (*glDeleteTextures)(GLsizei, const GLuint*) = NULL;
void (*glDeleteVertexArrays)(GLsizei, const GLuint*) = NULL;
void (*glDrawArrays)(GLenum, GLint, GLsizei) = NULL;
void (*glEnable)(GLenum) = NULL;
void (*glGenBuffers)(GLsizei, GLuint*) = NULL;
void (*glGenTextures)(GLsizei, GLuint*) = NULL;
void (*glGenVertexArrays)(GLsizei, GLuint*) = NULL;
GLint (*glGetAttribLocation)(GLuint, const char*) = NULL;
void (*glGetIntegerv)(GLenum, GLint*) = NULL;
GLint (*glGetUniformLocation)(GLuint, const char*) = NULL;
void (*glLinkProgram)(GLuint) = NULL;
void (*glShaderSource)(GLuint, GLsizei, const char*const*, const GLint*) = NULL;
void (*glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*) = NULL;
void (*glTexParameteri)(GLenum, GLenum, GLint) = NULL;
void (*glUniform1f)(GLint, GLfloat) = NULL;
void (*glUniform1i)(GLint, GLint) = NULL;
void (*glUniform2f)(GLint, GLfloat, GLfloat) = NULL;
void (*glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = NULL;
void (*glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (*glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat*) = NULL;
void (*glUseProgram)(GLuint) = NULL;
void (*glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) = NULL;
void (*glViewport)(GLint, GLint, GLsizei, GLsizei) = NULL;
void (*glEnableVertexAttribArray)(GLuint) = NULL;
void (*glDisableVertexAttribArray)(GLuint) = NULL;
const GLubyte* (*glGetString)(GLenum) = NULL;

/* Simple loader that uses platform-specific APIs */
#ifdef _WIN32
#include <windows.h>
static void* get_proc_address(const char* name) {
    return (void*)wglGetProcAddress(name);
}
#else
/* Linux/macOS would use glXGetProcAddress or NSGLGetProcAddress */
static void* get_proc_address(const char* name) {
    return NULL;
}
#endif

int gladLoadGL(GLADglproc loader) {
    GLADglproc load = loader ? loader : get_proc_address;
    
    /* Load function pointers */
    glActiveTexture = (void (*)(GLenum))load("glActiveTexture");
    glAttachShader = (void (*)(GLuint, GLuint))load("glAttachShader");
    glBindBuffer = (void (*)(GLenum, GLuint))load("glBindBuffer");
    glBindTexture = (void (*)(GLenum, GLuint))load("glBindTexture");
    glBindVertexArray = (void (*)(GLuint))load("glBindVertexArray");
    glBufferData = (void (*)(GLenum, GLsizei, const void*, GLenum))load("glBufferData");
    glClear = (void (*)(GLbitfield))load("glClear");
    glClearColor = (void (*)(GLfloat, GLfloat, GLfloat, GLfloat))load("glClearColor");
    glCompileShader = (void (*)(GLuint))load("glCompileShader");
    glCreateProgram = (GLuint (*)(void))load("glCreateProgram");
    glCreateShader = (GLuint (*)(GLenum))load("glCreateShader");
    glDeleteBuffers = (void (*)(GLsizei, const GLuint*))load("glDeleteBuffers");
    glDeleteProgram = (void (*)(GLuint))load("glDeleteProgram");
    glDeleteShader = (void (*)(GLuint))load("glDeleteShader");
    glDeleteTextures = (void (*)(GLsizei, const GLuint*))load("glDeleteTextures");
    glDeleteVertexArrays = (void (*)(GLsizei, const GLuint*))load("glDeleteVertexArrays");
    glDrawArrays = (void (*)(GLenum, GLint, GLsizei))load("glDrawArrays");
    glEnable = (void (*)(GLenum))load("glEnable");
    glGenBuffers = (void (*)(GLsizei, GLuint*))load("glGenBuffers");
    glGenTextures = (void (*)(GLsizei, GLuint*))load("glGenTextures");
    glGenVertexArrays = (void (*)(GLsizei, GLuint*))load("glGenVertexArrays");
    glGetAttribLocation = (GLint (*)(GLuint, const char*))load("glGetAttribLocation");
    glGetIntegerv = (void (*)(GLenum, GLint*))load("glGetIntegerv");
    glGetUniformLocation = (GLint (*)(GLuint, const char*))load("glGetUniformLocation");
    glLinkProgram = (void (*)(GLuint))load("glLinkProgram");
    glShaderSource = (void (*)(GLuint, GLsizei, const char*const*, const GLint*))load("glShaderSource");
    glTexImage2D = (void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*))load("glTexImage2D");
    glTexParameteri = (void (*)(GLenum, GLenum, GLint))load("glTexParameteri");
    glUniform1f = (void (*)(GLint, GLfloat))load("glUniform1f");
    glUniform1i = (void (*)(GLint, GLint))load("glUniform1i");
    glUniform2f = (void (*)(GLint, GLfloat, GLfloat))load("glUniform2f");
    glUniform3f = (void (*)(GLint, GLfloat, GLfloat, GLfloat))load("glUniform3f");
    glUniform4f = (void (*)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))load("glUniform4f");
    glUniformMatrix4fv = (void (*)(GLint, GLsizei, GLboolean, const GLfloat*))load("glUniformMatrix4fv");
    glUseProgram = (void (*)(GLuint))load("glUseProgram");
    glVertexAttribPointer = (void (*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*))load("glVertexAttribPointer");
    glViewport = (void (*)(GLint, GLint, GLsizei, GLsizei))load("glViewport");
    glEnableVertexAttribArray = (void (*)(GLuint))load("glEnableVertexAttribArray");
    glDisableVertexAttribArray = (void (*)(GLuint))load("glDisableVertexAttribArray");
    glGetString = (const GLubyte* (*)(GLenum))load("glGetString");
    
    /* Check if essential functions were loaded */
    if (!glCreateProgram || !glCreateShader || !glShaderSource || 
        !glCompileShader || !glAttachShader || !glLinkProgram ||
        !glUseProgram || !glGetUniformLocation || !glUniform1i ||
        !glGenVertexArrays || !glBindVertexArray || !glGenBuffers ||
        !glBindBuffer || !glBufferData || !glVertexAttribPointer ||
        !glEnableVertexAttribArray || !glDrawArrays || !glClear ||
        !glClearColor || !glViewport || !glGenTextures ||
        !glBindTexture || !glTexImage2D || !glTexParameteri) {
        return 0; /* Failed to load essential functions */
    }
    
    return 1; /* Success */
}

void gladUnloadGL(void) {
    /* Reset all function pointers */
    glActiveTexture = NULL;
    glAttachShader = NULL;
    glBindBuffer = NULL;
    glBindTexture = NULL;
    glBindVertexArray = NULL;
    glBufferData = NULL;
    glClear = NULL;
    glClearColor = NULL;
    glCompileShader = NULL;
    glCreateProgram = NULL;
    glCreateShader = NULL;
    glDeleteBuffers = NULL;
    glDeleteProgram = NULL;
    glDeleteShader = NULL;
    glDeleteTextures = NULL;
    glDeleteVertexArrays = NULL;
    glDrawArrays = NULL;
    glEnable = NULL;
    glGenBuffers = NULL;
    glGenTextures = NULL;
    glGenVertexArrays = NULL;
    glGetAttribLocation = NULL;
    glGetIntegerv = NULL;
    glGetUniformLocation = NULL;
    glLinkProgram = NULL;
    glShaderSource = NULL;
    glTexImage2D = NULL;
    glTexParameteri = NULL;
    glUniform1f = NULL;
    glUniform1i = NULL;
    glUniform2f = NULL;
    glUniform3f = NULL;
    glUniform4f = NULL;
    glUniformMatrix4fv = NULL;
    glUseProgram = NULL;
    glVertexAttribPointer = NULL;
    glViewport = NULL;
    glEnableVertexAttribArray = NULL;
    glDisableVertexAttribArray = NULL;
    glGetString = NULL;
}