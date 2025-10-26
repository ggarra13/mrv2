
#include <tlCore/Image.h>

namespace mrv
{
    using namespace tl;
    
    /** 
     * Function to convert an input image to an output image of different
     * format.  Handles RGB->RGBA pixel conversion.  Images have to have the
     * the same scale.
     * 
     * @param outputImage 
     * @param inputImage 
     */
    void convertImage(std::shared_ptr<image::Image>& outputImage,
                      std::shared_ptr<image::Image>& inputImage);
}
