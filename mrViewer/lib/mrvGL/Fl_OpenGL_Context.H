#pragma once

#include <FL/platform.H>

// Creates a new opengl context that gets destroyed upon exit
class OpenGLContext
{
public:
    OpenGLContext();
    ~OpenGLContext();

protected:
    bool m_destroy;
    GLContext contextObject;
};
