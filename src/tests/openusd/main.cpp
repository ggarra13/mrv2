#define USE_GL 1

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>

#if USE_GL

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

#else

#include <FL/Fl_Vk_Window.H>

#endif

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/math.h>

#include <iostream>
#include <limits>
#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

class usd_window : public Fl_Gl_Window
{
public:
    usd_window(int X, int Y, int W, int H);

    void draw() override;
};

usd_window::usd_window(int X, int Y, int W, int H) :
    Fl_Gl_Window(X, Y, W, H)
{
    mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_STENCIL | FL_OPENGL3);
}

void usd_window::draw()
{
    if (!valid())
    {
        valid(1);
        glLoadIdentity();
        glViewport(0,0,pixel_w(),pixel_h());
    }
    glColor3f(1.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

int main(int argc, char **argv) {
    if (argc < 2)
    {
        std::cerr << argv[0] << " <file.usd>" << std::endl;
        exit(1);
    }
    
    Fl::use_high_res_GL(1);

    Fl_Window window(300, 330);
  
    usd_window sw(10, 10, 280, 280);

    window.end();
    window.show(argc,argv);
        
    return Fl::run();
}
