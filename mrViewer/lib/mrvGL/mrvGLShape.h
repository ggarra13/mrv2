#pragma once

#include <float.h>
#include <limits.h>
#include <cmath>
#include <vector>
#include <iostream>

#include <mrvDraw/Shape.h>

#include <tlCore/Matrix.h>


namespace mrv
{

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
