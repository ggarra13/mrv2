#pragma once

#include <float.h>
#include <limits.h>
#include <cmath>
#include <vector>
#include <iostream>

#include "mrvPoint.h"

#include <tlCore/Vector.h>
#include <tlCore/Matrix.h>

#include <tlTimeline/IRender.h>


namespace tl
{

    namespace draw
    {



        class Shape
        {
        public:
            Shape() : color( 0.F, 1.F, 0.F, 1.F ),
                      pen_size(5)
                {
                };

            virtual ~Shape() {};

            virtual void draw(
                const timeline::IRender&,
                const math::Matrix4x4f&) = 0;

        public:
            imaging::Color4f color;
            float pen_size;
        };

        class CircleShape : public Shape
        {
        public:
            CircleShape() : Shape(), radius(1.0)  {};
            virtual ~CircleShape() {};

            virtual void draw(
                const timeline::IRender&,
                const math::Matrix4x4f&) = 0;
    
            math::Vector3f center;
            double radius;
        };

        class PathShape : public Shape
        {
        public:

            PathShape() : Shape()  {};
            virtual ~PathShape() {};
    
            virtual void draw(
                const timeline::IRender&,
                const math::Matrix4x4f&) = 0;

            PointList pts;
        };

        class ArrowShape : public PathShape
        {
        public:
            ArrowShape() : PathShape()  {};
            virtual ~ArrowShape() {};
    
            virtual void draw(
                const timeline::IRender&,
                const math::Matrix4x4f&) = 0;
        };

        class RectangleShape : public PathShape
        {
        public:

            RectangleShape() : PathShape()  {};
            virtual ~RectangleShape() {};
    
            virtual void draw(
                const timeline::IRender&,
                const math::Matrix4x4f&) = 0;
        };

        class ErasePathShape : public PathShape
        {
        public:

            ErasePathShape() : PathShape()  {};
            virtual ~ErasePathShape() {};
    
            virtual void draw(
                const timeline::IRender&,
                const math::Matrix4x4f&) = 0;
        };


    }
}
