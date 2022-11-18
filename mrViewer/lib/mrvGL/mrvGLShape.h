#pragma once

#include <float.h>
#include <limits.h>
#include <cmath>
#include <vector>
#include <iostream>

#include <FL/fl_draw.H>

#include <mrvDraw/Shape.h>

#include <tlCore/Matrix.h>

#include <tlTimeline/IRender.h>

namespace mrv
{
    using namespace tl;
    
    class GLShape : public tl::draw::Shape
    {
    public:
        GLShape() : tl::draw::Shape() {}
        virtual ~GLShape() {};
    
        virtual void draw(
            const std::shared_ptr<timeline::IRender>&) = 0;
    };

    class GLCircleShape : public GLShape
    {
    public:
        GLCircleShape() : GLShape(), radius(1.0)  {};
        virtual ~GLCircleShape() {};

        void draw(
            const std::shared_ptr<timeline::IRender>&) override;
    
        math::Vector2i center;
        double radius;
    };

    class GLPathShape : public tl::draw::PathShape
    {
    public:
        GLPathShape() : tl::draw::PathShape()  {};
        virtual ~GLPathShape() {};
    
        void draw(
            const std::shared_ptr<timeline::IRender>&) override;
    };

    class GLArrowShape : public GLPathShape
    {
    public:
        GLArrowShape() : GLPathShape()  {};
        virtual ~GLArrowShape() {};
    };

// class RectangleShape : public GLPathShape
// {
// public:

//     RectangleShape() : PathShape()  {};
//     virtual ~RectangleShape() {};
    
// };

    class GLTextShape : public GLPathShape
    {
    public:
        GLTextShape( const std::shared_ptr<imaging::FontSystem> f ) :
            GLPathShape(),
            fontSize( 30 ),
            fontSystem( f )
            {};
        virtual ~GLTextShape() {};

        void draw(
            const std::shared_ptr<timeline::IRender>&) override;

    public:
        std::string text;
        uint16_t    fontSize;
        std::shared_ptr<imaging::FontSystem> fontSystem;
    };

    class GL2TextShape : public GLPathShape
    {
    public:
        GL2TextShape() :
            GLPathShape(),
            fontSize( 30 )
            {};
        virtual ~GL2TextShape() {};

        void draw(
            const std::shared_ptr<timeline::IRender>&) override;

    public:
        std::string text;
        Fl_Font     font;
        uint16_t    fontSize;
        double      m = 1.F;
        double      zoom = 1.F;
    };
    
    
    class GLErasePathShape : public GLPathShape
    {
    public:

        GLErasePathShape() : GLPathShape()  {};
        virtual ~GLErasePathShape() {};
    
        void draw(
            const std::shared_ptr<timeline::IRender>&) override;
    };


    typedef std::vector< std::shared_ptr< tl::draw::Shape > >    ShapeList;
}
