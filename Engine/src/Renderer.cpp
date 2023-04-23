#include "Renderer.h"
#include "Log.h"
#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    /*while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ") : " << function << " " << file << " : " << line << std::endl;
        return false;
    }
    return true;*/

    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR)
    {
        // Process/log the error.
        LOG_CORE_WARN("[OpenGL Error] ({0}) : {1} {2} : {3}", error, function, file, line);
    }
    return false;
}

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
        case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        LOG_CORE_ERROR("[OpenGL Error] {0} | {1} ({2})", error, file, line);
    }
    return errorCode;
}