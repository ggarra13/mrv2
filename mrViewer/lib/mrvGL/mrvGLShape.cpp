
#include "mrvGLUtil.h"
#include "mrvGLShape.h"

#include <tlGlad/gl.h>

#include <glm/gtc/matrix_transform.hpp>

namespace mrv
{
    void GLPathShape::draw(
        const std::shared_ptr<timeline::IRender>& render )
    {
        using namespace tl::draw;
        
        // Turn on Color Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    
        drawLines( render, pts, color, pen_size, matrix,
                   Polyline2D::JointStyle::ROUND,
                   Polyline2D::EndCapStyle::ROUND );
    }
    
    void GLCircleShape::draw(
        const std::shared_ptr<timeline::IRender>& render )
    {
        // Turn on Color Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        
        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        
        drawCursor( render, center, radius, pen_size, color, matrix );
    }
    
    void GLTextShape::draw(
        const std::shared_ptr<timeline::IRender>& render )
    {
        if ( text.empty() ) return;
        
        // Turn on Color Buffer
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        
        // Only write to the Stencil Buffer where 1 is not set
        glStencilFunc(GL_NOTEQUAL, 1, 0xFFFFFFFF);
        // Keep the content of the Stencil Buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

        static const std::string fontFamily = "NotoSans-Regular";

        const imaging::FontInfo fontInfo(fontFamily, fontSize);
        const imaging::FontMetrics fontMetrics =
            fontSystem->getMetrics(fontInfo);
        const auto& glyphs = fontSystem->getGlyphs( text, fontInfo );

        math::Vector2i pnt( pts[0].x, pts[0].y );
        render->setMatrix( matrix );
        render->drawText( glyphs, pnt, color );
    }

    void GLErasePathShape::draw(
        const std::shared_ptr<timeline::IRender>& render )
    {
        using namespace tl::draw;
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        //Set 1 into the stencil buffer
        glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFF);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        
        drawLines( render, pts, color, pen_size, matrix,
                   Polyline2D::JointStyle::ROUND,
                   Polyline2D::EndCapStyle::ROUND );
    }
}
