#ifndef GEOMETRY_HELPERS_INCLUDE_H
#define GEOMETRY_HELPERS_INCLUDE_H

#include <srl.hpp>
#include "saturn.hpp"

namespace GeometryHelpers
{

class Quad
{
public:
    static void setup(s16 x, 
                      s16 y, 
                      s16 width, 
                      s16 height, 
                      SRL::Math::Types::Vector2D points[4])
    {
        SRL::Math::Types::Fxp fx = (s16)(x - (s16)(HALF_SCREEN_WIDTH));
        SRL::Math::Types::Fxp fy = (s16)(y - (s16)(HALF_SCREEN_HEIGHT)); 
        SRL::Math::Types::Fxp fwidth = (s16)(width - 1);
        SRL::Math::Types::Fxp fheight = (s16)(height - 1);

        points[0].X = fx;
        points[0].Y = fy;
        points[1].X = fx + fwidth;
        points[1].Y = fy;
        points[2].X = fx + fwidth;
        points[2].Y = fy + fheight;
        points[3].X = fx;
        points[3].Y = fy + fheight;
    }
};
}
#endif