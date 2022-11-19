
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
        auto height = fontMetrics.lineHeight;

        // Set the projection matrix
        render->setMatrix( matrix );

        // Copy the text to process it
        txt = text;

        int x = pts[0].x;
        int y = pts[0].y;
        math::Vector2i pnt( x, y );
        std::size_t pos = txt.find('\n');
        for ( ; pos != std::string::npos; y += height, pos = txt.find('\n') )
        {
            pnt.y = y;
            std::string line = txt.substr( 0, pos );
            const auto glyphs = fontSystem->getGlyphs( line, fontInfo );
            render->drawText( glyphs, pnt, color );
            if ( txt.size() > pos ) txt = txt.substr( pos+1, txt.size() );
        }
        if ( !txt.empty() )
        {
            pnt.y = y;
            const auto glyphs = fontSystem->getGlyphs( txt, fontInfo );
            render->drawText( glyphs, pnt, color );
        }
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
