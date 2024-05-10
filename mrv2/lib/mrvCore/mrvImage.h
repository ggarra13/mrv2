#pragma once


namespace mrv
{
    
    inline
    void flipImageInY(uint8_t* pixels, const size_t width, const size_t height,
                      const int depth)
    {
        const size_t rowSize = width * depth;
        for (size_t y = 0; y < height / 2; ++y)
        {
            const size_t topRow = y * rowSize;
            const size_t bottomRow = (height - y - 1) * rowSize;

            for (size_t i = 0; i < rowSize; ++i)
            {
                const uint8_t temp = pixels[topRow + i];
                pixels[topRow + i] = pixels[bottomRow + i];
                pixels[bottomRow + i] = temp;
            }
        }
    }

}
