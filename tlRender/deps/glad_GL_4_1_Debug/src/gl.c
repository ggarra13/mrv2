/**
 * SPDX-License-Identifier: (WTFPL OR CC0-1.0) AND Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/gl.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */

#ifdef __cplusplus
extern "C" {
#endif



int GLAD_GL_VERSION_1_0 = 0;
int GLAD_GL_VERSION_1_1 = 0;
int GLAD_GL_VERSION_1_2 = 0;
int GLAD_GL_VERSION_1_3 = 0;
int GLAD_GL_VERSION_1_4 = 0;
int GLAD_GL_VERSION_1_5 = 0;
int GLAD_GL_VERSION_2_0 = 0;
int GLAD_GL_VERSION_2_1 = 0;
int GLAD_GL_VERSION_3_0 = 0;
int GLAD_GL_VERSION_3_1 = 0;
int GLAD_GL_VERSION_3_2 = 0;
int GLAD_GL_VERSION_3_3 = 0;
int GLAD_GL_VERSION_4_0 = 0;
int GLAD_GL_VERSION_4_1 = 0;
int GLAD_GL_KHR_debug = 0;


static void _pre_call_gl_callback_default(const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLAD_UNUSED(len_args);

    if (apiproc == NULL) {
        fprintf(stderr, "GLAD: ERROR %s is NULL!\n", name);
        return;
    }
    if (glad_glGetError == NULL) {
        fprintf(stderr, "GLAD: ERROR glGetError is NULL!\n");
        return;
    }

    (void) glad_glGetError();
}
static void _post_call_gl_callback_default(void *ret, const char *name, GLADapiproc apiproc, int len_args, ...) {
    GLenum error_code;

    GLAD_UNUSED(ret);
    GLAD_UNUSED(apiproc);
    GLAD_UNUSED(len_args);

    error_code = glad_glGetError();

    if (error_code != GL_NO_ERROR) {
        fprintf(stderr, "GLAD: ERROR %d in %s!\n", error_code, name);
    }
}

static GLADprecallback _pre_call_gl_callback = _pre_call_gl_callback_default;
void gladSetGLPreCallback(GLADprecallback cb) {
    _pre_call_gl_callback = cb;
}
static GLADpostcallback _post_call_gl_callback = _post_call_gl_callback_default;
void gladSetGLPostCallback(GLADpostcallback cb) {
    _post_call_gl_callback = cb;
}

PFNGLACTIVESHADERPROGRAMPROC glad_glActiveShaderProgram = NULL;
static void GLAD_API_PTR glad_debug_impl_glActiveShaderProgram(GLuint pipeline, GLuint program) {
    _pre_call_gl_callback("glActiveShaderProgram", (GLADapiproc) glad_glActiveShaderProgram, 2, pipeline, program);
    glad_glActiveShaderProgram(pipeline, program);
    _post_call_gl_callback(NULL, "glActiveShaderProgram", (GLADapiproc) glad_glActiveShaderProgram, 2, pipeline, program);
    
}
PFNGLACTIVESHADERPROGRAMPROC glad_debug_glActiveShaderProgram = glad_debug_impl_glActiveShaderProgram;
PFNGLACTIVETEXTUREPROC glad_glActiveTexture = NULL;
static void GLAD_API_PTR glad_debug_impl_glActiveTexture(GLenum texture) {
    _pre_call_gl_callback("glActiveTexture", (GLADapiproc) glad_glActiveTexture, 1, texture);
    glad_glActiveTexture(texture);
    _post_call_gl_callback(NULL, "glActiveTexture", (GLADapiproc) glad_glActiveTexture, 1, texture);
    
}
PFNGLACTIVETEXTUREPROC glad_debug_glActiveTexture = glad_debug_impl_glActiveTexture;
PFNGLATTACHSHADERPROC glad_glAttachShader = NULL;
static void GLAD_API_PTR glad_debug_impl_glAttachShader(GLuint program, GLuint shader) {
    _pre_call_gl_callback("glAttachShader", (GLADapiproc) glad_glAttachShader, 2, program, shader);
    glad_glAttachShader(program, shader);
    _post_call_gl_callback(NULL, "glAttachShader", (GLADapiproc) glad_glAttachShader, 2, program, shader);
    
}
PFNGLATTACHSHADERPROC glad_debug_glAttachShader = glad_debug_impl_glAttachShader;
PFNGLBEGINCONDITIONALRENDERPROC glad_glBeginConditionalRender = NULL;
static void GLAD_API_PTR glad_debug_impl_glBeginConditionalRender(GLuint id, GLenum mode) {
    _pre_call_gl_callback("glBeginConditionalRender", (GLADapiproc) glad_glBeginConditionalRender, 2, id, mode);
    glad_glBeginConditionalRender(id, mode);
    _post_call_gl_callback(NULL, "glBeginConditionalRender", (GLADapiproc) glad_glBeginConditionalRender, 2, id, mode);
    
}
PFNGLBEGINCONDITIONALRENDERPROC glad_debug_glBeginConditionalRender = glad_debug_impl_glBeginConditionalRender;
PFNGLBEGINQUERYPROC glad_glBeginQuery = NULL;
static void GLAD_API_PTR glad_debug_impl_glBeginQuery(GLenum target, GLuint id) {
    _pre_call_gl_callback("glBeginQuery", (GLADapiproc) glad_glBeginQuery, 2, target, id);
    glad_glBeginQuery(target, id);
    _post_call_gl_callback(NULL, "glBeginQuery", (GLADapiproc) glad_glBeginQuery, 2, target, id);
    
}
PFNGLBEGINQUERYPROC glad_debug_glBeginQuery = glad_debug_impl_glBeginQuery;
PFNGLBEGINQUERYINDEXEDPROC glad_glBeginQueryIndexed = NULL;
static void GLAD_API_PTR glad_debug_impl_glBeginQueryIndexed(GLenum target, GLuint index, GLuint id) {
    _pre_call_gl_callback("glBeginQueryIndexed", (GLADapiproc) glad_glBeginQueryIndexed, 3, target, index, id);
    glad_glBeginQueryIndexed(target, index, id);
    _post_call_gl_callback(NULL, "glBeginQueryIndexed", (GLADapiproc) glad_glBeginQueryIndexed, 3, target, index, id);
    
}
PFNGLBEGINQUERYINDEXEDPROC glad_debug_glBeginQueryIndexed = glad_debug_impl_glBeginQueryIndexed;
PFNGLBEGINTRANSFORMFEEDBACKPROC glad_glBeginTransformFeedback = NULL;
static void GLAD_API_PTR glad_debug_impl_glBeginTransformFeedback(GLenum primitiveMode) {
    _pre_call_gl_callback("glBeginTransformFeedback", (GLADapiproc) glad_glBeginTransformFeedback, 1, primitiveMode);
    glad_glBeginTransformFeedback(primitiveMode);
    _post_call_gl_callback(NULL, "glBeginTransformFeedback", (GLADapiproc) glad_glBeginTransformFeedback, 1, primitiveMode);
    
}
PFNGLBEGINTRANSFORMFEEDBACKPROC glad_debug_glBeginTransformFeedback = glad_debug_impl_glBeginTransformFeedback;
PFNGLBINDATTRIBLOCATIONPROC glad_glBindAttribLocation = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindAttribLocation(GLuint program, GLuint index, const GLchar * name) {
    _pre_call_gl_callback("glBindAttribLocation", (GLADapiproc) glad_glBindAttribLocation, 3, program, index, name);
    glad_glBindAttribLocation(program, index, name);
    _post_call_gl_callback(NULL, "glBindAttribLocation", (GLADapiproc) glad_glBindAttribLocation, 3, program, index, name);
    
}
PFNGLBINDATTRIBLOCATIONPROC glad_debug_glBindAttribLocation = glad_debug_impl_glBindAttribLocation;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindBuffer(GLenum target, GLuint buffer) {
    _pre_call_gl_callback("glBindBuffer", (GLADapiproc) glad_glBindBuffer, 2, target, buffer);
    glad_glBindBuffer(target, buffer);
    _post_call_gl_callback(NULL, "glBindBuffer", (GLADapiproc) glad_glBindBuffer, 2, target, buffer);
    
}
PFNGLBINDBUFFERPROC glad_debug_glBindBuffer = glad_debug_impl_glBindBuffer;
PFNGLBINDBUFFERBASEPROC glad_glBindBufferBase = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindBufferBase(GLenum target, GLuint index, GLuint buffer) {
    _pre_call_gl_callback("glBindBufferBase", (GLADapiproc) glad_glBindBufferBase, 3, target, index, buffer);
    glad_glBindBufferBase(target, index, buffer);
    _post_call_gl_callback(NULL, "glBindBufferBase", (GLADapiproc) glad_glBindBufferBase, 3, target, index, buffer);
    
}
PFNGLBINDBUFFERBASEPROC glad_debug_glBindBufferBase = glad_debug_impl_glBindBufferBase;
PFNGLBINDBUFFERRANGEPROC glad_glBindBufferRange = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size) {
    _pre_call_gl_callback("glBindBufferRange", (GLADapiproc) glad_glBindBufferRange, 5, target, index, buffer, offset, size);
    glad_glBindBufferRange(target, index, buffer, offset, size);
    _post_call_gl_callback(NULL, "glBindBufferRange", (GLADapiproc) glad_glBindBufferRange, 5, target, index, buffer, offset, size);
    
}
PFNGLBINDBUFFERRANGEPROC glad_debug_glBindBufferRange = glad_debug_impl_glBindBufferRange;
PFNGLBINDFRAGDATALOCATIONPROC glad_glBindFragDataLocation = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindFragDataLocation(GLuint program, GLuint color, const GLchar * name) {
    _pre_call_gl_callback("glBindFragDataLocation", (GLADapiproc) glad_glBindFragDataLocation, 3, program, color, name);
    glad_glBindFragDataLocation(program, color, name);
    _post_call_gl_callback(NULL, "glBindFragDataLocation", (GLADapiproc) glad_glBindFragDataLocation, 3, program, color, name);
    
}
PFNGLBINDFRAGDATALOCATIONPROC glad_debug_glBindFragDataLocation = glad_debug_impl_glBindFragDataLocation;
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glad_glBindFragDataLocationIndexed = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar * name) {
    _pre_call_gl_callback("glBindFragDataLocationIndexed", (GLADapiproc) glad_glBindFragDataLocationIndexed, 4, program, colorNumber, index, name);
    glad_glBindFragDataLocationIndexed(program, colorNumber, index, name);
    _post_call_gl_callback(NULL, "glBindFragDataLocationIndexed", (GLADapiproc) glad_glBindFragDataLocationIndexed, 4, program, colorNumber, index, name);
    
}
PFNGLBINDFRAGDATALOCATIONINDEXEDPROC glad_debug_glBindFragDataLocationIndexed = glad_debug_impl_glBindFragDataLocationIndexed;
PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindFramebuffer(GLenum target, GLuint framebuffer) {
    _pre_call_gl_callback("glBindFramebuffer", (GLADapiproc) glad_glBindFramebuffer, 2, target, framebuffer);
    glad_glBindFramebuffer(target, framebuffer);
    _post_call_gl_callback(NULL, "glBindFramebuffer", (GLADapiproc) glad_glBindFramebuffer, 2, target, framebuffer);
    
}
PFNGLBINDFRAMEBUFFERPROC glad_debug_glBindFramebuffer = glad_debug_impl_glBindFramebuffer;
PFNGLBINDPROGRAMPIPELINEPROC glad_glBindProgramPipeline = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindProgramPipeline(GLuint pipeline) {
    _pre_call_gl_callback("glBindProgramPipeline", (GLADapiproc) glad_glBindProgramPipeline, 1, pipeline);
    glad_glBindProgramPipeline(pipeline);
    _post_call_gl_callback(NULL, "glBindProgramPipeline", (GLADapiproc) glad_glBindProgramPipeline, 1, pipeline);
    
}
PFNGLBINDPROGRAMPIPELINEPROC glad_debug_glBindProgramPipeline = glad_debug_impl_glBindProgramPipeline;
PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindRenderbuffer(GLenum target, GLuint renderbuffer) {
    _pre_call_gl_callback("glBindRenderbuffer", (GLADapiproc) glad_glBindRenderbuffer, 2, target, renderbuffer);
    glad_glBindRenderbuffer(target, renderbuffer);
    _post_call_gl_callback(NULL, "glBindRenderbuffer", (GLADapiproc) glad_glBindRenderbuffer, 2, target, renderbuffer);
    
}
PFNGLBINDRENDERBUFFERPROC glad_debug_glBindRenderbuffer = glad_debug_impl_glBindRenderbuffer;
PFNGLBINDSAMPLERPROC glad_glBindSampler = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindSampler(GLuint unit, GLuint sampler) {
    _pre_call_gl_callback("glBindSampler", (GLADapiproc) glad_glBindSampler, 2, unit, sampler);
    glad_glBindSampler(unit, sampler);
    _post_call_gl_callback(NULL, "glBindSampler", (GLADapiproc) glad_glBindSampler, 2, unit, sampler);
    
}
PFNGLBINDSAMPLERPROC glad_debug_glBindSampler = glad_debug_impl_glBindSampler;
PFNGLBINDTEXTUREPROC glad_glBindTexture = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindTexture(GLenum target, GLuint texture) {
    _pre_call_gl_callback("glBindTexture", (GLADapiproc) glad_glBindTexture, 2, target, texture);
    glad_glBindTexture(target, texture);
    _post_call_gl_callback(NULL, "glBindTexture", (GLADapiproc) glad_glBindTexture, 2, target, texture);
    
}
PFNGLBINDTEXTUREPROC glad_debug_glBindTexture = glad_debug_impl_glBindTexture;
PFNGLBINDTRANSFORMFEEDBACKPROC glad_glBindTransformFeedback = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindTransformFeedback(GLenum target, GLuint id) {
    _pre_call_gl_callback("glBindTransformFeedback", (GLADapiproc) glad_glBindTransformFeedback, 2, target, id);
    glad_glBindTransformFeedback(target, id);
    _post_call_gl_callback(NULL, "glBindTransformFeedback", (GLADapiproc) glad_glBindTransformFeedback, 2, target, id);
    
}
PFNGLBINDTRANSFORMFEEDBACKPROC glad_debug_glBindTransformFeedback = glad_debug_impl_glBindTransformFeedback;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray = NULL;
static void GLAD_API_PTR glad_debug_impl_glBindVertexArray(GLuint array) {
    _pre_call_gl_callback("glBindVertexArray", (GLADapiproc) glad_glBindVertexArray, 1, array);
    glad_glBindVertexArray(array);
    _post_call_gl_callback(NULL, "glBindVertexArray", (GLADapiproc) glad_glBindVertexArray, 1, array);
    
}
PFNGLBINDVERTEXARRAYPROC glad_debug_glBindVertexArray = glad_debug_impl_glBindVertexArray;
PFNGLBLENDCOLORPROC glad_glBlendColor = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    _pre_call_gl_callback("glBlendColor", (GLADapiproc) glad_glBlendColor, 4, red, green, blue, alpha);
    glad_glBlendColor(red, green, blue, alpha);
    _post_call_gl_callback(NULL, "glBlendColor", (GLADapiproc) glad_glBlendColor, 4, red, green, blue, alpha);
    
}
PFNGLBLENDCOLORPROC glad_debug_glBlendColor = glad_debug_impl_glBlendColor;
PFNGLBLENDEQUATIONPROC glad_glBlendEquation = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendEquation(GLenum mode) {
    _pre_call_gl_callback("glBlendEquation", (GLADapiproc) glad_glBlendEquation, 1, mode);
    glad_glBlendEquation(mode);
    _post_call_gl_callback(NULL, "glBlendEquation", (GLADapiproc) glad_glBlendEquation, 1, mode);
    
}
PFNGLBLENDEQUATIONPROC glad_debug_glBlendEquation = glad_debug_impl_glBlendEquation;
PFNGLBLENDEQUATIONSEPARATEPROC glad_glBlendEquationSeparate = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha) {
    _pre_call_gl_callback("glBlendEquationSeparate", (GLADapiproc) glad_glBlendEquationSeparate, 2, modeRGB, modeAlpha);
    glad_glBlendEquationSeparate(modeRGB, modeAlpha);
    _post_call_gl_callback(NULL, "glBlendEquationSeparate", (GLADapiproc) glad_glBlendEquationSeparate, 2, modeRGB, modeAlpha);
    
}
PFNGLBLENDEQUATIONSEPARATEPROC glad_debug_glBlendEquationSeparate = glad_debug_impl_glBlendEquationSeparate;
PFNGLBLENDEQUATIONSEPARATEIPROC glad_glBlendEquationSeparatei = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha) {
    _pre_call_gl_callback("glBlendEquationSeparatei", (GLADapiproc) glad_glBlendEquationSeparatei, 3, buf, modeRGB, modeAlpha);
    glad_glBlendEquationSeparatei(buf, modeRGB, modeAlpha);
    _post_call_gl_callback(NULL, "glBlendEquationSeparatei", (GLADapiproc) glad_glBlendEquationSeparatei, 3, buf, modeRGB, modeAlpha);
    
}
PFNGLBLENDEQUATIONSEPARATEIPROC glad_debug_glBlendEquationSeparatei = glad_debug_impl_glBlendEquationSeparatei;
PFNGLBLENDEQUATIONIPROC glad_glBlendEquationi = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendEquationi(GLuint buf, GLenum mode) {
    _pre_call_gl_callback("glBlendEquationi", (GLADapiproc) glad_glBlendEquationi, 2, buf, mode);
    glad_glBlendEquationi(buf, mode);
    _post_call_gl_callback(NULL, "glBlendEquationi", (GLADapiproc) glad_glBlendEquationi, 2, buf, mode);
    
}
PFNGLBLENDEQUATIONIPROC glad_debug_glBlendEquationi = glad_debug_impl_glBlendEquationi;
PFNGLBLENDFUNCPROC glad_glBlendFunc = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendFunc(GLenum sfactor, GLenum dfactor) {
    _pre_call_gl_callback("glBlendFunc", (GLADapiproc) glad_glBlendFunc, 2, sfactor, dfactor);
    glad_glBlendFunc(sfactor, dfactor);
    _post_call_gl_callback(NULL, "glBlendFunc", (GLADapiproc) glad_glBlendFunc, 2, sfactor, dfactor);
    
}
PFNGLBLENDFUNCPROC glad_debug_glBlendFunc = glad_debug_impl_glBlendFunc;
PFNGLBLENDFUNCSEPARATEPROC glad_glBlendFuncSeparate = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha) {
    _pre_call_gl_callback("glBlendFuncSeparate", (GLADapiproc) glad_glBlendFuncSeparate, 4, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    glad_glBlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    _post_call_gl_callback(NULL, "glBlendFuncSeparate", (GLADapiproc) glad_glBlendFuncSeparate, 4, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
    
}
PFNGLBLENDFUNCSEPARATEPROC glad_debug_glBlendFuncSeparate = glad_debug_impl_glBlendFuncSeparate;
PFNGLBLENDFUNCSEPARATEIPROC glad_glBlendFuncSeparatei = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) {
    _pre_call_gl_callback("glBlendFuncSeparatei", (GLADapiproc) glad_glBlendFuncSeparatei, 5, buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
    glad_glBlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
    _post_call_gl_callback(NULL, "glBlendFuncSeparatei", (GLADapiproc) glad_glBlendFuncSeparatei, 5, buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
    
}
PFNGLBLENDFUNCSEPARATEIPROC glad_debug_glBlendFuncSeparatei = glad_debug_impl_glBlendFuncSeparatei;
PFNGLBLENDFUNCIPROC glad_glBlendFunci = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlendFunci(GLuint buf, GLenum src, GLenum dst) {
    _pre_call_gl_callback("glBlendFunci", (GLADapiproc) glad_glBlendFunci, 3, buf, src, dst);
    glad_glBlendFunci(buf, src, dst);
    _post_call_gl_callback(NULL, "glBlendFunci", (GLADapiproc) glad_glBlendFunci, 3, buf, src, dst);
    
}
PFNGLBLENDFUNCIPROC glad_debug_glBlendFunci = glad_debug_impl_glBlendFunci;
PFNGLBLITFRAMEBUFFERPROC glad_glBlitFramebuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
    _pre_call_gl_callback("glBlitFramebuffer", (GLADapiproc) glad_glBlitFramebuffer, 10, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    glad_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    _post_call_gl_callback(NULL, "glBlitFramebuffer", (GLADapiproc) glad_glBlitFramebuffer, 10, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
    
}
PFNGLBLITFRAMEBUFFERPROC glad_debug_glBlitFramebuffer = glad_debug_impl_glBlitFramebuffer;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
static void GLAD_API_PTR glad_debug_impl_glBufferData(GLenum target, GLsizeiptr size, const void * data, GLenum usage) {
    _pre_call_gl_callback("glBufferData", (GLADapiproc) glad_glBufferData, 4, target, size, data, usage);
    glad_glBufferData(target, size, data, usage);
    _post_call_gl_callback(NULL, "glBufferData", (GLADapiproc) glad_glBufferData, 4, target, size, data, usage);
    
}
PFNGLBUFFERDATAPROC glad_debug_glBufferData = glad_debug_impl_glBufferData;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData = NULL;
static void GLAD_API_PTR glad_debug_impl_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void * data) {
    _pre_call_gl_callback("glBufferSubData", (GLADapiproc) glad_glBufferSubData, 4, target, offset, size, data);
    glad_glBufferSubData(target, offset, size, data);
    _post_call_gl_callback(NULL, "glBufferSubData", (GLADapiproc) glad_glBufferSubData, 4, target, offset, size, data);
    
}
PFNGLBUFFERSUBDATAPROC glad_debug_glBufferSubData = glad_debug_impl_glBufferSubData;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus = NULL;
static GLenum GLAD_API_PTR glad_debug_impl_glCheckFramebufferStatus(GLenum target) {
    GLenum ret;
    _pre_call_gl_callback("glCheckFramebufferStatus", (GLADapiproc) glad_glCheckFramebufferStatus, 1, target);
    ret = glad_glCheckFramebufferStatus(target);
    _post_call_gl_callback((void*) &ret, "glCheckFramebufferStatus", (GLADapiproc) glad_glCheckFramebufferStatus, 1, target);
    return ret;
}
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_debug_glCheckFramebufferStatus = glad_debug_impl_glCheckFramebufferStatus;
PFNGLCLAMPCOLORPROC glad_glClampColor = NULL;
static void GLAD_API_PTR glad_debug_impl_glClampColor(GLenum target, GLenum clamp) {
    _pre_call_gl_callback("glClampColor", (GLADapiproc) glad_glClampColor, 2, target, clamp);
    glad_glClampColor(target, clamp);
    _post_call_gl_callback(NULL, "glClampColor", (GLADapiproc) glad_glClampColor, 2, target, clamp);
    
}
PFNGLCLAMPCOLORPROC glad_debug_glClampColor = glad_debug_impl_glClampColor;
PFNGLCLEARPROC glad_glClear = NULL;
static void GLAD_API_PTR glad_debug_impl_glClear(GLbitfield mask) {
    _pre_call_gl_callback("glClear", (GLADapiproc) glad_glClear, 1, mask);
    glad_glClear(mask);
    _post_call_gl_callback(NULL, "glClear", (GLADapiproc) glad_glClear, 1, mask);
    
}
PFNGLCLEARPROC glad_debug_glClear = glad_debug_impl_glClear;
PFNGLCLEARBUFFERFIPROC glad_glClearBufferfi = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
    _pre_call_gl_callback("glClearBufferfi", (GLADapiproc) glad_glClearBufferfi, 4, buffer, drawbuffer, depth, stencil);
    glad_glClearBufferfi(buffer, drawbuffer, depth, stencil);
    _post_call_gl_callback(NULL, "glClearBufferfi", (GLADapiproc) glad_glClearBufferfi, 4, buffer, drawbuffer, depth, stencil);
    
}
PFNGLCLEARBUFFERFIPROC glad_debug_glClearBufferfi = glad_debug_impl_glClearBufferfi;
PFNGLCLEARBUFFERFVPROC glad_glClearBufferfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat * value) {
    _pre_call_gl_callback("glClearBufferfv", (GLADapiproc) glad_glClearBufferfv, 3, buffer, drawbuffer, value);
    glad_glClearBufferfv(buffer, drawbuffer, value);
    _post_call_gl_callback(NULL, "glClearBufferfv", (GLADapiproc) glad_glClearBufferfv, 3, buffer, drawbuffer, value);
    
}
PFNGLCLEARBUFFERFVPROC glad_debug_glClearBufferfv = glad_debug_impl_glClearBufferfv;
PFNGLCLEARBUFFERIVPROC glad_glClearBufferiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint * value) {
    _pre_call_gl_callback("glClearBufferiv", (GLADapiproc) glad_glClearBufferiv, 3, buffer, drawbuffer, value);
    glad_glClearBufferiv(buffer, drawbuffer, value);
    _post_call_gl_callback(NULL, "glClearBufferiv", (GLADapiproc) glad_glClearBufferiv, 3, buffer, drawbuffer, value);
    
}
PFNGLCLEARBUFFERIVPROC glad_debug_glClearBufferiv = glad_debug_impl_glClearBufferiv;
PFNGLCLEARBUFFERUIVPROC glad_glClearBufferuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint * value) {
    _pre_call_gl_callback("glClearBufferuiv", (GLADapiproc) glad_glClearBufferuiv, 3, buffer, drawbuffer, value);
    glad_glClearBufferuiv(buffer, drawbuffer, value);
    _post_call_gl_callback(NULL, "glClearBufferuiv", (GLADapiproc) glad_glClearBufferuiv, 3, buffer, drawbuffer, value);
    
}
PFNGLCLEARBUFFERUIVPROC glad_debug_glClearBufferuiv = glad_debug_impl_glClearBufferuiv;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    _pre_call_gl_callback("glClearColor", (GLADapiproc) glad_glClearColor, 4, red, green, blue, alpha);
    glad_glClearColor(red, green, blue, alpha);
    _post_call_gl_callback(NULL, "glClearColor", (GLADapiproc) glad_glClearColor, 4, red, green, blue, alpha);
    
}
PFNGLCLEARCOLORPROC glad_debug_glClearColor = glad_debug_impl_glClearColor;
PFNGLCLEARDEPTHPROC glad_glClearDepth = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearDepth(GLdouble depth) {
    _pre_call_gl_callback("glClearDepth", (GLADapiproc) glad_glClearDepth, 1, depth);
    glad_glClearDepth(depth);
    _post_call_gl_callback(NULL, "glClearDepth", (GLADapiproc) glad_glClearDepth, 1, depth);
    
}
PFNGLCLEARDEPTHPROC glad_debug_glClearDepth = glad_debug_impl_glClearDepth;
PFNGLCLEARDEPTHFPROC glad_glClearDepthf = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearDepthf(GLfloat d) {
    _pre_call_gl_callback("glClearDepthf", (GLADapiproc) glad_glClearDepthf, 1, d);
    glad_glClearDepthf(d);
    _post_call_gl_callback(NULL, "glClearDepthf", (GLADapiproc) glad_glClearDepthf, 1, d);
    
}
PFNGLCLEARDEPTHFPROC glad_debug_glClearDepthf = glad_debug_impl_glClearDepthf;
PFNGLCLEARSTENCILPROC glad_glClearStencil = NULL;
static void GLAD_API_PTR glad_debug_impl_glClearStencil(GLint s) {
    _pre_call_gl_callback("glClearStencil", (GLADapiproc) glad_glClearStencil, 1, s);
    glad_glClearStencil(s);
    _post_call_gl_callback(NULL, "glClearStencil", (GLADapiproc) glad_glClearStencil, 1, s);
    
}
PFNGLCLEARSTENCILPROC glad_debug_glClearStencil = glad_debug_impl_glClearStencil;
PFNGLCLIENTWAITSYNCPROC glad_glClientWaitSync = NULL;
static GLenum GLAD_API_PTR glad_debug_impl_glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
    GLenum ret;
    _pre_call_gl_callback("glClientWaitSync", (GLADapiproc) glad_glClientWaitSync, 3, sync, flags, timeout);
    ret = glad_glClientWaitSync(sync, flags, timeout);
    _post_call_gl_callback((void*) &ret, "glClientWaitSync", (GLADapiproc) glad_glClientWaitSync, 3, sync, flags, timeout);
    return ret;
}
PFNGLCLIENTWAITSYNCPROC glad_debug_glClientWaitSync = glad_debug_impl_glClientWaitSync;
PFNGLCOLORMASKPROC glad_glColorMask = NULL;
static void GLAD_API_PTR glad_debug_impl_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) {
    _pre_call_gl_callback("glColorMask", (GLADapiproc) glad_glColorMask, 4, red, green, blue, alpha);
    glad_glColorMask(red, green, blue, alpha);
    _post_call_gl_callback(NULL, "glColorMask", (GLADapiproc) glad_glColorMask, 4, red, green, blue, alpha);
    
}
PFNGLCOLORMASKPROC glad_debug_glColorMask = glad_debug_impl_glColorMask;
PFNGLCOLORMASKIPROC glad_glColorMaski = NULL;
static void GLAD_API_PTR glad_debug_impl_glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) {
    _pre_call_gl_callback("glColorMaski", (GLADapiproc) glad_glColorMaski, 5, index, r, g, b, a);
    glad_glColorMaski(index, r, g, b, a);
    _post_call_gl_callback(NULL, "glColorMaski", (GLADapiproc) glad_glColorMaski, 5, index, r, g, b, a);
    
}
PFNGLCOLORMASKIPROC glad_debug_glColorMaski = glad_debug_impl_glColorMaski;
PFNGLCOMPILESHADERPROC glad_glCompileShader = NULL;
static void GLAD_API_PTR glad_debug_impl_glCompileShader(GLuint shader) {
    _pre_call_gl_callback("glCompileShader", (GLADapiproc) glad_glCompileShader, 1, shader);
    glad_glCompileShader(shader);
    _post_call_gl_callback(NULL, "glCompileShader", (GLADapiproc) glad_glCompileShader, 1, shader);
    
}
PFNGLCOMPILESHADERPROC glad_debug_glCompileShader = glad_debug_impl_glCompileShader;
PFNGLCOMPRESSEDTEXIMAGE1DPROC glad_glCompressedTexImage1D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void * data) {
    _pre_call_gl_callback("glCompressedTexImage1D", (GLADapiproc) glad_glCompressedTexImage1D, 7, target, level, internalformat, width, border, imageSize, data);
    glad_glCompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
    _post_call_gl_callback(NULL, "glCompressedTexImage1D", (GLADapiproc) glad_glCompressedTexImage1D, 7, target, level, internalformat, width, border, imageSize, data);
    
}
PFNGLCOMPRESSEDTEXIMAGE1DPROC glad_debug_glCompressedTexImage1D = glad_debug_impl_glCompressedTexImage1D;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glad_glCompressedTexImage2D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data) {
    _pre_call_gl_callback("glCompressedTexImage2D", (GLADapiproc) glad_glCompressedTexImage2D, 8, target, level, internalformat, width, height, border, imageSize, data);
    glad_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
    _post_call_gl_callback(NULL, "glCompressedTexImage2D", (GLADapiproc) glad_glCompressedTexImage2D, 8, target, level, internalformat, width, height, border, imageSize, data);
    
}
PFNGLCOMPRESSEDTEXIMAGE2DPROC glad_debug_glCompressedTexImage2D = glad_debug_impl_glCompressedTexImage2D;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glad_glCompressedTexImage3D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void * data) {
    _pre_call_gl_callback("glCompressedTexImage3D", (GLADapiproc) glad_glCompressedTexImage3D, 9, target, level, internalformat, width, height, depth, border, imageSize, data);
    glad_glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
    _post_call_gl_callback(NULL, "glCompressedTexImage3D", (GLADapiproc) glad_glCompressedTexImage3D, 9, target, level, internalformat, width, height, depth, border, imageSize, data);
    
}
PFNGLCOMPRESSEDTEXIMAGE3DPROC glad_debug_glCompressedTexImage3D = glad_debug_impl_glCompressedTexImage3D;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glad_glCompressedTexSubImage1D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void * data) {
    _pre_call_gl_callback("glCompressedTexSubImage1D", (GLADapiproc) glad_glCompressedTexSubImage1D, 7, target, level, xoffset, width, format, imageSize, data);
    glad_glCompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
    _post_call_gl_callback(NULL, "glCompressedTexSubImage1D", (GLADapiproc) glad_glCompressedTexSubImage1D, 7, target, level, xoffset, width, format, imageSize, data);
    
}
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glad_debug_glCompressedTexSubImage1D = glad_debug_impl_glCompressedTexSubImage1D;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glad_glCompressedTexSubImage2D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data) {
    _pre_call_gl_callback("glCompressedTexSubImage2D", (GLADapiproc) glad_glCompressedTexSubImage2D, 9, target, level, xoffset, yoffset, width, height, format, imageSize, data);
    glad_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
    _post_call_gl_callback(NULL, "glCompressedTexSubImage2D", (GLADapiproc) glad_glCompressedTexSubImage2D, 9, target, level, xoffset, yoffset, width, height, format, imageSize, data);
    
}
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glad_debug_glCompressedTexSubImage2D = glad_debug_impl_glCompressedTexSubImage2D;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glad_glCompressedTexSubImage3D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void * data) {
    _pre_call_gl_callback("glCompressedTexSubImage3D", (GLADapiproc) glad_glCompressedTexSubImage3D, 11, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    glad_glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    _post_call_gl_callback(NULL, "glCompressedTexSubImage3D", (GLADapiproc) glad_glCompressedTexSubImage3D, 11, target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
    
}
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glad_debug_glCompressedTexSubImage3D = glad_debug_impl_glCompressedTexSubImage3D;
PFNGLCOPYBUFFERSUBDATAPROC glad_glCopyBufferSubData = NULL;
static void GLAD_API_PTR glad_debug_impl_glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size) {
    _pre_call_gl_callback("glCopyBufferSubData", (GLADapiproc) glad_glCopyBufferSubData, 5, readTarget, writeTarget, readOffset, writeOffset, size);
    glad_glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
    _post_call_gl_callback(NULL, "glCopyBufferSubData", (GLADapiproc) glad_glCopyBufferSubData, 5, readTarget, writeTarget, readOffset, writeOffset, size);
    
}
PFNGLCOPYBUFFERSUBDATAPROC glad_debug_glCopyBufferSubData = glad_debug_impl_glCopyBufferSubData;
PFNGLCOPYTEXIMAGE1DPROC glad_glCopyTexImage1D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border) {
    _pre_call_gl_callback("glCopyTexImage1D", (GLADapiproc) glad_glCopyTexImage1D, 7, target, level, internalformat, x, y, width, border);
    glad_glCopyTexImage1D(target, level, internalformat, x, y, width, border);
    _post_call_gl_callback(NULL, "glCopyTexImage1D", (GLADapiproc) glad_glCopyTexImage1D, 7, target, level, internalformat, x, y, width, border);
    
}
PFNGLCOPYTEXIMAGE1DPROC glad_debug_glCopyTexImage1D = glad_debug_impl_glCopyTexImage1D;
PFNGLCOPYTEXIMAGE2DPROC glad_glCopyTexImage2D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) {
    _pre_call_gl_callback("glCopyTexImage2D", (GLADapiproc) glad_glCopyTexImage2D, 8, target, level, internalformat, x, y, width, height, border);
    glad_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
    _post_call_gl_callback(NULL, "glCopyTexImage2D", (GLADapiproc) glad_glCopyTexImage2D, 8, target, level, internalformat, x, y, width, height, border);
    
}
PFNGLCOPYTEXIMAGE2DPROC glad_debug_glCopyTexImage2D = glad_debug_impl_glCopyTexImage2D;
PFNGLCOPYTEXSUBIMAGE1DPROC glad_glCopyTexSubImage1D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width) {
    _pre_call_gl_callback("glCopyTexSubImage1D", (GLADapiproc) glad_glCopyTexSubImage1D, 6, target, level, xoffset, x, y, width);
    glad_glCopyTexSubImage1D(target, level, xoffset, x, y, width);
    _post_call_gl_callback(NULL, "glCopyTexSubImage1D", (GLADapiproc) glad_glCopyTexSubImage1D, 6, target, level, xoffset, x, y, width);
    
}
PFNGLCOPYTEXSUBIMAGE1DPROC glad_debug_glCopyTexSubImage1D = glad_debug_impl_glCopyTexSubImage1D;
PFNGLCOPYTEXSUBIMAGE2DPROC glad_glCopyTexSubImage2D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    _pre_call_gl_callback("glCopyTexSubImage2D", (GLADapiproc) glad_glCopyTexSubImage2D, 8, target, level, xoffset, yoffset, x, y, width, height);
    glad_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
    _post_call_gl_callback(NULL, "glCopyTexSubImage2D", (GLADapiproc) glad_glCopyTexSubImage2D, 8, target, level, xoffset, yoffset, x, y, width, height);
    
}
PFNGLCOPYTEXSUBIMAGE2DPROC glad_debug_glCopyTexSubImage2D = glad_debug_impl_glCopyTexSubImage2D;
PFNGLCOPYTEXSUBIMAGE3DPROC glad_glCopyTexSubImage3D = NULL;
static void GLAD_API_PTR glad_debug_impl_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) {
    _pre_call_gl_callback("glCopyTexSubImage3D", (GLADapiproc) glad_glCopyTexSubImage3D, 9, target, level, xoffset, yoffset, zoffset, x, y, width, height);
    glad_glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
    _post_call_gl_callback(NULL, "glCopyTexSubImage3D", (GLADapiproc) glad_glCopyTexSubImage3D, 9, target, level, xoffset, yoffset, zoffset, x, y, width, height);
    
}
PFNGLCOPYTEXSUBIMAGE3DPROC glad_debug_glCopyTexSubImage3D = glad_debug_impl_glCopyTexSubImage3D;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram = NULL;
static GLuint GLAD_API_PTR glad_debug_impl_glCreateProgram(void) {
    GLuint ret;
    _pre_call_gl_callback("glCreateProgram", (GLADapiproc) glad_glCreateProgram, 0);
    ret = glad_glCreateProgram();
    _post_call_gl_callback((void*) &ret, "glCreateProgram", (GLADapiproc) glad_glCreateProgram, 0);
    return ret;
}
PFNGLCREATEPROGRAMPROC glad_debug_glCreateProgram = glad_debug_impl_glCreateProgram;
PFNGLCREATESHADERPROC glad_glCreateShader = NULL;
static GLuint GLAD_API_PTR glad_debug_impl_glCreateShader(GLenum type) {
    GLuint ret;
    _pre_call_gl_callback("glCreateShader", (GLADapiproc) glad_glCreateShader, 1, type);
    ret = glad_glCreateShader(type);
    _post_call_gl_callback((void*) &ret, "glCreateShader", (GLADapiproc) glad_glCreateShader, 1, type);
    return ret;
}
PFNGLCREATESHADERPROC glad_debug_glCreateShader = glad_debug_impl_glCreateShader;
PFNGLCREATESHADERPROGRAMVPROC glad_glCreateShaderProgramv = NULL;
static GLuint GLAD_API_PTR glad_debug_impl_glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const* strings) {
    GLuint ret;
    _pre_call_gl_callback("glCreateShaderProgramv", (GLADapiproc) glad_glCreateShaderProgramv, 3, type, count, strings);
    ret = glad_glCreateShaderProgramv(type, count, strings);
    _post_call_gl_callback((void*) &ret, "glCreateShaderProgramv", (GLADapiproc) glad_glCreateShaderProgramv, 3, type, count, strings);
    return ret;
}
PFNGLCREATESHADERPROGRAMVPROC glad_debug_glCreateShaderProgramv = glad_debug_impl_glCreateShaderProgramv;
PFNGLCULLFACEPROC glad_glCullFace = NULL;
static void GLAD_API_PTR glad_debug_impl_glCullFace(GLenum mode) {
    _pre_call_gl_callback("glCullFace", (GLADapiproc) glad_glCullFace, 1, mode);
    glad_glCullFace(mode);
    _post_call_gl_callback(NULL, "glCullFace", (GLADapiproc) glad_glCullFace, 1, mode);
    
}
PFNGLCULLFACEPROC glad_debug_glCullFace = glad_debug_impl_glCullFace;
PFNGLDEBUGMESSAGECALLBACKPROC glad_glDebugMessageCallback = NULL;
static void GLAD_API_PTR glad_debug_impl_glDebugMessageCallback(GLDEBUGPROC callback, const void * userParam) {
    _pre_call_gl_callback("glDebugMessageCallback", (GLADapiproc) glad_glDebugMessageCallback, 2, callback, userParam);
    glad_glDebugMessageCallback(callback, userParam);
    _post_call_gl_callback(NULL, "glDebugMessageCallback", (GLADapiproc) glad_glDebugMessageCallback, 2, callback, userParam);
    
}
PFNGLDEBUGMESSAGECALLBACKPROC glad_debug_glDebugMessageCallback = glad_debug_impl_glDebugMessageCallback;
PFNGLDEBUGMESSAGECONTROLPROC glad_glDebugMessageControl = NULL;
static void GLAD_API_PTR glad_debug_impl_glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint * ids, GLboolean enabled) {
    _pre_call_gl_callback("glDebugMessageControl", (GLADapiproc) glad_glDebugMessageControl, 6, source, type, severity, count, ids, enabled);
    glad_glDebugMessageControl(source, type, severity, count, ids, enabled);
    _post_call_gl_callback(NULL, "glDebugMessageControl", (GLADapiproc) glad_glDebugMessageControl, 6, source, type, severity, count, ids, enabled);
    
}
PFNGLDEBUGMESSAGECONTROLPROC glad_debug_glDebugMessageControl = glad_debug_impl_glDebugMessageControl;
PFNGLDEBUGMESSAGEINSERTPROC glad_glDebugMessageInsert = NULL;
static void GLAD_API_PTR glad_debug_impl_glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * buf) {
    _pre_call_gl_callback("glDebugMessageInsert", (GLADapiproc) glad_glDebugMessageInsert, 6, source, type, id, severity, length, buf);
    glad_glDebugMessageInsert(source, type, id, severity, length, buf);
    _post_call_gl_callback(NULL, "glDebugMessageInsert", (GLADapiproc) glad_glDebugMessageInsert, 6, source, type, id, severity, length, buf);
    
}
PFNGLDEBUGMESSAGEINSERTPROC glad_debug_glDebugMessageInsert = glad_debug_impl_glDebugMessageInsert;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteBuffers(GLsizei n, const GLuint * buffers) {
    _pre_call_gl_callback("glDeleteBuffers", (GLADapiproc) glad_glDeleteBuffers, 2, n, buffers);
    glad_glDeleteBuffers(n, buffers);
    _post_call_gl_callback(NULL, "glDeleteBuffers", (GLADapiproc) glad_glDeleteBuffers, 2, n, buffers);
    
}
PFNGLDELETEBUFFERSPROC glad_debug_glDeleteBuffers = glad_debug_impl_glDeleteBuffers;
PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteFramebuffers(GLsizei n, const GLuint * framebuffers) {
    _pre_call_gl_callback("glDeleteFramebuffers", (GLADapiproc) glad_glDeleteFramebuffers, 2, n, framebuffers);
    glad_glDeleteFramebuffers(n, framebuffers);
    _post_call_gl_callback(NULL, "glDeleteFramebuffers", (GLADapiproc) glad_glDeleteFramebuffers, 2, n, framebuffers);
    
}
PFNGLDELETEFRAMEBUFFERSPROC glad_debug_glDeleteFramebuffers = glad_debug_impl_glDeleteFramebuffers;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteProgram(GLuint program) {
    _pre_call_gl_callback("glDeleteProgram", (GLADapiproc) glad_glDeleteProgram, 1, program);
    glad_glDeleteProgram(program);
    _post_call_gl_callback(NULL, "glDeleteProgram", (GLADapiproc) glad_glDeleteProgram, 1, program);
    
}
PFNGLDELETEPROGRAMPROC glad_debug_glDeleteProgram = glad_debug_impl_glDeleteProgram;
PFNGLDELETEPROGRAMPIPELINESPROC glad_glDeleteProgramPipelines = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteProgramPipelines(GLsizei n, const GLuint * pipelines) {
    _pre_call_gl_callback("glDeleteProgramPipelines", (GLADapiproc) glad_glDeleteProgramPipelines, 2, n, pipelines);
    glad_glDeleteProgramPipelines(n, pipelines);
    _post_call_gl_callback(NULL, "glDeleteProgramPipelines", (GLADapiproc) glad_glDeleteProgramPipelines, 2, n, pipelines);
    
}
PFNGLDELETEPROGRAMPIPELINESPROC glad_debug_glDeleteProgramPipelines = glad_debug_impl_glDeleteProgramPipelines;
PFNGLDELETEQUERIESPROC glad_glDeleteQueries = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteQueries(GLsizei n, const GLuint * ids) {
    _pre_call_gl_callback("glDeleteQueries", (GLADapiproc) glad_glDeleteQueries, 2, n, ids);
    glad_glDeleteQueries(n, ids);
    _post_call_gl_callback(NULL, "glDeleteQueries", (GLADapiproc) glad_glDeleteQueries, 2, n, ids);
    
}
PFNGLDELETEQUERIESPROC glad_debug_glDeleteQueries = glad_debug_impl_glDeleteQueries;
PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteRenderbuffers(GLsizei n, const GLuint * renderbuffers) {
    _pre_call_gl_callback("glDeleteRenderbuffers", (GLADapiproc) glad_glDeleteRenderbuffers, 2, n, renderbuffers);
    glad_glDeleteRenderbuffers(n, renderbuffers);
    _post_call_gl_callback(NULL, "glDeleteRenderbuffers", (GLADapiproc) glad_glDeleteRenderbuffers, 2, n, renderbuffers);
    
}
PFNGLDELETERENDERBUFFERSPROC glad_debug_glDeleteRenderbuffers = glad_debug_impl_glDeleteRenderbuffers;
PFNGLDELETESAMPLERSPROC glad_glDeleteSamplers = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteSamplers(GLsizei count, const GLuint * samplers) {
    _pre_call_gl_callback("glDeleteSamplers", (GLADapiproc) glad_glDeleteSamplers, 2, count, samplers);
    glad_glDeleteSamplers(count, samplers);
    _post_call_gl_callback(NULL, "glDeleteSamplers", (GLADapiproc) glad_glDeleteSamplers, 2, count, samplers);
    
}
PFNGLDELETESAMPLERSPROC glad_debug_glDeleteSamplers = glad_debug_impl_glDeleteSamplers;
PFNGLDELETESHADERPROC glad_glDeleteShader = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteShader(GLuint shader) {
    _pre_call_gl_callback("glDeleteShader", (GLADapiproc) glad_glDeleteShader, 1, shader);
    glad_glDeleteShader(shader);
    _post_call_gl_callback(NULL, "glDeleteShader", (GLADapiproc) glad_glDeleteShader, 1, shader);
    
}
PFNGLDELETESHADERPROC glad_debug_glDeleteShader = glad_debug_impl_glDeleteShader;
PFNGLDELETESYNCPROC glad_glDeleteSync = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteSync(GLsync sync) {
    _pre_call_gl_callback("glDeleteSync", (GLADapiproc) glad_glDeleteSync, 1, sync);
    glad_glDeleteSync(sync);
    _post_call_gl_callback(NULL, "glDeleteSync", (GLADapiproc) glad_glDeleteSync, 1, sync);
    
}
PFNGLDELETESYNCPROC glad_debug_glDeleteSync = glad_debug_impl_glDeleteSync;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteTextures(GLsizei n, const GLuint * textures) {
    _pre_call_gl_callback("glDeleteTextures", (GLADapiproc) glad_glDeleteTextures, 2, n, textures);
    glad_glDeleteTextures(n, textures);
    _post_call_gl_callback(NULL, "glDeleteTextures", (GLADapiproc) glad_glDeleteTextures, 2, n, textures);
    
}
PFNGLDELETETEXTURESPROC glad_debug_glDeleteTextures = glad_debug_impl_glDeleteTextures;
PFNGLDELETETRANSFORMFEEDBACKSPROC glad_glDeleteTransformFeedbacks = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteTransformFeedbacks(GLsizei n, const GLuint * ids) {
    _pre_call_gl_callback("glDeleteTransformFeedbacks", (GLADapiproc) glad_glDeleteTransformFeedbacks, 2, n, ids);
    glad_glDeleteTransformFeedbacks(n, ids);
    _post_call_gl_callback(NULL, "glDeleteTransformFeedbacks", (GLADapiproc) glad_glDeleteTransformFeedbacks, 2, n, ids);
    
}
PFNGLDELETETRANSFORMFEEDBACKSPROC glad_debug_glDeleteTransformFeedbacks = glad_debug_impl_glDeleteTransformFeedbacks;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays = NULL;
static void GLAD_API_PTR glad_debug_impl_glDeleteVertexArrays(GLsizei n, const GLuint * arrays) {
    _pre_call_gl_callback("glDeleteVertexArrays", (GLADapiproc) glad_glDeleteVertexArrays, 2, n, arrays);
    glad_glDeleteVertexArrays(n, arrays);
    _post_call_gl_callback(NULL, "glDeleteVertexArrays", (GLADapiproc) glad_glDeleteVertexArrays, 2, n, arrays);
    
}
PFNGLDELETEVERTEXARRAYSPROC glad_debug_glDeleteVertexArrays = glad_debug_impl_glDeleteVertexArrays;
PFNGLDEPTHFUNCPROC glad_glDepthFunc = NULL;
static void GLAD_API_PTR glad_debug_impl_glDepthFunc(GLenum func) {
    _pre_call_gl_callback("glDepthFunc", (GLADapiproc) glad_glDepthFunc, 1, func);
    glad_glDepthFunc(func);
    _post_call_gl_callback(NULL, "glDepthFunc", (GLADapiproc) glad_glDepthFunc, 1, func);
    
}
PFNGLDEPTHFUNCPROC glad_debug_glDepthFunc = glad_debug_impl_glDepthFunc;
PFNGLDEPTHMASKPROC glad_glDepthMask = NULL;
static void GLAD_API_PTR glad_debug_impl_glDepthMask(GLboolean flag) {
    _pre_call_gl_callback("glDepthMask", (GLADapiproc) glad_glDepthMask, 1, flag);
    glad_glDepthMask(flag);
    _post_call_gl_callback(NULL, "glDepthMask", (GLADapiproc) glad_glDepthMask, 1, flag);
    
}
PFNGLDEPTHMASKPROC glad_debug_glDepthMask = glad_debug_impl_glDepthMask;
PFNGLDEPTHRANGEPROC glad_glDepthRange = NULL;
static void GLAD_API_PTR glad_debug_impl_glDepthRange(GLdouble n, GLdouble f) {
    _pre_call_gl_callback("glDepthRange", (GLADapiproc) glad_glDepthRange, 2, n, f);
    glad_glDepthRange(n, f);
    _post_call_gl_callback(NULL, "glDepthRange", (GLADapiproc) glad_glDepthRange, 2, n, f);
    
}
PFNGLDEPTHRANGEPROC glad_debug_glDepthRange = glad_debug_impl_glDepthRange;
PFNGLDEPTHRANGEARRAYVPROC glad_glDepthRangeArrayv = NULL;
static void GLAD_API_PTR glad_debug_impl_glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble * v) {
    _pre_call_gl_callback("glDepthRangeArrayv", (GLADapiproc) glad_glDepthRangeArrayv, 3, first, count, v);
    glad_glDepthRangeArrayv(first, count, v);
    _post_call_gl_callback(NULL, "glDepthRangeArrayv", (GLADapiproc) glad_glDepthRangeArrayv, 3, first, count, v);
    
}
PFNGLDEPTHRANGEARRAYVPROC glad_debug_glDepthRangeArrayv = glad_debug_impl_glDepthRangeArrayv;
PFNGLDEPTHRANGEINDEXEDPROC glad_glDepthRangeIndexed = NULL;
static void GLAD_API_PTR glad_debug_impl_glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f) {
    _pre_call_gl_callback("glDepthRangeIndexed", (GLADapiproc) glad_glDepthRangeIndexed, 3, index, n, f);
    glad_glDepthRangeIndexed(index, n, f);
    _post_call_gl_callback(NULL, "glDepthRangeIndexed", (GLADapiproc) glad_glDepthRangeIndexed, 3, index, n, f);
    
}
PFNGLDEPTHRANGEINDEXEDPROC glad_debug_glDepthRangeIndexed = glad_debug_impl_glDepthRangeIndexed;
PFNGLDEPTHRANGEFPROC glad_glDepthRangef = NULL;
static void GLAD_API_PTR glad_debug_impl_glDepthRangef(GLfloat n, GLfloat f) {
    _pre_call_gl_callback("glDepthRangef", (GLADapiproc) glad_glDepthRangef, 2, n, f);
    glad_glDepthRangef(n, f);
    _post_call_gl_callback(NULL, "glDepthRangef", (GLADapiproc) glad_glDepthRangef, 2, n, f);
    
}
PFNGLDEPTHRANGEFPROC glad_debug_glDepthRangef = glad_debug_impl_glDepthRangef;
PFNGLDETACHSHADERPROC glad_glDetachShader = NULL;
static void GLAD_API_PTR glad_debug_impl_glDetachShader(GLuint program, GLuint shader) {
    _pre_call_gl_callback("glDetachShader", (GLADapiproc) glad_glDetachShader, 2, program, shader);
    glad_glDetachShader(program, shader);
    _post_call_gl_callback(NULL, "glDetachShader", (GLADapiproc) glad_glDetachShader, 2, program, shader);
    
}
PFNGLDETACHSHADERPROC glad_debug_glDetachShader = glad_debug_impl_glDetachShader;
PFNGLDISABLEPROC glad_glDisable = NULL;
static void GLAD_API_PTR glad_debug_impl_glDisable(GLenum cap) {
    _pre_call_gl_callback("glDisable", (GLADapiproc) glad_glDisable, 1, cap);
    glad_glDisable(cap);
    _post_call_gl_callback(NULL, "glDisable", (GLADapiproc) glad_glDisable, 1, cap);
    
}
PFNGLDISABLEPROC glad_debug_glDisable = glad_debug_impl_glDisable;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_glDisableVertexAttribArray = NULL;
static void GLAD_API_PTR glad_debug_impl_glDisableVertexAttribArray(GLuint index) {
    _pre_call_gl_callback("glDisableVertexAttribArray", (GLADapiproc) glad_glDisableVertexAttribArray, 1, index);
    glad_glDisableVertexAttribArray(index);
    _post_call_gl_callback(NULL, "glDisableVertexAttribArray", (GLADapiproc) glad_glDisableVertexAttribArray, 1, index);
    
}
PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_debug_glDisableVertexAttribArray = glad_debug_impl_glDisableVertexAttribArray;
PFNGLDISABLEIPROC glad_glDisablei = NULL;
static void GLAD_API_PTR glad_debug_impl_glDisablei(GLenum target, GLuint index) {
    _pre_call_gl_callback("glDisablei", (GLADapiproc) glad_glDisablei, 2, target, index);
    glad_glDisablei(target, index);
    _post_call_gl_callback(NULL, "glDisablei", (GLADapiproc) glad_glDisablei, 2, target, index);
    
}
PFNGLDISABLEIPROC glad_debug_glDisablei = glad_debug_impl_glDisablei;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawArrays(GLenum mode, GLint first, GLsizei count) {
    _pre_call_gl_callback("glDrawArrays", (GLADapiproc) glad_glDrawArrays, 3, mode, first, count);
    glad_glDrawArrays(mode, first, count);
    _post_call_gl_callback(NULL, "glDrawArrays", (GLADapiproc) glad_glDrawArrays, 3, mode, first, count);
    
}
PFNGLDRAWARRAYSPROC glad_debug_glDrawArrays = glad_debug_impl_glDrawArrays;
PFNGLDRAWARRAYSINDIRECTPROC glad_glDrawArraysIndirect = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawArraysIndirect(GLenum mode, const void * indirect) {
    _pre_call_gl_callback("glDrawArraysIndirect", (GLADapiproc) glad_glDrawArraysIndirect, 2, mode, indirect);
    glad_glDrawArraysIndirect(mode, indirect);
    _post_call_gl_callback(NULL, "glDrawArraysIndirect", (GLADapiproc) glad_glDrawArraysIndirect, 2, mode, indirect);
    
}
PFNGLDRAWARRAYSINDIRECTPROC glad_debug_glDrawArraysIndirect = glad_debug_impl_glDrawArraysIndirect;
PFNGLDRAWARRAYSINSTANCEDPROC glad_glDrawArraysInstanced = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
    _pre_call_gl_callback("glDrawArraysInstanced", (GLADapiproc) glad_glDrawArraysInstanced, 4, mode, first, count, instancecount);
    glad_glDrawArraysInstanced(mode, first, count, instancecount);
    _post_call_gl_callback(NULL, "glDrawArraysInstanced", (GLADapiproc) glad_glDrawArraysInstanced, 4, mode, first, count, instancecount);
    
}
PFNGLDRAWARRAYSINSTANCEDPROC glad_debug_glDrawArraysInstanced = glad_debug_impl_glDrawArraysInstanced;
PFNGLDRAWBUFFERPROC glad_glDrawBuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawBuffer(GLenum buf) {
    _pre_call_gl_callback("glDrawBuffer", (GLADapiproc) glad_glDrawBuffer, 1, buf);
    glad_glDrawBuffer(buf);
    _post_call_gl_callback(NULL, "glDrawBuffer", (GLADapiproc) glad_glDrawBuffer, 1, buf);
    
}
PFNGLDRAWBUFFERPROC glad_debug_glDrawBuffer = glad_debug_impl_glDrawBuffer;
PFNGLDRAWBUFFERSPROC glad_glDrawBuffers = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawBuffers(GLsizei n, const GLenum * bufs) {
    _pre_call_gl_callback("glDrawBuffers", (GLADapiproc) glad_glDrawBuffers, 2, n, bufs);
    glad_glDrawBuffers(n, bufs);
    _post_call_gl_callback(NULL, "glDrawBuffers", (GLADapiproc) glad_glDrawBuffers, 2, n, bufs);
    
}
PFNGLDRAWBUFFERSPROC glad_debug_glDrawBuffers = glad_debug_impl_glDrawBuffers;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void * indices) {
    _pre_call_gl_callback("glDrawElements", (GLADapiproc) glad_glDrawElements, 4, mode, count, type, indices);
    glad_glDrawElements(mode, count, type, indices);
    _post_call_gl_callback(NULL, "glDrawElements", (GLADapiproc) glad_glDrawElements, 4, mode, count, type, indices);
    
}
PFNGLDRAWELEMENTSPROC glad_debug_glDrawElements = glad_debug_impl_glDrawElements;
PFNGLDRAWELEMENTSBASEVERTEXPROC glad_glDrawElementsBaseVertex = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void * indices, GLint basevertex) {
    _pre_call_gl_callback("glDrawElementsBaseVertex", (GLADapiproc) glad_glDrawElementsBaseVertex, 5, mode, count, type, indices, basevertex);
    glad_glDrawElementsBaseVertex(mode, count, type, indices, basevertex);
    _post_call_gl_callback(NULL, "glDrawElementsBaseVertex", (GLADapiproc) glad_glDrawElementsBaseVertex, 5, mode, count, type, indices, basevertex);
    
}
PFNGLDRAWELEMENTSBASEVERTEXPROC glad_debug_glDrawElementsBaseVertex = glad_debug_impl_glDrawElementsBaseVertex;
PFNGLDRAWELEMENTSINDIRECTPROC glad_glDrawElementsIndirect = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawElementsIndirect(GLenum mode, GLenum type, const void * indirect) {
    _pre_call_gl_callback("glDrawElementsIndirect", (GLADapiproc) glad_glDrawElementsIndirect, 3, mode, type, indirect);
    glad_glDrawElementsIndirect(mode, type, indirect);
    _post_call_gl_callback(NULL, "glDrawElementsIndirect", (GLADapiproc) glad_glDrawElementsIndirect, 3, mode, type, indirect);
    
}
PFNGLDRAWELEMENTSINDIRECTPROC glad_debug_glDrawElementsIndirect = glad_debug_impl_glDrawElementsIndirect;
PFNGLDRAWELEMENTSINSTANCEDPROC glad_glDrawElementsInstanced = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount) {
    _pre_call_gl_callback("glDrawElementsInstanced", (GLADapiproc) glad_glDrawElementsInstanced, 5, mode, count, type, indices, instancecount);
    glad_glDrawElementsInstanced(mode, count, type, indices, instancecount);
    _post_call_gl_callback(NULL, "glDrawElementsInstanced", (GLADapiproc) glad_glDrawElementsInstanced, 5, mode, count, type, indices, instancecount);
    
}
PFNGLDRAWELEMENTSINSTANCEDPROC glad_debug_glDrawElementsInstanced = glad_debug_impl_glDrawElementsInstanced;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glad_glDrawElementsInstancedBaseVertex = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei instancecount, GLint basevertex) {
    _pre_call_gl_callback("glDrawElementsInstancedBaseVertex", (GLADapiproc) glad_glDrawElementsInstancedBaseVertex, 6, mode, count, type, indices, instancecount, basevertex);
    glad_glDrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
    _post_call_gl_callback(NULL, "glDrawElementsInstancedBaseVertex", (GLADapiproc) glad_glDrawElementsInstancedBaseVertex, 6, mode, count, type, indices, instancecount, basevertex);
    
}
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC glad_debug_glDrawElementsInstancedBaseVertex = glad_debug_impl_glDrawElementsInstancedBaseVertex;
PFNGLDRAWRANGEELEMENTSPROC glad_glDrawRangeElements = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices) {
    _pre_call_gl_callback("glDrawRangeElements", (GLADapiproc) glad_glDrawRangeElements, 6, mode, start, end, count, type, indices);
    glad_glDrawRangeElements(mode, start, end, count, type, indices);
    _post_call_gl_callback(NULL, "glDrawRangeElements", (GLADapiproc) glad_glDrawRangeElements, 6, mode, start, end, count, type, indices);
    
}
PFNGLDRAWRANGEELEMENTSPROC glad_debug_glDrawRangeElements = glad_debug_impl_glDrawRangeElements;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glad_glDrawRangeElementsBaseVertex = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void * indices, GLint basevertex) {
    _pre_call_gl_callback("glDrawRangeElementsBaseVertex", (GLADapiproc) glad_glDrawRangeElementsBaseVertex, 7, mode, start, end, count, type, indices, basevertex);
    glad_glDrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
    _post_call_gl_callback(NULL, "glDrawRangeElementsBaseVertex", (GLADapiproc) glad_glDrawRangeElementsBaseVertex, 7, mode, start, end, count, type, indices, basevertex);
    
}
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC glad_debug_glDrawRangeElementsBaseVertex = glad_debug_impl_glDrawRangeElementsBaseVertex;
PFNGLDRAWTRANSFORMFEEDBACKPROC glad_glDrawTransformFeedback = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawTransformFeedback(GLenum mode, GLuint id) {
    _pre_call_gl_callback("glDrawTransformFeedback", (GLADapiproc) glad_glDrawTransformFeedback, 2, mode, id);
    glad_glDrawTransformFeedback(mode, id);
    _post_call_gl_callback(NULL, "glDrawTransformFeedback", (GLADapiproc) glad_glDrawTransformFeedback, 2, mode, id);
    
}
PFNGLDRAWTRANSFORMFEEDBACKPROC glad_debug_glDrawTransformFeedback = glad_debug_impl_glDrawTransformFeedback;
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glad_glDrawTransformFeedbackStream = NULL;
static void GLAD_API_PTR glad_debug_impl_glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream) {
    _pre_call_gl_callback("glDrawTransformFeedbackStream", (GLADapiproc) glad_glDrawTransformFeedbackStream, 3, mode, id, stream);
    glad_glDrawTransformFeedbackStream(mode, id, stream);
    _post_call_gl_callback(NULL, "glDrawTransformFeedbackStream", (GLADapiproc) glad_glDrawTransformFeedbackStream, 3, mode, id, stream);
    
}
PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC glad_debug_glDrawTransformFeedbackStream = glad_debug_impl_glDrawTransformFeedbackStream;
PFNGLENABLEPROC glad_glEnable = NULL;
static void GLAD_API_PTR glad_debug_impl_glEnable(GLenum cap) {
    _pre_call_gl_callback("glEnable", (GLADapiproc) glad_glEnable, 1, cap);
    glad_glEnable(cap);
    _post_call_gl_callback(NULL, "glEnable", (GLADapiproc) glad_glEnable, 1, cap);
    
}
PFNGLENABLEPROC glad_debug_glEnable = glad_debug_impl_glEnable;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
static void GLAD_API_PTR glad_debug_impl_glEnableVertexAttribArray(GLuint index) {
    _pre_call_gl_callback("glEnableVertexAttribArray", (GLADapiproc) glad_glEnableVertexAttribArray, 1, index);
    glad_glEnableVertexAttribArray(index);
    _post_call_gl_callback(NULL, "glEnableVertexAttribArray", (GLADapiproc) glad_glEnableVertexAttribArray, 1, index);
    
}
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_debug_glEnableVertexAttribArray = glad_debug_impl_glEnableVertexAttribArray;
PFNGLENABLEIPROC glad_glEnablei = NULL;
static void GLAD_API_PTR glad_debug_impl_glEnablei(GLenum target, GLuint index) {
    _pre_call_gl_callback("glEnablei", (GLADapiproc) glad_glEnablei, 2, target, index);
    glad_glEnablei(target, index);
    _post_call_gl_callback(NULL, "glEnablei", (GLADapiproc) glad_glEnablei, 2, target, index);
    
}
PFNGLENABLEIPROC glad_debug_glEnablei = glad_debug_impl_glEnablei;
PFNGLENDCONDITIONALRENDERPROC glad_glEndConditionalRender = NULL;
static void GLAD_API_PTR glad_debug_impl_glEndConditionalRender(void) {
    _pre_call_gl_callback("glEndConditionalRender", (GLADapiproc) glad_glEndConditionalRender, 0);
    glad_glEndConditionalRender();
    _post_call_gl_callback(NULL, "glEndConditionalRender", (GLADapiproc) glad_glEndConditionalRender, 0);
    
}
PFNGLENDCONDITIONALRENDERPROC glad_debug_glEndConditionalRender = glad_debug_impl_glEndConditionalRender;
PFNGLENDQUERYPROC glad_glEndQuery = NULL;
static void GLAD_API_PTR glad_debug_impl_glEndQuery(GLenum target) {
    _pre_call_gl_callback("glEndQuery", (GLADapiproc) glad_glEndQuery, 1, target);
    glad_glEndQuery(target);
    _post_call_gl_callback(NULL, "glEndQuery", (GLADapiproc) glad_glEndQuery, 1, target);
    
}
PFNGLENDQUERYPROC glad_debug_glEndQuery = glad_debug_impl_glEndQuery;
PFNGLENDQUERYINDEXEDPROC glad_glEndQueryIndexed = NULL;
static void GLAD_API_PTR glad_debug_impl_glEndQueryIndexed(GLenum target, GLuint index) {
    _pre_call_gl_callback("glEndQueryIndexed", (GLADapiproc) glad_glEndQueryIndexed, 2, target, index);
    glad_glEndQueryIndexed(target, index);
    _post_call_gl_callback(NULL, "glEndQueryIndexed", (GLADapiproc) glad_glEndQueryIndexed, 2, target, index);
    
}
PFNGLENDQUERYINDEXEDPROC glad_debug_glEndQueryIndexed = glad_debug_impl_glEndQueryIndexed;
PFNGLENDTRANSFORMFEEDBACKPROC glad_glEndTransformFeedback = NULL;
static void GLAD_API_PTR glad_debug_impl_glEndTransformFeedback(void) {
    _pre_call_gl_callback("glEndTransformFeedback", (GLADapiproc) glad_glEndTransformFeedback, 0);
    glad_glEndTransformFeedback();
    _post_call_gl_callback(NULL, "glEndTransformFeedback", (GLADapiproc) glad_glEndTransformFeedback, 0);
    
}
PFNGLENDTRANSFORMFEEDBACKPROC glad_debug_glEndTransformFeedback = glad_debug_impl_glEndTransformFeedback;
PFNGLFENCESYNCPROC glad_glFenceSync = NULL;
static GLsync GLAD_API_PTR glad_debug_impl_glFenceSync(GLenum condition, GLbitfield flags) {
    GLsync ret;
    _pre_call_gl_callback("glFenceSync", (GLADapiproc) glad_glFenceSync, 2, condition, flags);
    ret = glad_glFenceSync(condition, flags);
    _post_call_gl_callback((void*) &ret, "glFenceSync", (GLADapiproc) glad_glFenceSync, 2, condition, flags);
    return ret;
}
PFNGLFENCESYNCPROC glad_debug_glFenceSync = glad_debug_impl_glFenceSync;
PFNGLFINISHPROC glad_glFinish = NULL;
static void GLAD_API_PTR glad_debug_impl_glFinish(void) {
    _pre_call_gl_callback("glFinish", (GLADapiproc) glad_glFinish, 0);
    glad_glFinish();
    _post_call_gl_callback(NULL, "glFinish", (GLADapiproc) glad_glFinish, 0);
    
}
PFNGLFINISHPROC glad_debug_glFinish = glad_debug_impl_glFinish;
PFNGLFLUSHPROC glad_glFlush = NULL;
static void GLAD_API_PTR glad_debug_impl_glFlush(void) {
    _pre_call_gl_callback("glFlush", (GLADapiproc) glad_glFlush, 0);
    glad_glFlush();
    _post_call_gl_callback(NULL, "glFlush", (GLADapiproc) glad_glFlush, 0);
    
}
PFNGLFLUSHPROC glad_debug_glFlush = glad_debug_impl_glFlush;
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glad_glFlushMappedBufferRange = NULL;
static void GLAD_API_PTR glad_debug_impl_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length) {
    _pre_call_gl_callback("glFlushMappedBufferRange", (GLADapiproc) glad_glFlushMappedBufferRange, 3, target, offset, length);
    glad_glFlushMappedBufferRange(target, offset, length);
    _post_call_gl_callback(NULL, "glFlushMappedBufferRange", (GLADapiproc) glad_glFlushMappedBufferRange, 3, target, offset, length);
    
}
PFNGLFLUSHMAPPEDBUFFERRANGEPROC glad_debug_glFlushMappedBufferRange = glad_debug_impl_glFlushMappedBufferRange;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) {
    _pre_call_gl_callback("glFramebufferRenderbuffer", (GLADapiproc) glad_glFramebufferRenderbuffer, 4, target, attachment, renderbuffertarget, renderbuffer);
    glad_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
    _post_call_gl_callback(NULL, "glFramebufferRenderbuffer", (GLADapiproc) glad_glFramebufferRenderbuffer, 4, target, attachment, renderbuffertarget, renderbuffer);
    
}
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_debug_glFramebufferRenderbuffer = glad_debug_impl_glFramebufferRenderbuffer;
PFNGLFRAMEBUFFERTEXTUREPROC glad_glFramebufferTexture = NULL;
static void GLAD_API_PTR glad_debug_impl_glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) {
    _pre_call_gl_callback("glFramebufferTexture", (GLADapiproc) glad_glFramebufferTexture, 4, target, attachment, texture, level);
    glad_glFramebufferTexture(target, attachment, texture, level);
    _post_call_gl_callback(NULL, "glFramebufferTexture", (GLADapiproc) glad_glFramebufferTexture, 4, target, attachment, texture, level);
    
}
PFNGLFRAMEBUFFERTEXTUREPROC glad_debug_glFramebufferTexture = glad_debug_impl_glFramebufferTexture;
PFNGLFRAMEBUFFERTEXTURE1DPROC glad_glFramebufferTexture1D = NULL;
static void GLAD_API_PTR glad_debug_impl_glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    _pre_call_gl_callback("glFramebufferTexture1D", (GLADapiproc) glad_glFramebufferTexture1D, 5, target, attachment, textarget, texture, level);
    glad_glFramebufferTexture1D(target, attachment, textarget, texture, level);
    _post_call_gl_callback(NULL, "glFramebufferTexture1D", (GLADapiproc) glad_glFramebufferTexture1D, 5, target, attachment, textarget, texture, level);
    
}
PFNGLFRAMEBUFFERTEXTURE1DPROC glad_debug_glFramebufferTexture1D = glad_debug_impl_glFramebufferTexture1D;
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D = NULL;
static void GLAD_API_PTR glad_debug_impl_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    _pre_call_gl_callback("glFramebufferTexture2D", (GLADapiproc) glad_glFramebufferTexture2D, 5, target, attachment, textarget, texture, level);
    glad_glFramebufferTexture2D(target, attachment, textarget, texture, level);
    _post_call_gl_callback(NULL, "glFramebufferTexture2D", (GLADapiproc) glad_glFramebufferTexture2D, 5, target, attachment, textarget, texture, level);
    
}
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_debug_glFramebufferTexture2D = glad_debug_impl_glFramebufferTexture2D;
PFNGLFRAMEBUFFERTEXTURE3DPROC glad_glFramebufferTexture3D = NULL;
static void GLAD_API_PTR glad_debug_impl_glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) {
    _pre_call_gl_callback("glFramebufferTexture3D", (GLADapiproc) glad_glFramebufferTexture3D, 6, target, attachment, textarget, texture, level, zoffset);
    glad_glFramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
    _post_call_gl_callback(NULL, "glFramebufferTexture3D", (GLADapiproc) glad_glFramebufferTexture3D, 6, target, attachment, textarget, texture, level, zoffset);
    
}
PFNGLFRAMEBUFFERTEXTURE3DPROC glad_debug_glFramebufferTexture3D = glad_debug_impl_glFramebufferTexture3D;
PFNGLFRAMEBUFFERTEXTURELAYERPROC glad_glFramebufferTextureLayer = NULL;
static void GLAD_API_PTR glad_debug_impl_glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer) {
    _pre_call_gl_callback("glFramebufferTextureLayer", (GLADapiproc) glad_glFramebufferTextureLayer, 5, target, attachment, texture, level, layer);
    glad_glFramebufferTextureLayer(target, attachment, texture, level, layer);
    _post_call_gl_callback(NULL, "glFramebufferTextureLayer", (GLADapiproc) glad_glFramebufferTextureLayer, 5, target, attachment, texture, level, layer);
    
}
PFNGLFRAMEBUFFERTEXTURELAYERPROC glad_debug_glFramebufferTextureLayer = glad_debug_impl_glFramebufferTextureLayer;
PFNGLFRONTFACEPROC glad_glFrontFace = NULL;
static void GLAD_API_PTR glad_debug_impl_glFrontFace(GLenum mode) {
    _pre_call_gl_callback("glFrontFace", (GLADapiproc) glad_glFrontFace, 1, mode);
    glad_glFrontFace(mode);
    _post_call_gl_callback(NULL, "glFrontFace", (GLADapiproc) glad_glFrontFace, 1, mode);
    
}
PFNGLFRONTFACEPROC glad_debug_glFrontFace = glad_debug_impl_glFrontFace;
PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenBuffers(GLsizei n, GLuint * buffers) {
    _pre_call_gl_callback("glGenBuffers", (GLADapiproc) glad_glGenBuffers, 2, n, buffers);
    glad_glGenBuffers(n, buffers);
    _post_call_gl_callback(NULL, "glGenBuffers", (GLADapiproc) glad_glGenBuffers, 2, n, buffers);
    
}
PFNGLGENBUFFERSPROC glad_debug_glGenBuffers = glad_debug_impl_glGenBuffers;
PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenFramebuffers(GLsizei n, GLuint * framebuffers) {
    _pre_call_gl_callback("glGenFramebuffers", (GLADapiproc) glad_glGenFramebuffers, 2, n, framebuffers);
    glad_glGenFramebuffers(n, framebuffers);
    _post_call_gl_callback(NULL, "glGenFramebuffers", (GLADapiproc) glad_glGenFramebuffers, 2, n, framebuffers);
    
}
PFNGLGENFRAMEBUFFERSPROC glad_debug_glGenFramebuffers = glad_debug_impl_glGenFramebuffers;
PFNGLGENPROGRAMPIPELINESPROC glad_glGenProgramPipelines = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenProgramPipelines(GLsizei n, GLuint * pipelines) {
    _pre_call_gl_callback("glGenProgramPipelines", (GLADapiproc) glad_glGenProgramPipelines, 2, n, pipelines);
    glad_glGenProgramPipelines(n, pipelines);
    _post_call_gl_callback(NULL, "glGenProgramPipelines", (GLADapiproc) glad_glGenProgramPipelines, 2, n, pipelines);
    
}
PFNGLGENPROGRAMPIPELINESPROC glad_debug_glGenProgramPipelines = glad_debug_impl_glGenProgramPipelines;
PFNGLGENQUERIESPROC glad_glGenQueries = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenQueries(GLsizei n, GLuint * ids) {
    _pre_call_gl_callback("glGenQueries", (GLADapiproc) glad_glGenQueries, 2, n, ids);
    glad_glGenQueries(n, ids);
    _post_call_gl_callback(NULL, "glGenQueries", (GLADapiproc) glad_glGenQueries, 2, n, ids);
    
}
PFNGLGENQUERIESPROC glad_debug_glGenQueries = glad_debug_impl_glGenQueries;
PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenRenderbuffers(GLsizei n, GLuint * renderbuffers) {
    _pre_call_gl_callback("glGenRenderbuffers", (GLADapiproc) glad_glGenRenderbuffers, 2, n, renderbuffers);
    glad_glGenRenderbuffers(n, renderbuffers);
    _post_call_gl_callback(NULL, "glGenRenderbuffers", (GLADapiproc) glad_glGenRenderbuffers, 2, n, renderbuffers);
    
}
PFNGLGENRENDERBUFFERSPROC glad_debug_glGenRenderbuffers = glad_debug_impl_glGenRenderbuffers;
PFNGLGENSAMPLERSPROC glad_glGenSamplers = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenSamplers(GLsizei count, GLuint * samplers) {
    _pre_call_gl_callback("glGenSamplers", (GLADapiproc) glad_glGenSamplers, 2, count, samplers);
    glad_glGenSamplers(count, samplers);
    _post_call_gl_callback(NULL, "glGenSamplers", (GLADapiproc) glad_glGenSamplers, 2, count, samplers);
    
}
PFNGLGENSAMPLERSPROC glad_debug_glGenSamplers = glad_debug_impl_glGenSamplers;
PFNGLGENTEXTURESPROC glad_glGenTextures = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenTextures(GLsizei n, GLuint * textures) {
    _pre_call_gl_callback("glGenTextures", (GLADapiproc) glad_glGenTextures, 2, n, textures);
    glad_glGenTextures(n, textures);
    _post_call_gl_callback(NULL, "glGenTextures", (GLADapiproc) glad_glGenTextures, 2, n, textures);
    
}
PFNGLGENTEXTURESPROC glad_debug_glGenTextures = glad_debug_impl_glGenTextures;
PFNGLGENTRANSFORMFEEDBACKSPROC glad_glGenTransformFeedbacks = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenTransformFeedbacks(GLsizei n, GLuint * ids) {
    _pre_call_gl_callback("glGenTransformFeedbacks", (GLADapiproc) glad_glGenTransformFeedbacks, 2, n, ids);
    glad_glGenTransformFeedbacks(n, ids);
    _post_call_gl_callback(NULL, "glGenTransformFeedbacks", (GLADapiproc) glad_glGenTransformFeedbacks, 2, n, ids);
    
}
PFNGLGENTRANSFORMFEEDBACKSPROC glad_debug_glGenTransformFeedbacks = glad_debug_impl_glGenTransformFeedbacks;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenVertexArrays(GLsizei n, GLuint * arrays) {
    _pre_call_gl_callback("glGenVertexArrays", (GLADapiproc) glad_glGenVertexArrays, 2, n, arrays);
    glad_glGenVertexArrays(n, arrays);
    _post_call_gl_callback(NULL, "glGenVertexArrays", (GLADapiproc) glad_glGenVertexArrays, 2, n, arrays);
    
}
PFNGLGENVERTEXARRAYSPROC glad_debug_glGenVertexArrays = glad_debug_impl_glGenVertexArrays;
PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap = NULL;
static void GLAD_API_PTR glad_debug_impl_glGenerateMipmap(GLenum target) {
    _pre_call_gl_callback("glGenerateMipmap", (GLADapiproc) glad_glGenerateMipmap, 1, target);
    glad_glGenerateMipmap(target);
    _post_call_gl_callback(NULL, "glGenerateMipmap", (GLADapiproc) glad_glGenerateMipmap, 1, target);
    
}
PFNGLGENERATEMIPMAPPROC glad_debug_glGenerateMipmap = glad_debug_impl_glGenerateMipmap;
PFNGLGETACTIVEATTRIBPROC glad_glGetActiveAttrib = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name) {
    _pre_call_gl_callback("glGetActiveAttrib", (GLADapiproc) glad_glGetActiveAttrib, 7, program, index, bufSize, length, size, type, name);
    glad_glGetActiveAttrib(program, index, bufSize, length, size, type, name);
    _post_call_gl_callback(NULL, "glGetActiveAttrib", (GLADapiproc) glad_glGetActiveAttrib, 7, program, index, bufSize, length, size, type, name);
    
}
PFNGLGETACTIVEATTRIBPROC glad_debug_glGetActiveAttrib = glad_debug_impl_glGetActiveAttrib;
PFNGLGETACTIVESUBROUTINENAMEPROC glad_glGetActiveSubroutineName = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei * length, GLchar * name) {
    _pre_call_gl_callback("glGetActiveSubroutineName", (GLADapiproc) glad_glGetActiveSubroutineName, 6, program, shadertype, index, bufSize, length, name);
    glad_glGetActiveSubroutineName(program, shadertype, index, bufSize, length, name);
    _post_call_gl_callback(NULL, "glGetActiveSubroutineName", (GLADapiproc) glad_glGetActiveSubroutineName, 6, program, shadertype, index, bufSize, length, name);
    
}
PFNGLGETACTIVESUBROUTINENAMEPROC glad_debug_glGetActiveSubroutineName = glad_debug_impl_glGetActiveSubroutineName;
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glad_glGetActiveSubroutineUniformName = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei * length, GLchar * name) {
    _pre_call_gl_callback("glGetActiveSubroutineUniformName", (GLADapiproc) glad_glGetActiveSubroutineUniformName, 6, program, shadertype, index, bufSize, length, name);
    glad_glGetActiveSubroutineUniformName(program, shadertype, index, bufSize, length, name);
    _post_call_gl_callback(NULL, "glGetActiveSubroutineUniformName", (GLADapiproc) glad_glGetActiveSubroutineUniformName, 6, program, shadertype, index, bufSize, length, name);
    
}
PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC glad_debug_glGetActiveSubroutineUniformName = glad_debug_impl_glGetActiveSubroutineUniformName;
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glad_glGetActiveSubroutineUniformiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint * values) {
    _pre_call_gl_callback("glGetActiveSubroutineUniformiv", (GLADapiproc) glad_glGetActiveSubroutineUniformiv, 5, program, shadertype, index, pname, values);
    glad_glGetActiveSubroutineUniformiv(program, shadertype, index, pname, values);
    _post_call_gl_callback(NULL, "glGetActiveSubroutineUniformiv", (GLADapiproc) glad_glGetActiveSubroutineUniformiv, 5, program, shadertype, index, pname, values);
    
}
PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC glad_debug_glGetActiveSubroutineUniformiv = glad_debug_impl_glGetActiveSubroutineUniformiv;
PFNGLGETACTIVEUNIFORMPROC glad_glGetActiveUniform = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name) {
    _pre_call_gl_callback("glGetActiveUniform", (GLADapiproc) glad_glGetActiveUniform, 7, program, index, bufSize, length, size, type, name);
    glad_glGetActiveUniform(program, index, bufSize, length, size, type, name);
    _post_call_gl_callback(NULL, "glGetActiveUniform", (GLADapiproc) glad_glGetActiveUniform, 7, program, index, bufSize, length, size, type, name);
    
}
PFNGLGETACTIVEUNIFORMPROC glad_debug_glGetActiveUniform = glad_debug_impl_glGetActiveUniform;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glad_glGetActiveUniformBlockName = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformBlockName) {
    _pre_call_gl_callback("glGetActiveUniformBlockName", (GLADapiproc) glad_glGetActiveUniformBlockName, 5, program, uniformBlockIndex, bufSize, length, uniformBlockName);
    glad_glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
    _post_call_gl_callback(NULL, "glGetActiveUniformBlockName", (GLADapiproc) glad_glGetActiveUniformBlockName, 5, program, uniformBlockIndex, bufSize, length, uniformBlockName);
    
}
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC glad_debug_glGetActiveUniformBlockName = glad_debug_impl_glGetActiveUniformBlockName;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glad_glGetActiveUniformBlockiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetActiveUniformBlockiv", (GLADapiproc) glad_glGetActiveUniformBlockiv, 4, program, uniformBlockIndex, pname, params);
    glad_glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
    _post_call_gl_callback(NULL, "glGetActiveUniformBlockiv", (GLADapiproc) glad_glGetActiveUniformBlockiv, 4, program, uniformBlockIndex, pname, params);
    
}
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glad_debug_glGetActiveUniformBlockiv = glad_debug_impl_glGetActiveUniformBlockiv;
PFNGLGETACTIVEUNIFORMNAMEPROC glad_glGetActiveUniformName = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei * length, GLchar * uniformName) {
    _pre_call_gl_callback("glGetActiveUniformName", (GLADapiproc) glad_glGetActiveUniformName, 5, program, uniformIndex, bufSize, length, uniformName);
    glad_glGetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
    _post_call_gl_callback(NULL, "glGetActiveUniformName", (GLADapiproc) glad_glGetActiveUniformName, 5, program, uniformIndex, bufSize, length, uniformName);
    
}
PFNGLGETACTIVEUNIFORMNAMEPROC glad_debug_glGetActiveUniformName = glad_debug_impl_glGetActiveUniformName;
PFNGLGETACTIVEUNIFORMSIVPROC glad_glGetActiveUniformsiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint * uniformIndices, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetActiveUniformsiv", (GLADapiproc) glad_glGetActiveUniformsiv, 5, program, uniformCount, uniformIndices, pname, params);
    glad_glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
    _post_call_gl_callback(NULL, "glGetActiveUniformsiv", (GLADapiproc) glad_glGetActiveUniformsiv, 5, program, uniformCount, uniformIndices, pname, params);
    
}
PFNGLGETACTIVEUNIFORMSIVPROC glad_debug_glGetActiveUniformsiv = glad_debug_impl_glGetActiveUniformsiv;
PFNGLGETATTACHEDSHADERSPROC glad_glGetAttachedShaders = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders) {
    _pre_call_gl_callback("glGetAttachedShaders", (GLADapiproc) glad_glGetAttachedShaders, 4, program, maxCount, count, shaders);
    glad_glGetAttachedShaders(program, maxCount, count, shaders);
    _post_call_gl_callback(NULL, "glGetAttachedShaders", (GLADapiproc) glad_glGetAttachedShaders, 4, program, maxCount, count, shaders);
    
}
PFNGLGETATTACHEDSHADERSPROC glad_debug_glGetAttachedShaders = glad_debug_impl_glGetAttachedShaders;
PFNGLGETATTRIBLOCATIONPROC glad_glGetAttribLocation = NULL;
static GLint GLAD_API_PTR glad_debug_impl_glGetAttribLocation(GLuint program, const GLchar * name) {
    GLint ret;
    _pre_call_gl_callback("glGetAttribLocation", (GLADapiproc) glad_glGetAttribLocation, 2, program, name);
    ret = glad_glGetAttribLocation(program, name);
    _post_call_gl_callback((void*) &ret, "glGetAttribLocation", (GLADapiproc) glad_glGetAttribLocation, 2, program, name);
    return ret;
}
PFNGLGETATTRIBLOCATIONPROC glad_debug_glGetAttribLocation = glad_debug_impl_glGetAttribLocation;
PFNGLGETBOOLEANI_VPROC glad_glGetBooleani_v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetBooleani_v(GLenum target, GLuint index, GLboolean * data) {
    _pre_call_gl_callback("glGetBooleani_v", (GLADapiproc) glad_glGetBooleani_v, 3, target, index, data);
    glad_glGetBooleani_v(target, index, data);
    _post_call_gl_callback(NULL, "glGetBooleani_v", (GLADapiproc) glad_glGetBooleani_v, 3, target, index, data);
    
}
PFNGLGETBOOLEANI_VPROC glad_debug_glGetBooleani_v = glad_debug_impl_glGetBooleani_v;
PFNGLGETBOOLEANVPROC glad_glGetBooleanv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetBooleanv(GLenum pname, GLboolean * data) {
    _pre_call_gl_callback("glGetBooleanv", (GLADapiproc) glad_glGetBooleanv, 2, pname, data);
    glad_glGetBooleanv(pname, data);
    _post_call_gl_callback(NULL, "glGetBooleanv", (GLADapiproc) glad_glGetBooleanv, 2, pname, data);
    
}
PFNGLGETBOOLEANVPROC glad_debug_glGetBooleanv = glad_debug_impl_glGetBooleanv;
PFNGLGETBUFFERPARAMETERI64VPROC glad_glGetBufferParameteri64v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 * params) {
    _pre_call_gl_callback("glGetBufferParameteri64v", (GLADapiproc) glad_glGetBufferParameteri64v, 3, target, pname, params);
    glad_glGetBufferParameteri64v(target, pname, params);
    _post_call_gl_callback(NULL, "glGetBufferParameteri64v", (GLADapiproc) glad_glGetBufferParameteri64v, 3, target, pname, params);
    
}
PFNGLGETBUFFERPARAMETERI64VPROC glad_debug_glGetBufferParameteri64v = glad_debug_impl_glGetBufferParameteri64v;
PFNGLGETBUFFERPARAMETERIVPROC glad_glGetBufferParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetBufferParameteriv(GLenum target, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetBufferParameteriv", (GLADapiproc) glad_glGetBufferParameteriv, 3, target, pname, params);
    glad_glGetBufferParameteriv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetBufferParameteriv", (GLADapiproc) glad_glGetBufferParameteriv, 3, target, pname, params);
    
}
PFNGLGETBUFFERPARAMETERIVPROC glad_debug_glGetBufferParameteriv = glad_debug_impl_glGetBufferParameteriv;
PFNGLGETBUFFERPOINTERVPROC glad_glGetBufferPointerv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetBufferPointerv(GLenum target, GLenum pname, void ** params) {
    _pre_call_gl_callback("glGetBufferPointerv", (GLADapiproc) glad_glGetBufferPointerv, 3, target, pname, params);
    glad_glGetBufferPointerv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetBufferPointerv", (GLADapiproc) glad_glGetBufferPointerv, 3, target, pname, params);
    
}
PFNGLGETBUFFERPOINTERVPROC glad_debug_glGetBufferPointerv = glad_debug_impl_glGetBufferPointerv;
PFNGLGETBUFFERSUBDATAPROC glad_glGetBufferSubData = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void * data) {
    _pre_call_gl_callback("glGetBufferSubData", (GLADapiproc) glad_glGetBufferSubData, 4, target, offset, size, data);
    glad_glGetBufferSubData(target, offset, size, data);
    _post_call_gl_callback(NULL, "glGetBufferSubData", (GLADapiproc) glad_glGetBufferSubData, 4, target, offset, size, data);
    
}
PFNGLGETBUFFERSUBDATAPROC glad_debug_glGetBufferSubData = glad_debug_impl_glGetBufferSubData;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glad_glGetCompressedTexImage = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetCompressedTexImage(GLenum target, GLint level, void * img) {
    _pre_call_gl_callback("glGetCompressedTexImage", (GLADapiproc) glad_glGetCompressedTexImage, 3, target, level, img);
    glad_glGetCompressedTexImage(target, level, img);
    _post_call_gl_callback(NULL, "glGetCompressedTexImage", (GLADapiproc) glad_glGetCompressedTexImage, 3, target, level, img);
    
}
PFNGLGETCOMPRESSEDTEXIMAGEPROC glad_debug_glGetCompressedTexImage = glad_debug_impl_glGetCompressedTexImage;
PFNGLGETDEBUGMESSAGELOGPROC glad_glGetDebugMessageLog = NULL;
static GLuint GLAD_API_PTR glad_debug_impl_glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum * sources, GLenum * types, GLuint * ids, GLenum * severities, GLsizei * lengths, GLchar * messageLog) {
    GLuint ret;
    _pre_call_gl_callback("glGetDebugMessageLog", (GLADapiproc) glad_glGetDebugMessageLog, 8, count, bufSize, sources, types, ids, severities, lengths, messageLog);
    ret = glad_glGetDebugMessageLog(count, bufSize, sources, types, ids, severities, lengths, messageLog);
    _post_call_gl_callback((void*) &ret, "glGetDebugMessageLog", (GLADapiproc) glad_glGetDebugMessageLog, 8, count, bufSize, sources, types, ids, severities, lengths, messageLog);
    return ret;
}
PFNGLGETDEBUGMESSAGELOGPROC glad_debug_glGetDebugMessageLog = glad_debug_impl_glGetDebugMessageLog;
PFNGLGETDOUBLEI_VPROC glad_glGetDoublei_v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetDoublei_v(GLenum target, GLuint index, GLdouble * data) {
    _pre_call_gl_callback("glGetDoublei_v", (GLADapiproc) glad_glGetDoublei_v, 3, target, index, data);
    glad_glGetDoublei_v(target, index, data);
    _post_call_gl_callback(NULL, "glGetDoublei_v", (GLADapiproc) glad_glGetDoublei_v, 3, target, index, data);
    
}
PFNGLGETDOUBLEI_VPROC glad_debug_glGetDoublei_v = glad_debug_impl_glGetDoublei_v;
PFNGLGETDOUBLEVPROC glad_glGetDoublev = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetDoublev(GLenum pname, GLdouble * data) {
    _pre_call_gl_callback("glGetDoublev", (GLADapiproc) glad_glGetDoublev, 2, pname, data);
    glad_glGetDoublev(pname, data);
    _post_call_gl_callback(NULL, "glGetDoublev", (GLADapiproc) glad_glGetDoublev, 2, pname, data);
    
}
PFNGLGETDOUBLEVPROC glad_debug_glGetDoublev = glad_debug_impl_glGetDoublev;
PFNGLGETERRORPROC glad_glGetError = NULL;
static GLenum GLAD_API_PTR glad_debug_impl_glGetError(void) {
    GLenum ret;
    _pre_call_gl_callback("glGetError", (GLADapiproc) glad_glGetError, 0);
    ret = glad_glGetError();
    _post_call_gl_callback((void*) &ret, "glGetError", (GLADapiproc) glad_glGetError, 0);
    return ret;
}
PFNGLGETERRORPROC glad_debug_glGetError = glad_debug_impl_glGetError;
PFNGLGETFLOATI_VPROC glad_glGetFloati_v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetFloati_v(GLenum target, GLuint index, GLfloat * data) {
    _pre_call_gl_callback("glGetFloati_v", (GLADapiproc) glad_glGetFloati_v, 3, target, index, data);
    glad_glGetFloati_v(target, index, data);
    _post_call_gl_callback(NULL, "glGetFloati_v", (GLADapiproc) glad_glGetFloati_v, 3, target, index, data);
    
}
PFNGLGETFLOATI_VPROC glad_debug_glGetFloati_v = glad_debug_impl_glGetFloati_v;
PFNGLGETFLOATVPROC glad_glGetFloatv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetFloatv(GLenum pname, GLfloat * data) {
    _pre_call_gl_callback("glGetFloatv", (GLADapiproc) glad_glGetFloatv, 2, pname, data);
    glad_glGetFloatv(pname, data);
    _post_call_gl_callback(NULL, "glGetFloatv", (GLADapiproc) glad_glGetFloatv, 2, pname, data);
    
}
PFNGLGETFLOATVPROC glad_debug_glGetFloatv = glad_debug_impl_glGetFloatv;
PFNGLGETFRAGDATAINDEXPROC glad_glGetFragDataIndex = NULL;
static GLint GLAD_API_PTR glad_debug_impl_glGetFragDataIndex(GLuint program, const GLchar * name) {
    GLint ret;
    _pre_call_gl_callback("glGetFragDataIndex", (GLADapiproc) glad_glGetFragDataIndex, 2, program, name);
    ret = glad_glGetFragDataIndex(program, name);
    _post_call_gl_callback((void*) &ret, "glGetFragDataIndex", (GLADapiproc) glad_glGetFragDataIndex, 2, program, name);
    return ret;
}
PFNGLGETFRAGDATAINDEXPROC glad_debug_glGetFragDataIndex = glad_debug_impl_glGetFragDataIndex;
PFNGLGETFRAGDATALOCATIONPROC glad_glGetFragDataLocation = NULL;
static GLint GLAD_API_PTR glad_debug_impl_glGetFragDataLocation(GLuint program, const GLchar * name) {
    GLint ret;
    _pre_call_gl_callback("glGetFragDataLocation", (GLADapiproc) glad_glGetFragDataLocation, 2, program, name);
    ret = glad_glGetFragDataLocation(program, name);
    _post_call_gl_callback((void*) &ret, "glGetFragDataLocation", (GLADapiproc) glad_glGetFragDataLocation, 2, program, name);
    return ret;
}
PFNGLGETFRAGDATALOCATIONPROC glad_debug_glGetFragDataLocation = glad_debug_impl_glGetFragDataLocation;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glad_glGetFramebufferAttachmentParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetFramebufferAttachmentParameteriv", (GLADapiproc) glad_glGetFramebufferAttachmentParameteriv, 4, target, attachment, pname, params);
    glad_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
    _post_call_gl_callback(NULL, "glGetFramebufferAttachmentParameteriv", (GLADapiproc) glad_glGetFramebufferAttachmentParameteriv, 4, target, attachment, pname, params);
    
}
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glad_debug_glGetFramebufferAttachmentParameteriv = glad_debug_impl_glGetFramebufferAttachmentParameteriv;
PFNGLGETINTEGER64I_VPROC glad_glGetInteger64i_v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetInteger64i_v(GLenum target, GLuint index, GLint64 * data) {
    _pre_call_gl_callback("glGetInteger64i_v", (GLADapiproc) glad_glGetInteger64i_v, 3, target, index, data);
    glad_glGetInteger64i_v(target, index, data);
    _post_call_gl_callback(NULL, "glGetInteger64i_v", (GLADapiproc) glad_glGetInteger64i_v, 3, target, index, data);
    
}
PFNGLGETINTEGER64I_VPROC glad_debug_glGetInteger64i_v = glad_debug_impl_glGetInteger64i_v;
PFNGLGETINTEGER64VPROC glad_glGetInteger64v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetInteger64v(GLenum pname, GLint64 * data) {
    _pre_call_gl_callback("glGetInteger64v", (GLADapiproc) glad_glGetInteger64v, 2, pname, data);
    glad_glGetInteger64v(pname, data);
    _post_call_gl_callback(NULL, "glGetInteger64v", (GLADapiproc) glad_glGetInteger64v, 2, pname, data);
    
}
PFNGLGETINTEGER64VPROC glad_debug_glGetInteger64v = glad_debug_impl_glGetInteger64v;
PFNGLGETINTEGERI_VPROC glad_glGetIntegeri_v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetIntegeri_v(GLenum target, GLuint index, GLint * data) {
    _pre_call_gl_callback("glGetIntegeri_v", (GLADapiproc) glad_glGetIntegeri_v, 3, target, index, data);
    glad_glGetIntegeri_v(target, index, data);
    _post_call_gl_callback(NULL, "glGetIntegeri_v", (GLADapiproc) glad_glGetIntegeri_v, 3, target, index, data);
    
}
PFNGLGETINTEGERI_VPROC glad_debug_glGetIntegeri_v = glad_debug_impl_glGetIntegeri_v;
PFNGLGETINTEGERVPROC glad_glGetIntegerv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetIntegerv(GLenum pname, GLint * data) {
    _pre_call_gl_callback("glGetIntegerv", (GLADapiproc) glad_glGetIntegerv, 2, pname, data);
    glad_glGetIntegerv(pname, data);
    _post_call_gl_callback(NULL, "glGetIntegerv", (GLADapiproc) glad_glGetIntegerv, 2, pname, data);
    
}
PFNGLGETINTEGERVPROC glad_debug_glGetIntegerv = glad_debug_impl_glGetIntegerv;
PFNGLGETMULTISAMPLEFVPROC glad_glGetMultisamplefv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetMultisamplefv(GLenum pname, GLuint index, GLfloat * val) {
    _pre_call_gl_callback("glGetMultisamplefv", (GLADapiproc) glad_glGetMultisamplefv, 3, pname, index, val);
    glad_glGetMultisamplefv(pname, index, val);
    _post_call_gl_callback(NULL, "glGetMultisamplefv", (GLADapiproc) glad_glGetMultisamplefv, 3, pname, index, val);
    
}
PFNGLGETMULTISAMPLEFVPROC glad_debug_glGetMultisamplefv = glad_debug_impl_glGetMultisamplefv;
PFNGLGETOBJECTLABELPROC glad_glGetObjectLabel = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei * length, GLchar * label) {
    _pre_call_gl_callback("glGetObjectLabel", (GLADapiproc) glad_glGetObjectLabel, 5, identifier, name, bufSize, length, label);
    glad_glGetObjectLabel(identifier, name, bufSize, length, label);
    _post_call_gl_callback(NULL, "glGetObjectLabel", (GLADapiproc) glad_glGetObjectLabel, 5, identifier, name, bufSize, length, label);
    
}
PFNGLGETOBJECTLABELPROC glad_debug_glGetObjectLabel = glad_debug_impl_glGetObjectLabel;
PFNGLGETOBJECTPTRLABELPROC glad_glGetObjectPtrLabel = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetObjectPtrLabel(const void * ptr, GLsizei bufSize, GLsizei * length, GLchar * label) {
    _pre_call_gl_callback("glGetObjectPtrLabel", (GLADapiproc) glad_glGetObjectPtrLabel, 4, ptr, bufSize, length, label);
    glad_glGetObjectPtrLabel(ptr, bufSize, length, label);
    _post_call_gl_callback(NULL, "glGetObjectPtrLabel", (GLADapiproc) glad_glGetObjectPtrLabel, 4, ptr, bufSize, length, label);
    
}
PFNGLGETOBJECTPTRLABELPROC glad_debug_glGetObjectPtrLabel = glad_debug_impl_glGetObjectPtrLabel;
PFNGLGETPOINTERVPROC glad_glGetPointerv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetPointerv(GLenum pname, void ** params) {
    _pre_call_gl_callback("glGetPointerv", (GLADapiproc) glad_glGetPointerv, 2, pname, params);
    glad_glGetPointerv(pname, params);
    _post_call_gl_callback(NULL, "glGetPointerv", (GLADapiproc) glad_glGetPointerv, 2, pname, params);
    
}
PFNGLGETPOINTERVPROC glad_debug_glGetPointerv = glad_debug_impl_glGetPointerv;
PFNGLGETPROGRAMBINARYPROC glad_glGetProgramBinary = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei * length, GLenum * binaryFormat, void * binary) {
    _pre_call_gl_callback("glGetProgramBinary", (GLADapiproc) glad_glGetProgramBinary, 5, program, bufSize, length, binaryFormat, binary);
    glad_glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
    _post_call_gl_callback(NULL, "glGetProgramBinary", (GLADapiproc) glad_glGetProgramBinary, 5, program, bufSize, length, binaryFormat, binary);
    
}
PFNGLGETPROGRAMBINARYPROC glad_debug_glGetProgramBinary = glad_debug_impl_glGetProgramBinary;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog) {
    _pre_call_gl_callback("glGetProgramInfoLog", (GLADapiproc) glad_glGetProgramInfoLog, 4, program, bufSize, length, infoLog);
    glad_glGetProgramInfoLog(program, bufSize, length, infoLog);
    _post_call_gl_callback(NULL, "glGetProgramInfoLog", (GLADapiproc) glad_glGetProgramInfoLog, 4, program, bufSize, length, infoLog);
    
}
PFNGLGETPROGRAMINFOLOGPROC glad_debug_glGetProgramInfoLog = glad_debug_impl_glGetProgramInfoLog;
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glad_glGetProgramPipelineInfoLog = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei * length, GLchar * infoLog) {
    _pre_call_gl_callback("glGetProgramPipelineInfoLog", (GLADapiproc) glad_glGetProgramPipelineInfoLog, 4, pipeline, bufSize, length, infoLog);
    glad_glGetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
    _post_call_gl_callback(NULL, "glGetProgramPipelineInfoLog", (GLADapiproc) glad_glGetProgramPipelineInfoLog, 4, pipeline, bufSize, length, infoLog);
    
}
PFNGLGETPROGRAMPIPELINEINFOLOGPROC glad_debug_glGetProgramPipelineInfoLog = glad_debug_impl_glGetProgramPipelineInfoLog;
PFNGLGETPROGRAMPIPELINEIVPROC glad_glGetProgramPipelineiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetProgramPipelineiv", (GLADapiproc) glad_glGetProgramPipelineiv, 3, pipeline, pname, params);
    glad_glGetProgramPipelineiv(pipeline, pname, params);
    _post_call_gl_callback(NULL, "glGetProgramPipelineiv", (GLADapiproc) glad_glGetProgramPipelineiv, 3, pipeline, pname, params);
    
}
PFNGLGETPROGRAMPIPELINEIVPROC glad_debug_glGetProgramPipelineiv = glad_debug_impl_glGetProgramPipelineiv;
PFNGLGETPROGRAMSTAGEIVPROC glad_glGetProgramStageiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint * values) {
    _pre_call_gl_callback("glGetProgramStageiv", (GLADapiproc) glad_glGetProgramStageiv, 4, program, shadertype, pname, values);
    glad_glGetProgramStageiv(program, shadertype, pname, values);
    _post_call_gl_callback(NULL, "glGetProgramStageiv", (GLADapiproc) glad_glGetProgramStageiv, 4, program, shadertype, pname, values);
    
}
PFNGLGETPROGRAMSTAGEIVPROC glad_debug_glGetProgramStageiv = glad_debug_impl_glGetProgramStageiv;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetProgramiv(GLuint program, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetProgramiv", (GLADapiproc) glad_glGetProgramiv, 3, program, pname, params);
    glad_glGetProgramiv(program, pname, params);
    _post_call_gl_callback(NULL, "glGetProgramiv", (GLADapiproc) glad_glGetProgramiv, 3, program, pname, params);
    
}
PFNGLGETPROGRAMIVPROC glad_debug_glGetProgramiv = glad_debug_impl_glGetProgramiv;
PFNGLGETQUERYINDEXEDIVPROC glad_glGetQueryIndexediv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetQueryIndexediv", (GLADapiproc) glad_glGetQueryIndexediv, 4, target, index, pname, params);
    glad_glGetQueryIndexediv(target, index, pname, params);
    _post_call_gl_callback(NULL, "glGetQueryIndexediv", (GLADapiproc) glad_glGetQueryIndexediv, 4, target, index, pname, params);
    
}
PFNGLGETQUERYINDEXEDIVPROC glad_debug_glGetQueryIndexediv = glad_debug_impl_glGetQueryIndexediv;
PFNGLGETQUERYOBJECTI64VPROC glad_glGetQueryObjecti64v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 * params) {
    _pre_call_gl_callback("glGetQueryObjecti64v", (GLADapiproc) glad_glGetQueryObjecti64v, 3, id, pname, params);
    glad_glGetQueryObjecti64v(id, pname, params);
    _post_call_gl_callback(NULL, "glGetQueryObjecti64v", (GLADapiproc) glad_glGetQueryObjecti64v, 3, id, pname, params);
    
}
PFNGLGETQUERYOBJECTI64VPROC glad_debug_glGetQueryObjecti64v = glad_debug_impl_glGetQueryObjecti64v;
PFNGLGETQUERYOBJECTIVPROC glad_glGetQueryObjectiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetQueryObjectiv(GLuint id, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetQueryObjectiv", (GLADapiproc) glad_glGetQueryObjectiv, 3, id, pname, params);
    glad_glGetQueryObjectiv(id, pname, params);
    _post_call_gl_callback(NULL, "glGetQueryObjectiv", (GLADapiproc) glad_glGetQueryObjectiv, 3, id, pname, params);
    
}
PFNGLGETQUERYOBJECTIVPROC glad_debug_glGetQueryObjectiv = glad_debug_impl_glGetQueryObjectiv;
PFNGLGETQUERYOBJECTUI64VPROC glad_glGetQueryObjectui64v = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 * params) {
    _pre_call_gl_callback("glGetQueryObjectui64v", (GLADapiproc) glad_glGetQueryObjectui64v, 3, id, pname, params);
    glad_glGetQueryObjectui64v(id, pname, params);
    _post_call_gl_callback(NULL, "glGetQueryObjectui64v", (GLADapiproc) glad_glGetQueryObjectui64v, 3, id, pname, params);
    
}
PFNGLGETQUERYOBJECTUI64VPROC glad_debug_glGetQueryObjectui64v = glad_debug_impl_glGetQueryObjectui64v;
PFNGLGETQUERYOBJECTUIVPROC glad_glGetQueryObjectuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint * params) {
    _pre_call_gl_callback("glGetQueryObjectuiv", (GLADapiproc) glad_glGetQueryObjectuiv, 3, id, pname, params);
    glad_glGetQueryObjectuiv(id, pname, params);
    _post_call_gl_callback(NULL, "glGetQueryObjectuiv", (GLADapiproc) glad_glGetQueryObjectuiv, 3, id, pname, params);
    
}
PFNGLGETQUERYOBJECTUIVPROC glad_debug_glGetQueryObjectuiv = glad_debug_impl_glGetQueryObjectuiv;
PFNGLGETQUERYIVPROC glad_glGetQueryiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetQueryiv(GLenum target, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetQueryiv", (GLADapiproc) glad_glGetQueryiv, 3, target, pname, params);
    glad_glGetQueryiv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetQueryiv", (GLADapiproc) glad_glGetQueryiv, 3, target, pname, params);
    
}
PFNGLGETQUERYIVPROC glad_debug_glGetQueryiv = glad_debug_impl_glGetQueryiv;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glad_glGetRenderbufferParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetRenderbufferParameteriv", (GLADapiproc) glad_glGetRenderbufferParameteriv, 3, target, pname, params);
    glad_glGetRenderbufferParameteriv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetRenderbufferParameteriv", (GLADapiproc) glad_glGetRenderbufferParameteriv, 3, target, pname, params);
    
}
PFNGLGETRENDERBUFFERPARAMETERIVPROC glad_debug_glGetRenderbufferParameteriv = glad_debug_impl_glGetRenderbufferParameteriv;
PFNGLGETSAMPLERPARAMETERIIVPROC glad_glGetSamplerParameterIiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetSamplerParameterIiv", (GLADapiproc) glad_glGetSamplerParameterIiv, 3, sampler, pname, params);
    glad_glGetSamplerParameterIiv(sampler, pname, params);
    _post_call_gl_callback(NULL, "glGetSamplerParameterIiv", (GLADapiproc) glad_glGetSamplerParameterIiv, 3, sampler, pname, params);
    
}
PFNGLGETSAMPLERPARAMETERIIVPROC glad_debug_glGetSamplerParameterIiv = glad_debug_impl_glGetSamplerParameterIiv;
PFNGLGETSAMPLERPARAMETERIUIVPROC glad_glGetSamplerParameterIuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint * params) {
    _pre_call_gl_callback("glGetSamplerParameterIuiv", (GLADapiproc) glad_glGetSamplerParameterIuiv, 3, sampler, pname, params);
    glad_glGetSamplerParameterIuiv(sampler, pname, params);
    _post_call_gl_callback(NULL, "glGetSamplerParameterIuiv", (GLADapiproc) glad_glGetSamplerParameterIuiv, 3, sampler, pname, params);
    
}
PFNGLGETSAMPLERPARAMETERIUIVPROC glad_debug_glGetSamplerParameterIuiv = glad_debug_impl_glGetSamplerParameterIuiv;
PFNGLGETSAMPLERPARAMETERFVPROC glad_glGetSamplerParameterfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat * params) {
    _pre_call_gl_callback("glGetSamplerParameterfv", (GLADapiproc) glad_glGetSamplerParameterfv, 3, sampler, pname, params);
    glad_glGetSamplerParameterfv(sampler, pname, params);
    _post_call_gl_callback(NULL, "glGetSamplerParameterfv", (GLADapiproc) glad_glGetSamplerParameterfv, 3, sampler, pname, params);
    
}
PFNGLGETSAMPLERPARAMETERFVPROC glad_debug_glGetSamplerParameterfv = glad_debug_impl_glGetSamplerParameterfv;
PFNGLGETSAMPLERPARAMETERIVPROC glad_glGetSamplerParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetSamplerParameteriv", (GLADapiproc) glad_glGetSamplerParameteriv, 3, sampler, pname, params);
    glad_glGetSamplerParameteriv(sampler, pname, params);
    _post_call_gl_callback(NULL, "glGetSamplerParameteriv", (GLADapiproc) glad_glGetSamplerParameteriv, 3, sampler, pname, params);
    
}
PFNGLGETSAMPLERPARAMETERIVPROC glad_debug_glGetSamplerParameteriv = glad_debug_impl_glGetSamplerParameteriv;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog) {
    _pre_call_gl_callback("glGetShaderInfoLog", (GLADapiproc) glad_glGetShaderInfoLog, 4, shader, bufSize, length, infoLog);
    glad_glGetShaderInfoLog(shader, bufSize, length, infoLog);
    _post_call_gl_callback(NULL, "glGetShaderInfoLog", (GLADapiproc) glad_glGetShaderInfoLog, 4, shader, bufSize, length, infoLog);
    
}
PFNGLGETSHADERINFOLOGPROC glad_debug_glGetShaderInfoLog = glad_debug_impl_glGetShaderInfoLog;
PFNGLGETSHADERPRECISIONFORMATPROC glad_glGetShaderPrecisionFormat = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision) {
    _pre_call_gl_callback("glGetShaderPrecisionFormat", (GLADapiproc) glad_glGetShaderPrecisionFormat, 4, shadertype, precisiontype, range, precision);
    glad_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
    _post_call_gl_callback(NULL, "glGetShaderPrecisionFormat", (GLADapiproc) glad_glGetShaderPrecisionFormat, 4, shadertype, precisiontype, range, precision);
    
}
PFNGLGETSHADERPRECISIONFORMATPROC glad_debug_glGetShaderPrecisionFormat = glad_debug_impl_glGetShaderPrecisionFormat;
PFNGLGETSHADERSOURCEPROC glad_glGetShaderSource = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source) {
    _pre_call_gl_callback("glGetShaderSource", (GLADapiproc) glad_glGetShaderSource, 4, shader, bufSize, length, source);
    glad_glGetShaderSource(shader, bufSize, length, source);
    _post_call_gl_callback(NULL, "glGetShaderSource", (GLADapiproc) glad_glGetShaderSource, 4, shader, bufSize, length, source);
    
}
PFNGLGETSHADERSOURCEPROC glad_debug_glGetShaderSource = glad_debug_impl_glGetShaderSource;
PFNGLGETSHADERIVPROC glad_glGetShaderiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetShaderiv(GLuint shader, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetShaderiv", (GLADapiproc) glad_glGetShaderiv, 3, shader, pname, params);
    glad_glGetShaderiv(shader, pname, params);
    _post_call_gl_callback(NULL, "glGetShaderiv", (GLADapiproc) glad_glGetShaderiv, 3, shader, pname, params);
    
}
PFNGLGETSHADERIVPROC glad_debug_glGetShaderiv = glad_debug_impl_glGetShaderiv;
PFNGLGETSTRINGPROC glad_glGetString = NULL;
static const GLubyte * GLAD_API_PTR glad_debug_impl_glGetString(GLenum name) {
    const GLubyte * ret;
    _pre_call_gl_callback("glGetString", (GLADapiproc) glad_glGetString, 1, name);
    ret = glad_glGetString(name);
    _post_call_gl_callback((void*) &ret, "glGetString", (GLADapiproc) glad_glGetString, 1, name);
    return ret;
}
PFNGLGETSTRINGPROC glad_debug_glGetString = glad_debug_impl_glGetString;
PFNGLGETSTRINGIPROC glad_glGetStringi = NULL;
static const GLubyte * GLAD_API_PTR glad_debug_impl_glGetStringi(GLenum name, GLuint index) {
    const GLubyte * ret;
    _pre_call_gl_callback("glGetStringi", (GLADapiproc) glad_glGetStringi, 2, name, index);
    ret = glad_glGetStringi(name, index);
    _post_call_gl_callback((void*) &ret, "glGetStringi", (GLADapiproc) glad_glGetStringi, 2, name, index);
    return ret;
}
PFNGLGETSTRINGIPROC glad_debug_glGetStringi = glad_debug_impl_glGetStringi;
PFNGLGETSUBROUTINEINDEXPROC glad_glGetSubroutineIndex = NULL;
static GLuint GLAD_API_PTR glad_debug_impl_glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar * name) {
    GLuint ret;
    _pre_call_gl_callback("glGetSubroutineIndex", (GLADapiproc) glad_glGetSubroutineIndex, 3, program, shadertype, name);
    ret = glad_glGetSubroutineIndex(program, shadertype, name);
    _post_call_gl_callback((void*) &ret, "glGetSubroutineIndex", (GLADapiproc) glad_glGetSubroutineIndex, 3, program, shadertype, name);
    return ret;
}
PFNGLGETSUBROUTINEINDEXPROC glad_debug_glGetSubroutineIndex = glad_debug_impl_glGetSubroutineIndex;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glad_glGetSubroutineUniformLocation = NULL;
static GLint GLAD_API_PTR glad_debug_impl_glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar * name) {
    GLint ret;
    _pre_call_gl_callback("glGetSubroutineUniformLocation", (GLADapiproc) glad_glGetSubroutineUniformLocation, 3, program, shadertype, name);
    ret = glad_glGetSubroutineUniformLocation(program, shadertype, name);
    _post_call_gl_callback((void*) &ret, "glGetSubroutineUniformLocation", (GLADapiproc) glad_glGetSubroutineUniformLocation, 3, program, shadertype, name);
    return ret;
}
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glad_debug_glGetSubroutineUniformLocation = glad_debug_impl_glGetSubroutineUniformLocation;
PFNGLGETSYNCIVPROC glad_glGetSynciv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetSynciv(GLsync sync, GLenum pname, GLsizei count, GLsizei * length, GLint * values) {
    _pre_call_gl_callback("glGetSynciv", (GLADapiproc) glad_glGetSynciv, 5, sync, pname, count, length, values);
    glad_glGetSynciv(sync, pname, count, length, values);
    _post_call_gl_callback(NULL, "glGetSynciv", (GLADapiproc) glad_glGetSynciv, 5, sync, pname, count, length, values);
    
}
PFNGLGETSYNCIVPROC glad_debug_glGetSynciv = glad_debug_impl_glGetSynciv;
PFNGLGETTEXIMAGEPROC glad_glGetTexImage = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void * pixels) {
    _pre_call_gl_callback("glGetTexImage", (GLADapiproc) glad_glGetTexImage, 5, target, level, format, type, pixels);
    glad_glGetTexImage(target, level, format, type, pixels);
    _post_call_gl_callback(NULL, "glGetTexImage", (GLADapiproc) glad_glGetTexImage, 5, target, level, format, type, pixels);
    
}
PFNGLGETTEXIMAGEPROC glad_debug_glGetTexImage = glad_debug_impl_glGetTexImage;
PFNGLGETTEXLEVELPARAMETERFVPROC glad_glGetTexLevelParameterfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat * params) {
    _pre_call_gl_callback("glGetTexLevelParameterfv", (GLADapiproc) glad_glGetTexLevelParameterfv, 4, target, level, pname, params);
    glad_glGetTexLevelParameterfv(target, level, pname, params);
    _post_call_gl_callback(NULL, "glGetTexLevelParameterfv", (GLADapiproc) glad_glGetTexLevelParameterfv, 4, target, level, pname, params);
    
}
PFNGLGETTEXLEVELPARAMETERFVPROC glad_debug_glGetTexLevelParameterfv = glad_debug_impl_glGetTexLevelParameterfv;
PFNGLGETTEXLEVELPARAMETERIVPROC glad_glGetTexLevelParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetTexLevelParameteriv", (GLADapiproc) glad_glGetTexLevelParameteriv, 4, target, level, pname, params);
    glad_glGetTexLevelParameteriv(target, level, pname, params);
    _post_call_gl_callback(NULL, "glGetTexLevelParameteriv", (GLADapiproc) glad_glGetTexLevelParameteriv, 4, target, level, pname, params);
    
}
PFNGLGETTEXLEVELPARAMETERIVPROC glad_debug_glGetTexLevelParameteriv = glad_debug_impl_glGetTexLevelParameteriv;
PFNGLGETTEXPARAMETERIIVPROC glad_glGetTexParameterIiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTexParameterIiv(GLenum target, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetTexParameterIiv", (GLADapiproc) glad_glGetTexParameterIiv, 3, target, pname, params);
    glad_glGetTexParameterIiv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetTexParameterIiv", (GLADapiproc) glad_glGetTexParameterIiv, 3, target, pname, params);
    
}
PFNGLGETTEXPARAMETERIIVPROC glad_debug_glGetTexParameterIiv = glad_debug_impl_glGetTexParameterIiv;
PFNGLGETTEXPARAMETERIUIVPROC glad_glGetTexParameterIuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint * params) {
    _pre_call_gl_callback("glGetTexParameterIuiv", (GLADapiproc) glad_glGetTexParameterIuiv, 3, target, pname, params);
    glad_glGetTexParameterIuiv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetTexParameterIuiv", (GLADapiproc) glad_glGetTexParameterIuiv, 3, target, pname, params);
    
}
PFNGLGETTEXPARAMETERIUIVPROC glad_debug_glGetTexParameterIuiv = glad_debug_impl_glGetTexParameterIuiv;
PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat * params) {
    _pre_call_gl_callback("glGetTexParameterfv", (GLADapiproc) glad_glGetTexParameterfv, 3, target, pname, params);
    glad_glGetTexParameterfv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetTexParameterfv", (GLADapiproc) glad_glGetTexParameterfv, 3, target, pname, params);
    
}
PFNGLGETTEXPARAMETERFVPROC glad_debug_glGetTexParameterfv = glad_debug_impl_glGetTexParameterfv;
PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTexParameteriv(GLenum target, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetTexParameteriv", (GLADapiproc) glad_glGetTexParameteriv, 3, target, pname, params);
    glad_glGetTexParameteriv(target, pname, params);
    _post_call_gl_callback(NULL, "glGetTexParameteriv", (GLADapiproc) glad_glGetTexParameteriv, 3, target, pname, params);
    
}
PFNGLGETTEXPARAMETERIVPROC glad_debug_glGetTexParameteriv = glad_debug_impl_glGetTexParameteriv;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glad_glGetTransformFeedbackVarying = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, GLchar * name) {
    _pre_call_gl_callback("glGetTransformFeedbackVarying", (GLADapiproc) glad_glGetTransformFeedbackVarying, 7, program, index, bufSize, length, size, type, name);
    glad_glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
    _post_call_gl_callback(NULL, "glGetTransformFeedbackVarying", (GLADapiproc) glad_glGetTransformFeedbackVarying, 7, program, index, bufSize, length, size, type, name);
    
}
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC glad_debug_glGetTransformFeedbackVarying = glad_debug_impl_glGetTransformFeedbackVarying;
PFNGLGETUNIFORMBLOCKINDEXPROC glad_glGetUniformBlockIndex = NULL;
static GLuint GLAD_API_PTR glad_debug_impl_glGetUniformBlockIndex(GLuint program, const GLchar * uniformBlockName) {
    GLuint ret;
    _pre_call_gl_callback("glGetUniformBlockIndex", (GLADapiproc) glad_glGetUniformBlockIndex, 2, program, uniformBlockName);
    ret = glad_glGetUniformBlockIndex(program, uniformBlockName);
    _post_call_gl_callback((void*) &ret, "glGetUniformBlockIndex", (GLADapiproc) glad_glGetUniformBlockIndex, 2, program, uniformBlockName);
    return ret;
}
PFNGLGETUNIFORMBLOCKINDEXPROC glad_debug_glGetUniformBlockIndex = glad_debug_impl_glGetUniformBlockIndex;
PFNGLGETUNIFORMINDICESPROC glad_glGetUniformIndices = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const* uniformNames, GLuint * uniformIndices) {
    _pre_call_gl_callback("glGetUniformIndices", (GLADapiproc) glad_glGetUniformIndices, 4, program, uniformCount, uniformNames, uniformIndices);
    glad_glGetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
    _post_call_gl_callback(NULL, "glGetUniformIndices", (GLADapiproc) glad_glGetUniformIndices, 4, program, uniformCount, uniformNames, uniformIndices);
    
}
PFNGLGETUNIFORMINDICESPROC glad_debug_glGetUniformIndices = glad_debug_impl_glGetUniformIndices;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation = NULL;
static GLint GLAD_API_PTR glad_debug_impl_glGetUniformLocation(GLuint program, const GLchar * name) {
    GLint ret;
    _pre_call_gl_callback("glGetUniformLocation", (GLADapiproc) glad_glGetUniformLocation, 2, program, name);
    ret = glad_glGetUniformLocation(program, name);
    _post_call_gl_callback((void*) &ret, "glGetUniformLocation", (GLADapiproc) glad_glGetUniformLocation, 2, program, name);
    return ret;
}
PFNGLGETUNIFORMLOCATIONPROC glad_debug_glGetUniformLocation = glad_debug_impl_glGetUniformLocation;
PFNGLGETUNIFORMSUBROUTINEUIVPROC glad_glGetUniformSubroutineuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint * params) {
    _pre_call_gl_callback("glGetUniformSubroutineuiv", (GLADapiproc) glad_glGetUniformSubroutineuiv, 3, shadertype, location, params);
    glad_glGetUniformSubroutineuiv(shadertype, location, params);
    _post_call_gl_callback(NULL, "glGetUniformSubroutineuiv", (GLADapiproc) glad_glGetUniformSubroutineuiv, 3, shadertype, location, params);
    
}
PFNGLGETUNIFORMSUBROUTINEUIVPROC glad_debug_glGetUniformSubroutineuiv = glad_debug_impl_glGetUniformSubroutineuiv;
PFNGLGETUNIFORMDVPROC glad_glGetUniformdv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetUniformdv(GLuint program, GLint location, GLdouble * params) {
    _pre_call_gl_callback("glGetUniformdv", (GLADapiproc) glad_glGetUniformdv, 3, program, location, params);
    glad_glGetUniformdv(program, location, params);
    _post_call_gl_callback(NULL, "glGetUniformdv", (GLADapiproc) glad_glGetUniformdv, 3, program, location, params);
    
}
PFNGLGETUNIFORMDVPROC glad_debug_glGetUniformdv = glad_debug_impl_glGetUniformdv;
PFNGLGETUNIFORMFVPROC glad_glGetUniformfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetUniformfv(GLuint program, GLint location, GLfloat * params) {
    _pre_call_gl_callback("glGetUniformfv", (GLADapiproc) glad_glGetUniformfv, 3, program, location, params);
    glad_glGetUniformfv(program, location, params);
    _post_call_gl_callback(NULL, "glGetUniformfv", (GLADapiproc) glad_glGetUniformfv, 3, program, location, params);
    
}
PFNGLGETUNIFORMFVPROC glad_debug_glGetUniformfv = glad_debug_impl_glGetUniformfv;
PFNGLGETUNIFORMIVPROC glad_glGetUniformiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetUniformiv(GLuint program, GLint location, GLint * params) {
    _pre_call_gl_callback("glGetUniformiv", (GLADapiproc) glad_glGetUniformiv, 3, program, location, params);
    glad_glGetUniformiv(program, location, params);
    _post_call_gl_callback(NULL, "glGetUniformiv", (GLADapiproc) glad_glGetUniformiv, 3, program, location, params);
    
}
PFNGLGETUNIFORMIVPROC glad_debug_glGetUniformiv = glad_debug_impl_glGetUniformiv;
PFNGLGETUNIFORMUIVPROC glad_glGetUniformuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetUniformuiv(GLuint program, GLint location, GLuint * params) {
    _pre_call_gl_callback("glGetUniformuiv", (GLADapiproc) glad_glGetUniformuiv, 3, program, location, params);
    glad_glGetUniformuiv(program, location, params);
    _post_call_gl_callback(NULL, "glGetUniformuiv", (GLADapiproc) glad_glGetUniformuiv, 3, program, location, params);
    
}
PFNGLGETUNIFORMUIVPROC glad_debug_glGetUniformuiv = glad_debug_impl_glGetUniformuiv;
PFNGLGETVERTEXATTRIBIIVPROC glad_glGetVertexAttribIiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetVertexAttribIiv(GLuint index, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetVertexAttribIiv", (GLADapiproc) glad_glGetVertexAttribIiv, 3, index, pname, params);
    glad_glGetVertexAttribIiv(index, pname, params);
    _post_call_gl_callback(NULL, "glGetVertexAttribIiv", (GLADapiproc) glad_glGetVertexAttribIiv, 3, index, pname, params);
    
}
PFNGLGETVERTEXATTRIBIIVPROC glad_debug_glGetVertexAttribIiv = glad_debug_impl_glGetVertexAttribIiv;
PFNGLGETVERTEXATTRIBIUIVPROC glad_glGetVertexAttribIuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint * params) {
    _pre_call_gl_callback("glGetVertexAttribIuiv", (GLADapiproc) glad_glGetVertexAttribIuiv, 3, index, pname, params);
    glad_glGetVertexAttribIuiv(index, pname, params);
    _post_call_gl_callback(NULL, "glGetVertexAttribIuiv", (GLADapiproc) glad_glGetVertexAttribIuiv, 3, index, pname, params);
    
}
PFNGLGETVERTEXATTRIBIUIVPROC glad_debug_glGetVertexAttribIuiv = glad_debug_impl_glGetVertexAttribIuiv;
PFNGLGETVERTEXATTRIBLDVPROC glad_glGetVertexAttribLdv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble * params) {
    _pre_call_gl_callback("glGetVertexAttribLdv", (GLADapiproc) glad_glGetVertexAttribLdv, 3, index, pname, params);
    glad_glGetVertexAttribLdv(index, pname, params);
    _post_call_gl_callback(NULL, "glGetVertexAttribLdv", (GLADapiproc) glad_glGetVertexAttribLdv, 3, index, pname, params);
    
}
PFNGLGETVERTEXATTRIBLDVPROC glad_debug_glGetVertexAttribLdv = glad_debug_impl_glGetVertexAttribLdv;
PFNGLGETVERTEXATTRIBPOINTERVPROC glad_glGetVertexAttribPointerv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetVertexAttribPointerv(GLuint index, GLenum pname, void ** pointer) {
    _pre_call_gl_callback("glGetVertexAttribPointerv", (GLADapiproc) glad_glGetVertexAttribPointerv, 3, index, pname, pointer);
    glad_glGetVertexAttribPointerv(index, pname, pointer);
    _post_call_gl_callback(NULL, "glGetVertexAttribPointerv", (GLADapiproc) glad_glGetVertexAttribPointerv, 3, index, pname, pointer);
    
}
PFNGLGETVERTEXATTRIBPOINTERVPROC glad_debug_glGetVertexAttribPointerv = glad_debug_impl_glGetVertexAttribPointerv;
PFNGLGETVERTEXATTRIBDVPROC glad_glGetVertexAttribdv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble * params) {
    _pre_call_gl_callback("glGetVertexAttribdv", (GLADapiproc) glad_glGetVertexAttribdv, 3, index, pname, params);
    glad_glGetVertexAttribdv(index, pname, params);
    _post_call_gl_callback(NULL, "glGetVertexAttribdv", (GLADapiproc) glad_glGetVertexAttribdv, 3, index, pname, params);
    
}
PFNGLGETVERTEXATTRIBDVPROC glad_debug_glGetVertexAttribdv = glad_debug_impl_glGetVertexAttribdv;
PFNGLGETVERTEXATTRIBFVPROC glad_glGetVertexAttribfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat * params) {
    _pre_call_gl_callback("glGetVertexAttribfv", (GLADapiproc) glad_glGetVertexAttribfv, 3, index, pname, params);
    glad_glGetVertexAttribfv(index, pname, params);
    _post_call_gl_callback(NULL, "glGetVertexAttribfv", (GLADapiproc) glad_glGetVertexAttribfv, 3, index, pname, params);
    
}
PFNGLGETVERTEXATTRIBFVPROC glad_debug_glGetVertexAttribfv = glad_debug_impl_glGetVertexAttribfv;
PFNGLGETVERTEXATTRIBIVPROC glad_glGetVertexAttribiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glGetVertexAttribiv(GLuint index, GLenum pname, GLint * params) {
    _pre_call_gl_callback("glGetVertexAttribiv", (GLADapiproc) glad_glGetVertexAttribiv, 3, index, pname, params);
    glad_glGetVertexAttribiv(index, pname, params);
    _post_call_gl_callback(NULL, "glGetVertexAttribiv", (GLADapiproc) glad_glGetVertexAttribiv, 3, index, pname, params);
    
}
PFNGLGETVERTEXATTRIBIVPROC glad_debug_glGetVertexAttribiv = glad_debug_impl_glGetVertexAttribiv;
PFNGLHINTPROC glad_glHint = NULL;
static void GLAD_API_PTR glad_debug_impl_glHint(GLenum target, GLenum mode) {
    _pre_call_gl_callback("glHint", (GLADapiproc) glad_glHint, 2, target, mode);
    glad_glHint(target, mode);
    _post_call_gl_callback(NULL, "glHint", (GLADapiproc) glad_glHint, 2, target, mode);
    
}
PFNGLHINTPROC glad_debug_glHint = glad_debug_impl_glHint;
PFNGLISBUFFERPROC glad_glIsBuffer = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsBuffer(GLuint buffer) {
    GLboolean ret;
    _pre_call_gl_callback("glIsBuffer", (GLADapiproc) glad_glIsBuffer, 1, buffer);
    ret = glad_glIsBuffer(buffer);
    _post_call_gl_callback((void*) &ret, "glIsBuffer", (GLADapiproc) glad_glIsBuffer, 1, buffer);
    return ret;
}
PFNGLISBUFFERPROC glad_debug_glIsBuffer = glad_debug_impl_glIsBuffer;
PFNGLISENABLEDPROC glad_glIsEnabled = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsEnabled(GLenum cap) {
    GLboolean ret;
    _pre_call_gl_callback("glIsEnabled", (GLADapiproc) glad_glIsEnabled, 1, cap);
    ret = glad_glIsEnabled(cap);
    _post_call_gl_callback((void*) &ret, "glIsEnabled", (GLADapiproc) glad_glIsEnabled, 1, cap);
    return ret;
}
PFNGLISENABLEDPROC glad_debug_glIsEnabled = glad_debug_impl_glIsEnabled;
PFNGLISENABLEDIPROC glad_glIsEnabledi = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsEnabledi(GLenum target, GLuint index) {
    GLboolean ret;
    _pre_call_gl_callback("glIsEnabledi", (GLADapiproc) glad_glIsEnabledi, 2, target, index);
    ret = glad_glIsEnabledi(target, index);
    _post_call_gl_callback((void*) &ret, "glIsEnabledi", (GLADapiproc) glad_glIsEnabledi, 2, target, index);
    return ret;
}
PFNGLISENABLEDIPROC glad_debug_glIsEnabledi = glad_debug_impl_glIsEnabledi;
PFNGLISFRAMEBUFFERPROC glad_glIsFramebuffer = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsFramebuffer(GLuint framebuffer) {
    GLboolean ret;
    _pre_call_gl_callback("glIsFramebuffer", (GLADapiproc) glad_glIsFramebuffer, 1, framebuffer);
    ret = glad_glIsFramebuffer(framebuffer);
    _post_call_gl_callback((void*) &ret, "glIsFramebuffer", (GLADapiproc) glad_glIsFramebuffer, 1, framebuffer);
    return ret;
}
PFNGLISFRAMEBUFFERPROC glad_debug_glIsFramebuffer = glad_debug_impl_glIsFramebuffer;
PFNGLISPROGRAMPROC glad_glIsProgram = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsProgram(GLuint program) {
    GLboolean ret;
    _pre_call_gl_callback("glIsProgram", (GLADapiproc) glad_glIsProgram, 1, program);
    ret = glad_glIsProgram(program);
    _post_call_gl_callback((void*) &ret, "glIsProgram", (GLADapiproc) glad_glIsProgram, 1, program);
    return ret;
}
PFNGLISPROGRAMPROC glad_debug_glIsProgram = glad_debug_impl_glIsProgram;
PFNGLISPROGRAMPIPELINEPROC glad_glIsProgramPipeline = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsProgramPipeline(GLuint pipeline) {
    GLboolean ret;
    _pre_call_gl_callback("glIsProgramPipeline", (GLADapiproc) glad_glIsProgramPipeline, 1, pipeline);
    ret = glad_glIsProgramPipeline(pipeline);
    _post_call_gl_callback((void*) &ret, "glIsProgramPipeline", (GLADapiproc) glad_glIsProgramPipeline, 1, pipeline);
    return ret;
}
PFNGLISPROGRAMPIPELINEPROC glad_debug_glIsProgramPipeline = glad_debug_impl_glIsProgramPipeline;
PFNGLISQUERYPROC glad_glIsQuery = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsQuery(GLuint id) {
    GLboolean ret;
    _pre_call_gl_callback("glIsQuery", (GLADapiproc) glad_glIsQuery, 1, id);
    ret = glad_glIsQuery(id);
    _post_call_gl_callback((void*) &ret, "glIsQuery", (GLADapiproc) glad_glIsQuery, 1, id);
    return ret;
}
PFNGLISQUERYPROC glad_debug_glIsQuery = glad_debug_impl_glIsQuery;
PFNGLISRENDERBUFFERPROC glad_glIsRenderbuffer = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsRenderbuffer(GLuint renderbuffer) {
    GLboolean ret;
    _pre_call_gl_callback("glIsRenderbuffer", (GLADapiproc) glad_glIsRenderbuffer, 1, renderbuffer);
    ret = glad_glIsRenderbuffer(renderbuffer);
    _post_call_gl_callback((void*) &ret, "glIsRenderbuffer", (GLADapiproc) glad_glIsRenderbuffer, 1, renderbuffer);
    return ret;
}
PFNGLISRENDERBUFFERPROC glad_debug_glIsRenderbuffer = glad_debug_impl_glIsRenderbuffer;
PFNGLISSAMPLERPROC glad_glIsSampler = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsSampler(GLuint sampler) {
    GLboolean ret;
    _pre_call_gl_callback("glIsSampler", (GLADapiproc) glad_glIsSampler, 1, sampler);
    ret = glad_glIsSampler(sampler);
    _post_call_gl_callback((void*) &ret, "glIsSampler", (GLADapiproc) glad_glIsSampler, 1, sampler);
    return ret;
}
PFNGLISSAMPLERPROC glad_debug_glIsSampler = glad_debug_impl_glIsSampler;
PFNGLISSHADERPROC glad_glIsShader = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsShader(GLuint shader) {
    GLboolean ret;
    _pre_call_gl_callback("glIsShader", (GLADapiproc) glad_glIsShader, 1, shader);
    ret = glad_glIsShader(shader);
    _post_call_gl_callback((void*) &ret, "glIsShader", (GLADapiproc) glad_glIsShader, 1, shader);
    return ret;
}
PFNGLISSHADERPROC glad_debug_glIsShader = glad_debug_impl_glIsShader;
PFNGLISSYNCPROC glad_glIsSync = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsSync(GLsync sync) {
    GLboolean ret;
    _pre_call_gl_callback("glIsSync", (GLADapiproc) glad_glIsSync, 1, sync);
    ret = glad_glIsSync(sync);
    _post_call_gl_callback((void*) &ret, "glIsSync", (GLADapiproc) glad_glIsSync, 1, sync);
    return ret;
}
PFNGLISSYNCPROC glad_debug_glIsSync = glad_debug_impl_glIsSync;
PFNGLISTEXTUREPROC glad_glIsTexture = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsTexture(GLuint texture) {
    GLboolean ret;
    _pre_call_gl_callback("glIsTexture", (GLADapiproc) glad_glIsTexture, 1, texture);
    ret = glad_glIsTexture(texture);
    _post_call_gl_callback((void*) &ret, "glIsTexture", (GLADapiproc) glad_glIsTexture, 1, texture);
    return ret;
}
PFNGLISTEXTUREPROC glad_debug_glIsTexture = glad_debug_impl_glIsTexture;
PFNGLISTRANSFORMFEEDBACKPROC glad_glIsTransformFeedback = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsTransformFeedback(GLuint id) {
    GLboolean ret;
    _pre_call_gl_callback("glIsTransformFeedback", (GLADapiproc) glad_glIsTransformFeedback, 1, id);
    ret = glad_glIsTransformFeedback(id);
    _post_call_gl_callback((void*) &ret, "glIsTransformFeedback", (GLADapiproc) glad_glIsTransformFeedback, 1, id);
    return ret;
}
PFNGLISTRANSFORMFEEDBACKPROC glad_debug_glIsTransformFeedback = glad_debug_impl_glIsTransformFeedback;
PFNGLISVERTEXARRAYPROC glad_glIsVertexArray = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glIsVertexArray(GLuint array) {
    GLboolean ret;
    _pre_call_gl_callback("glIsVertexArray", (GLADapiproc) glad_glIsVertexArray, 1, array);
    ret = glad_glIsVertexArray(array);
    _post_call_gl_callback((void*) &ret, "glIsVertexArray", (GLADapiproc) glad_glIsVertexArray, 1, array);
    return ret;
}
PFNGLISVERTEXARRAYPROC glad_debug_glIsVertexArray = glad_debug_impl_glIsVertexArray;
PFNGLLINEWIDTHPROC glad_glLineWidth = NULL;
static void GLAD_API_PTR glad_debug_impl_glLineWidth(GLfloat width) {
    _pre_call_gl_callback("glLineWidth", (GLADapiproc) glad_glLineWidth, 1, width);
    glad_glLineWidth(width);
    _post_call_gl_callback(NULL, "glLineWidth", (GLADapiproc) glad_glLineWidth, 1, width);
    
}
PFNGLLINEWIDTHPROC glad_debug_glLineWidth = glad_debug_impl_glLineWidth;
PFNGLLINKPROGRAMPROC glad_glLinkProgram = NULL;
static void GLAD_API_PTR glad_debug_impl_glLinkProgram(GLuint program) {
    _pre_call_gl_callback("glLinkProgram", (GLADapiproc) glad_glLinkProgram, 1, program);
    glad_glLinkProgram(program);
    _post_call_gl_callback(NULL, "glLinkProgram", (GLADapiproc) glad_glLinkProgram, 1, program);
    
}
PFNGLLINKPROGRAMPROC glad_debug_glLinkProgram = glad_debug_impl_glLinkProgram;
PFNGLLOGICOPPROC glad_glLogicOp = NULL;
static void GLAD_API_PTR glad_debug_impl_glLogicOp(GLenum opcode) {
    _pre_call_gl_callback("glLogicOp", (GLADapiproc) glad_glLogicOp, 1, opcode);
    glad_glLogicOp(opcode);
    _post_call_gl_callback(NULL, "glLogicOp", (GLADapiproc) glad_glLogicOp, 1, opcode);
    
}
PFNGLLOGICOPPROC glad_debug_glLogicOp = glad_debug_impl_glLogicOp;
PFNGLMAPBUFFERPROC glad_glMapBuffer = NULL;
static void * GLAD_API_PTR glad_debug_impl_glMapBuffer(GLenum target, GLenum access) {
    void * ret;
    _pre_call_gl_callback("glMapBuffer", (GLADapiproc) glad_glMapBuffer, 2, target, access);
    ret = glad_glMapBuffer(target, access);
    _post_call_gl_callback((void*) &ret, "glMapBuffer", (GLADapiproc) glad_glMapBuffer, 2, target, access);
    return ret;
}
PFNGLMAPBUFFERPROC glad_debug_glMapBuffer = glad_debug_impl_glMapBuffer;
PFNGLMAPBUFFERRANGEPROC glad_glMapBufferRange = NULL;
static void * GLAD_API_PTR glad_debug_impl_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {
    void * ret;
    _pre_call_gl_callback("glMapBufferRange", (GLADapiproc) glad_glMapBufferRange, 4, target, offset, length, access);
    ret = glad_glMapBufferRange(target, offset, length, access);
    _post_call_gl_callback((void*) &ret, "glMapBufferRange", (GLADapiproc) glad_glMapBufferRange, 4, target, offset, length, access);
    return ret;
}
PFNGLMAPBUFFERRANGEPROC glad_debug_glMapBufferRange = glad_debug_impl_glMapBufferRange;
PFNGLMINSAMPLESHADINGPROC glad_glMinSampleShading = NULL;
static void GLAD_API_PTR glad_debug_impl_glMinSampleShading(GLfloat value) {
    _pre_call_gl_callback("glMinSampleShading", (GLADapiproc) glad_glMinSampleShading, 1, value);
    glad_glMinSampleShading(value);
    _post_call_gl_callback(NULL, "glMinSampleShading", (GLADapiproc) glad_glMinSampleShading, 1, value);
    
}
PFNGLMINSAMPLESHADINGPROC glad_debug_glMinSampleShading = glad_debug_impl_glMinSampleShading;
PFNGLMULTIDRAWARRAYSPROC glad_glMultiDrawArrays = NULL;
static void GLAD_API_PTR glad_debug_impl_glMultiDrawArrays(GLenum mode, const GLint * first, const GLsizei * count, GLsizei drawcount) {
    _pre_call_gl_callback("glMultiDrawArrays", (GLADapiproc) glad_glMultiDrawArrays, 4, mode, first, count, drawcount);
    glad_glMultiDrawArrays(mode, first, count, drawcount);
    _post_call_gl_callback(NULL, "glMultiDrawArrays", (GLADapiproc) glad_glMultiDrawArrays, 4, mode, first, count, drawcount);
    
}
PFNGLMULTIDRAWARRAYSPROC glad_debug_glMultiDrawArrays = glad_debug_impl_glMultiDrawArrays;
PFNGLMULTIDRAWELEMENTSPROC glad_glMultiDrawElements = NULL;
static void GLAD_API_PTR glad_debug_impl_glMultiDrawElements(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount) {
    _pre_call_gl_callback("glMultiDrawElements", (GLADapiproc) glad_glMultiDrawElements, 5, mode, count, type, indices, drawcount);
    glad_glMultiDrawElements(mode, count, type, indices, drawcount);
    _post_call_gl_callback(NULL, "glMultiDrawElements", (GLADapiproc) glad_glMultiDrawElements, 5, mode, count, type, indices, drawcount);
    
}
PFNGLMULTIDRAWELEMENTSPROC glad_debug_glMultiDrawElements = glad_debug_impl_glMultiDrawElements;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glad_glMultiDrawElementsBaseVertex = NULL;
static void GLAD_API_PTR glad_debug_impl_glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei * count, GLenum type, const void *const* indices, GLsizei drawcount, const GLint * basevertex) {
    _pre_call_gl_callback("glMultiDrawElementsBaseVertex", (GLADapiproc) glad_glMultiDrawElementsBaseVertex, 6, mode, count, type, indices, drawcount, basevertex);
    glad_glMultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
    _post_call_gl_callback(NULL, "glMultiDrawElementsBaseVertex", (GLADapiproc) glad_glMultiDrawElementsBaseVertex, 6, mode, count, type, indices, drawcount, basevertex);
    
}
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC glad_debug_glMultiDrawElementsBaseVertex = glad_debug_impl_glMultiDrawElementsBaseVertex;
PFNGLOBJECTLABELPROC glad_glObjectLabel = NULL;
static void GLAD_API_PTR glad_debug_impl_glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar * label) {
    _pre_call_gl_callback("glObjectLabel", (GLADapiproc) glad_glObjectLabel, 4, identifier, name, length, label);
    glad_glObjectLabel(identifier, name, length, label);
    _post_call_gl_callback(NULL, "glObjectLabel", (GLADapiproc) glad_glObjectLabel, 4, identifier, name, length, label);
    
}
PFNGLOBJECTLABELPROC glad_debug_glObjectLabel = glad_debug_impl_glObjectLabel;
PFNGLOBJECTPTRLABELPROC glad_glObjectPtrLabel = NULL;
static void GLAD_API_PTR glad_debug_impl_glObjectPtrLabel(const void * ptr, GLsizei length, const GLchar * label) {
    _pre_call_gl_callback("glObjectPtrLabel", (GLADapiproc) glad_glObjectPtrLabel, 3, ptr, length, label);
    glad_glObjectPtrLabel(ptr, length, label);
    _post_call_gl_callback(NULL, "glObjectPtrLabel", (GLADapiproc) glad_glObjectPtrLabel, 3, ptr, length, label);
    
}
PFNGLOBJECTPTRLABELPROC glad_debug_glObjectPtrLabel = glad_debug_impl_glObjectPtrLabel;
PFNGLPATCHPARAMETERFVPROC glad_glPatchParameterfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glPatchParameterfv(GLenum pname, const GLfloat * values) {
    _pre_call_gl_callback("glPatchParameterfv", (GLADapiproc) glad_glPatchParameterfv, 2, pname, values);
    glad_glPatchParameterfv(pname, values);
    _post_call_gl_callback(NULL, "glPatchParameterfv", (GLADapiproc) glad_glPatchParameterfv, 2, pname, values);
    
}
PFNGLPATCHPARAMETERFVPROC glad_debug_glPatchParameterfv = glad_debug_impl_glPatchParameterfv;
PFNGLPATCHPARAMETERIPROC glad_glPatchParameteri = NULL;
static void GLAD_API_PTR glad_debug_impl_glPatchParameteri(GLenum pname, GLint value) {
    _pre_call_gl_callback("glPatchParameteri", (GLADapiproc) glad_glPatchParameteri, 2, pname, value);
    glad_glPatchParameteri(pname, value);
    _post_call_gl_callback(NULL, "glPatchParameteri", (GLADapiproc) glad_glPatchParameteri, 2, pname, value);
    
}
PFNGLPATCHPARAMETERIPROC glad_debug_glPatchParameteri = glad_debug_impl_glPatchParameteri;
PFNGLPAUSETRANSFORMFEEDBACKPROC glad_glPauseTransformFeedback = NULL;
static void GLAD_API_PTR glad_debug_impl_glPauseTransformFeedback(void) {
    _pre_call_gl_callback("glPauseTransformFeedback", (GLADapiproc) glad_glPauseTransformFeedback, 0);
    glad_glPauseTransformFeedback();
    _post_call_gl_callback(NULL, "glPauseTransformFeedback", (GLADapiproc) glad_glPauseTransformFeedback, 0);
    
}
PFNGLPAUSETRANSFORMFEEDBACKPROC glad_debug_glPauseTransformFeedback = glad_debug_impl_glPauseTransformFeedback;
PFNGLPIXELSTOREFPROC glad_glPixelStoref = NULL;
static void GLAD_API_PTR glad_debug_impl_glPixelStoref(GLenum pname, GLfloat param) {
    _pre_call_gl_callback("glPixelStoref", (GLADapiproc) glad_glPixelStoref, 2, pname, param);
    glad_glPixelStoref(pname, param);
    _post_call_gl_callback(NULL, "glPixelStoref", (GLADapiproc) glad_glPixelStoref, 2, pname, param);
    
}
PFNGLPIXELSTOREFPROC glad_debug_glPixelStoref = glad_debug_impl_glPixelStoref;
PFNGLPIXELSTOREIPROC glad_glPixelStorei = NULL;
static void GLAD_API_PTR glad_debug_impl_glPixelStorei(GLenum pname, GLint param) {
    _pre_call_gl_callback("glPixelStorei", (GLADapiproc) glad_glPixelStorei, 2, pname, param);
    glad_glPixelStorei(pname, param);
    _post_call_gl_callback(NULL, "glPixelStorei", (GLADapiproc) glad_glPixelStorei, 2, pname, param);
    
}
PFNGLPIXELSTOREIPROC glad_debug_glPixelStorei = glad_debug_impl_glPixelStorei;
PFNGLPOINTPARAMETERFPROC glad_glPointParameterf = NULL;
static void GLAD_API_PTR glad_debug_impl_glPointParameterf(GLenum pname, GLfloat param) {
    _pre_call_gl_callback("glPointParameterf", (GLADapiproc) glad_glPointParameterf, 2, pname, param);
    glad_glPointParameterf(pname, param);
    _post_call_gl_callback(NULL, "glPointParameterf", (GLADapiproc) glad_glPointParameterf, 2, pname, param);
    
}
PFNGLPOINTPARAMETERFPROC glad_debug_glPointParameterf = glad_debug_impl_glPointParameterf;
PFNGLPOINTPARAMETERFVPROC glad_glPointParameterfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glPointParameterfv(GLenum pname, const GLfloat * params) {
    _pre_call_gl_callback("glPointParameterfv", (GLADapiproc) glad_glPointParameterfv, 2, pname, params);
    glad_glPointParameterfv(pname, params);
    _post_call_gl_callback(NULL, "glPointParameterfv", (GLADapiproc) glad_glPointParameterfv, 2, pname, params);
    
}
PFNGLPOINTPARAMETERFVPROC glad_debug_glPointParameterfv = glad_debug_impl_glPointParameterfv;
PFNGLPOINTPARAMETERIPROC glad_glPointParameteri = NULL;
static void GLAD_API_PTR glad_debug_impl_glPointParameteri(GLenum pname, GLint param) {
    _pre_call_gl_callback("glPointParameteri", (GLADapiproc) glad_glPointParameteri, 2, pname, param);
    glad_glPointParameteri(pname, param);
    _post_call_gl_callback(NULL, "glPointParameteri", (GLADapiproc) glad_glPointParameteri, 2, pname, param);
    
}
PFNGLPOINTPARAMETERIPROC glad_debug_glPointParameteri = glad_debug_impl_glPointParameteri;
PFNGLPOINTPARAMETERIVPROC glad_glPointParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glPointParameteriv(GLenum pname, const GLint * params) {
    _pre_call_gl_callback("glPointParameteriv", (GLADapiproc) glad_glPointParameteriv, 2, pname, params);
    glad_glPointParameteriv(pname, params);
    _post_call_gl_callback(NULL, "glPointParameteriv", (GLADapiproc) glad_glPointParameteriv, 2, pname, params);
    
}
PFNGLPOINTPARAMETERIVPROC glad_debug_glPointParameteriv = glad_debug_impl_glPointParameteriv;
PFNGLPOINTSIZEPROC glad_glPointSize = NULL;
static void GLAD_API_PTR glad_debug_impl_glPointSize(GLfloat size) {
    _pre_call_gl_callback("glPointSize", (GLADapiproc) glad_glPointSize, 1, size);
    glad_glPointSize(size);
    _post_call_gl_callback(NULL, "glPointSize", (GLADapiproc) glad_glPointSize, 1, size);
    
}
PFNGLPOINTSIZEPROC glad_debug_glPointSize = glad_debug_impl_glPointSize;
PFNGLPOLYGONMODEPROC glad_glPolygonMode = NULL;
static void GLAD_API_PTR glad_debug_impl_glPolygonMode(GLenum face, GLenum mode) {
    _pre_call_gl_callback("glPolygonMode", (GLADapiproc) glad_glPolygonMode, 2, face, mode);
    glad_glPolygonMode(face, mode);
    _post_call_gl_callback(NULL, "glPolygonMode", (GLADapiproc) glad_glPolygonMode, 2, face, mode);
    
}
PFNGLPOLYGONMODEPROC glad_debug_glPolygonMode = glad_debug_impl_glPolygonMode;
PFNGLPOLYGONOFFSETPROC glad_glPolygonOffset = NULL;
static void GLAD_API_PTR glad_debug_impl_glPolygonOffset(GLfloat factor, GLfloat units) {
    _pre_call_gl_callback("glPolygonOffset", (GLADapiproc) glad_glPolygonOffset, 2, factor, units);
    glad_glPolygonOffset(factor, units);
    _post_call_gl_callback(NULL, "glPolygonOffset", (GLADapiproc) glad_glPolygonOffset, 2, factor, units);
    
}
PFNGLPOLYGONOFFSETPROC glad_debug_glPolygonOffset = glad_debug_impl_glPolygonOffset;
PFNGLPOPDEBUGGROUPPROC glad_glPopDebugGroup = NULL;
static void GLAD_API_PTR glad_debug_impl_glPopDebugGroup(void) {
    _pre_call_gl_callback("glPopDebugGroup", (GLADapiproc) glad_glPopDebugGroup, 0);
    glad_glPopDebugGroup();
    _post_call_gl_callback(NULL, "glPopDebugGroup", (GLADapiproc) glad_glPopDebugGroup, 0);
    
}
PFNGLPOPDEBUGGROUPPROC glad_debug_glPopDebugGroup = glad_debug_impl_glPopDebugGroup;
PFNGLPRIMITIVERESTARTINDEXPROC glad_glPrimitiveRestartIndex = NULL;
static void GLAD_API_PTR glad_debug_impl_glPrimitiveRestartIndex(GLuint index) {
    _pre_call_gl_callback("glPrimitiveRestartIndex", (GLADapiproc) glad_glPrimitiveRestartIndex, 1, index);
    glad_glPrimitiveRestartIndex(index);
    _post_call_gl_callback(NULL, "glPrimitiveRestartIndex", (GLADapiproc) glad_glPrimitiveRestartIndex, 1, index);
    
}
PFNGLPRIMITIVERESTARTINDEXPROC glad_debug_glPrimitiveRestartIndex = glad_debug_impl_glPrimitiveRestartIndex;
PFNGLPROGRAMBINARYPROC glad_glProgramBinary = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramBinary(GLuint program, GLenum binaryFormat, const void * binary, GLsizei length) {
    _pre_call_gl_callback("glProgramBinary", (GLADapiproc) glad_glProgramBinary, 4, program, binaryFormat, binary, length);
    glad_glProgramBinary(program, binaryFormat, binary, length);
    _post_call_gl_callback(NULL, "glProgramBinary", (GLADapiproc) glad_glProgramBinary, 4, program, binaryFormat, binary, length);
    
}
PFNGLPROGRAMBINARYPROC glad_debug_glProgramBinary = glad_debug_impl_glProgramBinary;
PFNGLPROGRAMPARAMETERIPROC glad_glProgramParameteri = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramParameteri(GLuint program, GLenum pname, GLint value) {
    _pre_call_gl_callback("glProgramParameteri", (GLADapiproc) glad_glProgramParameteri, 3, program, pname, value);
    glad_glProgramParameteri(program, pname, value);
    _post_call_gl_callback(NULL, "glProgramParameteri", (GLADapiproc) glad_glProgramParameteri, 3, program, pname, value);
    
}
PFNGLPROGRAMPARAMETERIPROC glad_debug_glProgramParameteri = glad_debug_impl_glProgramParameteri;
PFNGLPROGRAMUNIFORM1DPROC glad_glProgramUniform1d = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1d(GLuint program, GLint location, GLdouble v0) {
    _pre_call_gl_callback("glProgramUniform1d", (GLADapiproc) glad_glProgramUniform1d, 3, program, location, v0);
    glad_glProgramUniform1d(program, location, v0);
    _post_call_gl_callback(NULL, "glProgramUniform1d", (GLADapiproc) glad_glProgramUniform1d, 3, program, location, v0);
    
}
PFNGLPROGRAMUNIFORM1DPROC glad_debug_glProgramUniform1d = glad_debug_impl_glProgramUniform1d;
PFNGLPROGRAMUNIFORM1DVPROC glad_glProgramUniform1dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniform1dv", (GLADapiproc) glad_glProgramUniform1dv, 4, program, location, count, value);
    glad_glProgramUniform1dv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform1dv", (GLADapiproc) glad_glProgramUniform1dv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM1DVPROC glad_debug_glProgramUniform1dv = glad_debug_impl_glProgramUniform1dv;
PFNGLPROGRAMUNIFORM1FPROC glad_glProgramUniform1f = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1f(GLuint program, GLint location, GLfloat v0) {
    _pre_call_gl_callback("glProgramUniform1f", (GLADapiproc) glad_glProgramUniform1f, 3, program, location, v0);
    glad_glProgramUniform1f(program, location, v0);
    _post_call_gl_callback(NULL, "glProgramUniform1f", (GLADapiproc) glad_glProgramUniform1f, 3, program, location, v0);
    
}
PFNGLPROGRAMUNIFORM1FPROC glad_debug_glProgramUniform1f = glad_debug_impl_glProgramUniform1f;
PFNGLPROGRAMUNIFORM1FVPROC glad_glProgramUniform1fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniform1fv", (GLADapiproc) glad_glProgramUniform1fv, 4, program, location, count, value);
    glad_glProgramUniform1fv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform1fv", (GLADapiproc) glad_glProgramUniform1fv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM1FVPROC glad_debug_glProgramUniform1fv = glad_debug_impl_glProgramUniform1fv;
PFNGLPROGRAMUNIFORM1IPROC glad_glProgramUniform1i = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1i(GLuint program, GLint location, GLint v0) {
    _pre_call_gl_callback("glProgramUniform1i", (GLADapiproc) glad_glProgramUniform1i, 3, program, location, v0);
    glad_glProgramUniform1i(program, location, v0);
    _post_call_gl_callback(NULL, "glProgramUniform1i", (GLADapiproc) glad_glProgramUniform1i, 3, program, location, v0);
    
}
PFNGLPROGRAMUNIFORM1IPROC glad_debug_glProgramUniform1i = glad_debug_impl_glProgramUniform1i;
PFNGLPROGRAMUNIFORM1IVPROC glad_glProgramUniform1iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glProgramUniform1iv", (GLADapiproc) glad_glProgramUniform1iv, 4, program, location, count, value);
    glad_glProgramUniform1iv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform1iv", (GLADapiproc) glad_glProgramUniform1iv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM1IVPROC glad_debug_glProgramUniform1iv = glad_debug_impl_glProgramUniform1iv;
PFNGLPROGRAMUNIFORM1UIPROC glad_glProgramUniform1ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1ui(GLuint program, GLint location, GLuint v0) {
    _pre_call_gl_callback("glProgramUniform1ui", (GLADapiproc) glad_glProgramUniform1ui, 3, program, location, v0);
    glad_glProgramUniform1ui(program, location, v0);
    _post_call_gl_callback(NULL, "glProgramUniform1ui", (GLADapiproc) glad_glProgramUniform1ui, 3, program, location, v0);
    
}
PFNGLPROGRAMUNIFORM1UIPROC glad_debug_glProgramUniform1ui = glad_debug_impl_glProgramUniform1ui;
PFNGLPROGRAMUNIFORM1UIVPROC glad_glProgramUniform1uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glProgramUniform1uiv", (GLADapiproc) glad_glProgramUniform1uiv, 4, program, location, count, value);
    glad_glProgramUniform1uiv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform1uiv", (GLADapiproc) glad_glProgramUniform1uiv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM1UIVPROC glad_debug_glProgramUniform1uiv = glad_debug_impl_glProgramUniform1uiv;
PFNGLPROGRAMUNIFORM2DPROC glad_glProgramUniform2d = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1) {
    _pre_call_gl_callback("glProgramUniform2d", (GLADapiproc) glad_glProgramUniform2d, 4, program, location, v0, v1);
    glad_glProgramUniform2d(program, location, v0, v1);
    _post_call_gl_callback(NULL, "glProgramUniform2d", (GLADapiproc) glad_glProgramUniform2d, 4, program, location, v0, v1);
    
}
PFNGLPROGRAMUNIFORM2DPROC glad_debug_glProgramUniform2d = glad_debug_impl_glProgramUniform2d;
PFNGLPROGRAMUNIFORM2DVPROC glad_glProgramUniform2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniform2dv", (GLADapiproc) glad_glProgramUniform2dv, 4, program, location, count, value);
    glad_glProgramUniform2dv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform2dv", (GLADapiproc) glad_glProgramUniform2dv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM2DVPROC glad_debug_glProgramUniform2dv = glad_debug_impl_glProgramUniform2dv;
PFNGLPROGRAMUNIFORM2FPROC glad_glProgramUniform2f = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1) {
    _pre_call_gl_callback("glProgramUniform2f", (GLADapiproc) glad_glProgramUniform2f, 4, program, location, v0, v1);
    glad_glProgramUniform2f(program, location, v0, v1);
    _post_call_gl_callback(NULL, "glProgramUniform2f", (GLADapiproc) glad_glProgramUniform2f, 4, program, location, v0, v1);
    
}
PFNGLPROGRAMUNIFORM2FPROC glad_debug_glProgramUniform2f = glad_debug_impl_glProgramUniform2f;
PFNGLPROGRAMUNIFORM2FVPROC glad_glProgramUniform2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniform2fv", (GLADapiproc) glad_glProgramUniform2fv, 4, program, location, count, value);
    glad_glProgramUniform2fv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform2fv", (GLADapiproc) glad_glProgramUniform2fv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM2FVPROC glad_debug_glProgramUniform2fv = glad_debug_impl_glProgramUniform2fv;
PFNGLPROGRAMUNIFORM2IPROC glad_glProgramUniform2i = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1) {
    _pre_call_gl_callback("glProgramUniform2i", (GLADapiproc) glad_glProgramUniform2i, 4, program, location, v0, v1);
    glad_glProgramUniform2i(program, location, v0, v1);
    _post_call_gl_callback(NULL, "glProgramUniform2i", (GLADapiproc) glad_glProgramUniform2i, 4, program, location, v0, v1);
    
}
PFNGLPROGRAMUNIFORM2IPROC glad_debug_glProgramUniform2i = glad_debug_impl_glProgramUniform2i;
PFNGLPROGRAMUNIFORM2IVPROC glad_glProgramUniform2iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glProgramUniform2iv", (GLADapiproc) glad_glProgramUniform2iv, 4, program, location, count, value);
    glad_glProgramUniform2iv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform2iv", (GLADapiproc) glad_glProgramUniform2iv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM2IVPROC glad_debug_glProgramUniform2iv = glad_debug_impl_glProgramUniform2iv;
PFNGLPROGRAMUNIFORM2UIPROC glad_glProgramUniform2ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1) {
    _pre_call_gl_callback("glProgramUniform2ui", (GLADapiproc) glad_glProgramUniform2ui, 4, program, location, v0, v1);
    glad_glProgramUniform2ui(program, location, v0, v1);
    _post_call_gl_callback(NULL, "glProgramUniform2ui", (GLADapiproc) glad_glProgramUniform2ui, 4, program, location, v0, v1);
    
}
PFNGLPROGRAMUNIFORM2UIPROC glad_debug_glProgramUniform2ui = glad_debug_impl_glProgramUniform2ui;
PFNGLPROGRAMUNIFORM2UIVPROC glad_glProgramUniform2uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glProgramUniform2uiv", (GLADapiproc) glad_glProgramUniform2uiv, 4, program, location, count, value);
    glad_glProgramUniform2uiv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform2uiv", (GLADapiproc) glad_glProgramUniform2uiv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM2UIVPROC glad_debug_glProgramUniform2uiv = glad_debug_impl_glProgramUniform2uiv;
PFNGLPROGRAMUNIFORM3DPROC glad_glProgramUniform3d = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2) {
    _pre_call_gl_callback("glProgramUniform3d", (GLADapiproc) glad_glProgramUniform3d, 5, program, location, v0, v1, v2);
    glad_glProgramUniform3d(program, location, v0, v1, v2);
    _post_call_gl_callback(NULL, "glProgramUniform3d", (GLADapiproc) glad_glProgramUniform3d, 5, program, location, v0, v1, v2);
    
}
PFNGLPROGRAMUNIFORM3DPROC glad_debug_glProgramUniform3d = glad_debug_impl_glProgramUniform3d;
PFNGLPROGRAMUNIFORM3DVPROC glad_glProgramUniform3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniform3dv", (GLADapiproc) glad_glProgramUniform3dv, 4, program, location, count, value);
    glad_glProgramUniform3dv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform3dv", (GLADapiproc) glad_glProgramUniform3dv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM3DVPROC glad_debug_glProgramUniform3dv = glad_debug_impl_glProgramUniform3dv;
PFNGLPROGRAMUNIFORM3FPROC glad_glProgramUniform3f = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    _pre_call_gl_callback("glProgramUniform3f", (GLADapiproc) glad_glProgramUniform3f, 5, program, location, v0, v1, v2);
    glad_glProgramUniform3f(program, location, v0, v1, v2);
    _post_call_gl_callback(NULL, "glProgramUniform3f", (GLADapiproc) glad_glProgramUniform3f, 5, program, location, v0, v1, v2);
    
}
PFNGLPROGRAMUNIFORM3FPROC glad_debug_glProgramUniform3f = glad_debug_impl_glProgramUniform3f;
PFNGLPROGRAMUNIFORM3FVPROC glad_glProgramUniform3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniform3fv", (GLADapiproc) glad_glProgramUniform3fv, 4, program, location, count, value);
    glad_glProgramUniform3fv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform3fv", (GLADapiproc) glad_glProgramUniform3fv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM3FVPROC glad_debug_glProgramUniform3fv = glad_debug_impl_glProgramUniform3fv;
PFNGLPROGRAMUNIFORM3IPROC glad_glProgramUniform3i = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2) {
    _pre_call_gl_callback("glProgramUniform3i", (GLADapiproc) glad_glProgramUniform3i, 5, program, location, v0, v1, v2);
    glad_glProgramUniform3i(program, location, v0, v1, v2);
    _post_call_gl_callback(NULL, "glProgramUniform3i", (GLADapiproc) glad_glProgramUniform3i, 5, program, location, v0, v1, v2);
    
}
PFNGLPROGRAMUNIFORM3IPROC glad_debug_glProgramUniform3i = glad_debug_impl_glProgramUniform3i;
PFNGLPROGRAMUNIFORM3IVPROC glad_glProgramUniform3iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glProgramUniform3iv", (GLADapiproc) glad_glProgramUniform3iv, 4, program, location, count, value);
    glad_glProgramUniform3iv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform3iv", (GLADapiproc) glad_glProgramUniform3iv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM3IVPROC glad_debug_glProgramUniform3iv = glad_debug_impl_glProgramUniform3iv;
PFNGLPROGRAMUNIFORM3UIPROC glad_glProgramUniform3ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2) {
    _pre_call_gl_callback("glProgramUniform3ui", (GLADapiproc) glad_glProgramUniform3ui, 5, program, location, v0, v1, v2);
    glad_glProgramUniform3ui(program, location, v0, v1, v2);
    _post_call_gl_callback(NULL, "glProgramUniform3ui", (GLADapiproc) glad_glProgramUniform3ui, 5, program, location, v0, v1, v2);
    
}
PFNGLPROGRAMUNIFORM3UIPROC glad_debug_glProgramUniform3ui = glad_debug_impl_glProgramUniform3ui;
PFNGLPROGRAMUNIFORM3UIVPROC glad_glProgramUniform3uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glProgramUniform3uiv", (GLADapiproc) glad_glProgramUniform3uiv, 4, program, location, count, value);
    glad_glProgramUniform3uiv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform3uiv", (GLADapiproc) glad_glProgramUniform3uiv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM3UIVPROC glad_debug_glProgramUniform3uiv = glad_debug_impl_glProgramUniform3uiv;
PFNGLPROGRAMUNIFORM4DPROC glad_glProgramUniform4d = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3) {
    _pre_call_gl_callback("glProgramUniform4d", (GLADapiproc) glad_glProgramUniform4d, 6, program, location, v0, v1, v2, v3);
    glad_glProgramUniform4d(program, location, v0, v1, v2, v3);
    _post_call_gl_callback(NULL, "glProgramUniform4d", (GLADapiproc) glad_glProgramUniform4d, 6, program, location, v0, v1, v2, v3);
    
}
PFNGLPROGRAMUNIFORM4DPROC glad_debug_glProgramUniform4d = glad_debug_impl_glProgramUniform4d;
PFNGLPROGRAMUNIFORM4DVPROC glad_glProgramUniform4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniform4dv", (GLADapiproc) glad_glProgramUniform4dv, 4, program, location, count, value);
    glad_glProgramUniform4dv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform4dv", (GLADapiproc) glad_glProgramUniform4dv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM4DVPROC glad_debug_glProgramUniform4dv = glad_debug_impl_glProgramUniform4dv;
PFNGLPROGRAMUNIFORM4FPROC glad_glProgramUniform4f = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    _pre_call_gl_callback("glProgramUniform4f", (GLADapiproc) glad_glProgramUniform4f, 6, program, location, v0, v1, v2, v3);
    glad_glProgramUniform4f(program, location, v0, v1, v2, v3);
    _post_call_gl_callback(NULL, "glProgramUniform4f", (GLADapiproc) glad_glProgramUniform4f, 6, program, location, v0, v1, v2, v3);
    
}
PFNGLPROGRAMUNIFORM4FPROC glad_debug_glProgramUniform4f = glad_debug_impl_glProgramUniform4f;
PFNGLPROGRAMUNIFORM4FVPROC glad_glProgramUniform4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniform4fv", (GLADapiproc) glad_glProgramUniform4fv, 4, program, location, count, value);
    glad_glProgramUniform4fv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform4fv", (GLADapiproc) glad_glProgramUniform4fv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM4FVPROC glad_debug_glProgramUniform4fv = glad_debug_impl_glProgramUniform4fv;
PFNGLPROGRAMUNIFORM4IPROC glad_glProgramUniform4i = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    _pre_call_gl_callback("glProgramUniform4i", (GLADapiproc) glad_glProgramUniform4i, 6, program, location, v0, v1, v2, v3);
    glad_glProgramUniform4i(program, location, v0, v1, v2, v3);
    _post_call_gl_callback(NULL, "glProgramUniform4i", (GLADapiproc) glad_glProgramUniform4i, 6, program, location, v0, v1, v2, v3);
    
}
PFNGLPROGRAMUNIFORM4IPROC glad_debug_glProgramUniform4i = glad_debug_impl_glProgramUniform4i;
PFNGLPROGRAMUNIFORM4IVPROC glad_glProgramUniform4iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glProgramUniform4iv", (GLADapiproc) glad_glProgramUniform4iv, 4, program, location, count, value);
    glad_glProgramUniform4iv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform4iv", (GLADapiproc) glad_glProgramUniform4iv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM4IVPROC glad_debug_glProgramUniform4iv = glad_debug_impl_glProgramUniform4iv;
PFNGLPROGRAMUNIFORM4UIPROC glad_glProgramUniform4ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    _pre_call_gl_callback("glProgramUniform4ui", (GLADapiproc) glad_glProgramUniform4ui, 6, program, location, v0, v1, v2, v3);
    glad_glProgramUniform4ui(program, location, v0, v1, v2, v3);
    _post_call_gl_callback(NULL, "glProgramUniform4ui", (GLADapiproc) glad_glProgramUniform4ui, 6, program, location, v0, v1, v2, v3);
    
}
PFNGLPROGRAMUNIFORM4UIPROC glad_debug_glProgramUniform4ui = glad_debug_impl_glProgramUniform4ui;
PFNGLPROGRAMUNIFORM4UIVPROC glad_glProgramUniform4uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glProgramUniform4uiv", (GLADapiproc) glad_glProgramUniform4uiv, 4, program, location, count, value);
    glad_glProgramUniform4uiv(program, location, count, value);
    _post_call_gl_callback(NULL, "glProgramUniform4uiv", (GLADapiproc) glad_glProgramUniform4uiv, 4, program, location, count, value);
    
}
PFNGLPROGRAMUNIFORM4UIVPROC glad_debug_glProgramUniform4uiv = glad_debug_impl_glProgramUniform4uiv;
PFNGLPROGRAMUNIFORMMATRIX2DVPROC glad_glProgramUniformMatrix2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix2dv", (GLADapiproc) glad_glProgramUniformMatrix2dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix2dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix2dv", (GLADapiproc) glad_glProgramUniformMatrix2dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX2DVPROC glad_debug_glProgramUniformMatrix2dv = glad_debug_impl_glProgramUniformMatrix2dv;
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glad_glProgramUniformMatrix2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix2fv", (GLADapiproc) glad_glProgramUniformMatrix2fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix2fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix2fv", (GLADapiproc) glad_glProgramUniformMatrix2fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX2FVPROC glad_debug_glProgramUniformMatrix2fv = glad_debug_impl_glProgramUniformMatrix2fv;
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glad_glProgramUniformMatrix2x3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix2x3dv", (GLADapiproc) glad_glProgramUniformMatrix2x3dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix2x3dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix2x3dv", (GLADapiproc) glad_glProgramUniformMatrix2x3dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC glad_debug_glProgramUniformMatrix2x3dv = glad_debug_impl_glProgramUniformMatrix2x3dv;
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glad_glProgramUniformMatrix2x3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix2x3fv", (GLADapiproc) glad_glProgramUniformMatrix2x3fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix2x3fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix2x3fv", (GLADapiproc) glad_glProgramUniformMatrix2x3fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC glad_debug_glProgramUniformMatrix2x3fv = glad_debug_impl_glProgramUniformMatrix2x3fv;
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glad_glProgramUniformMatrix2x4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix2x4dv", (GLADapiproc) glad_glProgramUniformMatrix2x4dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix2x4dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix2x4dv", (GLADapiproc) glad_glProgramUniformMatrix2x4dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC glad_debug_glProgramUniformMatrix2x4dv = glad_debug_impl_glProgramUniformMatrix2x4dv;
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glad_glProgramUniformMatrix2x4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix2x4fv", (GLADapiproc) glad_glProgramUniformMatrix2x4fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix2x4fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix2x4fv", (GLADapiproc) glad_glProgramUniformMatrix2x4fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC glad_debug_glProgramUniformMatrix2x4fv = glad_debug_impl_glProgramUniformMatrix2x4fv;
PFNGLPROGRAMUNIFORMMATRIX3DVPROC glad_glProgramUniformMatrix3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix3dv", (GLADapiproc) glad_glProgramUniformMatrix3dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix3dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix3dv", (GLADapiproc) glad_glProgramUniformMatrix3dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX3DVPROC glad_debug_glProgramUniformMatrix3dv = glad_debug_impl_glProgramUniformMatrix3dv;
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glad_glProgramUniformMatrix3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix3fv", (GLADapiproc) glad_glProgramUniformMatrix3fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix3fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix3fv", (GLADapiproc) glad_glProgramUniformMatrix3fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX3FVPROC glad_debug_glProgramUniformMatrix3fv = glad_debug_impl_glProgramUniformMatrix3fv;
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glad_glProgramUniformMatrix3x2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix3x2dv", (GLADapiproc) glad_glProgramUniformMatrix3x2dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix3x2dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix3x2dv", (GLADapiproc) glad_glProgramUniformMatrix3x2dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC glad_debug_glProgramUniformMatrix3x2dv = glad_debug_impl_glProgramUniformMatrix3x2dv;
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glad_glProgramUniformMatrix3x2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix3x2fv", (GLADapiproc) glad_glProgramUniformMatrix3x2fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix3x2fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix3x2fv", (GLADapiproc) glad_glProgramUniformMatrix3x2fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC glad_debug_glProgramUniformMatrix3x2fv = glad_debug_impl_glProgramUniformMatrix3x2fv;
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glad_glProgramUniformMatrix3x4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix3x4dv", (GLADapiproc) glad_glProgramUniformMatrix3x4dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix3x4dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix3x4dv", (GLADapiproc) glad_glProgramUniformMatrix3x4dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC glad_debug_glProgramUniformMatrix3x4dv = glad_debug_impl_glProgramUniformMatrix3x4dv;
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glad_glProgramUniformMatrix3x4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix3x4fv", (GLADapiproc) glad_glProgramUniformMatrix3x4fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix3x4fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix3x4fv", (GLADapiproc) glad_glProgramUniformMatrix3x4fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC glad_debug_glProgramUniformMatrix3x4fv = glad_debug_impl_glProgramUniformMatrix3x4fv;
PFNGLPROGRAMUNIFORMMATRIX4DVPROC glad_glProgramUniformMatrix4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix4dv", (GLADapiproc) glad_glProgramUniformMatrix4dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix4dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix4dv", (GLADapiproc) glad_glProgramUniformMatrix4dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX4DVPROC glad_debug_glProgramUniformMatrix4dv = glad_debug_impl_glProgramUniformMatrix4dv;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glad_glProgramUniformMatrix4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix4fv", (GLADapiproc) glad_glProgramUniformMatrix4fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix4fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix4fv", (GLADapiproc) glad_glProgramUniformMatrix4fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glad_debug_glProgramUniformMatrix4fv = glad_debug_impl_glProgramUniformMatrix4fv;
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glad_glProgramUniformMatrix4x2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix4x2dv", (GLADapiproc) glad_glProgramUniformMatrix4x2dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix4x2dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix4x2dv", (GLADapiproc) glad_glProgramUniformMatrix4x2dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC glad_debug_glProgramUniformMatrix4x2dv = glad_debug_impl_glProgramUniformMatrix4x2dv;
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glad_glProgramUniformMatrix4x2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix4x2fv", (GLADapiproc) glad_glProgramUniformMatrix4x2fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix4x2fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix4x2fv", (GLADapiproc) glad_glProgramUniformMatrix4x2fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC glad_debug_glProgramUniformMatrix4x2fv = glad_debug_impl_glProgramUniformMatrix4x2fv;
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glad_glProgramUniformMatrix4x3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glProgramUniformMatrix4x3dv", (GLADapiproc) glad_glProgramUniformMatrix4x3dv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix4x3dv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix4x3dv", (GLADapiproc) glad_glProgramUniformMatrix4x3dv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC glad_debug_glProgramUniformMatrix4x3dv = glad_debug_impl_glProgramUniformMatrix4x3dv;
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glad_glProgramUniformMatrix4x3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glProgramUniformMatrix4x3fv", (GLADapiproc) glad_glProgramUniformMatrix4x3fv, 5, program, location, count, transpose, value);
    glad_glProgramUniformMatrix4x3fv(program, location, count, transpose, value);
    _post_call_gl_callback(NULL, "glProgramUniformMatrix4x3fv", (GLADapiproc) glad_glProgramUniformMatrix4x3fv, 5, program, location, count, transpose, value);
    
}
PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC glad_debug_glProgramUniformMatrix4x3fv = glad_debug_impl_glProgramUniformMatrix4x3fv;
PFNGLPROVOKINGVERTEXPROC glad_glProvokingVertex = NULL;
static void GLAD_API_PTR glad_debug_impl_glProvokingVertex(GLenum mode) {
    _pre_call_gl_callback("glProvokingVertex", (GLADapiproc) glad_glProvokingVertex, 1, mode);
    glad_glProvokingVertex(mode);
    _post_call_gl_callback(NULL, "glProvokingVertex", (GLADapiproc) glad_glProvokingVertex, 1, mode);
    
}
PFNGLPROVOKINGVERTEXPROC glad_debug_glProvokingVertex = glad_debug_impl_glProvokingVertex;
PFNGLPUSHDEBUGGROUPPROC glad_glPushDebugGroup = NULL;
static void GLAD_API_PTR glad_debug_impl_glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar * message) {
    _pre_call_gl_callback("glPushDebugGroup", (GLADapiproc) glad_glPushDebugGroup, 4, source, id, length, message);
    glad_glPushDebugGroup(source, id, length, message);
    _post_call_gl_callback(NULL, "glPushDebugGroup", (GLADapiproc) glad_glPushDebugGroup, 4, source, id, length, message);
    
}
PFNGLPUSHDEBUGGROUPPROC glad_debug_glPushDebugGroup = glad_debug_impl_glPushDebugGroup;
PFNGLQUERYCOUNTERPROC glad_glQueryCounter = NULL;
static void GLAD_API_PTR glad_debug_impl_glQueryCounter(GLuint id, GLenum target) {
    _pre_call_gl_callback("glQueryCounter", (GLADapiproc) glad_glQueryCounter, 2, id, target);
    glad_glQueryCounter(id, target);
    _post_call_gl_callback(NULL, "glQueryCounter", (GLADapiproc) glad_glQueryCounter, 2, id, target);
    
}
PFNGLQUERYCOUNTERPROC glad_debug_glQueryCounter = glad_debug_impl_glQueryCounter;
PFNGLREADBUFFERPROC glad_glReadBuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glReadBuffer(GLenum src) {
    _pre_call_gl_callback("glReadBuffer", (GLADapiproc) glad_glReadBuffer, 1, src);
    glad_glReadBuffer(src);
    _post_call_gl_callback(NULL, "glReadBuffer", (GLADapiproc) glad_glReadBuffer, 1, src);
    
}
PFNGLREADBUFFERPROC glad_debug_glReadBuffer = glad_debug_impl_glReadBuffer;
PFNGLREADPIXELSPROC glad_glReadPixels = NULL;
static void GLAD_API_PTR glad_debug_impl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels) {
    _pre_call_gl_callback("glReadPixels", (GLADapiproc) glad_glReadPixels, 7, x, y, width, height, format, type, pixels);
    glad_glReadPixels(x, y, width, height, format, type, pixels);
    _post_call_gl_callback(NULL, "glReadPixels", (GLADapiproc) glad_glReadPixels, 7, x, y, width, height, format, type, pixels);
    
}
PFNGLREADPIXELSPROC glad_debug_glReadPixels = glad_debug_impl_glReadPixels;
PFNGLRELEASESHADERCOMPILERPROC glad_glReleaseShaderCompiler = NULL;
static void GLAD_API_PTR glad_debug_impl_glReleaseShaderCompiler(void) {
    _pre_call_gl_callback("glReleaseShaderCompiler", (GLADapiproc) glad_glReleaseShaderCompiler, 0);
    glad_glReleaseShaderCompiler();
    _post_call_gl_callback(NULL, "glReleaseShaderCompiler", (GLADapiproc) glad_glReleaseShaderCompiler, 0);
    
}
PFNGLRELEASESHADERCOMPILERPROC glad_debug_glReleaseShaderCompiler = glad_debug_impl_glReleaseShaderCompiler;
PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage = NULL;
static void GLAD_API_PTR glad_debug_impl_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) {
    _pre_call_gl_callback("glRenderbufferStorage", (GLADapiproc) glad_glRenderbufferStorage, 4, target, internalformat, width, height);
    glad_glRenderbufferStorage(target, internalformat, width, height);
    _post_call_gl_callback(NULL, "glRenderbufferStorage", (GLADapiproc) glad_glRenderbufferStorage, 4, target, internalformat, width, height);
    
}
PFNGLRENDERBUFFERSTORAGEPROC glad_debug_glRenderbufferStorage = glad_debug_impl_glRenderbufferStorage;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glad_glRenderbufferStorageMultisample = NULL;
static void GLAD_API_PTR glad_debug_impl_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) {
    _pre_call_gl_callback("glRenderbufferStorageMultisample", (GLADapiproc) glad_glRenderbufferStorageMultisample, 5, target, samples, internalformat, width, height);
    glad_glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
    _post_call_gl_callback(NULL, "glRenderbufferStorageMultisample", (GLADapiproc) glad_glRenderbufferStorageMultisample, 5, target, samples, internalformat, width, height);
    
}
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glad_debug_glRenderbufferStorageMultisample = glad_debug_impl_glRenderbufferStorageMultisample;
PFNGLRESUMETRANSFORMFEEDBACKPROC glad_glResumeTransformFeedback = NULL;
static void GLAD_API_PTR glad_debug_impl_glResumeTransformFeedback(void) {
    _pre_call_gl_callback("glResumeTransformFeedback", (GLADapiproc) glad_glResumeTransformFeedback, 0);
    glad_glResumeTransformFeedback();
    _post_call_gl_callback(NULL, "glResumeTransformFeedback", (GLADapiproc) glad_glResumeTransformFeedback, 0);
    
}
PFNGLRESUMETRANSFORMFEEDBACKPROC glad_debug_glResumeTransformFeedback = glad_debug_impl_glResumeTransformFeedback;
PFNGLSAMPLECOVERAGEPROC glad_glSampleCoverage = NULL;
static void GLAD_API_PTR glad_debug_impl_glSampleCoverage(GLfloat value, GLboolean invert) {
    _pre_call_gl_callback("glSampleCoverage", (GLADapiproc) glad_glSampleCoverage, 2, value, invert);
    glad_glSampleCoverage(value, invert);
    _post_call_gl_callback(NULL, "glSampleCoverage", (GLADapiproc) glad_glSampleCoverage, 2, value, invert);
    
}
PFNGLSAMPLECOVERAGEPROC glad_debug_glSampleCoverage = glad_debug_impl_glSampleCoverage;
PFNGLSAMPLEMASKIPROC glad_glSampleMaski = NULL;
static void GLAD_API_PTR glad_debug_impl_glSampleMaski(GLuint maskNumber, GLbitfield mask) {
    _pre_call_gl_callback("glSampleMaski", (GLADapiproc) glad_glSampleMaski, 2, maskNumber, mask);
    glad_glSampleMaski(maskNumber, mask);
    _post_call_gl_callback(NULL, "glSampleMaski", (GLADapiproc) glad_glSampleMaski, 2, maskNumber, mask);
    
}
PFNGLSAMPLEMASKIPROC glad_debug_glSampleMaski = glad_debug_impl_glSampleMaski;
PFNGLSAMPLERPARAMETERIIVPROC glad_glSamplerParameterIiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint * param) {
    _pre_call_gl_callback("glSamplerParameterIiv", (GLADapiproc) glad_glSamplerParameterIiv, 3, sampler, pname, param);
    glad_glSamplerParameterIiv(sampler, pname, param);
    _post_call_gl_callback(NULL, "glSamplerParameterIiv", (GLADapiproc) glad_glSamplerParameterIiv, 3, sampler, pname, param);
    
}
PFNGLSAMPLERPARAMETERIIVPROC glad_debug_glSamplerParameterIiv = glad_debug_impl_glSamplerParameterIiv;
PFNGLSAMPLERPARAMETERIUIVPROC glad_glSamplerParameterIuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint * param) {
    _pre_call_gl_callback("glSamplerParameterIuiv", (GLADapiproc) glad_glSamplerParameterIuiv, 3, sampler, pname, param);
    glad_glSamplerParameterIuiv(sampler, pname, param);
    _post_call_gl_callback(NULL, "glSamplerParameterIuiv", (GLADapiproc) glad_glSamplerParameterIuiv, 3, sampler, pname, param);
    
}
PFNGLSAMPLERPARAMETERIUIVPROC glad_debug_glSamplerParameterIuiv = glad_debug_impl_glSamplerParameterIuiv;
PFNGLSAMPLERPARAMETERFPROC glad_glSamplerParameterf = NULL;
static void GLAD_API_PTR glad_debug_impl_glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param) {
    _pre_call_gl_callback("glSamplerParameterf", (GLADapiproc) glad_glSamplerParameterf, 3, sampler, pname, param);
    glad_glSamplerParameterf(sampler, pname, param);
    _post_call_gl_callback(NULL, "glSamplerParameterf", (GLADapiproc) glad_glSamplerParameterf, 3, sampler, pname, param);
    
}
PFNGLSAMPLERPARAMETERFPROC glad_debug_glSamplerParameterf = glad_debug_impl_glSamplerParameterf;
PFNGLSAMPLERPARAMETERFVPROC glad_glSamplerParameterfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat * param) {
    _pre_call_gl_callback("glSamplerParameterfv", (GLADapiproc) glad_glSamplerParameterfv, 3, sampler, pname, param);
    glad_glSamplerParameterfv(sampler, pname, param);
    _post_call_gl_callback(NULL, "glSamplerParameterfv", (GLADapiproc) glad_glSamplerParameterfv, 3, sampler, pname, param);
    
}
PFNGLSAMPLERPARAMETERFVPROC glad_debug_glSamplerParameterfv = glad_debug_impl_glSamplerParameterfv;
PFNGLSAMPLERPARAMETERIPROC glad_glSamplerParameteri = NULL;
static void GLAD_API_PTR glad_debug_impl_glSamplerParameteri(GLuint sampler, GLenum pname, GLint param) {
    _pre_call_gl_callback("glSamplerParameteri", (GLADapiproc) glad_glSamplerParameteri, 3, sampler, pname, param);
    glad_glSamplerParameteri(sampler, pname, param);
    _post_call_gl_callback(NULL, "glSamplerParameteri", (GLADapiproc) glad_glSamplerParameteri, 3, sampler, pname, param);
    
}
PFNGLSAMPLERPARAMETERIPROC glad_debug_glSamplerParameteri = glad_debug_impl_glSamplerParameteri;
PFNGLSAMPLERPARAMETERIVPROC glad_glSamplerParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint * param) {
    _pre_call_gl_callback("glSamplerParameteriv", (GLADapiproc) glad_glSamplerParameteriv, 3, sampler, pname, param);
    glad_glSamplerParameteriv(sampler, pname, param);
    _post_call_gl_callback(NULL, "glSamplerParameteriv", (GLADapiproc) glad_glSamplerParameteriv, 3, sampler, pname, param);
    
}
PFNGLSAMPLERPARAMETERIVPROC glad_debug_glSamplerParameteriv = glad_debug_impl_glSamplerParameteriv;
PFNGLSCISSORPROC glad_glScissor = NULL;
static void GLAD_API_PTR glad_debug_impl_glScissor(GLint x, GLint y, GLsizei width, GLsizei height) {
    _pre_call_gl_callback("glScissor", (GLADapiproc) glad_glScissor, 4, x, y, width, height);
    glad_glScissor(x, y, width, height);
    _post_call_gl_callback(NULL, "glScissor", (GLADapiproc) glad_glScissor, 4, x, y, width, height);
    
}
PFNGLSCISSORPROC glad_debug_glScissor = glad_debug_impl_glScissor;
PFNGLSCISSORARRAYVPROC glad_glScissorArrayv = NULL;
static void GLAD_API_PTR glad_debug_impl_glScissorArrayv(GLuint first, GLsizei count, const GLint * v) {
    _pre_call_gl_callback("glScissorArrayv", (GLADapiproc) glad_glScissorArrayv, 3, first, count, v);
    glad_glScissorArrayv(first, count, v);
    _post_call_gl_callback(NULL, "glScissorArrayv", (GLADapiproc) glad_glScissorArrayv, 3, first, count, v);
    
}
PFNGLSCISSORARRAYVPROC glad_debug_glScissorArrayv = glad_debug_impl_glScissorArrayv;
PFNGLSCISSORINDEXEDPROC glad_glScissorIndexed = NULL;
static void GLAD_API_PTR glad_debug_impl_glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height) {
    _pre_call_gl_callback("glScissorIndexed", (GLADapiproc) glad_glScissorIndexed, 5, index, left, bottom, width, height);
    glad_glScissorIndexed(index, left, bottom, width, height);
    _post_call_gl_callback(NULL, "glScissorIndexed", (GLADapiproc) glad_glScissorIndexed, 5, index, left, bottom, width, height);
    
}
PFNGLSCISSORINDEXEDPROC glad_debug_glScissorIndexed = glad_debug_impl_glScissorIndexed;
PFNGLSCISSORINDEXEDVPROC glad_glScissorIndexedv = NULL;
static void GLAD_API_PTR glad_debug_impl_glScissorIndexedv(GLuint index, const GLint * v) {
    _pre_call_gl_callback("glScissorIndexedv", (GLADapiproc) glad_glScissorIndexedv, 2, index, v);
    glad_glScissorIndexedv(index, v);
    _post_call_gl_callback(NULL, "glScissorIndexedv", (GLADapiproc) glad_glScissorIndexedv, 2, index, v);
    
}
PFNGLSCISSORINDEXEDVPROC glad_debug_glScissorIndexedv = glad_debug_impl_glScissorIndexedv;
PFNGLSHADERBINARYPROC glad_glShaderBinary = NULL;
static void GLAD_API_PTR glad_debug_impl_glShaderBinary(GLsizei count, const GLuint * shaders, GLenum binaryFormat, const void * binary, GLsizei length) {
    _pre_call_gl_callback("glShaderBinary", (GLADapiproc) glad_glShaderBinary, 5, count, shaders, binaryFormat, binary, length);
    glad_glShaderBinary(count, shaders, binaryFormat, binary, length);
    _post_call_gl_callback(NULL, "glShaderBinary", (GLADapiproc) glad_glShaderBinary, 5, count, shaders, binaryFormat, binary, length);
    
}
PFNGLSHADERBINARYPROC glad_debug_glShaderBinary = glad_debug_impl_glShaderBinary;
PFNGLSHADERSOURCEPROC glad_glShaderSource = NULL;
static void GLAD_API_PTR glad_debug_impl_glShaderSource(GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length) {
    _pre_call_gl_callback("glShaderSource", (GLADapiproc) glad_glShaderSource, 4, shader, count, string, length);
    glad_glShaderSource(shader, count, string, length);
    _post_call_gl_callback(NULL, "glShaderSource", (GLADapiproc) glad_glShaderSource, 4, shader, count, string, length);
    
}
PFNGLSHADERSOURCEPROC glad_debug_glShaderSource = glad_debug_impl_glShaderSource;
PFNGLSTENCILFUNCPROC glad_glStencilFunc = NULL;
static void GLAD_API_PTR glad_debug_impl_glStencilFunc(GLenum func, GLint ref, GLuint mask) {
    _pre_call_gl_callback("glStencilFunc", (GLADapiproc) glad_glStencilFunc, 3, func, ref, mask);
    glad_glStencilFunc(func, ref, mask);
    _post_call_gl_callback(NULL, "glStencilFunc", (GLADapiproc) glad_glStencilFunc, 3, func, ref, mask);
    
}
PFNGLSTENCILFUNCPROC glad_debug_glStencilFunc = glad_debug_impl_glStencilFunc;
PFNGLSTENCILFUNCSEPARATEPROC glad_glStencilFuncSeparate = NULL;
static void GLAD_API_PTR glad_debug_impl_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask) {
    _pre_call_gl_callback("glStencilFuncSeparate", (GLADapiproc) glad_glStencilFuncSeparate, 4, face, func, ref, mask);
    glad_glStencilFuncSeparate(face, func, ref, mask);
    _post_call_gl_callback(NULL, "glStencilFuncSeparate", (GLADapiproc) glad_glStencilFuncSeparate, 4, face, func, ref, mask);
    
}
PFNGLSTENCILFUNCSEPARATEPROC glad_debug_glStencilFuncSeparate = glad_debug_impl_glStencilFuncSeparate;
PFNGLSTENCILMASKPROC glad_glStencilMask = NULL;
static void GLAD_API_PTR glad_debug_impl_glStencilMask(GLuint mask) {
    _pre_call_gl_callback("glStencilMask", (GLADapiproc) glad_glStencilMask, 1, mask);
    glad_glStencilMask(mask);
    _post_call_gl_callback(NULL, "glStencilMask", (GLADapiproc) glad_glStencilMask, 1, mask);
    
}
PFNGLSTENCILMASKPROC glad_debug_glStencilMask = glad_debug_impl_glStencilMask;
PFNGLSTENCILMASKSEPARATEPROC glad_glStencilMaskSeparate = NULL;
static void GLAD_API_PTR glad_debug_impl_glStencilMaskSeparate(GLenum face, GLuint mask) {
    _pre_call_gl_callback("glStencilMaskSeparate", (GLADapiproc) glad_glStencilMaskSeparate, 2, face, mask);
    glad_glStencilMaskSeparate(face, mask);
    _post_call_gl_callback(NULL, "glStencilMaskSeparate", (GLADapiproc) glad_glStencilMaskSeparate, 2, face, mask);
    
}
PFNGLSTENCILMASKSEPARATEPROC glad_debug_glStencilMaskSeparate = glad_debug_impl_glStencilMaskSeparate;
PFNGLSTENCILOPPROC glad_glStencilOp = NULL;
static void GLAD_API_PTR glad_debug_impl_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    _pre_call_gl_callback("glStencilOp", (GLADapiproc) glad_glStencilOp, 3, fail, zfail, zpass);
    glad_glStencilOp(fail, zfail, zpass);
    _post_call_gl_callback(NULL, "glStencilOp", (GLADapiproc) glad_glStencilOp, 3, fail, zfail, zpass);
    
}
PFNGLSTENCILOPPROC glad_debug_glStencilOp = glad_debug_impl_glStencilOp;
PFNGLSTENCILOPSEPARATEPROC glad_glStencilOpSeparate = NULL;
static void GLAD_API_PTR glad_debug_impl_glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass) {
    _pre_call_gl_callback("glStencilOpSeparate", (GLADapiproc) glad_glStencilOpSeparate, 4, face, sfail, dpfail, dppass);
    glad_glStencilOpSeparate(face, sfail, dpfail, dppass);
    _post_call_gl_callback(NULL, "glStencilOpSeparate", (GLADapiproc) glad_glStencilOpSeparate, 4, face, sfail, dpfail, dppass);
    
}
PFNGLSTENCILOPSEPARATEPROC glad_debug_glStencilOpSeparate = glad_debug_impl_glStencilOpSeparate;
PFNGLTEXBUFFERPROC glad_glTexBuffer = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer) {
    _pre_call_gl_callback("glTexBuffer", (GLADapiproc) glad_glTexBuffer, 3, target, internalformat, buffer);
    glad_glTexBuffer(target, internalformat, buffer);
    _post_call_gl_callback(NULL, "glTexBuffer", (GLADapiproc) glad_glTexBuffer, 3, target, internalformat, buffer);
    
}
PFNGLTEXBUFFERPROC glad_debug_glTexBuffer = glad_debug_impl_glTexBuffer;
PFNGLTEXIMAGE1DPROC glad_glTexImage1D = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void * pixels) {
    _pre_call_gl_callback("glTexImage1D", (GLADapiproc) glad_glTexImage1D, 8, target, level, internalformat, width, border, format, type, pixels);
    glad_glTexImage1D(target, level, internalformat, width, border, format, type, pixels);
    _post_call_gl_callback(NULL, "glTexImage1D", (GLADapiproc) glad_glTexImage1D, 8, target, level, internalformat, width, border, format, type, pixels);
    
}
PFNGLTEXIMAGE1DPROC glad_debug_glTexImage1D = glad_debug_impl_glTexImage1D;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels) {
    _pre_call_gl_callback("glTexImage2D", (GLADapiproc) glad_glTexImage2D, 9, target, level, internalformat, width, height, border, format, type, pixels);
    glad_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
    _post_call_gl_callback(NULL, "glTexImage2D", (GLADapiproc) glad_glTexImage2D, 9, target, level, internalformat, width, height, border, format, type, pixels);
    
}
PFNGLTEXIMAGE2DPROC glad_debug_glTexImage2D = glad_debug_impl_glTexImage2D;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glad_glTexImage2DMultisample = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
    _pre_call_gl_callback("glTexImage2DMultisample", (GLADapiproc) glad_glTexImage2DMultisample, 6, target, samples, internalformat, width, height, fixedsamplelocations);
    glad_glTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
    _post_call_gl_callback(NULL, "glTexImage2DMultisample", (GLADapiproc) glad_glTexImage2DMultisample, 6, target, samples, internalformat, width, height, fixedsamplelocations);
    
}
PFNGLTEXIMAGE2DMULTISAMPLEPROC glad_debug_glTexImage2DMultisample = glad_debug_impl_glTexImage2DMultisample;
PFNGLTEXIMAGE3DPROC glad_glTexImage3D = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * pixels) {
    _pre_call_gl_callback("glTexImage3D", (GLADapiproc) glad_glTexImage3D, 10, target, level, internalformat, width, height, depth, border, format, type, pixels);
    glad_glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
    _post_call_gl_callback(NULL, "glTexImage3D", (GLADapiproc) glad_glTexImage3D, 10, target, level, internalformat, width, height, depth, border, format, type, pixels);
    
}
PFNGLTEXIMAGE3DPROC glad_debug_glTexImage3D = glad_debug_impl_glTexImage3D;
PFNGLTEXIMAGE3DMULTISAMPLEPROC glad_glTexImage3DMultisample = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) {
    _pre_call_gl_callback("glTexImage3DMultisample", (GLADapiproc) glad_glTexImage3DMultisample, 7, target, samples, internalformat, width, height, depth, fixedsamplelocations);
    glad_glTexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
    _post_call_gl_callback(NULL, "glTexImage3DMultisample", (GLADapiproc) glad_glTexImage3DMultisample, 7, target, samples, internalformat, width, height, depth, fixedsamplelocations);
    
}
PFNGLTEXIMAGE3DMULTISAMPLEPROC glad_debug_glTexImage3DMultisample = glad_debug_impl_glTexImage3DMultisample;
PFNGLTEXPARAMETERIIVPROC glad_glTexParameterIiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexParameterIiv(GLenum target, GLenum pname, const GLint * params) {
    _pre_call_gl_callback("glTexParameterIiv", (GLADapiproc) glad_glTexParameterIiv, 3, target, pname, params);
    glad_glTexParameterIiv(target, pname, params);
    _post_call_gl_callback(NULL, "glTexParameterIiv", (GLADapiproc) glad_glTexParameterIiv, 3, target, pname, params);
    
}
PFNGLTEXPARAMETERIIVPROC glad_debug_glTexParameterIiv = glad_debug_impl_glTexParameterIiv;
PFNGLTEXPARAMETERIUIVPROC glad_glTexParameterIuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexParameterIuiv(GLenum target, GLenum pname, const GLuint * params) {
    _pre_call_gl_callback("glTexParameterIuiv", (GLADapiproc) glad_glTexParameterIuiv, 3, target, pname, params);
    glad_glTexParameterIuiv(target, pname, params);
    _post_call_gl_callback(NULL, "glTexParameterIuiv", (GLADapiproc) glad_glTexParameterIuiv, 3, target, pname, params);
    
}
PFNGLTEXPARAMETERIUIVPROC glad_debug_glTexParameterIuiv = glad_debug_impl_glTexParameterIuiv;
PFNGLTEXPARAMETERFPROC glad_glTexParameterf = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexParameterf(GLenum target, GLenum pname, GLfloat param) {
    _pre_call_gl_callback("glTexParameterf", (GLADapiproc) glad_glTexParameterf, 3, target, pname, param);
    glad_glTexParameterf(target, pname, param);
    _post_call_gl_callback(NULL, "glTexParameterf", (GLADapiproc) glad_glTexParameterf, 3, target, pname, param);
    
}
PFNGLTEXPARAMETERFPROC glad_debug_glTexParameterf = glad_debug_impl_glTexParameterf;
PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexParameterfv(GLenum target, GLenum pname, const GLfloat * params) {
    _pre_call_gl_callback("glTexParameterfv", (GLADapiproc) glad_glTexParameterfv, 3, target, pname, params);
    glad_glTexParameterfv(target, pname, params);
    _post_call_gl_callback(NULL, "glTexParameterfv", (GLADapiproc) glad_glTexParameterfv, 3, target, pname, params);
    
}
PFNGLTEXPARAMETERFVPROC glad_debug_glTexParameterfv = glad_debug_impl_glTexParameterfv;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexParameteri(GLenum target, GLenum pname, GLint param) {
    _pre_call_gl_callback("glTexParameteri", (GLADapiproc) glad_glTexParameteri, 3, target, pname, param);
    glad_glTexParameteri(target, pname, param);
    _post_call_gl_callback(NULL, "glTexParameteri", (GLADapiproc) glad_glTexParameteri, 3, target, pname, param);
    
}
PFNGLTEXPARAMETERIPROC glad_debug_glTexParameteri = glad_debug_impl_glTexParameteri;
PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexParameteriv(GLenum target, GLenum pname, const GLint * params) {
    _pre_call_gl_callback("glTexParameteriv", (GLADapiproc) glad_glTexParameteriv, 3, target, pname, params);
    glad_glTexParameteriv(target, pname, params);
    _post_call_gl_callback(NULL, "glTexParameteriv", (GLADapiproc) glad_glTexParameteriv, 3, target, pname, params);
    
}
PFNGLTEXPARAMETERIVPROC glad_debug_glTexParameteriv = glad_debug_impl_glTexParameteriv;
PFNGLTEXSUBIMAGE1DPROC glad_glTexSubImage1D = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void * pixels) {
    _pre_call_gl_callback("glTexSubImage1D", (GLADapiproc) glad_glTexSubImage1D, 7, target, level, xoffset, width, format, type, pixels);
    glad_glTexSubImage1D(target, level, xoffset, width, format, type, pixels);
    _post_call_gl_callback(NULL, "glTexSubImage1D", (GLADapiproc) glad_glTexSubImage1D, 7, target, level, xoffset, width, format, type, pixels);
    
}
PFNGLTEXSUBIMAGE1DPROC glad_debug_glTexSubImage1D = glad_debug_impl_glTexSubImage1D;
PFNGLTEXSUBIMAGE2DPROC glad_glTexSubImage2D = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels) {
    _pre_call_gl_callback("glTexSubImage2D", (GLADapiproc) glad_glTexSubImage2D, 9, target, level, xoffset, yoffset, width, height, format, type, pixels);
    glad_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
    _post_call_gl_callback(NULL, "glTexSubImage2D", (GLADapiproc) glad_glTexSubImage2D, 9, target, level, xoffset, yoffset, width, height, format, type, pixels);
    
}
PFNGLTEXSUBIMAGE2DPROC glad_debug_glTexSubImage2D = glad_debug_impl_glTexSubImage2D;
PFNGLTEXSUBIMAGE3DPROC glad_glTexSubImage3D = NULL;
static void GLAD_API_PTR glad_debug_impl_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void * pixels) {
    _pre_call_gl_callback("glTexSubImage3D", (GLADapiproc) glad_glTexSubImage3D, 11, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    glad_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    _post_call_gl_callback(NULL, "glTexSubImage3D", (GLADapiproc) glad_glTexSubImage3D, 11, target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
    
}
PFNGLTEXSUBIMAGE3DPROC glad_debug_glTexSubImage3D = glad_debug_impl_glTexSubImage3D;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glad_glTransformFeedbackVaryings = NULL;
static void GLAD_API_PTR glad_debug_impl_glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode) {
    _pre_call_gl_callback("glTransformFeedbackVaryings", (GLADapiproc) glad_glTransformFeedbackVaryings, 4, program, count, varyings, bufferMode);
    glad_glTransformFeedbackVaryings(program, count, varyings, bufferMode);
    _post_call_gl_callback(NULL, "glTransformFeedbackVaryings", (GLADapiproc) glad_glTransformFeedbackVaryings, 4, program, count, varyings, bufferMode);
    
}
PFNGLTRANSFORMFEEDBACKVARYINGSPROC glad_debug_glTransformFeedbackVaryings = glad_debug_impl_glTransformFeedbackVaryings;
PFNGLUNIFORM1DPROC glad_glUniform1d = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1d(GLint location, GLdouble x) {
    _pre_call_gl_callback("glUniform1d", (GLADapiproc) glad_glUniform1d, 2, location, x);
    glad_glUniform1d(location, x);
    _post_call_gl_callback(NULL, "glUniform1d", (GLADapiproc) glad_glUniform1d, 2, location, x);
    
}
PFNGLUNIFORM1DPROC glad_debug_glUniform1d = glad_debug_impl_glUniform1d;
PFNGLUNIFORM1DVPROC glad_glUniform1dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1dv(GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glUniform1dv", (GLADapiproc) glad_glUniform1dv, 3, location, count, value);
    glad_glUniform1dv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform1dv", (GLADapiproc) glad_glUniform1dv, 3, location, count, value);
    
}
PFNGLUNIFORM1DVPROC glad_debug_glUniform1dv = glad_debug_impl_glUniform1dv;
PFNGLUNIFORM1FPROC glad_glUniform1f = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1f(GLint location, GLfloat v0) {
    _pre_call_gl_callback("glUniform1f", (GLADapiproc) glad_glUniform1f, 2, location, v0);
    glad_glUniform1f(location, v0);
    _post_call_gl_callback(NULL, "glUniform1f", (GLADapiproc) glad_glUniform1f, 2, location, v0);
    
}
PFNGLUNIFORM1FPROC glad_debug_glUniform1f = glad_debug_impl_glUniform1f;
PFNGLUNIFORM1FVPROC glad_glUniform1fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1fv(GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glUniform1fv", (GLADapiproc) glad_glUniform1fv, 3, location, count, value);
    glad_glUniform1fv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform1fv", (GLADapiproc) glad_glUniform1fv, 3, location, count, value);
    
}
PFNGLUNIFORM1FVPROC glad_debug_glUniform1fv = glad_debug_impl_glUniform1fv;
PFNGLUNIFORM1IPROC glad_glUniform1i = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1i(GLint location, GLint v0) {
    _pre_call_gl_callback("glUniform1i", (GLADapiproc) glad_glUniform1i, 2, location, v0);
    glad_glUniform1i(location, v0);
    _post_call_gl_callback(NULL, "glUniform1i", (GLADapiproc) glad_glUniform1i, 2, location, v0);
    
}
PFNGLUNIFORM1IPROC glad_debug_glUniform1i = glad_debug_impl_glUniform1i;
PFNGLUNIFORM1IVPROC glad_glUniform1iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1iv(GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glUniform1iv", (GLADapiproc) glad_glUniform1iv, 3, location, count, value);
    glad_glUniform1iv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform1iv", (GLADapiproc) glad_glUniform1iv, 3, location, count, value);
    
}
PFNGLUNIFORM1IVPROC glad_debug_glUniform1iv = glad_debug_impl_glUniform1iv;
PFNGLUNIFORM1UIPROC glad_glUniform1ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1ui(GLint location, GLuint v0) {
    _pre_call_gl_callback("glUniform1ui", (GLADapiproc) glad_glUniform1ui, 2, location, v0);
    glad_glUniform1ui(location, v0);
    _post_call_gl_callback(NULL, "glUniform1ui", (GLADapiproc) glad_glUniform1ui, 2, location, v0);
    
}
PFNGLUNIFORM1UIPROC glad_debug_glUniform1ui = glad_debug_impl_glUniform1ui;
PFNGLUNIFORM1UIVPROC glad_glUniform1uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform1uiv(GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glUniform1uiv", (GLADapiproc) glad_glUniform1uiv, 3, location, count, value);
    glad_glUniform1uiv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform1uiv", (GLADapiproc) glad_glUniform1uiv, 3, location, count, value);
    
}
PFNGLUNIFORM1UIVPROC glad_debug_glUniform1uiv = glad_debug_impl_glUniform1uiv;
PFNGLUNIFORM2DPROC glad_glUniform2d = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2d(GLint location, GLdouble x, GLdouble y) {
    _pre_call_gl_callback("glUniform2d", (GLADapiproc) glad_glUniform2d, 3, location, x, y);
    glad_glUniform2d(location, x, y);
    _post_call_gl_callback(NULL, "glUniform2d", (GLADapiproc) glad_glUniform2d, 3, location, x, y);
    
}
PFNGLUNIFORM2DPROC glad_debug_glUniform2d = glad_debug_impl_glUniform2d;
PFNGLUNIFORM2DVPROC glad_glUniform2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2dv(GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glUniform2dv", (GLADapiproc) glad_glUniform2dv, 3, location, count, value);
    glad_glUniform2dv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform2dv", (GLADapiproc) glad_glUniform2dv, 3, location, count, value);
    
}
PFNGLUNIFORM2DVPROC glad_debug_glUniform2dv = glad_debug_impl_glUniform2dv;
PFNGLUNIFORM2FPROC glad_glUniform2f = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2f(GLint location, GLfloat v0, GLfloat v1) {
    _pre_call_gl_callback("glUniform2f", (GLADapiproc) glad_glUniform2f, 3, location, v0, v1);
    glad_glUniform2f(location, v0, v1);
    _post_call_gl_callback(NULL, "glUniform2f", (GLADapiproc) glad_glUniform2f, 3, location, v0, v1);
    
}
PFNGLUNIFORM2FPROC glad_debug_glUniform2f = glad_debug_impl_glUniform2f;
PFNGLUNIFORM2FVPROC glad_glUniform2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2fv(GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glUniform2fv", (GLADapiproc) glad_glUniform2fv, 3, location, count, value);
    glad_glUniform2fv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform2fv", (GLADapiproc) glad_glUniform2fv, 3, location, count, value);
    
}
PFNGLUNIFORM2FVPROC glad_debug_glUniform2fv = glad_debug_impl_glUniform2fv;
PFNGLUNIFORM2IPROC glad_glUniform2i = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2i(GLint location, GLint v0, GLint v1) {
    _pre_call_gl_callback("glUniform2i", (GLADapiproc) glad_glUniform2i, 3, location, v0, v1);
    glad_glUniform2i(location, v0, v1);
    _post_call_gl_callback(NULL, "glUniform2i", (GLADapiproc) glad_glUniform2i, 3, location, v0, v1);
    
}
PFNGLUNIFORM2IPROC glad_debug_glUniform2i = glad_debug_impl_glUniform2i;
PFNGLUNIFORM2IVPROC glad_glUniform2iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2iv(GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glUniform2iv", (GLADapiproc) glad_glUniform2iv, 3, location, count, value);
    glad_glUniform2iv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform2iv", (GLADapiproc) glad_glUniform2iv, 3, location, count, value);
    
}
PFNGLUNIFORM2IVPROC glad_debug_glUniform2iv = glad_debug_impl_glUniform2iv;
PFNGLUNIFORM2UIPROC glad_glUniform2ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2ui(GLint location, GLuint v0, GLuint v1) {
    _pre_call_gl_callback("glUniform2ui", (GLADapiproc) glad_glUniform2ui, 3, location, v0, v1);
    glad_glUniform2ui(location, v0, v1);
    _post_call_gl_callback(NULL, "glUniform2ui", (GLADapiproc) glad_glUniform2ui, 3, location, v0, v1);
    
}
PFNGLUNIFORM2UIPROC glad_debug_glUniform2ui = glad_debug_impl_glUniform2ui;
PFNGLUNIFORM2UIVPROC glad_glUniform2uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform2uiv(GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glUniform2uiv", (GLADapiproc) glad_glUniform2uiv, 3, location, count, value);
    glad_glUniform2uiv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform2uiv", (GLADapiproc) glad_glUniform2uiv, 3, location, count, value);
    
}
PFNGLUNIFORM2UIVPROC glad_debug_glUniform2uiv = glad_debug_impl_glUniform2uiv;
PFNGLUNIFORM3DPROC glad_glUniform3d = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z) {
    _pre_call_gl_callback("glUniform3d", (GLADapiproc) glad_glUniform3d, 4, location, x, y, z);
    glad_glUniform3d(location, x, y, z);
    _post_call_gl_callback(NULL, "glUniform3d", (GLADapiproc) glad_glUniform3d, 4, location, x, y, z);
    
}
PFNGLUNIFORM3DPROC glad_debug_glUniform3d = glad_debug_impl_glUniform3d;
PFNGLUNIFORM3DVPROC glad_glUniform3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3dv(GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glUniform3dv", (GLADapiproc) glad_glUniform3dv, 3, location, count, value);
    glad_glUniform3dv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform3dv", (GLADapiproc) glad_glUniform3dv, 3, location, count, value);
    
}
PFNGLUNIFORM3DVPROC glad_debug_glUniform3dv = glad_debug_impl_glUniform3dv;
PFNGLUNIFORM3FPROC glad_glUniform3f = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    _pre_call_gl_callback("glUniform3f", (GLADapiproc) glad_glUniform3f, 4, location, v0, v1, v2);
    glad_glUniform3f(location, v0, v1, v2);
    _post_call_gl_callback(NULL, "glUniform3f", (GLADapiproc) glad_glUniform3f, 4, location, v0, v1, v2);
    
}
PFNGLUNIFORM3FPROC glad_debug_glUniform3f = glad_debug_impl_glUniform3f;
PFNGLUNIFORM3FVPROC glad_glUniform3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3fv(GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glUniform3fv", (GLADapiproc) glad_glUniform3fv, 3, location, count, value);
    glad_glUniform3fv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform3fv", (GLADapiproc) glad_glUniform3fv, 3, location, count, value);
    
}
PFNGLUNIFORM3FVPROC glad_debug_glUniform3fv = glad_debug_impl_glUniform3fv;
PFNGLUNIFORM3IPROC glad_glUniform3i = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3i(GLint location, GLint v0, GLint v1, GLint v2) {
    _pre_call_gl_callback("glUniform3i", (GLADapiproc) glad_glUniform3i, 4, location, v0, v1, v2);
    glad_glUniform3i(location, v0, v1, v2);
    _post_call_gl_callback(NULL, "glUniform3i", (GLADapiproc) glad_glUniform3i, 4, location, v0, v1, v2);
    
}
PFNGLUNIFORM3IPROC glad_debug_glUniform3i = glad_debug_impl_glUniform3i;
PFNGLUNIFORM3IVPROC glad_glUniform3iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3iv(GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glUniform3iv", (GLADapiproc) glad_glUniform3iv, 3, location, count, value);
    glad_glUniform3iv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform3iv", (GLADapiproc) glad_glUniform3iv, 3, location, count, value);
    
}
PFNGLUNIFORM3IVPROC glad_debug_glUniform3iv = glad_debug_impl_glUniform3iv;
PFNGLUNIFORM3UIPROC glad_glUniform3ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2) {
    _pre_call_gl_callback("glUniform3ui", (GLADapiproc) glad_glUniform3ui, 4, location, v0, v1, v2);
    glad_glUniform3ui(location, v0, v1, v2);
    _post_call_gl_callback(NULL, "glUniform3ui", (GLADapiproc) glad_glUniform3ui, 4, location, v0, v1, v2);
    
}
PFNGLUNIFORM3UIPROC glad_debug_glUniform3ui = glad_debug_impl_glUniform3ui;
PFNGLUNIFORM3UIVPROC glad_glUniform3uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform3uiv(GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glUniform3uiv", (GLADapiproc) glad_glUniform3uiv, 3, location, count, value);
    glad_glUniform3uiv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform3uiv", (GLADapiproc) glad_glUniform3uiv, 3, location, count, value);
    
}
PFNGLUNIFORM3UIVPROC glad_debug_glUniform3uiv = glad_debug_impl_glUniform3uiv;
PFNGLUNIFORM4DPROC glad_glUniform4d = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    _pre_call_gl_callback("glUniform4d", (GLADapiproc) glad_glUniform4d, 5, location, x, y, z, w);
    glad_glUniform4d(location, x, y, z, w);
    _post_call_gl_callback(NULL, "glUniform4d", (GLADapiproc) glad_glUniform4d, 5, location, x, y, z, w);
    
}
PFNGLUNIFORM4DPROC glad_debug_glUniform4d = glad_debug_impl_glUniform4d;
PFNGLUNIFORM4DVPROC glad_glUniform4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4dv(GLint location, GLsizei count, const GLdouble * value) {
    _pre_call_gl_callback("glUniform4dv", (GLADapiproc) glad_glUniform4dv, 3, location, count, value);
    glad_glUniform4dv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform4dv", (GLADapiproc) glad_glUniform4dv, 3, location, count, value);
    
}
PFNGLUNIFORM4DVPROC glad_debug_glUniform4dv = glad_debug_impl_glUniform4dv;
PFNGLUNIFORM4FPROC glad_glUniform4f = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    _pre_call_gl_callback("glUniform4f", (GLADapiproc) glad_glUniform4f, 5, location, v0, v1, v2, v3);
    glad_glUniform4f(location, v0, v1, v2, v3);
    _post_call_gl_callback(NULL, "glUniform4f", (GLADapiproc) glad_glUniform4f, 5, location, v0, v1, v2, v3);
    
}
PFNGLUNIFORM4FPROC glad_debug_glUniform4f = glad_debug_impl_glUniform4f;
PFNGLUNIFORM4FVPROC glad_glUniform4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4fv(GLint location, GLsizei count, const GLfloat * value) {
    _pre_call_gl_callback("glUniform4fv", (GLADapiproc) glad_glUniform4fv, 3, location, count, value);
    glad_glUniform4fv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform4fv", (GLADapiproc) glad_glUniform4fv, 3, location, count, value);
    
}
PFNGLUNIFORM4FVPROC glad_debug_glUniform4fv = glad_debug_impl_glUniform4fv;
PFNGLUNIFORM4IPROC glad_glUniform4i = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
    _pre_call_gl_callback("glUniform4i", (GLADapiproc) glad_glUniform4i, 5, location, v0, v1, v2, v3);
    glad_glUniform4i(location, v0, v1, v2, v3);
    _post_call_gl_callback(NULL, "glUniform4i", (GLADapiproc) glad_glUniform4i, 5, location, v0, v1, v2, v3);
    
}
PFNGLUNIFORM4IPROC glad_debug_glUniform4i = glad_debug_impl_glUniform4i;
PFNGLUNIFORM4IVPROC glad_glUniform4iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4iv(GLint location, GLsizei count, const GLint * value) {
    _pre_call_gl_callback("glUniform4iv", (GLADapiproc) glad_glUniform4iv, 3, location, count, value);
    glad_glUniform4iv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform4iv", (GLADapiproc) glad_glUniform4iv, 3, location, count, value);
    
}
PFNGLUNIFORM4IVPROC glad_debug_glUniform4iv = glad_debug_impl_glUniform4iv;
PFNGLUNIFORM4UIPROC glad_glUniform4ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
    _pre_call_gl_callback("glUniform4ui", (GLADapiproc) glad_glUniform4ui, 5, location, v0, v1, v2, v3);
    glad_glUniform4ui(location, v0, v1, v2, v3);
    _post_call_gl_callback(NULL, "glUniform4ui", (GLADapiproc) glad_glUniform4ui, 5, location, v0, v1, v2, v3);
    
}
PFNGLUNIFORM4UIPROC glad_debug_glUniform4ui = glad_debug_impl_glUniform4ui;
PFNGLUNIFORM4UIVPROC glad_glUniform4uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniform4uiv(GLint location, GLsizei count, const GLuint * value) {
    _pre_call_gl_callback("glUniform4uiv", (GLADapiproc) glad_glUniform4uiv, 3, location, count, value);
    glad_glUniform4uiv(location, count, value);
    _post_call_gl_callback(NULL, "glUniform4uiv", (GLADapiproc) glad_glUniform4uiv, 3, location, count, value);
    
}
PFNGLUNIFORM4UIVPROC glad_debug_glUniform4uiv = glad_debug_impl_glUniform4uiv;
PFNGLUNIFORMBLOCKBINDINGPROC glad_glUniformBlockBinding = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {
    _pre_call_gl_callback("glUniformBlockBinding", (GLADapiproc) glad_glUniformBlockBinding, 3, program, uniformBlockIndex, uniformBlockBinding);
    glad_glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
    _post_call_gl_callback(NULL, "glUniformBlockBinding", (GLADapiproc) glad_glUniformBlockBinding, 3, program, uniformBlockIndex, uniformBlockBinding);
    
}
PFNGLUNIFORMBLOCKBINDINGPROC glad_debug_glUniformBlockBinding = glad_debug_impl_glUniformBlockBinding;
PFNGLUNIFORMMATRIX2DVPROC glad_glUniformMatrix2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix2dv", (GLADapiproc) glad_glUniformMatrix2dv, 4, location, count, transpose, value);
    glad_glUniformMatrix2dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix2dv", (GLADapiproc) glad_glUniformMatrix2dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX2DVPROC glad_debug_glUniformMatrix2dv = glad_debug_impl_glUniformMatrix2dv;
PFNGLUNIFORMMATRIX2FVPROC glad_glUniformMatrix2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix2fv", (GLADapiproc) glad_glUniformMatrix2fv, 4, location, count, transpose, value);
    glad_glUniformMatrix2fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix2fv", (GLADapiproc) glad_glUniformMatrix2fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX2FVPROC glad_debug_glUniformMatrix2fv = glad_debug_impl_glUniformMatrix2fv;
PFNGLUNIFORMMATRIX2X3DVPROC glad_glUniformMatrix2x3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix2x3dv", (GLADapiproc) glad_glUniformMatrix2x3dv, 4, location, count, transpose, value);
    glad_glUniformMatrix2x3dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix2x3dv", (GLADapiproc) glad_glUniformMatrix2x3dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX2X3DVPROC glad_debug_glUniformMatrix2x3dv = glad_debug_impl_glUniformMatrix2x3dv;
PFNGLUNIFORMMATRIX2X3FVPROC glad_glUniformMatrix2x3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix2x3fv", (GLADapiproc) glad_glUniformMatrix2x3fv, 4, location, count, transpose, value);
    glad_glUniformMatrix2x3fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix2x3fv", (GLADapiproc) glad_glUniformMatrix2x3fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX2X3FVPROC glad_debug_glUniformMatrix2x3fv = glad_debug_impl_glUniformMatrix2x3fv;
PFNGLUNIFORMMATRIX2X4DVPROC glad_glUniformMatrix2x4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix2x4dv", (GLADapiproc) glad_glUniformMatrix2x4dv, 4, location, count, transpose, value);
    glad_glUniformMatrix2x4dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix2x4dv", (GLADapiproc) glad_glUniformMatrix2x4dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX2X4DVPROC glad_debug_glUniformMatrix2x4dv = glad_debug_impl_glUniformMatrix2x4dv;
PFNGLUNIFORMMATRIX2X4FVPROC glad_glUniformMatrix2x4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix2x4fv", (GLADapiproc) glad_glUniformMatrix2x4fv, 4, location, count, transpose, value);
    glad_glUniformMatrix2x4fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix2x4fv", (GLADapiproc) glad_glUniformMatrix2x4fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX2X4FVPROC glad_debug_glUniformMatrix2x4fv = glad_debug_impl_glUniformMatrix2x4fv;
PFNGLUNIFORMMATRIX3DVPROC glad_glUniformMatrix3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix3dv", (GLADapiproc) glad_glUniformMatrix3dv, 4, location, count, transpose, value);
    glad_glUniformMatrix3dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix3dv", (GLADapiproc) glad_glUniformMatrix3dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX3DVPROC glad_debug_glUniformMatrix3dv = glad_debug_impl_glUniformMatrix3dv;
PFNGLUNIFORMMATRIX3FVPROC glad_glUniformMatrix3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix3fv", (GLADapiproc) glad_glUniformMatrix3fv, 4, location, count, transpose, value);
    glad_glUniformMatrix3fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix3fv", (GLADapiproc) glad_glUniformMatrix3fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX3FVPROC glad_debug_glUniformMatrix3fv = glad_debug_impl_glUniformMatrix3fv;
PFNGLUNIFORMMATRIX3X2DVPROC glad_glUniformMatrix3x2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix3x2dv", (GLADapiproc) glad_glUniformMatrix3x2dv, 4, location, count, transpose, value);
    glad_glUniformMatrix3x2dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix3x2dv", (GLADapiproc) glad_glUniformMatrix3x2dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX3X2DVPROC glad_debug_glUniformMatrix3x2dv = glad_debug_impl_glUniformMatrix3x2dv;
PFNGLUNIFORMMATRIX3X2FVPROC glad_glUniformMatrix3x2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix3x2fv", (GLADapiproc) glad_glUniformMatrix3x2fv, 4, location, count, transpose, value);
    glad_glUniformMatrix3x2fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix3x2fv", (GLADapiproc) glad_glUniformMatrix3x2fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX3X2FVPROC glad_debug_glUniformMatrix3x2fv = glad_debug_impl_glUniformMatrix3x2fv;
PFNGLUNIFORMMATRIX3X4DVPROC glad_glUniformMatrix3x4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix3x4dv", (GLADapiproc) glad_glUniformMatrix3x4dv, 4, location, count, transpose, value);
    glad_glUniformMatrix3x4dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix3x4dv", (GLADapiproc) glad_glUniformMatrix3x4dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX3X4DVPROC glad_debug_glUniformMatrix3x4dv = glad_debug_impl_glUniformMatrix3x4dv;
PFNGLUNIFORMMATRIX3X4FVPROC glad_glUniformMatrix3x4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix3x4fv", (GLADapiproc) glad_glUniformMatrix3x4fv, 4, location, count, transpose, value);
    glad_glUniformMatrix3x4fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix3x4fv", (GLADapiproc) glad_glUniformMatrix3x4fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX3X4FVPROC glad_debug_glUniformMatrix3x4fv = glad_debug_impl_glUniformMatrix3x4fv;
PFNGLUNIFORMMATRIX4DVPROC glad_glUniformMatrix4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix4dv", (GLADapiproc) glad_glUniformMatrix4dv, 4, location, count, transpose, value);
    glad_glUniformMatrix4dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix4dv", (GLADapiproc) glad_glUniformMatrix4dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX4DVPROC glad_debug_glUniformMatrix4dv = glad_debug_impl_glUniformMatrix4dv;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix4fv", (GLADapiproc) glad_glUniformMatrix4fv, 4, location, count, transpose, value);
    glad_glUniformMatrix4fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix4fv", (GLADapiproc) glad_glUniformMatrix4fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX4FVPROC glad_debug_glUniformMatrix4fv = glad_debug_impl_glUniformMatrix4fv;
PFNGLUNIFORMMATRIX4X2DVPROC glad_glUniformMatrix4x2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix4x2dv", (GLADapiproc) glad_glUniformMatrix4x2dv, 4, location, count, transpose, value);
    glad_glUniformMatrix4x2dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix4x2dv", (GLADapiproc) glad_glUniformMatrix4x2dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX4X2DVPROC glad_debug_glUniformMatrix4x2dv = glad_debug_impl_glUniformMatrix4x2dv;
PFNGLUNIFORMMATRIX4X2FVPROC glad_glUniformMatrix4x2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix4x2fv", (GLADapiproc) glad_glUniformMatrix4x2fv, 4, location, count, transpose, value);
    glad_glUniformMatrix4x2fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix4x2fv", (GLADapiproc) glad_glUniformMatrix4x2fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX4X2FVPROC glad_debug_glUniformMatrix4x2fv = glad_debug_impl_glUniformMatrix4x2fv;
PFNGLUNIFORMMATRIX4X3DVPROC glad_glUniformMatrix4x3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble * value) {
    _pre_call_gl_callback("glUniformMatrix4x3dv", (GLADapiproc) glad_glUniformMatrix4x3dv, 4, location, count, transpose, value);
    glad_glUniformMatrix4x3dv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix4x3dv", (GLADapiproc) glad_glUniformMatrix4x3dv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX4X3DVPROC glad_debug_glUniformMatrix4x3dv = glad_debug_impl_glUniformMatrix4x3dv;
PFNGLUNIFORMMATRIX4X3FVPROC glad_glUniformMatrix4x3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value) {
    _pre_call_gl_callback("glUniformMatrix4x3fv", (GLADapiproc) glad_glUniformMatrix4x3fv, 4, location, count, transpose, value);
    glad_glUniformMatrix4x3fv(location, count, transpose, value);
    _post_call_gl_callback(NULL, "glUniformMatrix4x3fv", (GLADapiproc) glad_glUniformMatrix4x3fv, 4, location, count, transpose, value);
    
}
PFNGLUNIFORMMATRIX4X3FVPROC glad_debug_glUniformMatrix4x3fv = glad_debug_impl_glUniformMatrix4x3fv;
PFNGLUNIFORMSUBROUTINESUIVPROC glad_glUniformSubroutinesuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint * indices) {
    _pre_call_gl_callback("glUniformSubroutinesuiv", (GLADapiproc) glad_glUniformSubroutinesuiv, 3, shadertype, count, indices);
    glad_glUniformSubroutinesuiv(shadertype, count, indices);
    _post_call_gl_callback(NULL, "glUniformSubroutinesuiv", (GLADapiproc) glad_glUniformSubroutinesuiv, 3, shadertype, count, indices);
    
}
PFNGLUNIFORMSUBROUTINESUIVPROC glad_debug_glUniformSubroutinesuiv = glad_debug_impl_glUniformSubroutinesuiv;
PFNGLUNMAPBUFFERPROC glad_glUnmapBuffer = NULL;
static GLboolean GLAD_API_PTR glad_debug_impl_glUnmapBuffer(GLenum target) {
    GLboolean ret;
    _pre_call_gl_callback("glUnmapBuffer", (GLADapiproc) glad_glUnmapBuffer, 1, target);
    ret = glad_glUnmapBuffer(target);
    _post_call_gl_callback((void*) &ret, "glUnmapBuffer", (GLADapiproc) glad_glUnmapBuffer, 1, target);
    return ret;
}
PFNGLUNMAPBUFFERPROC glad_debug_glUnmapBuffer = glad_debug_impl_glUnmapBuffer;
PFNGLUSEPROGRAMPROC glad_glUseProgram = NULL;
static void GLAD_API_PTR glad_debug_impl_glUseProgram(GLuint program) {
    _pre_call_gl_callback("glUseProgram", (GLADapiproc) glad_glUseProgram, 1, program);
    glad_glUseProgram(program);
    _post_call_gl_callback(NULL, "glUseProgram", (GLADapiproc) glad_glUseProgram, 1, program);
    
}
PFNGLUSEPROGRAMPROC glad_debug_glUseProgram = glad_debug_impl_glUseProgram;
PFNGLUSEPROGRAMSTAGESPROC glad_glUseProgramStages = NULL;
static void GLAD_API_PTR glad_debug_impl_glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program) {
    _pre_call_gl_callback("glUseProgramStages", (GLADapiproc) glad_glUseProgramStages, 3, pipeline, stages, program);
    glad_glUseProgramStages(pipeline, stages, program);
    _post_call_gl_callback(NULL, "glUseProgramStages", (GLADapiproc) glad_glUseProgramStages, 3, pipeline, stages, program);
    
}
PFNGLUSEPROGRAMSTAGESPROC glad_debug_glUseProgramStages = glad_debug_impl_glUseProgramStages;
PFNGLVALIDATEPROGRAMPROC glad_glValidateProgram = NULL;
static void GLAD_API_PTR glad_debug_impl_glValidateProgram(GLuint program) {
    _pre_call_gl_callback("glValidateProgram", (GLADapiproc) glad_glValidateProgram, 1, program);
    glad_glValidateProgram(program);
    _post_call_gl_callback(NULL, "glValidateProgram", (GLADapiproc) glad_glValidateProgram, 1, program);
    
}
PFNGLVALIDATEPROGRAMPROC glad_debug_glValidateProgram = glad_debug_impl_glValidateProgram;
PFNGLVALIDATEPROGRAMPIPELINEPROC glad_glValidateProgramPipeline = NULL;
static void GLAD_API_PTR glad_debug_impl_glValidateProgramPipeline(GLuint pipeline) {
    _pre_call_gl_callback("glValidateProgramPipeline", (GLADapiproc) glad_glValidateProgramPipeline, 1, pipeline);
    glad_glValidateProgramPipeline(pipeline);
    _post_call_gl_callback(NULL, "glValidateProgramPipeline", (GLADapiproc) glad_glValidateProgramPipeline, 1, pipeline);
    
}
PFNGLVALIDATEPROGRAMPIPELINEPROC glad_debug_glValidateProgramPipeline = glad_debug_impl_glValidateProgramPipeline;
PFNGLVERTEXATTRIB1DPROC glad_glVertexAttrib1d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib1d(GLuint index, GLdouble x) {
    _pre_call_gl_callback("glVertexAttrib1d", (GLADapiproc) glad_glVertexAttrib1d, 2, index, x);
    glad_glVertexAttrib1d(index, x);
    _post_call_gl_callback(NULL, "glVertexAttrib1d", (GLADapiproc) glad_glVertexAttrib1d, 2, index, x);
    
}
PFNGLVERTEXATTRIB1DPROC glad_debug_glVertexAttrib1d = glad_debug_impl_glVertexAttrib1d;
PFNGLVERTEXATTRIB1DVPROC glad_glVertexAttrib1dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib1dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttrib1dv", (GLADapiproc) glad_glVertexAttrib1dv, 2, index, v);
    glad_glVertexAttrib1dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib1dv", (GLADapiproc) glad_glVertexAttrib1dv, 2, index, v);
    
}
PFNGLVERTEXATTRIB1DVPROC glad_debug_glVertexAttrib1dv = glad_debug_impl_glVertexAttrib1dv;
PFNGLVERTEXATTRIB1FPROC glad_glVertexAttrib1f = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib1f(GLuint index, GLfloat x) {
    _pre_call_gl_callback("glVertexAttrib1f", (GLADapiproc) glad_glVertexAttrib1f, 2, index, x);
    glad_glVertexAttrib1f(index, x);
    _post_call_gl_callback(NULL, "glVertexAttrib1f", (GLADapiproc) glad_glVertexAttrib1f, 2, index, x);
    
}
PFNGLVERTEXATTRIB1FPROC glad_debug_glVertexAttrib1f = glad_debug_impl_glVertexAttrib1f;
PFNGLVERTEXATTRIB1FVPROC glad_glVertexAttrib1fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib1fv(GLuint index, const GLfloat * v) {
    _pre_call_gl_callback("glVertexAttrib1fv", (GLADapiproc) glad_glVertexAttrib1fv, 2, index, v);
    glad_glVertexAttrib1fv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib1fv", (GLADapiproc) glad_glVertexAttrib1fv, 2, index, v);
    
}
PFNGLVERTEXATTRIB1FVPROC glad_debug_glVertexAttrib1fv = glad_debug_impl_glVertexAttrib1fv;
PFNGLVERTEXATTRIB1SPROC glad_glVertexAttrib1s = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib1s(GLuint index, GLshort x) {
    _pre_call_gl_callback("glVertexAttrib1s", (GLADapiproc) glad_glVertexAttrib1s, 2, index, x);
    glad_glVertexAttrib1s(index, x);
    _post_call_gl_callback(NULL, "glVertexAttrib1s", (GLADapiproc) glad_glVertexAttrib1s, 2, index, x);
    
}
PFNGLVERTEXATTRIB1SPROC glad_debug_glVertexAttrib1s = glad_debug_impl_glVertexAttrib1s;
PFNGLVERTEXATTRIB1SVPROC glad_glVertexAttrib1sv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib1sv(GLuint index, const GLshort * v) {
    _pre_call_gl_callback("glVertexAttrib1sv", (GLADapiproc) glad_glVertexAttrib1sv, 2, index, v);
    glad_glVertexAttrib1sv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib1sv", (GLADapiproc) glad_glVertexAttrib1sv, 2, index, v);
    
}
PFNGLVERTEXATTRIB1SVPROC glad_debug_glVertexAttrib1sv = glad_debug_impl_glVertexAttrib1sv;
PFNGLVERTEXATTRIB2DPROC glad_glVertexAttrib2d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y) {
    _pre_call_gl_callback("glVertexAttrib2d", (GLADapiproc) glad_glVertexAttrib2d, 3, index, x, y);
    glad_glVertexAttrib2d(index, x, y);
    _post_call_gl_callback(NULL, "glVertexAttrib2d", (GLADapiproc) glad_glVertexAttrib2d, 3, index, x, y);
    
}
PFNGLVERTEXATTRIB2DPROC glad_debug_glVertexAttrib2d = glad_debug_impl_glVertexAttrib2d;
PFNGLVERTEXATTRIB2DVPROC glad_glVertexAttrib2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib2dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttrib2dv", (GLADapiproc) glad_glVertexAttrib2dv, 2, index, v);
    glad_glVertexAttrib2dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib2dv", (GLADapiproc) glad_glVertexAttrib2dv, 2, index, v);
    
}
PFNGLVERTEXATTRIB2DVPROC glad_debug_glVertexAttrib2dv = glad_debug_impl_glVertexAttrib2dv;
PFNGLVERTEXATTRIB2FPROC glad_glVertexAttrib2f = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y) {
    _pre_call_gl_callback("glVertexAttrib2f", (GLADapiproc) glad_glVertexAttrib2f, 3, index, x, y);
    glad_glVertexAttrib2f(index, x, y);
    _post_call_gl_callback(NULL, "glVertexAttrib2f", (GLADapiproc) glad_glVertexAttrib2f, 3, index, x, y);
    
}
PFNGLVERTEXATTRIB2FPROC glad_debug_glVertexAttrib2f = glad_debug_impl_glVertexAttrib2f;
PFNGLVERTEXATTRIB2FVPROC glad_glVertexAttrib2fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib2fv(GLuint index, const GLfloat * v) {
    _pre_call_gl_callback("glVertexAttrib2fv", (GLADapiproc) glad_glVertexAttrib2fv, 2, index, v);
    glad_glVertexAttrib2fv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib2fv", (GLADapiproc) glad_glVertexAttrib2fv, 2, index, v);
    
}
PFNGLVERTEXATTRIB2FVPROC glad_debug_glVertexAttrib2fv = glad_debug_impl_glVertexAttrib2fv;
PFNGLVERTEXATTRIB2SPROC glad_glVertexAttrib2s = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib2s(GLuint index, GLshort x, GLshort y) {
    _pre_call_gl_callback("glVertexAttrib2s", (GLADapiproc) glad_glVertexAttrib2s, 3, index, x, y);
    glad_glVertexAttrib2s(index, x, y);
    _post_call_gl_callback(NULL, "glVertexAttrib2s", (GLADapiproc) glad_glVertexAttrib2s, 3, index, x, y);
    
}
PFNGLVERTEXATTRIB2SPROC glad_debug_glVertexAttrib2s = glad_debug_impl_glVertexAttrib2s;
PFNGLVERTEXATTRIB2SVPROC glad_glVertexAttrib2sv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib2sv(GLuint index, const GLshort * v) {
    _pre_call_gl_callback("glVertexAttrib2sv", (GLADapiproc) glad_glVertexAttrib2sv, 2, index, v);
    glad_glVertexAttrib2sv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib2sv", (GLADapiproc) glad_glVertexAttrib2sv, 2, index, v);
    
}
PFNGLVERTEXATTRIB2SVPROC glad_debug_glVertexAttrib2sv = glad_debug_impl_glVertexAttrib2sv;
PFNGLVERTEXATTRIB3DPROC glad_glVertexAttrib3d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    _pre_call_gl_callback("glVertexAttrib3d", (GLADapiproc) glad_glVertexAttrib3d, 4, index, x, y, z);
    glad_glVertexAttrib3d(index, x, y, z);
    _post_call_gl_callback(NULL, "glVertexAttrib3d", (GLADapiproc) glad_glVertexAttrib3d, 4, index, x, y, z);
    
}
PFNGLVERTEXATTRIB3DPROC glad_debug_glVertexAttrib3d = glad_debug_impl_glVertexAttrib3d;
PFNGLVERTEXATTRIB3DVPROC glad_glVertexAttrib3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib3dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttrib3dv", (GLADapiproc) glad_glVertexAttrib3dv, 2, index, v);
    glad_glVertexAttrib3dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib3dv", (GLADapiproc) glad_glVertexAttrib3dv, 2, index, v);
    
}
PFNGLVERTEXATTRIB3DVPROC glad_debug_glVertexAttrib3dv = glad_debug_impl_glVertexAttrib3dv;
PFNGLVERTEXATTRIB3FPROC glad_glVertexAttrib3f = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z) {
    _pre_call_gl_callback("glVertexAttrib3f", (GLADapiproc) glad_glVertexAttrib3f, 4, index, x, y, z);
    glad_glVertexAttrib3f(index, x, y, z);
    _post_call_gl_callback(NULL, "glVertexAttrib3f", (GLADapiproc) glad_glVertexAttrib3f, 4, index, x, y, z);
    
}
PFNGLVERTEXATTRIB3FPROC glad_debug_glVertexAttrib3f = glad_debug_impl_glVertexAttrib3f;
PFNGLVERTEXATTRIB3FVPROC glad_glVertexAttrib3fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib3fv(GLuint index, const GLfloat * v) {
    _pre_call_gl_callback("glVertexAttrib3fv", (GLADapiproc) glad_glVertexAttrib3fv, 2, index, v);
    glad_glVertexAttrib3fv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib3fv", (GLADapiproc) glad_glVertexAttrib3fv, 2, index, v);
    
}
PFNGLVERTEXATTRIB3FVPROC glad_debug_glVertexAttrib3fv = glad_debug_impl_glVertexAttrib3fv;
PFNGLVERTEXATTRIB3SPROC glad_glVertexAttrib3s = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z) {
    _pre_call_gl_callback("glVertexAttrib3s", (GLADapiproc) glad_glVertexAttrib3s, 4, index, x, y, z);
    glad_glVertexAttrib3s(index, x, y, z);
    _post_call_gl_callback(NULL, "glVertexAttrib3s", (GLADapiproc) glad_glVertexAttrib3s, 4, index, x, y, z);
    
}
PFNGLVERTEXATTRIB3SPROC glad_debug_glVertexAttrib3s = glad_debug_impl_glVertexAttrib3s;
PFNGLVERTEXATTRIB3SVPROC glad_glVertexAttrib3sv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib3sv(GLuint index, const GLshort * v) {
    _pre_call_gl_callback("glVertexAttrib3sv", (GLADapiproc) glad_glVertexAttrib3sv, 2, index, v);
    glad_glVertexAttrib3sv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib3sv", (GLADapiproc) glad_glVertexAttrib3sv, 2, index, v);
    
}
PFNGLVERTEXATTRIB3SVPROC glad_debug_glVertexAttrib3sv = glad_debug_impl_glVertexAttrib3sv;
PFNGLVERTEXATTRIB4NBVPROC glad_glVertexAttrib4Nbv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4Nbv(GLuint index, const GLbyte * v) {
    _pre_call_gl_callback("glVertexAttrib4Nbv", (GLADapiproc) glad_glVertexAttrib4Nbv, 2, index, v);
    glad_glVertexAttrib4Nbv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4Nbv", (GLADapiproc) glad_glVertexAttrib4Nbv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4NBVPROC glad_debug_glVertexAttrib4Nbv = glad_debug_impl_glVertexAttrib4Nbv;
PFNGLVERTEXATTRIB4NIVPROC glad_glVertexAttrib4Niv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4Niv(GLuint index, const GLint * v) {
    _pre_call_gl_callback("glVertexAttrib4Niv", (GLADapiproc) glad_glVertexAttrib4Niv, 2, index, v);
    glad_glVertexAttrib4Niv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4Niv", (GLADapiproc) glad_glVertexAttrib4Niv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4NIVPROC glad_debug_glVertexAttrib4Niv = glad_debug_impl_glVertexAttrib4Niv;
PFNGLVERTEXATTRIB4NSVPROC glad_glVertexAttrib4Nsv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4Nsv(GLuint index, const GLshort * v) {
    _pre_call_gl_callback("glVertexAttrib4Nsv", (GLADapiproc) glad_glVertexAttrib4Nsv, 2, index, v);
    glad_glVertexAttrib4Nsv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4Nsv", (GLADapiproc) glad_glVertexAttrib4Nsv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4NSVPROC glad_debug_glVertexAttrib4Nsv = glad_debug_impl_glVertexAttrib4Nsv;
PFNGLVERTEXATTRIB4NUBPROC glad_glVertexAttrib4Nub = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w) {
    _pre_call_gl_callback("glVertexAttrib4Nub", (GLADapiproc) glad_glVertexAttrib4Nub, 5, index, x, y, z, w);
    glad_glVertexAttrib4Nub(index, x, y, z, w);
    _post_call_gl_callback(NULL, "glVertexAttrib4Nub", (GLADapiproc) glad_glVertexAttrib4Nub, 5, index, x, y, z, w);
    
}
PFNGLVERTEXATTRIB4NUBPROC glad_debug_glVertexAttrib4Nub = glad_debug_impl_glVertexAttrib4Nub;
PFNGLVERTEXATTRIB4NUBVPROC glad_glVertexAttrib4Nubv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4Nubv(GLuint index, const GLubyte * v) {
    _pre_call_gl_callback("glVertexAttrib4Nubv", (GLADapiproc) glad_glVertexAttrib4Nubv, 2, index, v);
    glad_glVertexAttrib4Nubv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4Nubv", (GLADapiproc) glad_glVertexAttrib4Nubv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4NUBVPROC glad_debug_glVertexAttrib4Nubv = glad_debug_impl_glVertexAttrib4Nubv;
PFNGLVERTEXATTRIB4NUIVPROC glad_glVertexAttrib4Nuiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4Nuiv(GLuint index, const GLuint * v) {
    _pre_call_gl_callback("glVertexAttrib4Nuiv", (GLADapiproc) glad_glVertexAttrib4Nuiv, 2, index, v);
    glad_glVertexAttrib4Nuiv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4Nuiv", (GLADapiproc) glad_glVertexAttrib4Nuiv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4NUIVPROC glad_debug_glVertexAttrib4Nuiv = glad_debug_impl_glVertexAttrib4Nuiv;
PFNGLVERTEXATTRIB4NUSVPROC glad_glVertexAttrib4Nusv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4Nusv(GLuint index, const GLushort * v) {
    _pre_call_gl_callback("glVertexAttrib4Nusv", (GLADapiproc) glad_glVertexAttrib4Nusv, 2, index, v);
    glad_glVertexAttrib4Nusv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4Nusv", (GLADapiproc) glad_glVertexAttrib4Nusv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4NUSVPROC glad_debug_glVertexAttrib4Nusv = glad_debug_impl_glVertexAttrib4Nusv;
PFNGLVERTEXATTRIB4BVPROC glad_glVertexAttrib4bv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4bv(GLuint index, const GLbyte * v) {
    _pre_call_gl_callback("glVertexAttrib4bv", (GLADapiproc) glad_glVertexAttrib4bv, 2, index, v);
    glad_glVertexAttrib4bv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4bv", (GLADapiproc) glad_glVertexAttrib4bv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4BVPROC glad_debug_glVertexAttrib4bv = glad_debug_impl_glVertexAttrib4bv;
PFNGLVERTEXATTRIB4DPROC glad_glVertexAttrib4d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    _pre_call_gl_callback("glVertexAttrib4d", (GLADapiproc) glad_glVertexAttrib4d, 5, index, x, y, z, w);
    glad_glVertexAttrib4d(index, x, y, z, w);
    _post_call_gl_callback(NULL, "glVertexAttrib4d", (GLADapiproc) glad_glVertexAttrib4d, 5, index, x, y, z, w);
    
}
PFNGLVERTEXATTRIB4DPROC glad_debug_glVertexAttrib4d = glad_debug_impl_glVertexAttrib4d;
PFNGLVERTEXATTRIB4DVPROC glad_glVertexAttrib4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttrib4dv", (GLADapiproc) glad_glVertexAttrib4dv, 2, index, v);
    glad_glVertexAttrib4dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4dv", (GLADapiproc) glad_glVertexAttrib4dv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4DVPROC glad_debug_glVertexAttrib4dv = glad_debug_impl_glVertexAttrib4dv;
PFNGLVERTEXATTRIB4FPROC glad_glVertexAttrib4f = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
    _pre_call_gl_callback("glVertexAttrib4f", (GLADapiproc) glad_glVertexAttrib4f, 5, index, x, y, z, w);
    glad_glVertexAttrib4f(index, x, y, z, w);
    _post_call_gl_callback(NULL, "glVertexAttrib4f", (GLADapiproc) glad_glVertexAttrib4f, 5, index, x, y, z, w);
    
}
PFNGLVERTEXATTRIB4FPROC glad_debug_glVertexAttrib4f = glad_debug_impl_glVertexAttrib4f;
PFNGLVERTEXATTRIB4FVPROC glad_glVertexAttrib4fv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4fv(GLuint index, const GLfloat * v) {
    _pre_call_gl_callback("glVertexAttrib4fv", (GLADapiproc) glad_glVertexAttrib4fv, 2, index, v);
    glad_glVertexAttrib4fv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4fv", (GLADapiproc) glad_glVertexAttrib4fv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4FVPROC glad_debug_glVertexAttrib4fv = glad_debug_impl_glVertexAttrib4fv;
PFNGLVERTEXATTRIB4IVPROC glad_glVertexAttrib4iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4iv(GLuint index, const GLint * v) {
    _pre_call_gl_callback("glVertexAttrib4iv", (GLADapiproc) glad_glVertexAttrib4iv, 2, index, v);
    glad_glVertexAttrib4iv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4iv", (GLADapiproc) glad_glVertexAttrib4iv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4IVPROC glad_debug_glVertexAttrib4iv = glad_debug_impl_glVertexAttrib4iv;
PFNGLVERTEXATTRIB4SPROC glad_glVertexAttrib4s = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w) {
    _pre_call_gl_callback("glVertexAttrib4s", (GLADapiproc) glad_glVertexAttrib4s, 5, index, x, y, z, w);
    glad_glVertexAttrib4s(index, x, y, z, w);
    _post_call_gl_callback(NULL, "glVertexAttrib4s", (GLADapiproc) glad_glVertexAttrib4s, 5, index, x, y, z, w);
    
}
PFNGLVERTEXATTRIB4SPROC glad_debug_glVertexAttrib4s = glad_debug_impl_glVertexAttrib4s;
PFNGLVERTEXATTRIB4SVPROC glad_glVertexAttrib4sv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4sv(GLuint index, const GLshort * v) {
    _pre_call_gl_callback("glVertexAttrib4sv", (GLADapiproc) glad_glVertexAttrib4sv, 2, index, v);
    glad_glVertexAttrib4sv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4sv", (GLADapiproc) glad_glVertexAttrib4sv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4SVPROC glad_debug_glVertexAttrib4sv = glad_debug_impl_glVertexAttrib4sv;
PFNGLVERTEXATTRIB4UBVPROC glad_glVertexAttrib4ubv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4ubv(GLuint index, const GLubyte * v) {
    _pre_call_gl_callback("glVertexAttrib4ubv", (GLADapiproc) glad_glVertexAttrib4ubv, 2, index, v);
    glad_glVertexAttrib4ubv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4ubv", (GLADapiproc) glad_glVertexAttrib4ubv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4UBVPROC glad_debug_glVertexAttrib4ubv = glad_debug_impl_glVertexAttrib4ubv;
PFNGLVERTEXATTRIB4UIVPROC glad_glVertexAttrib4uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4uiv(GLuint index, const GLuint * v) {
    _pre_call_gl_callback("glVertexAttrib4uiv", (GLADapiproc) glad_glVertexAttrib4uiv, 2, index, v);
    glad_glVertexAttrib4uiv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4uiv", (GLADapiproc) glad_glVertexAttrib4uiv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4UIVPROC glad_debug_glVertexAttrib4uiv = glad_debug_impl_glVertexAttrib4uiv;
PFNGLVERTEXATTRIB4USVPROC glad_glVertexAttrib4usv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttrib4usv(GLuint index, const GLushort * v) {
    _pre_call_gl_callback("glVertexAttrib4usv", (GLADapiproc) glad_glVertexAttrib4usv, 2, index, v);
    glad_glVertexAttrib4usv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttrib4usv", (GLADapiproc) glad_glVertexAttrib4usv, 2, index, v);
    
}
PFNGLVERTEXATTRIB4USVPROC glad_debug_glVertexAttrib4usv = glad_debug_impl_glVertexAttrib4usv;
PFNGLVERTEXATTRIBDIVISORPROC glad_glVertexAttribDivisor = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribDivisor(GLuint index, GLuint divisor) {
    _pre_call_gl_callback("glVertexAttribDivisor", (GLADapiproc) glad_glVertexAttribDivisor, 2, index, divisor);
    glad_glVertexAttribDivisor(index, divisor);
    _post_call_gl_callback(NULL, "glVertexAttribDivisor", (GLADapiproc) glad_glVertexAttribDivisor, 2, index, divisor);
    
}
PFNGLVERTEXATTRIBDIVISORPROC glad_debug_glVertexAttribDivisor = glad_debug_impl_glVertexAttribDivisor;
PFNGLVERTEXATTRIBI1IPROC glad_glVertexAttribI1i = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI1i(GLuint index, GLint x) {
    _pre_call_gl_callback("glVertexAttribI1i", (GLADapiproc) glad_glVertexAttribI1i, 2, index, x);
    glad_glVertexAttribI1i(index, x);
    _post_call_gl_callback(NULL, "glVertexAttribI1i", (GLADapiproc) glad_glVertexAttribI1i, 2, index, x);
    
}
PFNGLVERTEXATTRIBI1IPROC glad_debug_glVertexAttribI1i = glad_debug_impl_glVertexAttribI1i;
PFNGLVERTEXATTRIBI1IVPROC glad_glVertexAttribI1iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI1iv(GLuint index, const GLint * v) {
    _pre_call_gl_callback("glVertexAttribI1iv", (GLADapiproc) glad_glVertexAttribI1iv, 2, index, v);
    glad_glVertexAttribI1iv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI1iv", (GLADapiproc) glad_glVertexAttribI1iv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI1IVPROC glad_debug_glVertexAttribI1iv = glad_debug_impl_glVertexAttribI1iv;
PFNGLVERTEXATTRIBI1UIPROC glad_glVertexAttribI1ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI1ui(GLuint index, GLuint x) {
    _pre_call_gl_callback("glVertexAttribI1ui", (GLADapiproc) glad_glVertexAttribI1ui, 2, index, x);
    glad_glVertexAttribI1ui(index, x);
    _post_call_gl_callback(NULL, "glVertexAttribI1ui", (GLADapiproc) glad_glVertexAttribI1ui, 2, index, x);
    
}
PFNGLVERTEXATTRIBI1UIPROC glad_debug_glVertexAttribI1ui = glad_debug_impl_glVertexAttribI1ui;
PFNGLVERTEXATTRIBI1UIVPROC glad_glVertexAttribI1uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI1uiv(GLuint index, const GLuint * v) {
    _pre_call_gl_callback("glVertexAttribI1uiv", (GLADapiproc) glad_glVertexAttribI1uiv, 2, index, v);
    glad_glVertexAttribI1uiv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI1uiv", (GLADapiproc) glad_glVertexAttribI1uiv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI1UIVPROC glad_debug_glVertexAttribI1uiv = glad_debug_impl_glVertexAttribI1uiv;
PFNGLVERTEXATTRIBI2IPROC glad_glVertexAttribI2i = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI2i(GLuint index, GLint x, GLint y) {
    _pre_call_gl_callback("glVertexAttribI2i", (GLADapiproc) glad_glVertexAttribI2i, 3, index, x, y);
    glad_glVertexAttribI2i(index, x, y);
    _post_call_gl_callback(NULL, "glVertexAttribI2i", (GLADapiproc) glad_glVertexAttribI2i, 3, index, x, y);
    
}
PFNGLVERTEXATTRIBI2IPROC glad_debug_glVertexAttribI2i = glad_debug_impl_glVertexAttribI2i;
PFNGLVERTEXATTRIBI2IVPROC glad_glVertexAttribI2iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI2iv(GLuint index, const GLint * v) {
    _pre_call_gl_callback("glVertexAttribI2iv", (GLADapiproc) glad_glVertexAttribI2iv, 2, index, v);
    glad_glVertexAttribI2iv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI2iv", (GLADapiproc) glad_glVertexAttribI2iv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI2IVPROC glad_debug_glVertexAttribI2iv = glad_debug_impl_glVertexAttribI2iv;
PFNGLVERTEXATTRIBI2UIPROC glad_glVertexAttribI2ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI2ui(GLuint index, GLuint x, GLuint y) {
    _pre_call_gl_callback("glVertexAttribI2ui", (GLADapiproc) glad_glVertexAttribI2ui, 3, index, x, y);
    glad_glVertexAttribI2ui(index, x, y);
    _post_call_gl_callback(NULL, "glVertexAttribI2ui", (GLADapiproc) glad_glVertexAttribI2ui, 3, index, x, y);
    
}
PFNGLVERTEXATTRIBI2UIPROC glad_debug_glVertexAttribI2ui = glad_debug_impl_glVertexAttribI2ui;
PFNGLVERTEXATTRIBI2UIVPROC glad_glVertexAttribI2uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI2uiv(GLuint index, const GLuint * v) {
    _pre_call_gl_callback("glVertexAttribI2uiv", (GLADapiproc) glad_glVertexAttribI2uiv, 2, index, v);
    glad_glVertexAttribI2uiv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI2uiv", (GLADapiproc) glad_glVertexAttribI2uiv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI2UIVPROC glad_debug_glVertexAttribI2uiv = glad_debug_impl_glVertexAttribI2uiv;
PFNGLVERTEXATTRIBI3IPROC glad_glVertexAttribI3i = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z) {
    _pre_call_gl_callback("glVertexAttribI3i", (GLADapiproc) glad_glVertexAttribI3i, 4, index, x, y, z);
    glad_glVertexAttribI3i(index, x, y, z);
    _post_call_gl_callback(NULL, "glVertexAttribI3i", (GLADapiproc) glad_glVertexAttribI3i, 4, index, x, y, z);
    
}
PFNGLVERTEXATTRIBI3IPROC glad_debug_glVertexAttribI3i = glad_debug_impl_glVertexAttribI3i;
PFNGLVERTEXATTRIBI3IVPROC glad_glVertexAttribI3iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI3iv(GLuint index, const GLint * v) {
    _pre_call_gl_callback("glVertexAttribI3iv", (GLADapiproc) glad_glVertexAttribI3iv, 2, index, v);
    glad_glVertexAttribI3iv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI3iv", (GLADapiproc) glad_glVertexAttribI3iv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI3IVPROC glad_debug_glVertexAttribI3iv = glad_debug_impl_glVertexAttribI3iv;
PFNGLVERTEXATTRIBI3UIPROC glad_glVertexAttribI3ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z) {
    _pre_call_gl_callback("glVertexAttribI3ui", (GLADapiproc) glad_glVertexAttribI3ui, 4, index, x, y, z);
    glad_glVertexAttribI3ui(index, x, y, z);
    _post_call_gl_callback(NULL, "glVertexAttribI3ui", (GLADapiproc) glad_glVertexAttribI3ui, 4, index, x, y, z);
    
}
PFNGLVERTEXATTRIBI3UIPROC glad_debug_glVertexAttribI3ui = glad_debug_impl_glVertexAttribI3ui;
PFNGLVERTEXATTRIBI3UIVPROC glad_glVertexAttribI3uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI3uiv(GLuint index, const GLuint * v) {
    _pre_call_gl_callback("glVertexAttribI3uiv", (GLADapiproc) glad_glVertexAttribI3uiv, 2, index, v);
    glad_glVertexAttribI3uiv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI3uiv", (GLADapiproc) glad_glVertexAttribI3uiv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI3UIVPROC glad_debug_glVertexAttribI3uiv = glad_debug_impl_glVertexAttribI3uiv;
PFNGLVERTEXATTRIBI4BVPROC glad_glVertexAttribI4bv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4bv(GLuint index, const GLbyte * v) {
    _pre_call_gl_callback("glVertexAttribI4bv", (GLADapiproc) glad_glVertexAttribI4bv, 2, index, v);
    glad_glVertexAttribI4bv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI4bv", (GLADapiproc) glad_glVertexAttribI4bv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI4BVPROC glad_debug_glVertexAttribI4bv = glad_debug_impl_glVertexAttribI4bv;
PFNGLVERTEXATTRIBI4IPROC glad_glVertexAttribI4i = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w) {
    _pre_call_gl_callback("glVertexAttribI4i", (GLADapiproc) glad_glVertexAttribI4i, 5, index, x, y, z, w);
    glad_glVertexAttribI4i(index, x, y, z, w);
    _post_call_gl_callback(NULL, "glVertexAttribI4i", (GLADapiproc) glad_glVertexAttribI4i, 5, index, x, y, z, w);
    
}
PFNGLVERTEXATTRIBI4IPROC glad_debug_glVertexAttribI4i = glad_debug_impl_glVertexAttribI4i;
PFNGLVERTEXATTRIBI4IVPROC glad_glVertexAttribI4iv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4iv(GLuint index, const GLint * v) {
    _pre_call_gl_callback("glVertexAttribI4iv", (GLADapiproc) glad_glVertexAttribI4iv, 2, index, v);
    glad_glVertexAttribI4iv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI4iv", (GLADapiproc) glad_glVertexAttribI4iv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI4IVPROC glad_debug_glVertexAttribI4iv = glad_debug_impl_glVertexAttribI4iv;
PFNGLVERTEXATTRIBI4SVPROC glad_glVertexAttribI4sv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4sv(GLuint index, const GLshort * v) {
    _pre_call_gl_callback("glVertexAttribI4sv", (GLADapiproc) glad_glVertexAttribI4sv, 2, index, v);
    glad_glVertexAttribI4sv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI4sv", (GLADapiproc) glad_glVertexAttribI4sv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI4SVPROC glad_debug_glVertexAttribI4sv = glad_debug_impl_glVertexAttribI4sv;
PFNGLVERTEXATTRIBI4UBVPROC glad_glVertexAttribI4ubv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4ubv(GLuint index, const GLubyte * v) {
    _pre_call_gl_callback("glVertexAttribI4ubv", (GLADapiproc) glad_glVertexAttribI4ubv, 2, index, v);
    glad_glVertexAttribI4ubv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI4ubv", (GLADapiproc) glad_glVertexAttribI4ubv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI4UBVPROC glad_debug_glVertexAttribI4ubv = glad_debug_impl_glVertexAttribI4ubv;
PFNGLVERTEXATTRIBI4UIPROC glad_glVertexAttribI4ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w) {
    _pre_call_gl_callback("glVertexAttribI4ui", (GLADapiproc) glad_glVertexAttribI4ui, 5, index, x, y, z, w);
    glad_glVertexAttribI4ui(index, x, y, z, w);
    _post_call_gl_callback(NULL, "glVertexAttribI4ui", (GLADapiproc) glad_glVertexAttribI4ui, 5, index, x, y, z, w);
    
}
PFNGLVERTEXATTRIBI4UIPROC glad_debug_glVertexAttribI4ui = glad_debug_impl_glVertexAttribI4ui;
PFNGLVERTEXATTRIBI4UIVPROC glad_glVertexAttribI4uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4uiv(GLuint index, const GLuint * v) {
    _pre_call_gl_callback("glVertexAttribI4uiv", (GLADapiproc) glad_glVertexAttribI4uiv, 2, index, v);
    glad_glVertexAttribI4uiv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI4uiv", (GLADapiproc) glad_glVertexAttribI4uiv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI4UIVPROC glad_debug_glVertexAttribI4uiv = glad_debug_impl_glVertexAttribI4uiv;
PFNGLVERTEXATTRIBI4USVPROC glad_glVertexAttribI4usv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribI4usv(GLuint index, const GLushort * v) {
    _pre_call_gl_callback("glVertexAttribI4usv", (GLADapiproc) glad_glVertexAttribI4usv, 2, index, v);
    glad_glVertexAttribI4usv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribI4usv", (GLADapiproc) glad_glVertexAttribI4usv, 2, index, v);
    
}
PFNGLVERTEXATTRIBI4USVPROC glad_debug_glVertexAttribI4usv = glad_debug_impl_glVertexAttribI4usv;
PFNGLVERTEXATTRIBIPOINTERPROC glad_glVertexAttribIPointer = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer) {
    _pre_call_gl_callback("glVertexAttribIPointer", (GLADapiproc) glad_glVertexAttribIPointer, 5, index, size, type, stride, pointer);
    glad_glVertexAttribIPointer(index, size, type, stride, pointer);
    _post_call_gl_callback(NULL, "glVertexAttribIPointer", (GLADapiproc) glad_glVertexAttribIPointer, 5, index, size, type, stride, pointer);
    
}
PFNGLVERTEXATTRIBIPOINTERPROC glad_debug_glVertexAttribIPointer = glad_debug_impl_glVertexAttribIPointer;
PFNGLVERTEXATTRIBL1DPROC glad_glVertexAttribL1d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL1d(GLuint index, GLdouble x) {
    _pre_call_gl_callback("glVertexAttribL1d", (GLADapiproc) glad_glVertexAttribL1d, 2, index, x);
    glad_glVertexAttribL1d(index, x);
    _post_call_gl_callback(NULL, "glVertexAttribL1d", (GLADapiproc) glad_glVertexAttribL1d, 2, index, x);
    
}
PFNGLVERTEXATTRIBL1DPROC glad_debug_glVertexAttribL1d = glad_debug_impl_glVertexAttribL1d;
PFNGLVERTEXATTRIBL1DVPROC glad_glVertexAttribL1dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL1dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttribL1dv", (GLADapiproc) glad_glVertexAttribL1dv, 2, index, v);
    glad_glVertexAttribL1dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribL1dv", (GLADapiproc) glad_glVertexAttribL1dv, 2, index, v);
    
}
PFNGLVERTEXATTRIBL1DVPROC glad_debug_glVertexAttribL1dv = glad_debug_impl_glVertexAttribL1dv;
PFNGLVERTEXATTRIBL2DPROC glad_glVertexAttribL2d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y) {
    _pre_call_gl_callback("glVertexAttribL2d", (GLADapiproc) glad_glVertexAttribL2d, 3, index, x, y);
    glad_glVertexAttribL2d(index, x, y);
    _post_call_gl_callback(NULL, "glVertexAttribL2d", (GLADapiproc) glad_glVertexAttribL2d, 3, index, x, y);
    
}
PFNGLVERTEXATTRIBL2DPROC glad_debug_glVertexAttribL2d = glad_debug_impl_glVertexAttribL2d;
PFNGLVERTEXATTRIBL2DVPROC glad_glVertexAttribL2dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL2dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttribL2dv", (GLADapiproc) glad_glVertexAttribL2dv, 2, index, v);
    glad_glVertexAttribL2dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribL2dv", (GLADapiproc) glad_glVertexAttribL2dv, 2, index, v);
    
}
PFNGLVERTEXATTRIBL2DVPROC glad_debug_glVertexAttribL2dv = glad_debug_impl_glVertexAttribL2dv;
PFNGLVERTEXATTRIBL3DPROC glad_glVertexAttribL3d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z) {
    _pre_call_gl_callback("glVertexAttribL3d", (GLADapiproc) glad_glVertexAttribL3d, 4, index, x, y, z);
    glad_glVertexAttribL3d(index, x, y, z);
    _post_call_gl_callback(NULL, "glVertexAttribL3d", (GLADapiproc) glad_glVertexAttribL3d, 4, index, x, y, z);
    
}
PFNGLVERTEXATTRIBL3DPROC glad_debug_glVertexAttribL3d = glad_debug_impl_glVertexAttribL3d;
PFNGLVERTEXATTRIBL3DVPROC glad_glVertexAttribL3dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL3dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttribL3dv", (GLADapiproc) glad_glVertexAttribL3dv, 2, index, v);
    glad_glVertexAttribL3dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribL3dv", (GLADapiproc) glad_glVertexAttribL3dv, 2, index, v);
    
}
PFNGLVERTEXATTRIBL3DVPROC glad_debug_glVertexAttribL3dv = glad_debug_impl_glVertexAttribL3dv;
PFNGLVERTEXATTRIBL4DPROC glad_glVertexAttribL4d = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w) {
    _pre_call_gl_callback("glVertexAttribL4d", (GLADapiproc) glad_glVertexAttribL4d, 5, index, x, y, z, w);
    glad_glVertexAttribL4d(index, x, y, z, w);
    _post_call_gl_callback(NULL, "glVertexAttribL4d", (GLADapiproc) glad_glVertexAttribL4d, 5, index, x, y, z, w);
    
}
PFNGLVERTEXATTRIBL4DPROC glad_debug_glVertexAttribL4d = glad_debug_impl_glVertexAttribL4d;
PFNGLVERTEXATTRIBL4DVPROC glad_glVertexAttribL4dv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribL4dv(GLuint index, const GLdouble * v) {
    _pre_call_gl_callback("glVertexAttribL4dv", (GLADapiproc) glad_glVertexAttribL4dv, 2, index, v);
    glad_glVertexAttribL4dv(index, v);
    _post_call_gl_callback(NULL, "glVertexAttribL4dv", (GLADapiproc) glad_glVertexAttribL4dv, 2, index, v);
    
}
PFNGLVERTEXATTRIBL4DVPROC glad_debug_glVertexAttribL4dv = glad_debug_impl_glVertexAttribL4dv;
PFNGLVERTEXATTRIBLPOINTERPROC glad_glVertexAttribLPointer = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void * pointer) {
    _pre_call_gl_callback("glVertexAttribLPointer", (GLADapiproc) glad_glVertexAttribLPointer, 5, index, size, type, stride, pointer);
    glad_glVertexAttribLPointer(index, size, type, stride, pointer);
    _post_call_gl_callback(NULL, "glVertexAttribLPointer", (GLADapiproc) glad_glVertexAttribLPointer, 5, index, size, type, stride, pointer);
    
}
PFNGLVERTEXATTRIBLPOINTERPROC glad_debug_glVertexAttribLPointer = glad_debug_impl_glVertexAttribLPointer;
PFNGLVERTEXATTRIBP1UIPROC glad_glVertexAttribP1ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    _pre_call_gl_callback("glVertexAttribP1ui", (GLADapiproc) glad_glVertexAttribP1ui, 4, index, type, normalized, value);
    glad_glVertexAttribP1ui(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP1ui", (GLADapiproc) glad_glVertexAttribP1ui, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP1UIPROC glad_debug_glVertexAttribP1ui = glad_debug_impl_glVertexAttribP1ui;
PFNGLVERTEXATTRIBP1UIVPROC glad_glVertexAttribP1uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value) {
    _pre_call_gl_callback("glVertexAttribP1uiv", (GLADapiproc) glad_glVertexAttribP1uiv, 4, index, type, normalized, value);
    glad_glVertexAttribP1uiv(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP1uiv", (GLADapiproc) glad_glVertexAttribP1uiv, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP1UIVPROC glad_debug_glVertexAttribP1uiv = glad_debug_impl_glVertexAttribP1uiv;
PFNGLVERTEXATTRIBP2UIPROC glad_glVertexAttribP2ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    _pre_call_gl_callback("glVertexAttribP2ui", (GLADapiproc) glad_glVertexAttribP2ui, 4, index, type, normalized, value);
    glad_glVertexAttribP2ui(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP2ui", (GLADapiproc) glad_glVertexAttribP2ui, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP2UIPROC glad_debug_glVertexAttribP2ui = glad_debug_impl_glVertexAttribP2ui;
PFNGLVERTEXATTRIBP2UIVPROC glad_glVertexAttribP2uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value) {
    _pre_call_gl_callback("glVertexAttribP2uiv", (GLADapiproc) glad_glVertexAttribP2uiv, 4, index, type, normalized, value);
    glad_glVertexAttribP2uiv(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP2uiv", (GLADapiproc) glad_glVertexAttribP2uiv, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP2UIVPROC glad_debug_glVertexAttribP2uiv = glad_debug_impl_glVertexAttribP2uiv;
PFNGLVERTEXATTRIBP3UIPROC glad_glVertexAttribP3ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    _pre_call_gl_callback("glVertexAttribP3ui", (GLADapiproc) glad_glVertexAttribP3ui, 4, index, type, normalized, value);
    glad_glVertexAttribP3ui(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP3ui", (GLADapiproc) glad_glVertexAttribP3ui, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP3UIPROC glad_debug_glVertexAttribP3ui = glad_debug_impl_glVertexAttribP3ui;
PFNGLVERTEXATTRIBP3UIVPROC glad_glVertexAttribP3uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value) {
    _pre_call_gl_callback("glVertexAttribP3uiv", (GLADapiproc) glad_glVertexAttribP3uiv, 4, index, type, normalized, value);
    glad_glVertexAttribP3uiv(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP3uiv", (GLADapiproc) glad_glVertexAttribP3uiv, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP3UIVPROC glad_debug_glVertexAttribP3uiv = glad_debug_impl_glVertexAttribP3uiv;
PFNGLVERTEXATTRIBP4UIPROC glad_glVertexAttribP4ui = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value) {
    _pre_call_gl_callback("glVertexAttribP4ui", (GLADapiproc) glad_glVertexAttribP4ui, 4, index, type, normalized, value);
    glad_glVertexAttribP4ui(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP4ui", (GLADapiproc) glad_glVertexAttribP4ui, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP4UIPROC glad_debug_glVertexAttribP4ui = glad_debug_impl_glVertexAttribP4ui;
PFNGLVERTEXATTRIBP4UIVPROC glad_glVertexAttribP4uiv = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint * value) {
    _pre_call_gl_callback("glVertexAttribP4uiv", (GLADapiproc) glad_glVertexAttribP4uiv, 4, index, type, normalized, value);
    glad_glVertexAttribP4uiv(index, type, normalized, value);
    _post_call_gl_callback(NULL, "glVertexAttribP4uiv", (GLADapiproc) glad_glVertexAttribP4uiv, 4, index, type, normalized, value);
    
}
PFNGLVERTEXATTRIBP4UIVPROC glad_debug_glVertexAttribP4uiv = glad_debug_impl_glVertexAttribP4uiv;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer = NULL;
static void GLAD_API_PTR glad_debug_impl_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer) {
    _pre_call_gl_callback("glVertexAttribPointer", (GLADapiproc) glad_glVertexAttribPointer, 6, index, size, type, normalized, stride, pointer);
    glad_glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    _post_call_gl_callback(NULL, "glVertexAttribPointer", (GLADapiproc) glad_glVertexAttribPointer, 6, index, size, type, normalized, stride, pointer);
    
}
PFNGLVERTEXATTRIBPOINTERPROC glad_debug_glVertexAttribPointer = glad_debug_impl_glVertexAttribPointer;
PFNGLVIEWPORTPROC glad_glViewport = NULL;
static void GLAD_API_PTR glad_debug_impl_glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    _pre_call_gl_callback("glViewport", (GLADapiproc) glad_glViewport, 4, x, y, width, height);
    glad_glViewport(x, y, width, height);
    _post_call_gl_callback(NULL, "glViewport", (GLADapiproc) glad_glViewport, 4, x, y, width, height);
    
}
PFNGLVIEWPORTPROC glad_debug_glViewport = glad_debug_impl_glViewport;
PFNGLVIEWPORTARRAYVPROC glad_glViewportArrayv = NULL;
static void GLAD_API_PTR glad_debug_impl_glViewportArrayv(GLuint first, GLsizei count, const GLfloat * v) {
    _pre_call_gl_callback("glViewportArrayv", (GLADapiproc) glad_glViewportArrayv, 3, first, count, v);
    glad_glViewportArrayv(first, count, v);
    _post_call_gl_callback(NULL, "glViewportArrayv", (GLADapiproc) glad_glViewportArrayv, 3, first, count, v);
    
}
PFNGLVIEWPORTARRAYVPROC glad_debug_glViewportArrayv = glad_debug_impl_glViewportArrayv;
PFNGLVIEWPORTINDEXEDFPROC glad_glViewportIndexedf = NULL;
static void GLAD_API_PTR glad_debug_impl_glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h) {
    _pre_call_gl_callback("glViewportIndexedf", (GLADapiproc) glad_glViewportIndexedf, 5, index, x, y, w, h);
    glad_glViewportIndexedf(index, x, y, w, h);
    _post_call_gl_callback(NULL, "glViewportIndexedf", (GLADapiproc) glad_glViewportIndexedf, 5, index, x, y, w, h);
    
}
PFNGLVIEWPORTINDEXEDFPROC glad_debug_glViewportIndexedf = glad_debug_impl_glViewportIndexedf;
PFNGLVIEWPORTINDEXEDFVPROC glad_glViewportIndexedfv = NULL;
static void GLAD_API_PTR glad_debug_impl_glViewportIndexedfv(GLuint index, const GLfloat * v) {
    _pre_call_gl_callback("glViewportIndexedfv", (GLADapiproc) glad_glViewportIndexedfv, 2, index, v);
    glad_glViewportIndexedfv(index, v);
    _post_call_gl_callback(NULL, "glViewportIndexedfv", (GLADapiproc) glad_glViewportIndexedfv, 2, index, v);
    
}
PFNGLVIEWPORTINDEXEDFVPROC glad_debug_glViewportIndexedfv = glad_debug_impl_glViewportIndexedfv;
PFNGLWAITSYNCPROC glad_glWaitSync = NULL;
static void GLAD_API_PTR glad_debug_impl_glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout) {
    _pre_call_gl_callback("glWaitSync", (GLADapiproc) glad_glWaitSync, 3, sync, flags, timeout);
    glad_glWaitSync(sync, flags, timeout);
    _post_call_gl_callback(NULL, "glWaitSync", (GLADapiproc) glad_glWaitSync, 3, sync, flags, timeout);
    
}
PFNGLWAITSYNCPROC glad_debug_glWaitSync = glad_debug_impl_glWaitSync;


static void glad_gl_load_GL_VERSION_1_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_1_0) return;
    glad_glBlendFunc = (PFNGLBLENDFUNCPROC) load(userptr, "glBlendFunc");
    glad_glClear = (PFNGLCLEARPROC) load(userptr, "glClear");
    glad_glClearColor = (PFNGLCLEARCOLORPROC) load(userptr, "glClearColor");
    glad_glClearDepth = (PFNGLCLEARDEPTHPROC) load(userptr, "glClearDepth");
    glad_glClearStencil = (PFNGLCLEARSTENCILPROC) load(userptr, "glClearStencil");
    glad_glColorMask = (PFNGLCOLORMASKPROC) load(userptr, "glColorMask");
    glad_glCullFace = (PFNGLCULLFACEPROC) load(userptr, "glCullFace");
    glad_glDepthFunc = (PFNGLDEPTHFUNCPROC) load(userptr, "glDepthFunc");
    glad_glDepthMask = (PFNGLDEPTHMASKPROC) load(userptr, "glDepthMask");
    glad_glDepthRange = (PFNGLDEPTHRANGEPROC) load(userptr, "glDepthRange");
    glad_glDisable = (PFNGLDISABLEPROC) load(userptr, "glDisable");
    glad_glDrawBuffer = (PFNGLDRAWBUFFERPROC) load(userptr, "glDrawBuffer");
    glad_glEnable = (PFNGLENABLEPROC) load(userptr, "glEnable");
    glad_glFinish = (PFNGLFINISHPROC) load(userptr, "glFinish");
    glad_glFlush = (PFNGLFLUSHPROC) load(userptr, "glFlush");
    glad_glFrontFace = (PFNGLFRONTFACEPROC) load(userptr, "glFrontFace");
    glad_glGetBooleanv = (PFNGLGETBOOLEANVPROC) load(userptr, "glGetBooleanv");
    glad_glGetDoublev = (PFNGLGETDOUBLEVPROC) load(userptr, "glGetDoublev");
    glad_glGetError = (PFNGLGETERRORPROC) load(userptr, "glGetError");
    glad_glGetFloatv = (PFNGLGETFLOATVPROC) load(userptr, "glGetFloatv");
    glad_glGetIntegerv = (PFNGLGETINTEGERVPROC) load(userptr, "glGetIntegerv");
    glad_glGetString = (PFNGLGETSTRINGPROC) load(userptr, "glGetString");
    glad_glGetTexImage = (PFNGLGETTEXIMAGEPROC) load(userptr, "glGetTexImage");
    glad_glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC) load(userptr, "glGetTexLevelParameterfv");
    glad_glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC) load(userptr, "glGetTexLevelParameteriv");
    glad_glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC) load(userptr, "glGetTexParameterfv");
    glad_glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC) load(userptr, "glGetTexParameteriv");
    glad_glHint = (PFNGLHINTPROC) load(userptr, "glHint");
    glad_glIsEnabled = (PFNGLISENABLEDPROC) load(userptr, "glIsEnabled");
    glad_glLineWidth = (PFNGLLINEWIDTHPROC) load(userptr, "glLineWidth");
    glad_glLogicOp = (PFNGLLOGICOPPROC) load(userptr, "glLogicOp");
    glad_glPixelStoref = (PFNGLPIXELSTOREFPROC) load(userptr, "glPixelStoref");
    glad_glPixelStorei = (PFNGLPIXELSTOREIPROC) load(userptr, "glPixelStorei");
    glad_glPointSize = (PFNGLPOINTSIZEPROC) load(userptr, "glPointSize");
    glad_glPolygonMode = (PFNGLPOLYGONMODEPROC) load(userptr, "glPolygonMode");
    glad_glReadBuffer = (PFNGLREADBUFFERPROC) load(userptr, "glReadBuffer");
    glad_glReadPixels = (PFNGLREADPIXELSPROC) load(userptr, "glReadPixels");
    glad_glScissor = (PFNGLSCISSORPROC) load(userptr, "glScissor");
    glad_glStencilFunc = (PFNGLSTENCILFUNCPROC) load(userptr, "glStencilFunc");
    glad_glStencilMask = (PFNGLSTENCILMASKPROC) load(userptr, "glStencilMask");
    glad_glStencilOp = (PFNGLSTENCILOPPROC) load(userptr, "glStencilOp");
    glad_glTexImage1D = (PFNGLTEXIMAGE1DPROC) load(userptr, "glTexImage1D");
    glad_glTexImage2D = (PFNGLTEXIMAGE2DPROC) load(userptr, "glTexImage2D");
    glad_glTexParameterf = (PFNGLTEXPARAMETERFPROC) load(userptr, "glTexParameterf");
    glad_glTexParameterfv = (PFNGLTEXPARAMETERFVPROC) load(userptr, "glTexParameterfv");
    glad_glTexParameteri = (PFNGLTEXPARAMETERIPROC) load(userptr, "glTexParameteri");
    glad_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC) load(userptr, "glTexParameteriv");
    glad_glViewport = (PFNGLVIEWPORTPROC) load(userptr, "glViewport");
}
static void glad_gl_load_GL_VERSION_1_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_1_1) return;
    glad_glBindTexture = (PFNGLBINDTEXTUREPROC) load(userptr, "glBindTexture");
    glad_glCopyTexImage1D = (PFNGLCOPYTEXIMAGE1DPROC) load(userptr, "glCopyTexImage1D");
    glad_glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC) load(userptr, "glCopyTexImage2D");
    glad_glCopyTexSubImage1D = (PFNGLCOPYTEXSUBIMAGE1DPROC) load(userptr, "glCopyTexSubImage1D");
    glad_glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC) load(userptr, "glCopyTexSubImage2D");
    glad_glDeleteTextures = (PFNGLDELETETEXTURESPROC) load(userptr, "glDeleteTextures");
    glad_glDrawArrays = (PFNGLDRAWARRAYSPROC) load(userptr, "glDrawArrays");
    glad_glDrawElements = (PFNGLDRAWELEMENTSPROC) load(userptr, "glDrawElements");
    glad_glGenTextures = (PFNGLGENTEXTURESPROC) load(userptr, "glGenTextures");
    glad_glGetPointerv = (PFNGLGETPOINTERVPROC) load(userptr, "glGetPointerv");
    glad_glIsTexture = (PFNGLISTEXTUREPROC) load(userptr, "glIsTexture");
    glad_glPolygonOffset = (PFNGLPOLYGONOFFSETPROC) load(userptr, "glPolygonOffset");
    glad_glTexSubImage1D = (PFNGLTEXSUBIMAGE1DPROC) load(userptr, "glTexSubImage1D");
    glad_glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC) load(userptr, "glTexSubImage2D");
}
static void glad_gl_load_GL_VERSION_1_2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_1_2) return;
    glad_glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC) load(userptr, "glCopyTexSubImage3D");
    glad_glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) load(userptr, "glDrawRangeElements");
    glad_glTexImage3D = (PFNGLTEXIMAGE3DPROC) load(userptr, "glTexImage3D");
    glad_glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) load(userptr, "glTexSubImage3D");
}
static void glad_gl_load_GL_VERSION_1_3( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_1_3) return;
    glad_glActiveTexture = (PFNGLACTIVETEXTUREPROC) load(userptr, "glActiveTexture");
    glad_glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC) load(userptr, "glCompressedTexImage1D");
    glad_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) load(userptr, "glCompressedTexImage2D");
    glad_glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC) load(userptr, "glCompressedTexImage3D");
    glad_glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) load(userptr, "glCompressedTexSubImage1D");
    glad_glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) load(userptr, "glCompressedTexSubImage2D");
    glad_glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) load(userptr, "glCompressedTexSubImage3D");
    glad_glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC) load(userptr, "glGetCompressedTexImage");
    glad_glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) load(userptr, "glSampleCoverage");
}
static void glad_gl_load_GL_VERSION_1_4( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_1_4) return;
    glad_glBlendColor = (PFNGLBLENDCOLORPROC) load(userptr, "glBlendColor");
    glad_glBlendEquation = (PFNGLBLENDEQUATIONPROC) load(userptr, "glBlendEquation");
    glad_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) load(userptr, "glBlendFuncSeparate");
    glad_glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC) load(userptr, "glMultiDrawArrays");
    glad_glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC) load(userptr, "glMultiDrawElements");
    glad_glPointParameterf = (PFNGLPOINTPARAMETERFPROC) load(userptr, "glPointParameterf");
    glad_glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC) load(userptr, "glPointParameterfv");
    glad_glPointParameteri = (PFNGLPOINTPARAMETERIPROC) load(userptr, "glPointParameteri");
    glad_glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC) load(userptr, "glPointParameteriv");
}
static void glad_gl_load_GL_VERSION_1_5( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_1_5) return;
    glad_glBeginQuery = (PFNGLBEGINQUERYPROC) load(userptr, "glBeginQuery");
    glad_glBindBuffer = (PFNGLBINDBUFFERPROC) load(userptr, "glBindBuffer");
    glad_glBufferData = (PFNGLBUFFERDATAPROC) load(userptr, "glBufferData");
    glad_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) load(userptr, "glBufferSubData");
    glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) load(userptr, "glDeleteBuffers");
    glad_glDeleteQueries = (PFNGLDELETEQUERIESPROC) load(userptr, "glDeleteQueries");
    glad_glEndQuery = (PFNGLENDQUERYPROC) load(userptr, "glEndQuery");
    glad_glGenBuffers = (PFNGLGENBUFFERSPROC) load(userptr, "glGenBuffers");
    glad_glGenQueries = (PFNGLGENQUERIESPROC) load(userptr, "glGenQueries");
    glad_glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) load(userptr, "glGetBufferParameteriv");
    glad_glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC) load(userptr, "glGetBufferPointerv");
    glad_glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC) load(userptr, "glGetBufferSubData");
    glad_glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC) load(userptr, "glGetQueryObjectiv");
    glad_glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC) load(userptr, "glGetQueryObjectuiv");
    glad_glGetQueryiv = (PFNGLGETQUERYIVPROC) load(userptr, "glGetQueryiv");
    glad_glIsBuffer = (PFNGLISBUFFERPROC) load(userptr, "glIsBuffer");
    glad_glIsQuery = (PFNGLISQUERYPROC) load(userptr, "glIsQuery");
    glad_glMapBuffer = (PFNGLMAPBUFFERPROC) load(userptr, "glMapBuffer");
    glad_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) load(userptr, "glUnmapBuffer");
}
static void glad_gl_load_GL_VERSION_2_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_2_0) return;
    glad_glAttachShader = (PFNGLATTACHSHADERPROC) load(userptr, "glAttachShader");
    glad_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) load(userptr, "glBindAttribLocation");
    glad_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) load(userptr, "glBlendEquationSeparate");
    glad_glCompileShader = (PFNGLCOMPILESHADERPROC) load(userptr, "glCompileShader");
    glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC) load(userptr, "glCreateProgram");
    glad_glCreateShader = (PFNGLCREATESHADERPROC) load(userptr, "glCreateShader");
    glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) load(userptr, "glDeleteProgram");
    glad_glDeleteShader = (PFNGLDELETESHADERPROC) load(userptr, "glDeleteShader");
    glad_glDetachShader = (PFNGLDETACHSHADERPROC) load(userptr, "glDetachShader");
    glad_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) load(userptr, "glDisableVertexAttribArray");
    glad_glDrawBuffers = (PFNGLDRAWBUFFERSPROC) load(userptr, "glDrawBuffers");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) load(userptr, "glEnableVertexAttribArray");
    glad_glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) load(userptr, "glGetActiveAttrib");
    glad_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) load(userptr, "glGetActiveUniform");
    glad_glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) load(userptr, "glGetAttachedShaders");
    glad_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) load(userptr, "glGetAttribLocation");
    glad_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) load(userptr, "glGetProgramInfoLog");
    glad_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) load(userptr, "glGetProgramiv");
    glad_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) load(userptr, "glGetShaderInfoLog");
    glad_glGetShaderSource = (PFNGLGETSHADERSOURCEPROC) load(userptr, "glGetShaderSource");
    glad_glGetShaderiv = (PFNGLGETSHADERIVPROC) load(userptr, "glGetShaderiv");
    glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) load(userptr, "glGetUniformLocation");
    glad_glGetUniformfv = (PFNGLGETUNIFORMFVPROC) load(userptr, "glGetUniformfv");
    glad_glGetUniformiv = (PFNGLGETUNIFORMIVPROC) load(userptr, "glGetUniformiv");
    glad_glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) load(userptr, "glGetVertexAttribPointerv");
    glad_glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC) load(userptr, "glGetVertexAttribdv");
    glad_glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) load(userptr, "glGetVertexAttribfv");
    glad_glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) load(userptr, "glGetVertexAttribiv");
    glad_glIsProgram = (PFNGLISPROGRAMPROC) load(userptr, "glIsProgram");
    glad_glIsShader = (PFNGLISSHADERPROC) load(userptr, "glIsShader");
    glad_glLinkProgram = (PFNGLLINKPROGRAMPROC) load(userptr, "glLinkProgram");
    glad_glShaderSource = (PFNGLSHADERSOURCEPROC) load(userptr, "glShaderSource");
    glad_glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) load(userptr, "glStencilFuncSeparate");
    glad_glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) load(userptr, "glStencilMaskSeparate");
    glad_glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) load(userptr, "glStencilOpSeparate");
    glad_glUniform1f = (PFNGLUNIFORM1FPROC) load(userptr, "glUniform1f");
    glad_glUniform1fv = (PFNGLUNIFORM1FVPROC) load(userptr, "glUniform1fv");
    glad_glUniform1i = (PFNGLUNIFORM1IPROC) load(userptr, "glUniform1i");
    glad_glUniform1iv = (PFNGLUNIFORM1IVPROC) load(userptr, "glUniform1iv");
    glad_glUniform2f = (PFNGLUNIFORM2FPROC) load(userptr, "glUniform2f");
    glad_glUniform2fv = (PFNGLUNIFORM2FVPROC) load(userptr, "glUniform2fv");
    glad_glUniform2i = (PFNGLUNIFORM2IPROC) load(userptr, "glUniform2i");
    glad_glUniform2iv = (PFNGLUNIFORM2IVPROC) load(userptr, "glUniform2iv");
    glad_glUniform3f = (PFNGLUNIFORM3FPROC) load(userptr, "glUniform3f");
    glad_glUniform3fv = (PFNGLUNIFORM3FVPROC) load(userptr, "glUniform3fv");
    glad_glUniform3i = (PFNGLUNIFORM3IPROC) load(userptr, "glUniform3i");
    glad_glUniform3iv = (PFNGLUNIFORM3IVPROC) load(userptr, "glUniform3iv");
    glad_glUniform4f = (PFNGLUNIFORM4FPROC) load(userptr, "glUniform4f");
    glad_glUniform4fv = (PFNGLUNIFORM4FVPROC) load(userptr, "glUniform4fv");
    glad_glUniform4i = (PFNGLUNIFORM4IPROC) load(userptr, "glUniform4i");
    glad_glUniform4iv = (PFNGLUNIFORM4IVPROC) load(userptr, "glUniform4iv");
    glad_glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) load(userptr, "glUniformMatrix2fv");
    glad_glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) load(userptr, "glUniformMatrix3fv");
    glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) load(userptr, "glUniformMatrix4fv");
    glad_glUseProgram = (PFNGLUSEPROGRAMPROC) load(userptr, "glUseProgram");
    glad_glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) load(userptr, "glValidateProgram");
    glad_glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC) load(userptr, "glVertexAttrib1d");
    glad_glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC) load(userptr, "glVertexAttrib1dv");
    glad_glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC) load(userptr, "glVertexAttrib1f");
    glad_glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC) load(userptr, "glVertexAttrib1fv");
    glad_glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC) load(userptr, "glVertexAttrib1s");
    glad_glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC) load(userptr, "glVertexAttrib1sv");
    glad_glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC) load(userptr, "glVertexAttrib2d");
    glad_glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC) load(userptr, "glVertexAttrib2dv");
    glad_glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC) load(userptr, "glVertexAttrib2f");
    glad_glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC) load(userptr, "glVertexAttrib2fv");
    glad_glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC) load(userptr, "glVertexAttrib2s");
    glad_glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC) load(userptr, "glVertexAttrib2sv");
    glad_glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC) load(userptr, "glVertexAttrib3d");
    glad_glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC) load(userptr, "glVertexAttrib3dv");
    glad_glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC) load(userptr, "glVertexAttrib3f");
    glad_glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC) load(userptr, "glVertexAttrib3fv");
    glad_glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC) load(userptr, "glVertexAttrib3s");
    glad_glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC) load(userptr, "glVertexAttrib3sv");
    glad_glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC) load(userptr, "glVertexAttrib4Nbv");
    glad_glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC) load(userptr, "glVertexAttrib4Niv");
    glad_glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC) load(userptr, "glVertexAttrib4Nsv");
    glad_glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC) load(userptr, "glVertexAttrib4Nub");
    glad_glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC) load(userptr, "glVertexAttrib4Nubv");
    glad_glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC) load(userptr, "glVertexAttrib4Nuiv");
    glad_glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC) load(userptr, "glVertexAttrib4Nusv");
    glad_glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC) load(userptr, "glVertexAttrib4bv");
    glad_glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC) load(userptr, "glVertexAttrib4d");
    glad_glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC) load(userptr, "glVertexAttrib4dv");
    glad_glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC) load(userptr, "glVertexAttrib4f");
    glad_glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC) load(userptr, "glVertexAttrib4fv");
    glad_glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC) load(userptr, "glVertexAttrib4iv");
    glad_glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC) load(userptr, "glVertexAttrib4s");
    glad_glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC) load(userptr, "glVertexAttrib4sv");
    glad_glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC) load(userptr, "glVertexAttrib4ubv");
    glad_glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC) load(userptr, "glVertexAttrib4uiv");
    glad_glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC) load(userptr, "glVertexAttrib4usv");
    glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) load(userptr, "glVertexAttribPointer");
}
static void glad_gl_load_GL_VERSION_2_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_2_1) return;
    glad_glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) load(userptr, "glUniformMatrix2x3fv");
    glad_glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) load(userptr, "glUniformMatrix2x4fv");
    glad_glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) load(userptr, "glUniformMatrix3x2fv");
    glad_glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) load(userptr, "glUniformMatrix3x4fv");
    glad_glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) load(userptr, "glUniformMatrix4x2fv");
    glad_glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) load(userptr, "glUniformMatrix4x3fv");
}
static void glad_gl_load_GL_VERSION_3_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_3_0) return;
    glad_glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC) load(userptr, "glBeginConditionalRender");
    glad_glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC) load(userptr, "glBeginTransformFeedback");
    glad_glBindBufferBase = (PFNGLBINDBUFFERBASEPROC) load(userptr, "glBindBufferBase");
    glad_glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC) load(userptr, "glBindBufferRange");
    glad_glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC) load(userptr, "glBindFragDataLocation");
    glad_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) load(userptr, "glBindFramebuffer");
    glad_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) load(userptr, "glBindRenderbuffer");
    glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) load(userptr, "glBindVertexArray");
    glad_glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) load(userptr, "glBlitFramebuffer");
    glad_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) load(userptr, "glCheckFramebufferStatus");
    glad_glClampColor = (PFNGLCLAMPCOLORPROC) load(userptr, "glClampColor");
    glad_glClearBufferfi = (PFNGLCLEARBUFFERFIPROC) load(userptr, "glClearBufferfi");
    glad_glClearBufferfv = (PFNGLCLEARBUFFERFVPROC) load(userptr, "glClearBufferfv");
    glad_glClearBufferiv = (PFNGLCLEARBUFFERIVPROC) load(userptr, "glClearBufferiv");
    glad_glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC) load(userptr, "glClearBufferuiv");
    glad_glColorMaski = (PFNGLCOLORMASKIPROC) load(userptr, "glColorMaski");
    glad_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) load(userptr, "glDeleteFramebuffers");
    glad_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) load(userptr, "glDeleteRenderbuffers");
    glad_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) load(userptr, "glDeleteVertexArrays");
    glad_glDisablei = (PFNGLDISABLEIPROC) load(userptr, "glDisablei");
    glad_glEnablei = (PFNGLENABLEIPROC) load(userptr, "glEnablei");
    glad_glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC) load(userptr, "glEndConditionalRender");
    glad_glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC) load(userptr, "glEndTransformFeedback");
    glad_glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC) load(userptr, "glFlushMappedBufferRange");
    glad_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) load(userptr, "glFramebufferRenderbuffer");
    glad_glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC) load(userptr, "glFramebufferTexture1D");
    glad_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) load(userptr, "glFramebufferTexture2D");
    glad_glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC) load(userptr, "glFramebufferTexture3D");
    glad_glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC) load(userptr, "glFramebufferTextureLayer");
    glad_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) load(userptr, "glGenFramebuffers");
    glad_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) load(userptr, "glGenRenderbuffers");
    glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) load(userptr, "glGenVertexArrays");
    glad_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) load(userptr, "glGenerateMipmap");
    glad_glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC) load(userptr, "glGetBooleani_v");
    glad_glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC) load(userptr, "glGetFragDataLocation");
    glad_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) load(userptr, "glGetFramebufferAttachmentParameteriv");
    glad_glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC) load(userptr, "glGetIntegeri_v");
    glad_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) load(userptr, "glGetRenderbufferParameteriv");
    glad_glGetStringi = (PFNGLGETSTRINGIPROC) load(userptr, "glGetStringi");
    glad_glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC) load(userptr, "glGetTexParameterIiv");
    glad_glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC) load(userptr, "glGetTexParameterIuiv");
    glad_glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) load(userptr, "glGetTransformFeedbackVarying");
    glad_glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC) load(userptr, "glGetUniformuiv");
    glad_glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC) load(userptr, "glGetVertexAttribIiv");
    glad_glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC) load(userptr, "glGetVertexAttribIuiv");
    glad_glIsEnabledi = (PFNGLISENABLEDIPROC) load(userptr, "glIsEnabledi");
    glad_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) load(userptr, "glIsFramebuffer");
    glad_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) load(userptr, "glIsRenderbuffer");
    glad_glIsVertexArray = (PFNGLISVERTEXARRAYPROC) load(userptr, "glIsVertexArray");
    glad_glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC) load(userptr, "glMapBufferRange");
    glad_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) load(userptr, "glRenderbufferStorage");
    glad_glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) load(userptr, "glRenderbufferStorageMultisample");
    glad_glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC) load(userptr, "glTexParameterIiv");
    glad_glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC) load(userptr, "glTexParameterIuiv");
    glad_glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC) load(userptr, "glTransformFeedbackVaryings");
    glad_glUniform1ui = (PFNGLUNIFORM1UIPROC) load(userptr, "glUniform1ui");
    glad_glUniform1uiv = (PFNGLUNIFORM1UIVPROC) load(userptr, "glUniform1uiv");
    glad_glUniform2ui = (PFNGLUNIFORM2UIPROC) load(userptr, "glUniform2ui");
    glad_glUniform2uiv = (PFNGLUNIFORM2UIVPROC) load(userptr, "glUniform2uiv");
    glad_glUniform3ui = (PFNGLUNIFORM3UIPROC) load(userptr, "glUniform3ui");
    glad_glUniform3uiv = (PFNGLUNIFORM3UIVPROC) load(userptr, "glUniform3uiv");
    glad_glUniform4ui = (PFNGLUNIFORM4UIPROC) load(userptr, "glUniform4ui");
    glad_glUniform4uiv = (PFNGLUNIFORM4UIVPROC) load(userptr, "glUniform4uiv");
    glad_glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC) load(userptr, "glVertexAttribI1i");
    glad_glVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IVPROC) load(userptr, "glVertexAttribI1iv");
    glad_glVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UIPROC) load(userptr, "glVertexAttribI1ui");
    glad_glVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIVPROC) load(userptr, "glVertexAttribI1uiv");
    glad_glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC) load(userptr, "glVertexAttribI2i");
    glad_glVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IVPROC) load(userptr, "glVertexAttribI2iv");
    glad_glVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UIPROC) load(userptr, "glVertexAttribI2ui");
    glad_glVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIVPROC) load(userptr, "glVertexAttribI2uiv");
    glad_glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC) load(userptr, "glVertexAttribI3i");
    glad_glVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IVPROC) load(userptr, "glVertexAttribI3iv");
    glad_glVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UIPROC) load(userptr, "glVertexAttribI3ui");
    glad_glVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIVPROC) load(userptr, "glVertexAttribI3uiv");
    glad_glVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BVPROC) load(userptr, "glVertexAttribI4bv");
    glad_glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC) load(userptr, "glVertexAttribI4i");
    glad_glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC) load(userptr, "glVertexAttribI4iv");
    glad_glVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SVPROC) load(userptr, "glVertexAttribI4sv");
    glad_glVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBVPROC) load(userptr, "glVertexAttribI4ubv");
    glad_glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC) load(userptr, "glVertexAttribI4ui");
    glad_glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC) load(userptr, "glVertexAttribI4uiv");
    glad_glVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USVPROC) load(userptr, "glVertexAttribI4usv");
    glad_glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC) load(userptr, "glVertexAttribIPointer");
}
static void glad_gl_load_GL_VERSION_3_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_3_1) return;
    glad_glBindBufferBase = (PFNGLBINDBUFFERBASEPROC) load(userptr, "glBindBufferBase");
    glad_glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC) load(userptr, "glBindBufferRange");
    glad_glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC) load(userptr, "glCopyBufferSubData");
    glad_glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC) load(userptr, "glDrawArraysInstanced");
    glad_glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC) load(userptr, "glDrawElementsInstanced");
    glad_glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC) load(userptr, "glGetActiveUniformBlockName");
    glad_glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC) load(userptr, "glGetActiveUniformBlockiv");
    glad_glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC) load(userptr, "glGetActiveUniformName");
    glad_glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC) load(userptr, "glGetActiveUniformsiv");
    glad_glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC) load(userptr, "glGetIntegeri_v");
    glad_glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC) load(userptr, "glGetUniformBlockIndex");
    glad_glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC) load(userptr, "glGetUniformIndices");
    glad_glPrimitiveRestartIndex = (PFNGLPRIMITIVERESTARTINDEXPROC) load(userptr, "glPrimitiveRestartIndex");
    glad_glTexBuffer = (PFNGLTEXBUFFERPROC) load(userptr, "glTexBuffer");
    glad_glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC) load(userptr, "glUniformBlockBinding");
}
static void glad_gl_load_GL_VERSION_3_2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_3_2) return;
    glad_glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC) load(userptr, "glClientWaitSync");
    glad_glDeleteSync = (PFNGLDELETESYNCPROC) load(userptr, "glDeleteSync");
    glad_glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC) load(userptr, "glDrawElementsBaseVertex");
    glad_glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC) load(userptr, "glDrawElementsInstancedBaseVertex");
    glad_glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC) load(userptr, "glDrawRangeElementsBaseVertex");
    glad_glFenceSync = (PFNGLFENCESYNCPROC) load(userptr, "glFenceSync");
    glad_glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC) load(userptr, "glFramebufferTexture");
    glad_glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC) load(userptr, "glGetBufferParameteri64v");
    glad_glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC) load(userptr, "glGetInteger64i_v");
    glad_glGetInteger64v = (PFNGLGETINTEGER64VPROC) load(userptr, "glGetInteger64v");
    glad_glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC) load(userptr, "glGetMultisamplefv");
    glad_glGetSynciv = (PFNGLGETSYNCIVPROC) load(userptr, "glGetSynciv");
    glad_glIsSync = (PFNGLISSYNCPROC) load(userptr, "glIsSync");
    glad_glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC) load(userptr, "glMultiDrawElementsBaseVertex");
    glad_glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC) load(userptr, "glProvokingVertex");
    glad_glSampleMaski = (PFNGLSAMPLEMASKIPROC) load(userptr, "glSampleMaski");
    glad_glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC) load(userptr, "glTexImage2DMultisample");
    glad_glTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLEPROC) load(userptr, "glTexImage3DMultisample");
    glad_glWaitSync = (PFNGLWAITSYNCPROC) load(userptr, "glWaitSync");
}
static void glad_gl_load_GL_VERSION_3_3( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_3_3) return;
    glad_glBindFragDataLocationIndexed = (PFNGLBINDFRAGDATALOCATIONINDEXEDPROC) load(userptr, "glBindFragDataLocationIndexed");
    glad_glBindSampler = (PFNGLBINDSAMPLERPROC) load(userptr, "glBindSampler");
    glad_glDeleteSamplers = (PFNGLDELETESAMPLERSPROC) load(userptr, "glDeleteSamplers");
    glad_glGenSamplers = (PFNGLGENSAMPLERSPROC) load(userptr, "glGenSamplers");
    glad_glGetFragDataIndex = (PFNGLGETFRAGDATAINDEXPROC) load(userptr, "glGetFragDataIndex");
    glad_glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC) load(userptr, "glGetQueryObjecti64v");
    glad_glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC) load(userptr, "glGetQueryObjectui64v");
    glad_glGetSamplerParameterIiv = (PFNGLGETSAMPLERPARAMETERIIVPROC) load(userptr, "glGetSamplerParameterIiv");
    glad_glGetSamplerParameterIuiv = (PFNGLGETSAMPLERPARAMETERIUIVPROC) load(userptr, "glGetSamplerParameterIuiv");
    glad_glGetSamplerParameterfv = (PFNGLGETSAMPLERPARAMETERFVPROC) load(userptr, "glGetSamplerParameterfv");
    glad_glGetSamplerParameteriv = (PFNGLGETSAMPLERPARAMETERIVPROC) load(userptr, "glGetSamplerParameteriv");
    glad_glIsSampler = (PFNGLISSAMPLERPROC) load(userptr, "glIsSampler");
    glad_glQueryCounter = (PFNGLQUERYCOUNTERPROC) load(userptr, "glQueryCounter");
    glad_glSamplerParameterIiv = (PFNGLSAMPLERPARAMETERIIVPROC) load(userptr, "glSamplerParameterIiv");
    glad_glSamplerParameterIuiv = (PFNGLSAMPLERPARAMETERIUIVPROC) load(userptr, "glSamplerParameterIuiv");
    glad_glSamplerParameterf = (PFNGLSAMPLERPARAMETERFPROC) load(userptr, "glSamplerParameterf");
    glad_glSamplerParameterfv = (PFNGLSAMPLERPARAMETERFVPROC) load(userptr, "glSamplerParameterfv");
    glad_glSamplerParameteri = (PFNGLSAMPLERPARAMETERIPROC) load(userptr, "glSamplerParameteri");
    glad_glSamplerParameteriv = (PFNGLSAMPLERPARAMETERIVPROC) load(userptr, "glSamplerParameteriv");
    glad_glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC) load(userptr, "glVertexAttribDivisor");
    glad_glVertexAttribP1ui = (PFNGLVERTEXATTRIBP1UIPROC) load(userptr, "glVertexAttribP1ui");
    glad_glVertexAttribP1uiv = (PFNGLVERTEXATTRIBP1UIVPROC) load(userptr, "glVertexAttribP1uiv");
    glad_glVertexAttribP2ui = (PFNGLVERTEXATTRIBP2UIPROC) load(userptr, "glVertexAttribP2ui");
    glad_glVertexAttribP2uiv = (PFNGLVERTEXATTRIBP2UIVPROC) load(userptr, "glVertexAttribP2uiv");
    glad_glVertexAttribP3ui = (PFNGLVERTEXATTRIBP3UIPROC) load(userptr, "glVertexAttribP3ui");
    glad_glVertexAttribP3uiv = (PFNGLVERTEXATTRIBP3UIVPROC) load(userptr, "glVertexAttribP3uiv");
    glad_glVertexAttribP4ui = (PFNGLVERTEXATTRIBP4UIPROC) load(userptr, "glVertexAttribP4ui");
    glad_glVertexAttribP4uiv = (PFNGLVERTEXATTRIBP4UIVPROC) load(userptr, "glVertexAttribP4uiv");
}
static void glad_gl_load_GL_VERSION_4_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_4_0) return;
    glad_glBeginQueryIndexed = (PFNGLBEGINQUERYINDEXEDPROC) load(userptr, "glBeginQueryIndexed");
    glad_glBindTransformFeedback = (PFNGLBINDTRANSFORMFEEDBACKPROC) load(userptr, "glBindTransformFeedback");
    glad_glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC) load(userptr, "glBlendEquationSeparatei");
    glad_glBlendEquationi = (PFNGLBLENDEQUATIONIPROC) load(userptr, "glBlendEquationi");
    glad_glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC) load(userptr, "glBlendFuncSeparatei");
    glad_glBlendFunci = (PFNGLBLENDFUNCIPROC) load(userptr, "glBlendFunci");
    glad_glDeleteTransformFeedbacks = (PFNGLDELETETRANSFORMFEEDBACKSPROC) load(userptr, "glDeleteTransformFeedbacks");
    glad_glDrawArraysIndirect = (PFNGLDRAWARRAYSINDIRECTPROC) load(userptr, "glDrawArraysIndirect");
    glad_glDrawElementsIndirect = (PFNGLDRAWELEMENTSINDIRECTPROC) load(userptr, "glDrawElementsIndirect");
    glad_glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKPROC) load(userptr, "glDrawTransformFeedback");
    glad_glDrawTransformFeedbackStream = (PFNGLDRAWTRANSFORMFEEDBACKSTREAMPROC) load(userptr, "glDrawTransformFeedbackStream");
    glad_glEndQueryIndexed = (PFNGLENDQUERYINDEXEDPROC) load(userptr, "glEndQueryIndexed");
    glad_glGenTransformFeedbacks = (PFNGLGENTRANSFORMFEEDBACKSPROC) load(userptr, "glGenTransformFeedbacks");
    glad_glGetActiveSubroutineName = (PFNGLGETACTIVESUBROUTINENAMEPROC) load(userptr, "glGetActiveSubroutineName");
    glad_glGetActiveSubroutineUniformName = (PFNGLGETACTIVESUBROUTINEUNIFORMNAMEPROC) load(userptr, "glGetActiveSubroutineUniformName");
    glad_glGetActiveSubroutineUniformiv = (PFNGLGETACTIVESUBROUTINEUNIFORMIVPROC) load(userptr, "glGetActiveSubroutineUniformiv");
    glad_glGetProgramStageiv = (PFNGLGETPROGRAMSTAGEIVPROC) load(userptr, "glGetProgramStageiv");
    glad_glGetQueryIndexediv = (PFNGLGETQUERYINDEXEDIVPROC) load(userptr, "glGetQueryIndexediv");
    glad_glGetSubroutineIndex = (PFNGLGETSUBROUTINEINDEXPROC) load(userptr, "glGetSubroutineIndex");
    glad_glGetSubroutineUniformLocation = (PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC) load(userptr, "glGetSubroutineUniformLocation");
    glad_glGetUniformSubroutineuiv = (PFNGLGETUNIFORMSUBROUTINEUIVPROC) load(userptr, "glGetUniformSubroutineuiv");
    glad_glGetUniformdv = (PFNGLGETUNIFORMDVPROC) load(userptr, "glGetUniformdv");
    glad_glIsTransformFeedback = (PFNGLISTRANSFORMFEEDBACKPROC) load(userptr, "glIsTransformFeedback");
    glad_glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC) load(userptr, "glMinSampleShading");
    glad_glPatchParameterfv = (PFNGLPATCHPARAMETERFVPROC) load(userptr, "glPatchParameterfv");
    glad_glPatchParameteri = (PFNGLPATCHPARAMETERIPROC) load(userptr, "glPatchParameteri");
    glad_glPauseTransformFeedback = (PFNGLPAUSETRANSFORMFEEDBACKPROC) load(userptr, "glPauseTransformFeedback");
    glad_glResumeTransformFeedback = (PFNGLRESUMETRANSFORMFEEDBACKPROC) load(userptr, "glResumeTransformFeedback");
    glad_glUniform1d = (PFNGLUNIFORM1DPROC) load(userptr, "glUniform1d");
    glad_glUniform1dv = (PFNGLUNIFORM1DVPROC) load(userptr, "glUniform1dv");
    glad_glUniform2d = (PFNGLUNIFORM2DPROC) load(userptr, "glUniform2d");
    glad_glUniform2dv = (PFNGLUNIFORM2DVPROC) load(userptr, "glUniform2dv");
    glad_glUniform3d = (PFNGLUNIFORM3DPROC) load(userptr, "glUniform3d");
    glad_glUniform3dv = (PFNGLUNIFORM3DVPROC) load(userptr, "glUniform3dv");
    glad_glUniform4d = (PFNGLUNIFORM4DPROC) load(userptr, "glUniform4d");
    glad_glUniform4dv = (PFNGLUNIFORM4DVPROC) load(userptr, "glUniform4dv");
    glad_glUniformMatrix2dv = (PFNGLUNIFORMMATRIX2DVPROC) load(userptr, "glUniformMatrix2dv");
    glad_glUniformMatrix2x3dv = (PFNGLUNIFORMMATRIX2X3DVPROC) load(userptr, "glUniformMatrix2x3dv");
    glad_glUniformMatrix2x4dv = (PFNGLUNIFORMMATRIX2X4DVPROC) load(userptr, "glUniformMatrix2x4dv");
    glad_glUniformMatrix3dv = (PFNGLUNIFORMMATRIX3DVPROC) load(userptr, "glUniformMatrix3dv");
    glad_glUniformMatrix3x2dv = (PFNGLUNIFORMMATRIX3X2DVPROC) load(userptr, "glUniformMatrix3x2dv");
    glad_glUniformMatrix3x4dv = (PFNGLUNIFORMMATRIX3X4DVPROC) load(userptr, "glUniformMatrix3x4dv");
    glad_glUniformMatrix4dv = (PFNGLUNIFORMMATRIX4DVPROC) load(userptr, "glUniformMatrix4dv");
    glad_glUniformMatrix4x2dv = (PFNGLUNIFORMMATRIX4X2DVPROC) load(userptr, "glUniformMatrix4x2dv");
    glad_glUniformMatrix4x3dv = (PFNGLUNIFORMMATRIX4X3DVPROC) load(userptr, "glUniformMatrix4x3dv");
    glad_glUniformSubroutinesuiv = (PFNGLUNIFORMSUBROUTINESUIVPROC) load(userptr, "glUniformSubroutinesuiv");
}
static void glad_gl_load_GL_VERSION_4_1( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_VERSION_4_1) return;
    glad_glActiveShaderProgram = (PFNGLACTIVESHADERPROGRAMPROC) load(userptr, "glActiveShaderProgram");
    glad_glBindProgramPipeline = (PFNGLBINDPROGRAMPIPELINEPROC) load(userptr, "glBindProgramPipeline");
    glad_glClearDepthf = (PFNGLCLEARDEPTHFPROC) load(userptr, "glClearDepthf");
    glad_glCreateShaderProgramv = (PFNGLCREATESHADERPROGRAMVPROC) load(userptr, "glCreateShaderProgramv");
    glad_glDeleteProgramPipelines = (PFNGLDELETEPROGRAMPIPELINESPROC) load(userptr, "glDeleteProgramPipelines");
    glad_glDepthRangeArrayv = (PFNGLDEPTHRANGEARRAYVPROC) load(userptr, "glDepthRangeArrayv");
    glad_glDepthRangeIndexed = (PFNGLDEPTHRANGEINDEXEDPROC) load(userptr, "glDepthRangeIndexed");
    glad_glDepthRangef = (PFNGLDEPTHRANGEFPROC) load(userptr, "glDepthRangef");
    glad_glGenProgramPipelines = (PFNGLGENPROGRAMPIPELINESPROC) load(userptr, "glGenProgramPipelines");
    glad_glGetDoublei_v = (PFNGLGETDOUBLEI_VPROC) load(userptr, "glGetDoublei_v");
    glad_glGetFloati_v = (PFNGLGETFLOATI_VPROC) load(userptr, "glGetFloati_v");
    glad_glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC) load(userptr, "glGetProgramBinary");
    glad_glGetProgramPipelineInfoLog = (PFNGLGETPROGRAMPIPELINEINFOLOGPROC) load(userptr, "glGetProgramPipelineInfoLog");
    glad_glGetProgramPipelineiv = (PFNGLGETPROGRAMPIPELINEIVPROC) load(userptr, "glGetProgramPipelineiv");
    glad_glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC) load(userptr, "glGetShaderPrecisionFormat");
    glad_glGetVertexAttribLdv = (PFNGLGETVERTEXATTRIBLDVPROC) load(userptr, "glGetVertexAttribLdv");
    glad_glIsProgramPipeline = (PFNGLISPROGRAMPIPELINEPROC) load(userptr, "glIsProgramPipeline");
    glad_glProgramBinary = (PFNGLPROGRAMBINARYPROC) load(userptr, "glProgramBinary");
    glad_glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC) load(userptr, "glProgramParameteri");
    glad_glProgramUniform1d = (PFNGLPROGRAMUNIFORM1DPROC) load(userptr, "glProgramUniform1d");
    glad_glProgramUniform1dv = (PFNGLPROGRAMUNIFORM1DVPROC) load(userptr, "glProgramUniform1dv");
    glad_glProgramUniform1f = (PFNGLPROGRAMUNIFORM1FPROC) load(userptr, "glProgramUniform1f");
    glad_glProgramUniform1fv = (PFNGLPROGRAMUNIFORM1FVPROC) load(userptr, "glProgramUniform1fv");
    glad_glProgramUniform1i = (PFNGLPROGRAMUNIFORM1IPROC) load(userptr, "glProgramUniform1i");
    glad_glProgramUniform1iv = (PFNGLPROGRAMUNIFORM1IVPROC) load(userptr, "glProgramUniform1iv");
    glad_glProgramUniform1ui = (PFNGLPROGRAMUNIFORM1UIPROC) load(userptr, "glProgramUniform1ui");
    glad_glProgramUniform1uiv = (PFNGLPROGRAMUNIFORM1UIVPROC) load(userptr, "glProgramUniform1uiv");
    glad_glProgramUniform2d = (PFNGLPROGRAMUNIFORM2DPROC) load(userptr, "glProgramUniform2d");
    glad_glProgramUniform2dv = (PFNGLPROGRAMUNIFORM2DVPROC) load(userptr, "glProgramUniform2dv");
    glad_glProgramUniform2f = (PFNGLPROGRAMUNIFORM2FPROC) load(userptr, "glProgramUniform2f");
    glad_glProgramUniform2fv = (PFNGLPROGRAMUNIFORM2FVPROC) load(userptr, "glProgramUniform2fv");
    glad_glProgramUniform2i = (PFNGLPROGRAMUNIFORM2IPROC) load(userptr, "glProgramUniform2i");
    glad_glProgramUniform2iv = (PFNGLPROGRAMUNIFORM2IVPROC) load(userptr, "glProgramUniform2iv");
    glad_glProgramUniform2ui = (PFNGLPROGRAMUNIFORM2UIPROC) load(userptr, "glProgramUniform2ui");
    glad_glProgramUniform2uiv = (PFNGLPROGRAMUNIFORM2UIVPROC) load(userptr, "glProgramUniform2uiv");
    glad_glProgramUniform3d = (PFNGLPROGRAMUNIFORM3DPROC) load(userptr, "glProgramUniform3d");
    glad_glProgramUniform3dv = (PFNGLPROGRAMUNIFORM3DVPROC) load(userptr, "glProgramUniform3dv");
    glad_glProgramUniform3f = (PFNGLPROGRAMUNIFORM3FPROC) load(userptr, "glProgramUniform3f");
    glad_glProgramUniform3fv = (PFNGLPROGRAMUNIFORM3FVPROC) load(userptr, "glProgramUniform3fv");
    glad_glProgramUniform3i = (PFNGLPROGRAMUNIFORM3IPROC) load(userptr, "glProgramUniform3i");
    glad_glProgramUniform3iv = (PFNGLPROGRAMUNIFORM3IVPROC) load(userptr, "glProgramUniform3iv");
    glad_glProgramUniform3ui = (PFNGLPROGRAMUNIFORM3UIPROC) load(userptr, "glProgramUniform3ui");
    glad_glProgramUniform3uiv = (PFNGLPROGRAMUNIFORM3UIVPROC) load(userptr, "glProgramUniform3uiv");
    glad_glProgramUniform4d = (PFNGLPROGRAMUNIFORM4DPROC) load(userptr, "glProgramUniform4d");
    glad_glProgramUniform4dv = (PFNGLPROGRAMUNIFORM4DVPROC) load(userptr, "glProgramUniform4dv");
    glad_glProgramUniform4f = (PFNGLPROGRAMUNIFORM4FPROC) load(userptr, "glProgramUniform4f");
    glad_glProgramUniform4fv = (PFNGLPROGRAMUNIFORM4FVPROC) load(userptr, "glProgramUniform4fv");
    glad_glProgramUniform4i = (PFNGLPROGRAMUNIFORM4IPROC) load(userptr, "glProgramUniform4i");
    glad_glProgramUniform4iv = (PFNGLPROGRAMUNIFORM4IVPROC) load(userptr, "glProgramUniform4iv");
    glad_glProgramUniform4ui = (PFNGLPROGRAMUNIFORM4UIPROC) load(userptr, "glProgramUniform4ui");
    glad_glProgramUniform4uiv = (PFNGLPROGRAMUNIFORM4UIVPROC) load(userptr, "glProgramUniform4uiv");
    glad_glProgramUniformMatrix2dv = (PFNGLPROGRAMUNIFORMMATRIX2DVPROC) load(userptr, "glProgramUniformMatrix2dv");
    glad_glProgramUniformMatrix2fv = (PFNGLPROGRAMUNIFORMMATRIX2FVPROC) load(userptr, "glProgramUniformMatrix2fv");
    glad_glProgramUniformMatrix2x3dv = (PFNGLPROGRAMUNIFORMMATRIX2X3DVPROC) load(userptr, "glProgramUniformMatrix2x3dv");
    glad_glProgramUniformMatrix2x3fv = (PFNGLPROGRAMUNIFORMMATRIX2X3FVPROC) load(userptr, "glProgramUniformMatrix2x3fv");
    glad_glProgramUniformMatrix2x4dv = (PFNGLPROGRAMUNIFORMMATRIX2X4DVPROC) load(userptr, "glProgramUniformMatrix2x4dv");
    glad_glProgramUniformMatrix2x4fv = (PFNGLPROGRAMUNIFORMMATRIX2X4FVPROC) load(userptr, "glProgramUniformMatrix2x4fv");
    glad_glProgramUniformMatrix3dv = (PFNGLPROGRAMUNIFORMMATRIX3DVPROC) load(userptr, "glProgramUniformMatrix3dv");
    glad_glProgramUniformMatrix3fv = (PFNGLPROGRAMUNIFORMMATRIX3FVPROC) load(userptr, "glProgramUniformMatrix3fv");
    glad_glProgramUniformMatrix3x2dv = (PFNGLPROGRAMUNIFORMMATRIX3X2DVPROC) load(userptr, "glProgramUniformMatrix3x2dv");
    glad_glProgramUniformMatrix3x2fv = (PFNGLPROGRAMUNIFORMMATRIX3X2FVPROC) load(userptr, "glProgramUniformMatrix3x2fv");
    glad_glProgramUniformMatrix3x4dv = (PFNGLPROGRAMUNIFORMMATRIX3X4DVPROC) load(userptr, "glProgramUniformMatrix3x4dv");
    glad_glProgramUniformMatrix3x4fv = (PFNGLPROGRAMUNIFORMMATRIX3X4FVPROC) load(userptr, "glProgramUniformMatrix3x4fv");
    glad_glProgramUniformMatrix4dv = (PFNGLPROGRAMUNIFORMMATRIX4DVPROC) load(userptr, "glProgramUniformMatrix4dv");
    glad_glProgramUniformMatrix4fv = (PFNGLPROGRAMUNIFORMMATRIX4FVPROC) load(userptr, "glProgramUniformMatrix4fv");
    glad_glProgramUniformMatrix4x2dv = (PFNGLPROGRAMUNIFORMMATRIX4X2DVPROC) load(userptr, "glProgramUniformMatrix4x2dv");
    glad_glProgramUniformMatrix4x2fv = (PFNGLPROGRAMUNIFORMMATRIX4X2FVPROC) load(userptr, "glProgramUniformMatrix4x2fv");
    glad_glProgramUniformMatrix4x3dv = (PFNGLPROGRAMUNIFORMMATRIX4X3DVPROC) load(userptr, "glProgramUniformMatrix4x3dv");
    glad_glProgramUniformMatrix4x3fv = (PFNGLPROGRAMUNIFORMMATRIX4X3FVPROC) load(userptr, "glProgramUniformMatrix4x3fv");
    glad_glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC) load(userptr, "glReleaseShaderCompiler");
    glad_glScissorArrayv = (PFNGLSCISSORARRAYVPROC) load(userptr, "glScissorArrayv");
    glad_glScissorIndexed = (PFNGLSCISSORINDEXEDPROC) load(userptr, "glScissorIndexed");
    glad_glScissorIndexedv = (PFNGLSCISSORINDEXEDVPROC) load(userptr, "glScissorIndexedv");
    glad_glShaderBinary = (PFNGLSHADERBINARYPROC) load(userptr, "glShaderBinary");
    glad_glUseProgramStages = (PFNGLUSEPROGRAMSTAGESPROC) load(userptr, "glUseProgramStages");
    glad_glValidateProgramPipeline = (PFNGLVALIDATEPROGRAMPIPELINEPROC) load(userptr, "glValidateProgramPipeline");
    glad_glVertexAttribL1d = (PFNGLVERTEXATTRIBL1DPROC) load(userptr, "glVertexAttribL1d");
    glad_glVertexAttribL1dv = (PFNGLVERTEXATTRIBL1DVPROC) load(userptr, "glVertexAttribL1dv");
    glad_glVertexAttribL2d = (PFNGLVERTEXATTRIBL2DPROC) load(userptr, "glVertexAttribL2d");
    glad_glVertexAttribL2dv = (PFNGLVERTEXATTRIBL2DVPROC) load(userptr, "glVertexAttribL2dv");
    glad_glVertexAttribL3d = (PFNGLVERTEXATTRIBL3DPROC) load(userptr, "glVertexAttribL3d");
    glad_glVertexAttribL3dv = (PFNGLVERTEXATTRIBL3DVPROC) load(userptr, "glVertexAttribL3dv");
    glad_glVertexAttribL4d = (PFNGLVERTEXATTRIBL4DPROC) load(userptr, "glVertexAttribL4d");
    glad_glVertexAttribL4dv = (PFNGLVERTEXATTRIBL4DVPROC) load(userptr, "glVertexAttribL4dv");
    glad_glVertexAttribLPointer = (PFNGLVERTEXATTRIBLPOINTERPROC) load(userptr, "glVertexAttribLPointer");
    glad_glViewportArrayv = (PFNGLVIEWPORTARRAYVPROC) load(userptr, "glViewportArrayv");
    glad_glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC) load(userptr, "glViewportIndexedf");
    glad_glViewportIndexedfv = (PFNGLVIEWPORTINDEXEDFVPROC) load(userptr, "glViewportIndexedfv");
}
static void glad_gl_load_GL_KHR_debug( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_debug) return;
    glad_glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC) load(userptr, "glDebugMessageCallback");
    glad_glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC) load(userptr, "glDebugMessageControl");
    glad_glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC) load(userptr, "glDebugMessageInsert");
    glad_glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC) load(userptr, "glGetDebugMessageLog");
    glad_glGetObjectLabel = (PFNGLGETOBJECTLABELPROC) load(userptr, "glGetObjectLabel");
    glad_glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC) load(userptr, "glGetObjectPtrLabel");
    glad_glGetPointerv = (PFNGLGETPOINTERVPROC) load(userptr, "glGetPointerv");
    glad_glObjectLabel = (PFNGLOBJECTLABELPROC) load(userptr, "glObjectLabel");
    glad_glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC) load(userptr, "glObjectPtrLabel");
    glad_glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC) load(userptr, "glPopDebugGroup");
    glad_glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC) load(userptr, "glPushDebugGroup");
}



#if defined(GL_ES_VERSION_3_0) || defined(GL_VERSION_3_0)
#define GLAD_GL_IS_SOME_NEW_VERSION 1
#else
#define GLAD_GL_IS_SOME_NEW_VERSION 0
#endif

static int glad_gl_get_extensions( int version, const char **out_exts, unsigned int *out_num_exts_i, char ***out_exts_i) {
#if GLAD_GL_IS_SOME_NEW_VERSION
    if(GLAD_VERSION_MAJOR(version) < 3) {
#else
    GLAD_UNUSED(version);
    GLAD_UNUSED(out_num_exts_i);
    GLAD_UNUSED(out_exts_i);
#endif
        if (glad_glGetString == NULL) {
            return 0;
        }
        *out_exts = (const char *)glad_glGetString(GL_EXTENSIONS);
#if GLAD_GL_IS_SOME_NEW_VERSION
    } else {
        unsigned int index = 0;
        unsigned int num_exts_i = 0;
        char **exts_i = NULL;
        if (glad_glGetStringi == NULL || glad_glGetIntegerv == NULL) {
            return 0;
        }
        glad_glGetIntegerv(GL_NUM_EXTENSIONS, (int*) &num_exts_i);
        if (num_exts_i > 0) {
            exts_i = (char **) malloc(num_exts_i * (sizeof *exts_i));
        }
        if (exts_i == NULL) {
            return 0;
        }
        for(index = 0; index < num_exts_i; index++) {
            const char *gl_str_tmp = (const char*) glad_glGetStringi(GL_EXTENSIONS, index);
            size_t len = strlen(gl_str_tmp) + 1;

            char *local_str = (char*) malloc(len * sizeof(char));
            if(local_str != NULL) {
                memcpy(local_str, gl_str_tmp, len * sizeof(char));
            }

            exts_i[index] = local_str;
        }

        *out_num_exts_i = num_exts_i;
        *out_exts_i = exts_i;
    }
#endif
    return 1;
}
static void glad_gl_free_extensions(char **exts_i, unsigned int num_exts_i) {
    if (exts_i != NULL) {
        unsigned int index;
        for(index = 0; index < num_exts_i; index++) {
            free((void *) (exts_i[index]));
        }
        free((void *)exts_i);
        exts_i = NULL;
    }
}
static int glad_gl_has_extension(int version, const char *exts, unsigned int num_exts_i, char **exts_i, const char *ext) {
    if(GLAD_VERSION_MAJOR(version) < 3 || !GLAD_GL_IS_SOME_NEW_VERSION) {
        const char *extensions;
        const char *loc;
        const char *terminator;
        extensions = exts;
        if(extensions == NULL || ext == NULL) {
            return 0;
        }
        while(1) {
            loc = strstr(extensions, ext);
            if(loc == NULL) {
                return 0;
            }
            terminator = loc + strlen(ext);
            if((loc == extensions || *(loc - 1) == ' ') &&
                (*terminator == ' ' || *terminator == '\0')) {
                return 1;
            }
            extensions = terminator;
        }
    } else {
        unsigned int index;
        for(index = 0; index < num_exts_i; index++) {
            const char *e = exts_i[index];
            if(strcmp(e, ext) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

static GLADapiproc glad_gl_get_proc_from_userptr(void *userptr, const char* name) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_gl_find_extensions_gl( int version) {
    const char *exts = NULL;
    unsigned int num_exts_i = 0;
    char **exts_i = NULL;
    if (!glad_gl_get_extensions(version, &exts, &num_exts_i, &exts_i)) return 0;

    GLAD_GL_KHR_debug = glad_gl_has_extension(version, exts, num_exts_i, exts_i, "GL_KHR_debug");

    glad_gl_free_extensions(exts_i, num_exts_i);

    return 1;
}

static int glad_gl_find_core_gl(void) {
    int i;
    const char* version;
    const char* prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        "OpenGL SC ",
        NULL
    };
    int major = 0;
    int minor = 0;
    version = (const char*) glad_glGetString(GL_VERSION);
    if (!version) return 0;
    for (i = 0;  prefixes[i];  i++) {
        const size_t length = strlen(prefixes[i]);
        if (strncmp(version, prefixes[i], length) == 0) {
            version += length;
            break;
        }
    }

    GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);

    GLAD_GL_VERSION_1_0 = (major == 1 && minor >= 0) || major > 1;
    GLAD_GL_VERSION_1_1 = (major == 1 && minor >= 1) || major > 1;
    GLAD_GL_VERSION_1_2 = (major == 1 && minor >= 2) || major > 1;
    GLAD_GL_VERSION_1_3 = (major == 1 && minor >= 3) || major > 1;
    GLAD_GL_VERSION_1_4 = (major == 1 && minor >= 4) || major > 1;
    GLAD_GL_VERSION_1_5 = (major == 1 && minor >= 5) || major > 1;
    GLAD_GL_VERSION_2_0 = (major == 2 && minor >= 0) || major > 2;
    GLAD_GL_VERSION_2_1 = (major == 2 && minor >= 1) || major > 2;
    GLAD_GL_VERSION_3_0 = (major == 3 && minor >= 0) || major > 3;
    GLAD_GL_VERSION_3_1 = (major == 3 && minor >= 1) || major > 3;
    GLAD_GL_VERSION_3_2 = (major == 3 && minor >= 2) || major > 3;
    GLAD_GL_VERSION_3_3 = (major == 3 && minor >= 3) || major > 3;
    GLAD_GL_VERSION_4_0 = (major == 4 && minor >= 0) || major > 4;
    GLAD_GL_VERSION_4_1 = (major == 4 && minor >= 1) || major > 4;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadGLUserPtr( GLADuserptrloadfunc load, void *userptr) {
    int version;

    glad_glGetString = (PFNGLGETSTRINGPROC) load(userptr, "glGetString");
    if(glad_glGetString == NULL) return 0;
    if(glad_glGetString(GL_VERSION) == NULL) return 0;
    version = glad_gl_find_core_gl();

    glad_gl_load_GL_VERSION_1_0(load, userptr);
    glad_gl_load_GL_VERSION_1_1(load, userptr);
    glad_gl_load_GL_VERSION_1_2(load, userptr);
    glad_gl_load_GL_VERSION_1_3(load, userptr);
    glad_gl_load_GL_VERSION_1_4(load, userptr);
    glad_gl_load_GL_VERSION_1_5(load, userptr);
    glad_gl_load_GL_VERSION_2_0(load, userptr);
    glad_gl_load_GL_VERSION_2_1(load, userptr);
    glad_gl_load_GL_VERSION_3_0(load, userptr);
    glad_gl_load_GL_VERSION_3_1(load, userptr);
    glad_gl_load_GL_VERSION_3_2(load, userptr);
    glad_gl_load_GL_VERSION_3_3(load, userptr);
    glad_gl_load_GL_VERSION_4_0(load, userptr);
    glad_gl_load_GL_VERSION_4_1(load, userptr);

    if (!glad_gl_find_extensions_gl(version)) return 0;
    glad_gl_load_GL_KHR_debug(load, userptr);



    return version;
}


int gladLoadGL( GLADloadfunc load) {
    return gladLoadGLUserPtr( glad_gl_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}



 
void gladInstallGLDebug() {
    glad_debug_glActiveShaderProgram = glad_debug_impl_glActiveShaderProgram;
    glad_debug_glActiveTexture = glad_debug_impl_glActiveTexture;
    glad_debug_glAttachShader = glad_debug_impl_glAttachShader;
    glad_debug_glBeginConditionalRender = glad_debug_impl_glBeginConditionalRender;
    glad_debug_glBeginQuery = glad_debug_impl_glBeginQuery;
    glad_debug_glBeginQueryIndexed = glad_debug_impl_glBeginQueryIndexed;
    glad_debug_glBeginTransformFeedback = glad_debug_impl_glBeginTransformFeedback;
    glad_debug_glBindAttribLocation = glad_debug_impl_glBindAttribLocation;
    glad_debug_glBindBuffer = glad_debug_impl_glBindBuffer;
    glad_debug_glBindBufferBase = glad_debug_impl_glBindBufferBase;
    glad_debug_glBindBufferRange = glad_debug_impl_glBindBufferRange;
    glad_debug_glBindFragDataLocation = glad_debug_impl_glBindFragDataLocation;
    glad_debug_glBindFragDataLocationIndexed = glad_debug_impl_glBindFragDataLocationIndexed;
    glad_debug_glBindFramebuffer = glad_debug_impl_glBindFramebuffer;
    glad_debug_glBindProgramPipeline = glad_debug_impl_glBindProgramPipeline;
    glad_debug_glBindRenderbuffer = glad_debug_impl_glBindRenderbuffer;
    glad_debug_glBindSampler = glad_debug_impl_glBindSampler;
    glad_debug_glBindTexture = glad_debug_impl_glBindTexture;
    glad_debug_glBindTransformFeedback = glad_debug_impl_glBindTransformFeedback;
    glad_debug_glBindVertexArray = glad_debug_impl_glBindVertexArray;
    glad_debug_glBlendColor = glad_debug_impl_glBlendColor;
    glad_debug_glBlendEquation = glad_debug_impl_glBlendEquation;
    glad_debug_glBlendEquationSeparate = glad_debug_impl_glBlendEquationSeparate;
    glad_debug_glBlendEquationSeparatei = glad_debug_impl_glBlendEquationSeparatei;
    glad_debug_glBlendEquationi = glad_debug_impl_glBlendEquationi;
    glad_debug_glBlendFunc = glad_debug_impl_glBlendFunc;
    glad_debug_glBlendFuncSeparate = glad_debug_impl_glBlendFuncSeparate;
    glad_debug_glBlendFuncSeparatei = glad_debug_impl_glBlendFuncSeparatei;
    glad_debug_glBlendFunci = glad_debug_impl_glBlendFunci;
    glad_debug_glBlitFramebuffer = glad_debug_impl_glBlitFramebuffer;
    glad_debug_glBufferData = glad_debug_impl_glBufferData;
    glad_debug_glBufferSubData = glad_debug_impl_glBufferSubData;
    glad_debug_glCheckFramebufferStatus = glad_debug_impl_glCheckFramebufferStatus;
    glad_debug_glClampColor = glad_debug_impl_glClampColor;
    glad_debug_glClear = glad_debug_impl_glClear;
    glad_debug_glClearBufferfi = glad_debug_impl_glClearBufferfi;
    glad_debug_glClearBufferfv = glad_debug_impl_glClearBufferfv;
    glad_debug_glClearBufferiv = glad_debug_impl_glClearBufferiv;
    glad_debug_glClearBufferuiv = glad_debug_impl_glClearBufferuiv;
    glad_debug_glClearColor = glad_debug_impl_glClearColor;
    glad_debug_glClearDepth = glad_debug_impl_glClearDepth;
    glad_debug_glClearDepthf = glad_debug_impl_glClearDepthf;
    glad_debug_glClearStencil = glad_debug_impl_glClearStencil;
    glad_debug_glClientWaitSync = glad_debug_impl_glClientWaitSync;
    glad_debug_glColorMask = glad_debug_impl_glColorMask;
    glad_debug_glColorMaski = glad_debug_impl_glColorMaski;
    glad_debug_glCompileShader = glad_debug_impl_glCompileShader;
    glad_debug_glCompressedTexImage1D = glad_debug_impl_glCompressedTexImage1D;
    glad_debug_glCompressedTexImage2D = glad_debug_impl_glCompressedTexImage2D;
    glad_debug_glCompressedTexImage3D = glad_debug_impl_glCompressedTexImage3D;
    glad_debug_glCompressedTexSubImage1D = glad_debug_impl_glCompressedTexSubImage1D;
    glad_debug_glCompressedTexSubImage2D = glad_debug_impl_glCompressedTexSubImage2D;
    glad_debug_glCompressedTexSubImage3D = glad_debug_impl_glCompressedTexSubImage3D;
    glad_debug_glCopyBufferSubData = glad_debug_impl_glCopyBufferSubData;
    glad_debug_glCopyTexImage1D = glad_debug_impl_glCopyTexImage1D;
    glad_debug_glCopyTexImage2D = glad_debug_impl_glCopyTexImage2D;
    glad_debug_glCopyTexSubImage1D = glad_debug_impl_glCopyTexSubImage1D;
    glad_debug_glCopyTexSubImage2D = glad_debug_impl_glCopyTexSubImage2D;
    glad_debug_glCopyTexSubImage3D = glad_debug_impl_glCopyTexSubImage3D;
    glad_debug_glCreateProgram = glad_debug_impl_glCreateProgram;
    glad_debug_glCreateShader = glad_debug_impl_glCreateShader;
    glad_debug_glCreateShaderProgramv = glad_debug_impl_glCreateShaderProgramv;
    glad_debug_glCullFace = glad_debug_impl_glCullFace;
    glad_debug_glDebugMessageCallback = glad_debug_impl_glDebugMessageCallback;
    glad_debug_glDebugMessageControl = glad_debug_impl_glDebugMessageControl;
    glad_debug_glDebugMessageInsert = glad_debug_impl_glDebugMessageInsert;
    glad_debug_glDeleteBuffers = glad_debug_impl_glDeleteBuffers;
    glad_debug_glDeleteFramebuffers = glad_debug_impl_glDeleteFramebuffers;
    glad_debug_glDeleteProgram = glad_debug_impl_glDeleteProgram;
    glad_debug_glDeleteProgramPipelines = glad_debug_impl_glDeleteProgramPipelines;
    glad_debug_glDeleteQueries = glad_debug_impl_glDeleteQueries;
    glad_debug_glDeleteRenderbuffers = glad_debug_impl_glDeleteRenderbuffers;
    glad_debug_glDeleteSamplers = glad_debug_impl_glDeleteSamplers;
    glad_debug_glDeleteShader = glad_debug_impl_glDeleteShader;
    glad_debug_glDeleteSync = glad_debug_impl_glDeleteSync;
    glad_debug_glDeleteTextures = glad_debug_impl_glDeleteTextures;
    glad_debug_glDeleteTransformFeedbacks = glad_debug_impl_glDeleteTransformFeedbacks;
    glad_debug_glDeleteVertexArrays = glad_debug_impl_glDeleteVertexArrays;
    glad_debug_glDepthFunc = glad_debug_impl_glDepthFunc;
    glad_debug_glDepthMask = glad_debug_impl_glDepthMask;
    glad_debug_glDepthRange = glad_debug_impl_glDepthRange;
    glad_debug_glDepthRangeArrayv = glad_debug_impl_glDepthRangeArrayv;
    glad_debug_glDepthRangeIndexed = glad_debug_impl_glDepthRangeIndexed;
    glad_debug_glDepthRangef = glad_debug_impl_glDepthRangef;
    glad_debug_glDetachShader = glad_debug_impl_glDetachShader;
    glad_debug_glDisable = glad_debug_impl_glDisable;
    glad_debug_glDisableVertexAttribArray = glad_debug_impl_glDisableVertexAttribArray;
    glad_debug_glDisablei = glad_debug_impl_glDisablei;
    glad_debug_glDrawArrays = glad_debug_impl_glDrawArrays;
    glad_debug_glDrawArraysIndirect = glad_debug_impl_glDrawArraysIndirect;
    glad_debug_glDrawArraysInstanced = glad_debug_impl_glDrawArraysInstanced;
    glad_debug_glDrawBuffer = glad_debug_impl_glDrawBuffer;
    glad_debug_glDrawBuffers = glad_debug_impl_glDrawBuffers;
    glad_debug_glDrawElements = glad_debug_impl_glDrawElements;
    glad_debug_glDrawElementsBaseVertex = glad_debug_impl_glDrawElementsBaseVertex;
    glad_debug_glDrawElementsIndirect = glad_debug_impl_glDrawElementsIndirect;
    glad_debug_glDrawElementsInstanced = glad_debug_impl_glDrawElementsInstanced;
    glad_debug_glDrawElementsInstancedBaseVertex = glad_debug_impl_glDrawElementsInstancedBaseVertex;
    glad_debug_glDrawRangeElements = glad_debug_impl_glDrawRangeElements;
    glad_debug_glDrawRangeElementsBaseVertex = glad_debug_impl_glDrawRangeElementsBaseVertex;
    glad_debug_glDrawTransformFeedback = glad_debug_impl_glDrawTransformFeedback;
    glad_debug_glDrawTransformFeedbackStream = glad_debug_impl_glDrawTransformFeedbackStream;
    glad_debug_glEnable = glad_debug_impl_glEnable;
    glad_debug_glEnableVertexAttribArray = glad_debug_impl_glEnableVertexAttribArray;
    glad_debug_glEnablei = glad_debug_impl_glEnablei;
    glad_debug_glEndConditionalRender = glad_debug_impl_glEndConditionalRender;
    glad_debug_glEndQuery = glad_debug_impl_glEndQuery;
    glad_debug_glEndQueryIndexed = glad_debug_impl_glEndQueryIndexed;
    glad_debug_glEndTransformFeedback = glad_debug_impl_glEndTransformFeedback;
    glad_debug_glFenceSync = glad_debug_impl_glFenceSync;
    glad_debug_glFinish = glad_debug_impl_glFinish;
    glad_debug_glFlush = glad_debug_impl_glFlush;
    glad_debug_glFlushMappedBufferRange = glad_debug_impl_glFlushMappedBufferRange;
    glad_debug_glFramebufferRenderbuffer = glad_debug_impl_glFramebufferRenderbuffer;
    glad_debug_glFramebufferTexture = glad_debug_impl_glFramebufferTexture;
    glad_debug_glFramebufferTexture1D = glad_debug_impl_glFramebufferTexture1D;
    glad_debug_glFramebufferTexture2D = glad_debug_impl_glFramebufferTexture2D;
    glad_debug_glFramebufferTexture3D = glad_debug_impl_glFramebufferTexture3D;
    glad_debug_glFramebufferTextureLayer = glad_debug_impl_glFramebufferTextureLayer;
    glad_debug_glFrontFace = glad_debug_impl_glFrontFace;
    glad_debug_glGenBuffers = glad_debug_impl_glGenBuffers;
    glad_debug_glGenFramebuffers = glad_debug_impl_glGenFramebuffers;
    glad_debug_glGenProgramPipelines = glad_debug_impl_glGenProgramPipelines;
    glad_debug_glGenQueries = glad_debug_impl_glGenQueries;
    glad_debug_glGenRenderbuffers = glad_debug_impl_glGenRenderbuffers;
    glad_debug_glGenSamplers = glad_debug_impl_glGenSamplers;
    glad_debug_glGenTextures = glad_debug_impl_glGenTextures;
    glad_debug_glGenTransformFeedbacks = glad_debug_impl_glGenTransformFeedbacks;
    glad_debug_glGenVertexArrays = glad_debug_impl_glGenVertexArrays;
    glad_debug_glGenerateMipmap = glad_debug_impl_glGenerateMipmap;
    glad_debug_glGetActiveAttrib = glad_debug_impl_glGetActiveAttrib;
    glad_debug_glGetActiveSubroutineName = glad_debug_impl_glGetActiveSubroutineName;
    glad_debug_glGetActiveSubroutineUniformName = glad_debug_impl_glGetActiveSubroutineUniformName;
    glad_debug_glGetActiveSubroutineUniformiv = glad_debug_impl_glGetActiveSubroutineUniformiv;
    glad_debug_glGetActiveUniform = glad_debug_impl_glGetActiveUniform;
    glad_debug_glGetActiveUniformBlockName = glad_debug_impl_glGetActiveUniformBlockName;
    glad_debug_glGetActiveUniformBlockiv = glad_debug_impl_glGetActiveUniformBlockiv;
    glad_debug_glGetActiveUniformName = glad_debug_impl_glGetActiveUniformName;
    glad_debug_glGetActiveUniformsiv = glad_debug_impl_glGetActiveUniformsiv;
    glad_debug_glGetAttachedShaders = glad_debug_impl_glGetAttachedShaders;
    glad_debug_glGetAttribLocation = glad_debug_impl_glGetAttribLocation;
    glad_debug_glGetBooleani_v = glad_debug_impl_glGetBooleani_v;
    glad_debug_glGetBooleanv = glad_debug_impl_glGetBooleanv;
    glad_debug_glGetBufferParameteri64v = glad_debug_impl_glGetBufferParameteri64v;
    glad_debug_glGetBufferParameteriv = glad_debug_impl_glGetBufferParameteriv;
    glad_debug_glGetBufferPointerv = glad_debug_impl_glGetBufferPointerv;
    glad_debug_glGetBufferSubData = glad_debug_impl_glGetBufferSubData;
    glad_debug_glGetCompressedTexImage = glad_debug_impl_glGetCompressedTexImage;
    glad_debug_glGetDebugMessageLog = glad_debug_impl_glGetDebugMessageLog;
    glad_debug_glGetDoublei_v = glad_debug_impl_glGetDoublei_v;
    glad_debug_glGetDoublev = glad_debug_impl_glGetDoublev;
    glad_debug_glGetError = glad_debug_impl_glGetError;
    glad_debug_glGetFloati_v = glad_debug_impl_glGetFloati_v;
    glad_debug_glGetFloatv = glad_debug_impl_glGetFloatv;
    glad_debug_glGetFragDataIndex = glad_debug_impl_glGetFragDataIndex;
    glad_debug_glGetFragDataLocation = glad_debug_impl_glGetFragDataLocation;
    glad_debug_glGetFramebufferAttachmentParameteriv = glad_debug_impl_glGetFramebufferAttachmentParameteriv;
    glad_debug_glGetInteger64i_v = glad_debug_impl_glGetInteger64i_v;
    glad_debug_glGetInteger64v = glad_debug_impl_glGetInteger64v;
    glad_debug_glGetIntegeri_v = glad_debug_impl_glGetIntegeri_v;
    glad_debug_glGetIntegerv = glad_debug_impl_glGetIntegerv;
    glad_debug_glGetMultisamplefv = glad_debug_impl_glGetMultisamplefv;
    glad_debug_glGetObjectLabel = glad_debug_impl_glGetObjectLabel;
    glad_debug_glGetObjectPtrLabel = glad_debug_impl_glGetObjectPtrLabel;
    glad_debug_glGetPointerv = glad_debug_impl_glGetPointerv;
    glad_debug_glGetProgramBinary = glad_debug_impl_glGetProgramBinary;
    glad_debug_glGetProgramInfoLog = glad_debug_impl_glGetProgramInfoLog;
    glad_debug_glGetProgramPipelineInfoLog = glad_debug_impl_glGetProgramPipelineInfoLog;
    glad_debug_glGetProgramPipelineiv = glad_debug_impl_glGetProgramPipelineiv;
    glad_debug_glGetProgramStageiv = glad_debug_impl_glGetProgramStageiv;
    glad_debug_glGetProgramiv = glad_debug_impl_glGetProgramiv;
    glad_debug_glGetQueryIndexediv = glad_debug_impl_glGetQueryIndexediv;
    glad_debug_glGetQueryObjecti64v = glad_debug_impl_glGetQueryObjecti64v;
    glad_debug_glGetQueryObjectiv = glad_debug_impl_glGetQueryObjectiv;
    glad_debug_glGetQueryObjectui64v = glad_debug_impl_glGetQueryObjectui64v;
    glad_debug_glGetQueryObjectuiv = glad_debug_impl_glGetQueryObjectuiv;
    glad_debug_glGetQueryiv = glad_debug_impl_glGetQueryiv;
    glad_debug_glGetRenderbufferParameteriv = glad_debug_impl_glGetRenderbufferParameteriv;
    glad_debug_glGetSamplerParameterIiv = glad_debug_impl_glGetSamplerParameterIiv;
    glad_debug_glGetSamplerParameterIuiv = glad_debug_impl_glGetSamplerParameterIuiv;
    glad_debug_glGetSamplerParameterfv = glad_debug_impl_glGetSamplerParameterfv;
    glad_debug_glGetSamplerParameteriv = glad_debug_impl_glGetSamplerParameteriv;
    glad_debug_glGetShaderInfoLog = glad_debug_impl_glGetShaderInfoLog;
    glad_debug_glGetShaderPrecisionFormat = glad_debug_impl_glGetShaderPrecisionFormat;
    glad_debug_glGetShaderSource = glad_debug_impl_glGetShaderSource;
    glad_debug_glGetShaderiv = glad_debug_impl_glGetShaderiv;
    glad_debug_glGetString = glad_debug_impl_glGetString;
    glad_debug_glGetStringi = glad_debug_impl_glGetStringi;
    glad_debug_glGetSubroutineIndex = glad_debug_impl_glGetSubroutineIndex;
    glad_debug_glGetSubroutineUniformLocation = glad_debug_impl_glGetSubroutineUniformLocation;
    glad_debug_glGetSynciv = glad_debug_impl_glGetSynciv;
    glad_debug_glGetTexImage = glad_debug_impl_glGetTexImage;
    glad_debug_glGetTexLevelParameterfv = glad_debug_impl_glGetTexLevelParameterfv;
    glad_debug_glGetTexLevelParameteriv = glad_debug_impl_glGetTexLevelParameteriv;
    glad_debug_glGetTexParameterIiv = glad_debug_impl_glGetTexParameterIiv;
    glad_debug_glGetTexParameterIuiv = glad_debug_impl_glGetTexParameterIuiv;
    glad_debug_glGetTexParameterfv = glad_debug_impl_glGetTexParameterfv;
    glad_debug_glGetTexParameteriv = glad_debug_impl_glGetTexParameteriv;
    glad_debug_glGetTransformFeedbackVarying = glad_debug_impl_glGetTransformFeedbackVarying;
    glad_debug_glGetUniformBlockIndex = glad_debug_impl_glGetUniformBlockIndex;
    glad_debug_glGetUniformIndices = glad_debug_impl_glGetUniformIndices;
    glad_debug_glGetUniformLocation = glad_debug_impl_glGetUniformLocation;
    glad_debug_glGetUniformSubroutineuiv = glad_debug_impl_glGetUniformSubroutineuiv;
    glad_debug_glGetUniformdv = glad_debug_impl_glGetUniformdv;
    glad_debug_glGetUniformfv = glad_debug_impl_glGetUniformfv;
    glad_debug_glGetUniformiv = glad_debug_impl_glGetUniformiv;
    glad_debug_glGetUniformuiv = glad_debug_impl_glGetUniformuiv;
    glad_debug_glGetVertexAttribIiv = glad_debug_impl_glGetVertexAttribIiv;
    glad_debug_glGetVertexAttribIuiv = glad_debug_impl_glGetVertexAttribIuiv;
    glad_debug_glGetVertexAttribLdv = glad_debug_impl_glGetVertexAttribLdv;
    glad_debug_glGetVertexAttribPointerv = glad_debug_impl_glGetVertexAttribPointerv;
    glad_debug_glGetVertexAttribdv = glad_debug_impl_glGetVertexAttribdv;
    glad_debug_glGetVertexAttribfv = glad_debug_impl_glGetVertexAttribfv;
    glad_debug_glGetVertexAttribiv = glad_debug_impl_glGetVertexAttribiv;
    glad_debug_glHint = glad_debug_impl_glHint;
    glad_debug_glIsBuffer = glad_debug_impl_glIsBuffer;
    glad_debug_glIsEnabled = glad_debug_impl_glIsEnabled;
    glad_debug_glIsEnabledi = glad_debug_impl_glIsEnabledi;
    glad_debug_glIsFramebuffer = glad_debug_impl_glIsFramebuffer;
    glad_debug_glIsProgram = glad_debug_impl_glIsProgram;
    glad_debug_glIsProgramPipeline = glad_debug_impl_glIsProgramPipeline;
    glad_debug_glIsQuery = glad_debug_impl_glIsQuery;
    glad_debug_glIsRenderbuffer = glad_debug_impl_glIsRenderbuffer;
    glad_debug_glIsSampler = glad_debug_impl_glIsSampler;
    glad_debug_glIsShader = glad_debug_impl_glIsShader;
    glad_debug_glIsSync = glad_debug_impl_glIsSync;
    glad_debug_glIsTexture = glad_debug_impl_glIsTexture;
    glad_debug_glIsTransformFeedback = glad_debug_impl_glIsTransformFeedback;
    glad_debug_glIsVertexArray = glad_debug_impl_glIsVertexArray;
    glad_debug_glLineWidth = glad_debug_impl_glLineWidth;
    glad_debug_glLinkProgram = glad_debug_impl_glLinkProgram;
    glad_debug_glLogicOp = glad_debug_impl_glLogicOp;
    glad_debug_glMapBuffer = glad_debug_impl_glMapBuffer;
    glad_debug_glMapBufferRange = glad_debug_impl_glMapBufferRange;
    glad_debug_glMinSampleShading = glad_debug_impl_glMinSampleShading;
    glad_debug_glMultiDrawArrays = glad_debug_impl_glMultiDrawArrays;
    glad_debug_glMultiDrawElements = glad_debug_impl_glMultiDrawElements;
    glad_debug_glMultiDrawElementsBaseVertex = glad_debug_impl_glMultiDrawElementsBaseVertex;
    glad_debug_glObjectLabel = glad_debug_impl_glObjectLabel;
    glad_debug_glObjectPtrLabel = glad_debug_impl_glObjectPtrLabel;
    glad_debug_glPatchParameterfv = glad_debug_impl_glPatchParameterfv;
    glad_debug_glPatchParameteri = glad_debug_impl_glPatchParameteri;
    glad_debug_glPauseTransformFeedback = glad_debug_impl_glPauseTransformFeedback;
    glad_debug_glPixelStoref = glad_debug_impl_glPixelStoref;
    glad_debug_glPixelStorei = glad_debug_impl_glPixelStorei;
    glad_debug_glPointParameterf = glad_debug_impl_glPointParameterf;
    glad_debug_glPointParameterfv = glad_debug_impl_glPointParameterfv;
    glad_debug_glPointParameteri = glad_debug_impl_glPointParameteri;
    glad_debug_glPointParameteriv = glad_debug_impl_glPointParameteriv;
    glad_debug_glPointSize = glad_debug_impl_glPointSize;
    glad_debug_glPolygonMode = glad_debug_impl_glPolygonMode;
    glad_debug_glPolygonOffset = glad_debug_impl_glPolygonOffset;
    glad_debug_glPopDebugGroup = glad_debug_impl_glPopDebugGroup;
    glad_debug_glPrimitiveRestartIndex = glad_debug_impl_glPrimitiveRestartIndex;
    glad_debug_glProgramBinary = glad_debug_impl_glProgramBinary;
    glad_debug_glProgramParameteri = glad_debug_impl_glProgramParameteri;
    glad_debug_glProgramUniform1d = glad_debug_impl_glProgramUniform1d;
    glad_debug_glProgramUniform1dv = glad_debug_impl_glProgramUniform1dv;
    glad_debug_glProgramUniform1f = glad_debug_impl_glProgramUniform1f;
    glad_debug_glProgramUniform1fv = glad_debug_impl_glProgramUniform1fv;
    glad_debug_glProgramUniform1i = glad_debug_impl_glProgramUniform1i;
    glad_debug_glProgramUniform1iv = glad_debug_impl_glProgramUniform1iv;
    glad_debug_glProgramUniform1ui = glad_debug_impl_glProgramUniform1ui;
    glad_debug_glProgramUniform1uiv = glad_debug_impl_glProgramUniform1uiv;
    glad_debug_glProgramUniform2d = glad_debug_impl_glProgramUniform2d;
    glad_debug_glProgramUniform2dv = glad_debug_impl_glProgramUniform2dv;
    glad_debug_glProgramUniform2f = glad_debug_impl_glProgramUniform2f;
    glad_debug_glProgramUniform2fv = glad_debug_impl_glProgramUniform2fv;
    glad_debug_glProgramUniform2i = glad_debug_impl_glProgramUniform2i;
    glad_debug_glProgramUniform2iv = glad_debug_impl_glProgramUniform2iv;
    glad_debug_glProgramUniform2ui = glad_debug_impl_glProgramUniform2ui;
    glad_debug_glProgramUniform2uiv = glad_debug_impl_glProgramUniform2uiv;
    glad_debug_glProgramUniform3d = glad_debug_impl_glProgramUniform3d;
    glad_debug_glProgramUniform3dv = glad_debug_impl_glProgramUniform3dv;
    glad_debug_glProgramUniform3f = glad_debug_impl_glProgramUniform3f;
    glad_debug_glProgramUniform3fv = glad_debug_impl_glProgramUniform3fv;
    glad_debug_glProgramUniform3i = glad_debug_impl_glProgramUniform3i;
    glad_debug_glProgramUniform3iv = glad_debug_impl_glProgramUniform3iv;
    glad_debug_glProgramUniform3ui = glad_debug_impl_glProgramUniform3ui;
    glad_debug_glProgramUniform3uiv = glad_debug_impl_glProgramUniform3uiv;
    glad_debug_glProgramUniform4d = glad_debug_impl_glProgramUniform4d;
    glad_debug_glProgramUniform4dv = glad_debug_impl_glProgramUniform4dv;
    glad_debug_glProgramUniform4f = glad_debug_impl_glProgramUniform4f;
    glad_debug_glProgramUniform4fv = glad_debug_impl_glProgramUniform4fv;
    glad_debug_glProgramUniform4i = glad_debug_impl_glProgramUniform4i;
    glad_debug_glProgramUniform4iv = glad_debug_impl_glProgramUniform4iv;
    glad_debug_glProgramUniform4ui = glad_debug_impl_glProgramUniform4ui;
    glad_debug_glProgramUniform4uiv = glad_debug_impl_glProgramUniform4uiv;
    glad_debug_glProgramUniformMatrix2dv = glad_debug_impl_glProgramUniformMatrix2dv;
    glad_debug_glProgramUniformMatrix2fv = glad_debug_impl_glProgramUniformMatrix2fv;
    glad_debug_glProgramUniformMatrix2x3dv = glad_debug_impl_glProgramUniformMatrix2x3dv;
    glad_debug_glProgramUniformMatrix2x3fv = glad_debug_impl_glProgramUniformMatrix2x3fv;
    glad_debug_glProgramUniformMatrix2x4dv = glad_debug_impl_glProgramUniformMatrix2x4dv;
    glad_debug_glProgramUniformMatrix2x4fv = glad_debug_impl_glProgramUniformMatrix2x4fv;
    glad_debug_glProgramUniformMatrix3dv = glad_debug_impl_glProgramUniformMatrix3dv;
    glad_debug_glProgramUniformMatrix3fv = glad_debug_impl_glProgramUniformMatrix3fv;
    glad_debug_glProgramUniformMatrix3x2dv = glad_debug_impl_glProgramUniformMatrix3x2dv;
    glad_debug_glProgramUniformMatrix3x2fv = glad_debug_impl_glProgramUniformMatrix3x2fv;
    glad_debug_glProgramUniformMatrix3x4dv = glad_debug_impl_glProgramUniformMatrix3x4dv;
    glad_debug_glProgramUniformMatrix3x4fv = glad_debug_impl_glProgramUniformMatrix3x4fv;
    glad_debug_glProgramUniformMatrix4dv = glad_debug_impl_glProgramUniformMatrix4dv;
    glad_debug_glProgramUniformMatrix4fv = glad_debug_impl_glProgramUniformMatrix4fv;
    glad_debug_glProgramUniformMatrix4x2dv = glad_debug_impl_glProgramUniformMatrix4x2dv;
    glad_debug_glProgramUniformMatrix4x2fv = glad_debug_impl_glProgramUniformMatrix4x2fv;
    glad_debug_glProgramUniformMatrix4x3dv = glad_debug_impl_glProgramUniformMatrix4x3dv;
    glad_debug_glProgramUniformMatrix4x3fv = glad_debug_impl_glProgramUniformMatrix4x3fv;
    glad_debug_glProvokingVertex = glad_debug_impl_glProvokingVertex;
    glad_debug_glPushDebugGroup = glad_debug_impl_glPushDebugGroup;
    glad_debug_glQueryCounter = glad_debug_impl_glQueryCounter;
    glad_debug_glReadBuffer = glad_debug_impl_glReadBuffer;
    glad_debug_glReadPixels = glad_debug_impl_glReadPixels;
    glad_debug_glReleaseShaderCompiler = glad_debug_impl_glReleaseShaderCompiler;
    glad_debug_glRenderbufferStorage = glad_debug_impl_glRenderbufferStorage;
    glad_debug_glRenderbufferStorageMultisample = glad_debug_impl_glRenderbufferStorageMultisample;
    glad_debug_glResumeTransformFeedback = glad_debug_impl_glResumeTransformFeedback;
    glad_debug_glSampleCoverage = glad_debug_impl_glSampleCoverage;
    glad_debug_glSampleMaski = glad_debug_impl_glSampleMaski;
    glad_debug_glSamplerParameterIiv = glad_debug_impl_glSamplerParameterIiv;
    glad_debug_glSamplerParameterIuiv = glad_debug_impl_glSamplerParameterIuiv;
    glad_debug_glSamplerParameterf = glad_debug_impl_glSamplerParameterf;
    glad_debug_glSamplerParameterfv = glad_debug_impl_glSamplerParameterfv;
    glad_debug_glSamplerParameteri = glad_debug_impl_glSamplerParameteri;
    glad_debug_glSamplerParameteriv = glad_debug_impl_glSamplerParameteriv;
    glad_debug_glScissor = glad_debug_impl_glScissor;
    glad_debug_glScissorArrayv = glad_debug_impl_glScissorArrayv;
    glad_debug_glScissorIndexed = glad_debug_impl_glScissorIndexed;
    glad_debug_glScissorIndexedv = glad_debug_impl_glScissorIndexedv;
    glad_debug_glShaderBinary = glad_debug_impl_glShaderBinary;
    glad_debug_glShaderSource = glad_debug_impl_glShaderSource;
    glad_debug_glStencilFunc = glad_debug_impl_glStencilFunc;
    glad_debug_glStencilFuncSeparate = glad_debug_impl_glStencilFuncSeparate;
    glad_debug_glStencilMask = glad_debug_impl_glStencilMask;
    glad_debug_glStencilMaskSeparate = glad_debug_impl_glStencilMaskSeparate;
    glad_debug_glStencilOp = glad_debug_impl_glStencilOp;
    glad_debug_glStencilOpSeparate = glad_debug_impl_glStencilOpSeparate;
    glad_debug_glTexBuffer = glad_debug_impl_glTexBuffer;
    glad_debug_glTexImage1D = glad_debug_impl_glTexImage1D;
    glad_debug_glTexImage2D = glad_debug_impl_glTexImage2D;
    glad_debug_glTexImage2DMultisample = glad_debug_impl_glTexImage2DMultisample;
    glad_debug_glTexImage3D = glad_debug_impl_glTexImage3D;
    glad_debug_glTexImage3DMultisample = glad_debug_impl_glTexImage3DMultisample;
    glad_debug_glTexParameterIiv = glad_debug_impl_glTexParameterIiv;
    glad_debug_glTexParameterIuiv = glad_debug_impl_glTexParameterIuiv;
    glad_debug_glTexParameterf = glad_debug_impl_glTexParameterf;
    glad_debug_glTexParameterfv = glad_debug_impl_glTexParameterfv;
    glad_debug_glTexParameteri = glad_debug_impl_glTexParameteri;
    glad_debug_glTexParameteriv = glad_debug_impl_glTexParameteriv;
    glad_debug_glTexSubImage1D = glad_debug_impl_glTexSubImage1D;
    glad_debug_glTexSubImage2D = glad_debug_impl_glTexSubImage2D;
    glad_debug_glTexSubImage3D = glad_debug_impl_glTexSubImage3D;
    glad_debug_glTransformFeedbackVaryings = glad_debug_impl_glTransformFeedbackVaryings;
    glad_debug_glUniform1d = glad_debug_impl_glUniform1d;
    glad_debug_glUniform1dv = glad_debug_impl_glUniform1dv;
    glad_debug_glUniform1f = glad_debug_impl_glUniform1f;
    glad_debug_glUniform1fv = glad_debug_impl_glUniform1fv;
    glad_debug_glUniform1i = glad_debug_impl_glUniform1i;
    glad_debug_glUniform1iv = glad_debug_impl_glUniform1iv;
    glad_debug_glUniform1ui = glad_debug_impl_glUniform1ui;
    glad_debug_glUniform1uiv = glad_debug_impl_glUniform1uiv;
    glad_debug_glUniform2d = glad_debug_impl_glUniform2d;
    glad_debug_glUniform2dv = glad_debug_impl_glUniform2dv;
    glad_debug_glUniform2f = glad_debug_impl_glUniform2f;
    glad_debug_glUniform2fv = glad_debug_impl_glUniform2fv;
    glad_debug_glUniform2i = glad_debug_impl_glUniform2i;
    glad_debug_glUniform2iv = glad_debug_impl_glUniform2iv;
    glad_debug_glUniform2ui = glad_debug_impl_glUniform2ui;
    glad_debug_glUniform2uiv = glad_debug_impl_glUniform2uiv;
    glad_debug_glUniform3d = glad_debug_impl_glUniform3d;
    glad_debug_glUniform3dv = glad_debug_impl_glUniform3dv;
    glad_debug_glUniform3f = glad_debug_impl_glUniform3f;
    glad_debug_glUniform3fv = glad_debug_impl_glUniform3fv;
    glad_debug_glUniform3i = glad_debug_impl_glUniform3i;
    glad_debug_glUniform3iv = glad_debug_impl_glUniform3iv;
    glad_debug_glUniform3ui = glad_debug_impl_glUniform3ui;
    glad_debug_glUniform3uiv = glad_debug_impl_glUniform3uiv;
    glad_debug_glUniform4d = glad_debug_impl_glUniform4d;
    glad_debug_glUniform4dv = glad_debug_impl_glUniform4dv;
    glad_debug_glUniform4f = glad_debug_impl_glUniform4f;
    glad_debug_glUniform4fv = glad_debug_impl_glUniform4fv;
    glad_debug_glUniform4i = glad_debug_impl_glUniform4i;
    glad_debug_glUniform4iv = glad_debug_impl_glUniform4iv;
    glad_debug_glUniform4ui = glad_debug_impl_glUniform4ui;
    glad_debug_glUniform4uiv = glad_debug_impl_glUniform4uiv;
    glad_debug_glUniformBlockBinding = glad_debug_impl_glUniformBlockBinding;
    glad_debug_glUniformMatrix2dv = glad_debug_impl_glUniformMatrix2dv;
    glad_debug_glUniformMatrix2fv = glad_debug_impl_glUniformMatrix2fv;
    glad_debug_glUniformMatrix2x3dv = glad_debug_impl_glUniformMatrix2x3dv;
    glad_debug_glUniformMatrix2x3fv = glad_debug_impl_glUniformMatrix2x3fv;
    glad_debug_glUniformMatrix2x4dv = glad_debug_impl_glUniformMatrix2x4dv;
    glad_debug_glUniformMatrix2x4fv = glad_debug_impl_glUniformMatrix2x4fv;
    glad_debug_glUniformMatrix3dv = glad_debug_impl_glUniformMatrix3dv;
    glad_debug_glUniformMatrix3fv = glad_debug_impl_glUniformMatrix3fv;
    glad_debug_glUniformMatrix3x2dv = glad_debug_impl_glUniformMatrix3x2dv;
    glad_debug_glUniformMatrix3x2fv = glad_debug_impl_glUniformMatrix3x2fv;
    glad_debug_glUniformMatrix3x4dv = glad_debug_impl_glUniformMatrix3x4dv;
    glad_debug_glUniformMatrix3x4fv = glad_debug_impl_glUniformMatrix3x4fv;
    glad_debug_glUniformMatrix4dv = glad_debug_impl_glUniformMatrix4dv;
    glad_debug_glUniformMatrix4fv = glad_debug_impl_glUniformMatrix4fv;
    glad_debug_glUniformMatrix4x2dv = glad_debug_impl_glUniformMatrix4x2dv;
    glad_debug_glUniformMatrix4x2fv = glad_debug_impl_glUniformMatrix4x2fv;
    glad_debug_glUniformMatrix4x3dv = glad_debug_impl_glUniformMatrix4x3dv;
    glad_debug_glUniformMatrix4x3fv = glad_debug_impl_glUniformMatrix4x3fv;
    glad_debug_glUniformSubroutinesuiv = glad_debug_impl_glUniformSubroutinesuiv;
    glad_debug_glUnmapBuffer = glad_debug_impl_glUnmapBuffer;
    glad_debug_glUseProgram = glad_debug_impl_glUseProgram;
    glad_debug_glUseProgramStages = glad_debug_impl_glUseProgramStages;
    glad_debug_glValidateProgram = glad_debug_impl_glValidateProgram;
    glad_debug_glValidateProgramPipeline = glad_debug_impl_glValidateProgramPipeline;
    glad_debug_glVertexAttrib1d = glad_debug_impl_glVertexAttrib1d;
    glad_debug_glVertexAttrib1dv = glad_debug_impl_glVertexAttrib1dv;
    glad_debug_glVertexAttrib1f = glad_debug_impl_glVertexAttrib1f;
    glad_debug_glVertexAttrib1fv = glad_debug_impl_glVertexAttrib1fv;
    glad_debug_glVertexAttrib1s = glad_debug_impl_glVertexAttrib1s;
    glad_debug_glVertexAttrib1sv = glad_debug_impl_glVertexAttrib1sv;
    glad_debug_glVertexAttrib2d = glad_debug_impl_glVertexAttrib2d;
    glad_debug_glVertexAttrib2dv = glad_debug_impl_glVertexAttrib2dv;
    glad_debug_glVertexAttrib2f = glad_debug_impl_glVertexAttrib2f;
    glad_debug_glVertexAttrib2fv = glad_debug_impl_glVertexAttrib2fv;
    glad_debug_glVertexAttrib2s = glad_debug_impl_glVertexAttrib2s;
    glad_debug_glVertexAttrib2sv = glad_debug_impl_glVertexAttrib2sv;
    glad_debug_glVertexAttrib3d = glad_debug_impl_glVertexAttrib3d;
    glad_debug_glVertexAttrib3dv = glad_debug_impl_glVertexAttrib3dv;
    glad_debug_glVertexAttrib3f = glad_debug_impl_glVertexAttrib3f;
    glad_debug_glVertexAttrib3fv = glad_debug_impl_glVertexAttrib3fv;
    glad_debug_glVertexAttrib3s = glad_debug_impl_glVertexAttrib3s;
    glad_debug_glVertexAttrib3sv = glad_debug_impl_glVertexAttrib3sv;
    glad_debug_glVertexAttrib4Nbv = glad_debug_impl_glVertexAttrib4Nbv;
    glad_debug_glVertexAttrib4Niv = glad_debug_impl_glVertexAttrib4Niv;
    glad_debug_glVertexAttrib4Nsv = glad_debug_impl_glVertexAttrib4Nsv;
    glad_debug_glVertexAttrib4Nub = glad_debug_impl_glVertexAttrib4Nub;
    glad_debug_glVertexAttrib4Nubv = glad_debug_impl_glVertexAttrib4Nubv;
    glad_debug_glVertexAttrib4Nuiv = glad_debug_impl_glVertexAttrib4Nuiv;
    glad_debug_glVertexAttrib4Nusv = glad_debug_impl_glVertexAttrib4Nusv;
    glad_debug_glVertexAttrib4bv = glad_debug_impl_glVertexAttrib4bv;
    glad_debug_glVertexAttrib4d = glad_debug_impl_glVertexAttrib4d;
    glad_debug_glVertexAttrib4dv = glad_debug_impl_glVertexAttrib4dv;
    glad_debug_glVertexAttrib4f = glad_debug_impl_glVertexAttrib4f;
    glad_debug_glVertexAttrib4fv = glad_debug_impl_glVertexAttrib4fv;
    glad_debug_glVertexAttrib4iv = glad_debug_impl_glVertexAttrib4iv;
    glad_debug_glVertexAttrib4s = glad_debug_impl_glVertexAttrib4s;
    glad_debug_glVertexAttrib4sv = glad_debug_impl_glVertexAttrib4sv;
    glad_debug_glVertexAttrib4ubv = glad_debug_impl_glVertexAttrib4ubv;
    glad_debug_glVertexAttrib4uiv = glad_debug_impl_glVertexAttrib4uiv;
    glad_debug_glVertexAttrib4usv = glad_debug_impl_glVertexAttrib4usv;
    glad_debug_glVertexAttribDivisor = glad_debug_impl_glVertexAttribDivisor;
    glad_debug_glVertexAttribI1i = glad_debug_impl_glVertexAttribI1i;
    glad_debug_glVertexAttribI1iv = glad_debug_impl_glVertexAttribI1iv;
    glad_debug_glVertexAttribI1ui = glad_debug_impl_glVertexAttribI1ui;
    glad_debug_glVertexAttribI1uiv = glad_debug_impl_glVertexAttribI1uiv;
    glad_debug_glVertexAttribI2i = glad_debug_impl_glVertexAttribI2i;
    glad_debug_glVertexAttribI2iv = glad_debug_impl_glVertexAttribI2iv;
    glad_debug_glVertexAttribI2ui = glad_debug_impl_glVertexAttribI2ui;
    glad_debug_glVertexAttribI2uiv = glad_debug_impl_glVertexAttribI2uiv;
    glad_debug_glVertexAttribI3i = glad_debug_impl_glVertexAttribI3i;
    glad_debug_glVertexAttribI3iv = glad_debug_impl_glVertexAttribI3iv;
    glad_debug_glVertexAttribI3ui = glad_debug_impl_glVertexAttribI3ui;
    glad_debug_glVertexAttribI3uiv = glad_debug_impl_glVertexAttribI3uiv;
    glad_debug_glVertexAttribI4bv = glad_debug_impl_glVertexAttribI4bv;
    glad_debug_glVertexAttribI4i = glad_debug_impl_glVertexAttribI4i;
    glad_debug_glVertexAttribI4iv = glad_debug_impl_glVertexAttribI4iv;
    glad_debug_glVertexAttribI4sv = glad_debug_impl_glVertexAttribI4sv;
    glad_debug_glVertexAttribI4ubv = glad_debug_impl_glVertexAttribI4ubv;
    glad_debug_glVertexAttribI4ui = glad_debug_impl_glVertexAttribI4ui;
    glad_debug_glVertexAttribI4uiv = glad_debug_impl_glVertexAttribI4uiv;
    glad_debug_glVertexAttribI4usv = glad_debug_impl_glVertexAttribI4usv;
    glad_debug_glVertexAttribIPointer = glad_debug_impl_glVertexAttribIPointer;
    glad_debug_glVertexAttribL1d = glad_debug_impl_glVertexAttribL1d;
    glad_debug_glVertexAttribL1dv = glad_debug_impl_glVertexAttribL1dv;
    glad_debug_glVertexAttribL2d = glad_debug_impl_glVertexAttribL2d;
    glad_debug_glVertexAttribL2dv = glad_debug_impl_glVertexAttribL2dv;
    glad_debug_glVertexAttribL3d = glad_debug_impl_glVertexAttribL3d;
    glad_debug_glVertexAttribL3dv = glad_debug_impl_glVertexAttribL3dv;
    glad_debug_glVertexAttribL4d = glad_debug_impl_glVertexAttribL4d;
    glad_debug_glVertexAttribL4dv = glad_debug_impl_glVertexAttribL4dv;
    glad_debug_glVertexAttribLPointer = glad_debug_impl_glVertexAttribLPointer;
    glad_debug_glVertexAttribP1ui = glad_debug_impl_glVertexAttribP1ui;
    glad_debug_glVertexAttribP1uiv = glad_debug_impl_glVertexAttribP1uiv;
    glad_debug_glVertexAttribP2ui = glad_debug_impl_glVertexAttribP2ui;
    glad_debug_glVertexAttribP2uiv = glad_debug_impl_glVertexAttribP2uiv;
    glad_debug_glVertexAttribP3ui = glad_debug_impl_glVertexAttribP3ui;
    glad_debug_glVertexAttribP3uiv = glad_debug_impl_glVertexAttribP3uiv;
    glad_debug_glVertexAttribP4ui = glad_debug_impl_glVertexAttribP4ui;
    glad_debug_glVertexAttribP4uiv = glad_debug_impl_glVertexAttribP4uiv;
    glad_debug_glVertexAttribPointer = glad_debug_impl_glVertexAttribPointer;
    glad_debug_glViewport = glad_debug_impl_glViewport;
    glad_debug_glViewportArrayv = glad_debug_impl_glViewportArrayv;
    glad_debug_glViewportIndexedf = glad_debug_impl_glViewportIndexedf;
    glad_debug_glViewportIndexedfv = glad_debug_impl_glViewportIndexedfv;
    glad_debug_glWaitSync = glad_debug_impl_glWaitSync;
}

void gladUninstallGLDebug() {
    glad_debug_glActiveShaderProgram = glad_glActiveShaderProgram;
    glad_debug_glActiveTexture = glad_glActiveTexture;
    glad_debug_glAttachShader = glad_glAttachShader;
    glad_debug_glBeginConditionalRender = glad_glBeginConditionalRender;
    glad_debug_glBeginQuery = glad_glBeginQuery;
    glad_debug_glBeginQueryIndexed = glad_glBeginQueryIndexed;
    glad_debug_glBeginTransformFeedback = glad_glBeginTransformFeedback;
    glad_debug_glBindAttribLocation = glad_glBindAttribLocation;
    glad_debug_glBindBuffer = glad_glBindBuffer;
    glad_debug_glBindBufferBase = glad_glBindBufferBase;
    glad_debug_glBindBufferRange = glad_glBindBufferRange;
    glad_debug_glBindFragDataLocation = glad_glBindFragDataLocation;
    glad_debug_glBindFragDataLocationIndexed = glad_glBindFragDataLocationIndexed;
    glad_debug_glBindFramebuffer = glad_glBindFramebuffer;
    glad_debug_glBindProgramPipeline = glad_glBindProgramPipeline;
    glad_debug_glBindRenderbuffer = glad_glBindRenderbuffer;
    glad_debug_glBindSampler = glad_glBindSampler;
    glad_debug_glBindTexture = glad_glBindTexture;
    glad_debug_glBindTransformFeedback = glad_glBindTransformFeedback;
    glad_debug_glBindVertexArray = glad_glBindVertexArray;
    glad_debug_glBlendColor = glad_glBlendColor;
    glad_debug_glBlendEquation = glad_glBlendEquation;
    glad_debug_glBlendEquationSeparate = glad_glBlendEquationSeparate;
    glad_debug_glBlendEquationSeparatei = glad_glBlendEquationSeparatei;
    glad_debug_glBlendEquationi = glad_glBlendEquationi;
    glad_debug_glBlendFunc = glad_glBlendFunc;
    glad_debug_glBlendFuncSeparate = glad_glBlendFuncSeparate;
    glad_debug_glBlendFuncSeparatei = glad_glBlendFuncSeparatei;
    glad_debug_glBlendFunci = glad_glBlendFunci;
    glad_debug_glBlitFramebuffer = glad_glBlitFramebuffer;
    glad_debug_glBufferData = glad_glBufferData;
    glad_debug_glBufferSubData = glad_glBufferSubData;
    glad_debug_glCheckFramebufferStatus = glad_glCheckFramebufferStatus;
    glad_debug_glClampColor = glad_glClampColor;
    glad_debug_glClear = glad_glClear;
    glad_debug_glClearBufferfi = glad_glClearBufferfi;
    glad_debug_glClearBufferfv = glad_glClearBufferfv;
    glad_debug_glClearBufferiv = glad_glClearBufferiv;
    glad_debug_glClearBufferuiv = glad_glClearBufferuiv;
    glad_debug_glClearColor = glad_glClearColor;
    glad_debug_glClearDepth = glad_glClearDepth;
    glad_debug_glClearDepthf = glad_glClearDepthf;
    glad_debug_glClearStencil = glad_glClearStencil;
    glad_debug_glClientWaitSync = glad_glClientWaitSync;
    glad_debug_glColorMask = glad_glColorMask;
    glad_debug_glColorMaski = glad_glColorMaski;
    glad_debug_glCompileShader = glad_glCompileShader;
    glad_debug_glCompressedTexImage1D = glad_glCompressedTexImage1D;
    glad_debug_glCompressedTexImage2D = glad_glCompressedTexImage2D;
    glad_debug_glCompressedTexImage3D = glad_glCompressedTexImage3D;
    glad_debug_glCompressedTexSubImage1D = glad_glCompressedTexSubImage1D;
    glad_debug_glCompressedTexSubImage2D = glad_glCompressedTexSubImage2D;
    glad_debug_glCompressedTexSubImage3D = glad_glCompressedTexSubImage3D;
    glad_debug_glCopyBufferSubData = glad_glCopyBufferSubData;
    glad_debug_glCopyTexImage1D = glad_glCopyTexImage1D;
    glad_debug_glCopyTexImage2D = glad_glCopyTexImage2D;
    glad_debug_glCopyTexSubImage1D = glad_glCopyTexSubImage1D;
    glad_debug_glCopyTexSubImage2D = glad_glCopyTexSubImage2D;
    glad_debug_glCopyTexSubImage3D = glad_glCopyTexSubImage3D;
    glad_debug_glCreateProgram = glad_glCreateProgram;
    glad_debug_glCreateShader = glad_glCreateShader;
    glad_debug_glCreateShaderProgramv = glad_glCreateShaderProgramv;
    glad_debug_glCullFace = glad_glCullFace;
    glad_debug_glDebugMessageCallback = glad_glDebugMessageCallback;
    glad_debug_glDebugMessageControl = glad_glDebugMessageControl;
    glad_debug_glDebugMessageInsert = glad_glDebugMessageInsert;
    glad_debug_glDeleteBuffers = glad_glDeleteBuffers;
    glad_debug_glDeleteFramebuffers = glad_glDeleteFramebuffers;
    glad_debug_glDeleteProgram = glad_glDeleteProgram;
    glad_debug_glDeleteProgramPipelines = glad_glDeleteProgramPipelines;
    glad_debug_glDeleteQueries = glad_glDeleteQueries;
    glad_debug_glDeleteRenderbuffers = glad_glDeleteRenderbuffers;
    glad_debug_glDeleteSamplers = glad_glDeleteSamplers;
    glad_debug_glDeleteShader = glad_glDeleteShader;
    glad_debug_glDeleteSync = glad_glDeleteSync;
    glad_debug_glDeleteTextures = glad_glDeleteTextures;
    glad_debug_glDeleteTransformFeedbacks = glad_glDeleteTransformFeedbacks;
    glad_debug_glDeleteVertexArrays = glad_glDeleteVertexArrays;
    glad_debug_glDepthFunc = glad_glDepthFunc;
    glad_debug_glDepthMask = glad_glDepthMask;
    glad_debug_glDepthRange = glad_glDepthRange;
    glad_debug_glDepthRangeArrayv = glad_glDepthRangeArrayv;
    glad_debug_glDepthRangeIndexed = glad_glDepthRangeIndexed;
    glad_debug_glDepthRangef = glad_glDepthRangef;
    glad_debug_glDetachShader = glad_glDetachShader;
    glad_debug_glDisable = glad_glDisable;
    glad_debug_glDisableVertexAttribArray = glad_glDisableVertexAttribArray;
    glad_debug_glDisablei = glad_glDisablei;
    glad_debug_glDrawArrays = glad_glDrawArrays;
    glad_debug_glDrawArraysIndirect = glad_glDrawArraysIndirect;
    glad_debug_glDrawArraysInstanced = glad_glDrawArraysInstanced;
    glad_debug_glDrawBuffer = glad_glDrawBuffer;
    glad_debug_glDrawBuffers = glad_glDrawBuffers;
    glad_debug_glDrawElements = glad_glDrawElements;
    glad_debug_glDrawElementsBaseVertex = glad_glDrawElementsBaseVertex;
    glad_debug_glDrawElementsIndirect = glad_glDrawElementsIndirect;
    glad_debug_glDrawElementsInstanced = glad_glDrawElementsInstanced;
    glad_debug_glDrawElementsInstancedBaseVertex = glad_glDrawElementsInstancedBaseVertex;
    glad_debug_glDrawRangeElements = glad_glDrawRangeElements;
    glad_debug_glDrawRangeElementsBaseVertex = glad_glDrawRangeElementsBaseVertex;
    glad_debug_glDrawTransformFeedback = glad_glDrawTransformFeedback;
    glad_debug_glDrawTransformFeedbackStream = glad_glDrawTransformFeedbackStream;
    glad_debug_glEnable = glad_glEnable;
    glad_debug_glEnableVertexAttribArray = glad_glEnableVertexAttribArray;
    glad_debug_glEnablei = glad_glEnablei;
    glad_debug_glEndConditionalRender = glad_glEndConditionalRender;
    glad_debug_glEndQuery = glad_glEndQuery;
    glad_debug_glEndQueryIndexed = glad_glEndQueryIndexed;
    glad_debug_glEndTransformFeedback = glad_glEndTransformFeedback;
    glad_debug_glFenceSync = glad_glFenceSync;
    glad_debug_glFinish = glad_glFinish;
    glad_debug_glFlush = glad_glFlush;
    glad_debug_glFlushMappedBufferRange = glad_glFlushMappedBufferRange;
    glad_debug_glFramebufferRenderbuffer = glad_glFramebufferRenderbuffer;
    glad_debug_glFramebufferTexture = glad_glFramebufferTexture;
    glad_debug_glFramebufferTexture1D = glad_glFramebufferTexture1D;
    glad_debug_glFramebufferTexture2D = glad_glFramebufferTexture2D;
    glad_debug_glFramebufferTexture3D = glad_glFramebufferTexture3D;
    glad_debug_glFramebufferTextureLayer = glad_glFramebufferTextureLayer;
    glad_debug_glFrontFace = glad_glFrontFace;
    glad_debug_glGenBuffers = glad_glGenBuffers;
    glad_debug_glGenFramebuffers = glad_glGenFramebuffers;
    glad_debug_glGenProgramPipelines = glad_glGenProgramPipelines;
    glad_debug_glGenQueries = glad_glGenQueries;
    glad_debug_glGenRenderbuffers = glad_glGenRenderbuffers;
    glad_debug_glGenSamplers = glad_glGenSamplers;
    glad_debug_glGenTextures = glad_glGenTextures;
    glad_debug_glGenTransformFeedbacks = glad_glGenTransformFeedbacks;
    glad_debug_glGenVertexArrays = glad_glGenVertexArrays;
    glad_debug_glGenerateMipmap = glad_glGenerateMipmap;
    glad_debug_glGetActiveAttrib = glad_glGetActiveAttrib;
    glad_debug_glGetActiveSubroutineName = glad_glGetActiveSubroutineName;
    glad_debug_glGetActiveSubroutineUniformName = glad_glGetActiveSubroutineUniformName;
    glad_debug_glGetActiveSubroutineUniformiv = glad_glGetActiveSubroutineUniformiv;
    glad_debug_glGetActiveUniform = glad_glGetActiveUniform;
    glad_debug_glGetActiveUniformBlockName = glad_glGetActiveUniformBlockName;
    glad_debug_glGetActiveUniformBlockiv = glad_glGetActiveUniformBlockiv;
    glad_debug_glGetActiveUniformName = glad_glGetActiveUniformName;
    glad_debug_glGetActiveUniformsiv = glad_glGetActiveUniformsiv;
    glad_debug_glGetAttachedShaders = glad_glGetAttachedShaders;
    glad_debug_glGetAttribLocation = glad_glGetAttribLocation;
    glad_debug_glGetBooleani_v = glad_glGetBooleani_v;
    glad_debug_glGetBooleanv = glad_glGetBooleanv;
    glad_debug_glGetBufferParameteri64v = glad_glGetBufferParameteri64v;
    glad_debug_glGetBufferParameteriv = glad_glGetBufferParameteriv;
    glad_debug_glGetBufferPointerv = glad_glGetBufferPointerv;
    glad_debug_glGetBufferSubData = glad_glGetBufferSubData;
    glad_debug_glGetCompressedTexImage = glad_glGetCompressedTexImage;
    glad_debug_glGetDebugMessageLog = glad_glGetDebugMessageLog;
    glad_debug_glGetDoublei_v = glad_glGetDoublei_v;
    glad_debug_glGetDoublev = glad_glGetDoublev;
    glad_debug_glGetError = glad_glGetError;
    glad_debug_glGetFloati_v = glad_glGetFloati_v;
    glad_debug_glGetFloatv = glad_glGetFloatv;
    glad_debug_glGetFragDataIndex = glad_glGetFragDataIndex;
    glad_debug_glGetFragDataLocation = glad_glGetFragDataLocation;
    glad_debug_glGetFramebufferAttachmentParameteriv = glad_glGetFramebufferAttachmentParameteriv;
    glad_debug_glGetInteger64i_v = glad_glGetInteger64i_v;
    glad_debug_glGetInteger64v = glad_glGetInteger64v;
    glad_debug_glGetIntegeri_v = glad_glGetIntegeri_v;
    glad_debug_glGetIntegerv = glad_glGetIntegerv;
    glad_debug_glGetMultisamplefv = glad_glGetMultisamplefv;
    glad_debug_glGetObjectLabel = glad_glGetObjectLabel;
    glad_debug_glGetObjectPtrLabel = glad_glGetObjectPtrLabel;
    glad_debug_glGetPointerv = glad_glGetPointerv;
    glad_debug_glGetProgramBinary = glad_glGetProgramBinary;
    glad_debug_glGetProgramInfoLog = glad_glGetProgramInfoLog;
    glad_debug_glGetProgramPipelineInfoLog = glad_glGetProgramPipelineInfoLog;
    glad_debug_glGetProgramPipelineiv = glad_glGetProgramPipelineiv;
    glad_debug_glGetProgramStageiv = glad_glGetProgramStageiv;
    glad_debug_glGetProgramiv = glad_glGetProgramiv;
    glad_debug_glGetQueryIndexediv = glad_glGetQueryIndexediv;
    glad_debug_glGetQueryObjecti64v = glad_glGetQueryObjecti64v;
    glad_debug_glGetQueryObjectiv = glad_glGetQueryObjectiv;
    glad_debug_glGetQueryObjectui64v = glad_glGetQueryObjectui64v;
    glad_debug_glGetQueryObjectuiv = glad_glGetQueryObjectuiv;
    glad_debug_glGetQueryiv = glad_glGetQueryiv;
    glad_debug_glGetRenderbufferParameteriv = glad_glGetRenderbufferParameteriv;
    glad_debug_glGetSamplerParameterIiv = glad_glGetSamplerParameterIiv;
    glad_debug_glGetSamplerParameterIuiv = glad_glGetSamplerParameterIuiv;
    glad_debug_glGetSamplerParameterfv = glad_glGetSamplerParameterfv;
    glad_debug_glGetSamplerParameteriv = glad_glGetSamplerParameteriv;
    glad_debug_glGetShaderInfoLog = glad_glGetShaderInfoLog;
    glad_debug_glGetShaderPrecisionFormat = glad_glGetShaderPrecisionFormat;
    glad_debug_glGetShaderSource = glad_glGetShaderSource;
    glad_debug_glGetShaderiv = glad_glGetShaderiv;
    glad_debug_glGetString = glad_glGetString;
    glad_debug_glGetStringi = glad_glGetStringi;
    glad_debug_glGetSubroutineIndex = glad_glGetSubroutineIndex;
    glad_debug_glGetSubroutineUniformLocation = glad_glGetSubroutineUniformLocation;
    glad_debug_glGetSynciv = glad_glGetSynciv;
    glad_debug_glGetTexImage = glad_glGetTexImage;
    glad_debug_glGetTexLevelParameterfv = glad_glGetTexLevelParameterfv;
    glad_debug_glGetTexLevelParameteriv = glad_glGetTexLevelParameteriv;
    glad_debug_glGetTexParameterIiv = glad_glGetTexParameterIiv;
    glad_debug_glGetTexParameterIuiv = glad_glGetTexParameterIuiv;
    glad_debug_glGetTexParameterfv = glad_glGetTexParameterfv;
    glad_debug_glGetTexParameteriv = glad_glGetTexParameteriv;
    glad_debug_glGetTransformFeedbackVarying = glad_glGetTransformFeedbackVarying;
    glad_debug_glGetUniformBlockIndex = glad_glGetUniformBlockIndex;
    glad_debug_glGetUniformIndices = glad_glGetUniformIndices;
    glad_debug_glGetUniformLocation = glad_glGetUniformLocation;
    glad_debug_glGetUniformSubroutineuiv = glad_glGetUniformSubroutineuiv;
    glad_debug_glGetUniformdv = glad_glGetUniformdv;
    glad_debug_glGetUniformfv = glad_glGetUniformfv;
    glad_debug_glGetUniformiv = glad_glGetUniformiv;
    glad_debug_glGetUniformuiv = glad_glGetUniformuiv;
    glad_debug_glGetVertexAttribIiv = glad_glGetVertexAttribIiv;
    glad_debug_glGetVertexAttribIuiv = glad_glGetVertexAttribIuiv;
    glad_debug_glGetVertexAttribLdv = glad_glGetVertexAttribLdv;
    glad_debug_glGetVertexAttribPointerv = glad_glGetVertexAttribPointerv;
    glad_debug_glGetVertexAttribdv = glad_glGetVertexAttribdv;
    glad_debug_glGetVertexAttribfv = glad_glGetVertexAttribfv;
    glad_debug_glGetVertexAttribiv = glad_glGetVertexAttribiv;
    glad_debug_glHint = glad_glHint;
    glad_debug_glIsBuffer = glad_glIsBuffer;
    glad_debug_glIsEnabled = glad_glIsEnabled;
    glad_debug_glIsEnabledi = glad_glIsEnabledi;
    glad_debug_glIsFramebuffer = glad_glIsFramebuffer;
    glad_debug_glIsProgram = glad_glIsProgram;
    glad_debug_glIsProgramPipeline = glad_glIsProgramPipeline;
    glad_debug_glIsQuery = glad_glIsQuery;
    glad_debug_glIsRenderbuffer = glad_glIsRenderbuffer;
    glad_debug_glIsSampler = glad_glIsSampler;
    glad_debug_glIsShader = glad_glIsShader;
    glad_debug_glIsSync = glad_glIsSync;
    glad_debug_glIsTexture = glad_glIsTexture;
    glad_debug_glIsTransformFeedback = glad_glIsTransformFeedback;
    glad_debug_glIsVertexArray = glad_glIsVertexArray;
    glad_debug_glLineWidth = glad_glLineWidth;
    glad_debug_glLinkProgram = glad_glLinkProgram;
    glad_debug_glLogicOp = glad_glLogicOp;
    glad_debug_glMapBuffer = glad_glMapBuffer;
    glad_debug_glMapBufferRange = glad_glMapBufferRange;
    glad_debug_glMinSampleShading = glad_glMinSampleShading;
    glad_debug_glMultiDrawArrays = glad_glMultiDrawArrays;
    glad_debug_glMultiDrawElements = glad_glMultiDrawElements;
    glad_debug_glMultiDrawElementsBaseVertex = glad_glMultiDrawElementsBaseVertex;
    glad_debug_glObjectLabel = glad_glObjectLabel;
    glad_debug_glObjectPtrLabel = glad_glObjectPtrLabel;
    glad_debug_glPatchParameterfv = glad_glPatchParameterfv;
    glad_debug_glPatchParameteri = glad_glPatchParameteri;
    glad_debug_glPauseTransformFeedback = glad_glPauseTransformFeedback;
    glad_debug_glPixelStoref = glad_glPixelStoref;
    glad_debug_glPixelStorei = glad_glPixelStorei;
    glad_debug_glPointParameterf = glad_glPointParameterf;
    glad_debug_glPointParameterfv = glad_glPointParameterfv;
    glad_debug_glPointParameteri = glad_glPointParameteri;
    glad_debug_glPointParameteriv = glad_glPointParameteriv;
    glad_debug_glPointSize = glad_glPointSize;
    glad_debug_glPolygonMode = glad_glPolygonMode;
    glad_debug_glPolygonOffset = glad_glPolygonOffset;
    glad_debug_glPopDebugGroup = glad_glPopDebugGroup;
    glad_debug_glPrimitiveRestartIndex = glad_glPrimitiveRestartIndex;
    glad_debug_glProgramBinary = glad_glProgramBinary;
    glad_debug_glProgramParameteri = glad_glProgramParameteri;
    glad_debug_glProgramUniform1d = glad_glProgramUniform1d;
    glad_debug_glProgramUniform1dv = glad_glProgramUniform1dv;
    glad_debug_glProgramUniform1f = glad_glProgramUniform1f;
    glad_debug_glProgramUniform1fv = glad_glProgramUniform1fv;
    glad_debug_glProgramUniform1i = glad_glProgramUniform1i;
    glad_debug_glProgramUniform1iv = glad_glProgramUniform1iv;
    glad_debug_glProgramUniform1ui = glad_glProgramUniform1ui;
    glad_debug_glProgramUniform1uiv = glad_glProgramUniform1uiv;
    glad_debug_glProgramUniform2d = glad_glProgramUniform2d;
    glad_debug_glProgramUniform2dv = glad_glProgramUniform2dv;
    glad_debug_glProgramUniform2f = glad_glProgramUniform2f;
    glad_debug_glProgramUniform2fv = glad_glProgramUniform2fv;
    glad_debug_glProgramUniform2i = glad_glProgramUniform2i;
    glad_debug_glProgramUniform2iv = glad_glProgramUniform2iv;
    glad_debug_glProgramUniform2ui = glad_glProgramUniform2ui;
    glad_debug_glProgramUniform2uiv = glad_glProgramUniform2uiv;
    glad_debug_glProgramUniform3d = glad_glProgramUniform3d;
    glad_debug_glProgramUniform3dv = glad_glProgramUniform3dv;
    glad_debug_glProgramUniform3f = glad_glProgramUniform3f;
    glad_debug_glProgramUniform3fv = glad_glProgramUniform3fv;
    glad_debug_glProgramUniform3i = glad_glProgramUniform3i;
    glad_debug_glProgramUniform3iv = glad_glProgramUniform3iv;
    glad_debug_glProgramUniform3ui = glad_glProgramUniform3ui;
    glad_debug_glProgramUniform3uiv = glad_glProgramUniform3uiv;
    glad_debug_glProgramUniform4d = glad_glProgramUniform4d;
    glad_debug_glProgramUniform4dv = glad_glProgramUniform4dv;
    glad_debug_glProgramUniform4f = glad_glProgramUniform4f;
    glad_debug_glProgramUniform4fv = glad_glProgramUniform4fv;
    glad_debug_glProgramUniform4i = glad_glProgramUniform4i;
    glad_debug_glProgramUniform4iv = glad_glProgramUniform4iv;
    glad_debug_glProgramUniform4ui = glad_glProgramUniform4ui;
    glad_debug_glProgramUniform4uiv = glad_glProgramUniform4uiv;
    glad_debug_glProgramUniformMatrix2dv = glad_glProgramUniformMatrix2dv;
    glad_debug_glProgramUniformMatrix2fv = glad_glProgramUniformMatrix2fv;
    glad_debug_glProgramUniformMatrix2x3dv = glad_glProgramUniformMatrix2x3dv;
    glad_debug_glProgramUniformMatrix2x3fv = glad_glProgramUniformMatrix2x3fv;
    glad_debug_glProgramUniformMatrix2x4dv = glad_glProgramUniformMatrix2x4dv;
    glad_debug_glProgramUniformMatrix2x4fv = glad_glProgramUniformMatrix2x4fv;
    glad_debug_glProgramUniformMatrix3dv = glad_glProgramUniformMatrix3dv;
    glad_debug_glProgramUniformMatrix3fv = glad_glProgramUniformMatrix3fv;
    glad_debug_glProgramUniformMatrix3x2dv = glad_glProgramUniformMatrix3x2dv;
    glad_debug_glProgramUniformMatrix3x2fv = glad_glProgramUniformMatrix3x2fv;
    glad_debug_glProgramUniformMatrix3x4dv = glad_glProgramUniformMatrix3x4dv;
    glad_debug_glProgramUniformMatrix3x4fv = glad_glProgramUniformMatrix3x4fv;
    glad_debug_glProgramUniformMatrix4dv = glad_glProgramUniformMatrix4dv;
    glad_debug_glProgramUniformMatrix4fv = glad_glProgramUniformMatrix4fv;
    glad_debug_glProgramUniformMatrix4x2dv = glad_glProgramUniformMatrix4x2dv;
    glad_debug_glProgramUniformMatrix4x2fv = glad_glProgramUniformMatrix4x2fv;
    glad_debug_glProgramUniformMatrix4x3dv = glad_glProgramUniformMatrix4x3dv;
    glad_debug_glProgramUniformMatrix4x3fv = glad_glProgramUniformMatrix4x3fv;
    glad_debug_glProvokingVertex = glad_glProvokingVertex;
    glad_debug_glPushDebugGroup = glad_glPushDebugGroup;
    glad_debug_glQueryCounter = glad_glQueryCounter;
    glad_debug_glReadBuffer = glad_glReadBuffer;
    glad_debug_glReadPixels = glad_glReadPixels;
    glad_debug_glReleaseShaderCompiler = glad_glReleaseShaderCompiler;
    glad_debug_glRenderbufferStorage = glad_glRenderbufferStorage;
    glad_debug_glRenderbufferStorageMultisample = glad_glRenderbufferStorageMultisample;
    glad_debug_glResumeTransformFeedback = glad_glResumeTransformFeedback;
    glad_debug_glSampleCoverage = glad_glSampleCoverage;
    glad_debug_glSampleMaski = glad_glSampleMaski;
    glad_debug_glSamplerParameterIiv = glad_glSamplerParameterIiv;
    glad_debug_glSamplerParameterIuiv = glad_glSamplerParameterIuiv;
    glad_debug_glSamplerParameterf = glad_glSamplerParameterf;
    glad_debug_glSamplerParameterfv = glad_glSamplerParameterfv;
    glad_debug_glSamplerParameteri = glad_glSamplerParameteri;
    glad_debug_glSamplerParameteriv = glad_glSamplerParameteriv;
    glad_debug_glScissor = glad_glScissor;
    glad_debug_glScissorArrayv = glad_glScissorArrayv;
    glad_debug_glScissorIndexed = glad_glScissorIndexed;
    glad_debug_glScissorIndexedv = glad_glScissorIndexedv;
    glad_debug_glShaderBinary = glad_glShaderBinary;
    glad_debug_glShaderSource = glad_glShaderSource;
    glad_debug_glStencilFunc = glad_glStencilFunc;
    glad_debug_glStencilFuncSeparate = glad_glStencilFuncSeparate;
    glad_debug_glStencilMask = glad_glStencilMask;
    glad_debug_glStencilMaskSeparate = glad_glStencilMaskSeparate;
    glad_debug_glStencilOp = glad_glStencilOp;
    glad_debug_glStencilOpSeparate = glad_glStencilOpSeparate;
    glad_debug_glTexBuffer = glad_glTexBuffer;
    glad_debug_glTexImage1D = glad_glTexImage1D;
    glad_debug_glTexImage2D = glad_glTexImage2D;
    glad_debug_glTexImage2DMultisample = glad_glTexImage2DMultisample;
    glad_debug_glTexImage3D = glad_glTexImage3D;
    glad_debug_glTexImage3DMultisample = glad_glTexImage3DMultisample;
    glad_debug_glTexParameterIiv = glad_glTexParameterIiv;
    glad_debug_glTexParameterIuiv = glad_glTexParameterIuiv;
    glad_debug_glTexParameterf = glad_glTexParameterf;
    glad_debug_glTexParameterfv = glad_glTexParameterfv;
    glad_debug_glTexParameteri = glad_glTexParameteri;
    glad_debug_glTexParameteriv = glad_glTexParameteriv;
    glad_debug_glTexSubImage1D = glad_glTexSubImage1D;
    glad_debug_glTexSubImage2D = glad_glTexSubImage2D;
    glad_debug_glTexSubImage3D = glad_glTexSubImage3D;
    glad_debug_glTransformFeedbackVaryings = glad_glTransformFeedbackVaryings;
    glad_debug_glUniform1d = glad_glUniform1d;
    glad_debug_glUniform1dv = glad_glUniform1dv;
    glad_debug_glUniform1f = glad_glUniform1f;
    glad_debug_glUniform1fv = glad_glUniform1fv;
    glad_debug_glUniform1i = glad_glUniform1i;
    glad_debug_glUniform1iv = glad_glUniform1iv;
    glad_debug_glUniform1ui = glad_glUniform1ui;
    glad_debug_glUniform1uiv = glad_glUniform1uiv;
    glad_debug_glUniform2d = glad_glUniform2d;
    glad_debug_glUniform2dv = glad_glUniform2dv;
    glad_debug_glUniform2f = glad_glUniform2f;
    glad_debug_glUniform2fv = glad_glUniform2fv;
    glad_debug_glUniform2i = glad_glUniform2i;
    glad_debug_glUniform2iv = glad_glUniform2iv;
    glad_debug_glUniform2ui = glad_glUniform2ui;
    glad_debug_glUniform2uiv = glad_glUniform2uiv;
    glad_debug_glUniform3d = glad_glUniform3d;
    glad_debug_glUniform3dv = glad_glUniform3dv;
    glad_debug_glUniform3f = glad_glUniform3f;
    glad_debug_glUniform3fv = glad_glUniform3fv;
    glad_debug_glUniform3i = glad_glUniform3i;
    glad_debug_glUniform3iv = glad_glUniform3iv;
    glad_debug_glUniform3ui = glad_glUniform3ui;
    glad_debug_glUniform3uiv = glad_glUniform3uiv;
    glad_debug_glUniform4d = glad_glUniform4d;
    glad_debug_glUniform4dv = glad_glUniform4dv;
    glad_debug_glUniform4f = glad_glUniform4f;
    glad_debug_glUniform4fv = glad_glUniform4fv;
    glad_debug_glUniform4i = glad_glUniform4i;
    glad_debug_glUniform4iv = glad_glUniform4iv;
    glad_debug_glUniform4ui = glad_glUniform4ui;
    glad_debug_glUniform4uiv = glad_glUniform4uiv;
    glad_debug_glUniformBlockBinding = glad_glUniformBlockBinding;
    glad_debug_glUniformMatrix2dv = glad_glUniformMatrix2dv;
    glad_debug_glUniformMatrix2fv = glad_glUniformMatrix2fv;
    glad_debug_glUniformMatrix2x3dv = glad_glUniformMatrix2x3dv;
    glad_debug_glUniformMatrix2x3fv = glad_glUniformMatrix2x3fv;
    glad_debug_glUniformMatrix2x4dv = glad_glUniformMatrix2x4dv;
    glad_debug_glUniformMatrix2x4fv = glad_glUniformMatrix2x4fv;
    glad_debug_glUniformMatrix3dv = glad_glUniformMatrix3dv;
    glad_debug_glUniformMatrix3fv = glad_glUniformMatrix3fv;
    glad_debug_glUniformMatrix3x2dv = glad_glUniformMatrix3x2dv;
    glad_debug_glUniformMatrix3x2fv = glad_glUniformMatrix3x2fv;
    glad_debug_glUniformMatrix3x4dv = glad_glUniformMatrix3x4dv;
    glad_debug_glUniformMatrix3x4fv = glad_glUniformMatrix3x4fv;
    glad_debug_glUniformMatrix4dv = glad_glUniformMatrix4dv;
    glad_debug_glUniformMatrix4fv = glad_glUniformMatrix4fv;
    glad_debug_glUniformMatrix4x2dv = glad_glUniformMatrix4x2dv;
    glad_debug_glUniformMatrix4x2fv = glad_glUniformMatrix4x2fv;
    glad_debug_glUniformMatrix4x3dv = glad_glUniformMatrix4x3dv;
    glad_debug_glUniformMatrix4x3fv = glad_glUniformMatrix4x3fv;
    glad_debug_glUniformSubroutinesuiv = glad_glUniformSubroutinesuiv;
    glad_debug_glUnmapBuffer = glad_glUnmapBuffer;
    glad_debug_glUseProgram = glad_glUseProgram;
    glad_debug_glUseProgramStages = glad_glUseProgramStages;
    glad_debug_glValidateProgram = glad_glValidateProgram;
    glad_debug_glValidateProgramPipeline = glad_glValidateProgramPipeline;
    glad_debug_glVertexAttrib1d = glad_glVertexAttrib1d;
    glad_debug_glVertexAttrib1dv = glad_glVertexAttrib1dv;
    glad_debug_glVertexAttrib1f = glad_glVertexAttrib1f;
    glad_debug_glVertexAttrib1fv = glad_glVertexAttrib1fv;
    glad_debug_glVertexAttrib1s = glad_glVertexAttrib1s;
    glad_debug_glVertexAttrib1sv = glad_glVertexAttrib1sv;
    glad_debug_glVertexAttrib2d = glad_glVertexAttrib2d;
    glad_debug_glVertexAttrib2dv = glad_glVertexAttrib2dv;
    glad_debug_glVertexAttrib2f = glad_glVertexAttrib2f;
    glad_debug_glVertexAttrib2fv = glad_glVertexAttrib2fv;
    glad_debug_glVertexAttrib2s = glad_glVertexAttrib2s;
    glad_debug_glVertexAttrib2sv = glad_glVertexAttrib2sv;
    glad_debug_glVertexAttrib3d = glad_glVertexAttrib3d;
    glad_debug_glVertexAttrib3dv = glad_glVertexAttrib3dv;
    glad_debug_glVertexAttrib3f = glad_glVertexAttrib3f;
    glad_debug_glVertexAttrib3fv = glad_glVertexAttrib3fv;
    glad_debug_glVertexAttrib3s = glad_glVertexAttrib3s;
    glad_debug_glVertexAttrib3sv = glad_glVertexAttrib3sv;
    glad_debug_glVertexAttrib4Nbv = glad_glVertexAttrib4Nbv;
    glad_debug_glVertexAttrib4Niv = glad_glVertexAttrib4Niv;
    glad_debug_glVertexAttrib4Nsv = glad_glVertexAttrib4Nsv;
    glad_debug_glVertexAttrib4Nub = glad_glVertexAttrib4Nub;
    glad_debug_glVertexAttrib4Nubv = glad_glVertexAttrib4Nubv;
    glad_debug_glVertexAttrib4Nuiv = glad_glVertexAttrib4Nuiv;
    glad_debug_glVertexAttrib4Nusv = glad_glVertexAttrib4Nusv;
    glad_debug_glVertexAttrib4bv = glad_glVertexAttrib4bv;
    glad_debug_glVertexAttrib4d = glad_glVertexAttrib4d;
    glad_debug_glVertexAttrib4dv = glad_glVertexAttrib4dv;
    glad_debug_glVertexAttrib4f = glad_glVertexAttrib4f;
    glad_debug_glVertexAttrib4fv = glad_glVertexAttrib4fv;
    glad_debug_glVertexAttrib4iv = glad_glVertexAttrib4iv;
    glad_debug_glVertexAttrib4s = glad_glVertexAttrib4s;
    glad_debug_glVertexAttrib4sv = glad_glVertexAttrib4sv;
    glad_debug_glVertexAttrib4ubv = glad_glVertexAttrib4ubv;
    glad_debug_glVertexAttrib4uiv = glad_glVertexAttrib4uiv;
    glad_debug_glVertexAttrib4usv = glad_glVertexAttrib4usv;
    glad_debug_glVertexAttribDivisor = glad_glVertexAttribDivisor;
    glad_debug_glVertexAttribI1i = glad_glVertexAttribI1i;
    glad_debug_glVertexAttribI1iv = glad_glVertexAttribI1iv;
    glad_debug_glVertexAttribI1ui = glad_glVertexAttribI1ui;
    glad_debug_glVertexAttribI1uiv = glad_glVertexAttribI1uiv;
    glad_debug_glVertexAttribI2i = glad_glVertexAttribI2i;
    glad_debug_glVertexAttribI2iv = glad_glVertexAttribI2iv;
    glad_debug_glVertexAttribI2ui = glad_glVertexAttribI2ui;
    glad_debug_glVertexAttribI2uiv = glad_glVertexAttribI2uiv;
    glad_debug_glVertexAttribI3i = glad_glVertexAttribI3i;
    glad_debug_glVertexAttribI3iv = glad_glVertexAttribI3iv;
    glad_debug_glVertexAttribI3ui = glad_glVertexAttribI3ui;
    glad_debug_glVertexAttribI3uiv = glad_glVertexAttribI3uiv;
    glad_debug_glVertexAttribI4bv = glad_glVertexAttribI4bv;
    glad_debug_glVertexAttribI4i = glad_glVertexAttribI4i;
    glad_debug_glVertexAttribI4iv = glad_glVertexAttribI4iv;
    glad_debug_glVertexAttribI4sv = glad_glVertexAttribI4sv;
    glad_debug_glVertexAttribI4ubv = glad_glVertexAttribI4ubv;
    glad_debug_glVertexAttribI4ui = glad_glVertexAttribI4ui;
    glad_debug_glVertexAttribI4uiv = glad_glVertexAttribI4uiv;
    glad_debug_glVertexAttribI4usv = glad_glVertexAttribI4usv;
    glad_debug_glVertexAttribIPointer = glad_glVertexAttribIPointer;
    glad_debug_glVertexAttribL1d = glad_glVertexAttribL1d;
    glad_debug_glVertexAttribL1dv = glad_glVertexAttribL1dv;
    glad_debug_glVertexAttribL2d = glad_glVertexAttribL2d;
    glad_debug_glVertexAttribL2dv = glad_glVertexAttribL2dv;
    glad_debug_glVertexAttribL3d = glad_glVertexAttribL3d;
    glad_debug_glVertexAttribL3dv = glad_glVertexAttribL3dv;
    glad_debug_glVertexAttribL4d = glad_glVertexAttribL4d;
    glad_debug_glVertexAttribL4dv = glad_glVertexAttribL4dv;
    glad_debug_glVertexAttribLPointer = glad_glVertexAttribLPointer;
    glad_debug_glVertexAttribP1ui = glad_glVertexAttribP1ui;
    glad_debug_glVertexAttribP1uiv = glad_glVertexAttribP1uiv;
    glad_debug_glVertexAttribP2ui = glad_glVertexAttribP2ui;
    glad_debug_glVertexAttribP2uiv = glad_glVertexAttribP2uiv;
    glad_debug_glVertexAttribP3ui = glad_glVertexAttribP3ui;
    glad_debug_glVertexAttribP3uiv = glad_glVertexAttribP3uiv;
    glad_debug_glVertexAttribP4ui = glad_glVertexAttribP4ui;
    glad_debug_glVertexAttribP4uiv = glad_glVertexAttribP4uiv;
    glad_debug_glVertexAttribPointer = glad_glVertexAttribPointer;
    glad_debug_glViewport = glad_glViewport;
    glad_debug_glViewportArrayv = glad_glViewportArrayv;
    glad_debug_glViewportIndexedf = glad_glViewportIndexedf;
    glad_debug_glViewportIndexedfv = glad_glViewportIndexedfv;
    glad_debug_glWaitSync = glad_glWaitSync;
}

#ifdef GLAD_GL

#ifndef GLAD_LOADER_LIBRARY_C_
#define GLAD_LOADER_LIBRARY_C_

#include <stddef.h>
#include <stdlib.h>

#if GLAD_PLATFORM_WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


static void* glad_get_dlopen_handle(const char *lib_names[], int length) {
    void *handle = NULL;
    int i;

    for (i = 0; i < length; ++i) {
#if GLAD_PLATFORM_WIN32
  #if GLAD_PLATFORM_UWP
        size_t buffer_size = (strlen(lib_names[i]) + 1) * sizeof(WCHAR);
        LPWSTR buffer = (LPWSTR) malloc(buffer_size);
        if (buffer != NULL) {
            int ret = MultiByteToWideChar(CP_ACP, 0, lib_names[i], -1, buffer, buffer_size);
            if (ret != 0) {
                handle = (void*) LoadPackagedLibrary(buffer, 0);
            }
            free((void*) buffer);
        }
  #else
        handle = (void*) LoadLibraryA(lib_names[i]);
  #endif
#else
        handle = dlopen(lib_names[i], RTLD_LAZY | RTLD_LOCAL);
#endif
        if (handle != NULL) {
            return handle;
        }
    }

    return NULL;
}

static void glad_close_dlopen_handle(void* handle) {
    if (handle != NULL) {
#if GLAD_PLATFORM_WIN32
        FreeLibrary((HMODULE) handle);
#else
        dlclose(handle);
#endif
    }
}

static GLADapiproc glad_dlsym_handle(void* handle, const char *name) {
    if (handle == NULL) {
        return NULL;
    }

#if GLAD_PLATFORM_WIN32
    return (GLADapiproc) GetProcAddress((HMODULE) handle, name);
#else
    return GLAD_GNUC_EXTENSION (GLADapiproc) dlsym(handle, name);
#endif
}

#endif /* GLAD_LOADER_LIBRARY_C_ */

typedef void* (GLAD_API_PTR *GLADglprocaddrfunc)(const char*);
struct _glad_gl_userptr {
    void *handle;
    GLADglprocaddrfunc gl_get_proc_address_ptr;
};

static GLADapiproc glad_gl_get_proc(void *vuserptr, const char *name) {
    struct _glad_gl_userptr userptr = *(struct _glad_gl_userptr*) vuserptr;
    GLADapiproc result = NULL;

    if(userptr.gl_get_proc_address_ptr != NULL) {
        result = GLAD_GNUC_EXTENSION (GLADapiproc) userptr.gl_get_proc_address_ptr(name);
    }
    if(result == NULL) {
        result = glad_dlsym_handle(userptr.handle, name);
    }

    return result;
}

static void* _glad_GL_loader_handle = NULL;

static void* glad_gl_dlopen_handle(void) {
#if GLAD_PLATFORM_APPLE
    static const char *NAMES[] = {
        "../Frameworks/OpenGL.framework/OpenGL",
        "/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/OpenGL",
        "/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL"
    };
#elif GLAD_PLATFORM_WIN32
    static const char *NAMES[] = {"opengl32.dll"};
#else
    static const char *NAMES[] = {
  #if defined(__CYGWIN__)
        "libGL-1.so",
  #endif
        "libGL.so.1",
        "libGL.so"
    };
#endif

    if (_glad_GL_loader_handle == NULL) {
        _glad_GL_loader_handle = glad_get_dlopen_handle(NAMES, sizeof(NAMES) / sizeof(NAMES[0]));
    }

    return _glad_GL_loader_handle;
}

static struct _glad_gl_userptr glad_gl_build_userptr(void *handle) {
    struct _glad_gl_userptr userptr;

    userptr.handle = handle;
#if GLAD_PLATFORM_APPLE || defined(__HAIKU__)
    userptr.gl_get_proc_address_ptr = NULL;
#elif GLAD_PLATFORM_WIN32
    userptr.gl_get_proc_address_ptr =
        (GLADglprocaddrfunc) glad_dlsym_handle(handle, "wglGetProcAddress");
#else
    userptr.gl_get_proc_address_ptr =
        (GLADglprocaddrfunc) glad_dlsym_handle(handle, "glXGetProcAddressARB");
#endif

    return userptr;
}

int gladLoaderLoadGL(void) {
    int version = 0;
    void *handle;
    int did_load = 0;
    struct _glad_gl_userptr userptr;

    did_load = _glad_GL_loader_handle == NULL;
    handle = glad_gl_dlopen_handle();
    if (handle) {
        userptr = glad_gl_build_userptr(handle);

        version = gladLoadGLUserPtr(glad_gl_get_proc, &userptr);

        if (did_load) {
            gladLoaderUnloadGL();
        }
    }

    return version;
}



void gladLoaderUnloadGL(void) {
    if (_glad_GL_loader_handle != NULL) {
        glad_close_dlopen_handle(_glad_GL_loader_handle);
        _glad_GL_loader_handle = NULL;
    }
}

#endif /* GLAD_GL */

#ifdef __cplusplus
}
#endif
