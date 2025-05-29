
namespace tl
{
    namespace timeline_vlk
    {
        // Texture sampling options
        struct UBOTexture
        {
            alignas(16) math::Vector4f yuvCoefficients;
            alignas(16) image::Color4f color;
            alignas(4)  int32_t pixelType;
            alignas(4)  int32_t videoLevels;
            alignas(4)  int32_t imageChannels;
            alignas(4)  int32_t mirrorX;
            alignas(4)  int32_t mirrorY;
        };
        
        // Levels
        struct UBOLevels
        {
            alignas(4) uint32_t enabled = 0;
            alignas(4) float inLow = 0.F;
            alignas(4) float inHigh = 1.F;
            alignas(4) float gamma = 1.F;
            alignas(4) float outLow = 0.F;
            alignas(4) float outHigh = 1.F;
        };

        // Color
        struct UBOColor
        {
            alignas(4)  uint32_t enabled;
            alignas(16) math::Vector3f add;
            alignas(16) math::Matrix4x4f matrix;
            alignas(4)  uint32_t invert;
        };
        
        // Normalize
        struct UBONormalize
        {
            alignas(4) uint32_t enabled = 0;
            alignas(16) math::Vector4f minimum = math::Vector4f(0.F, 0.F, 0.F, 0.F);
            alignas(16) math::Vector4f maximum = math::Vector4f(1.F, 1.F, 1.F, 1.F);
        };
        
        struct UBOOptions
        {
            alignas(4) int   channels;
            alignas(4) int   mirrorX;
            alignas(4) int   mirrorY;
            alignas(4) float softClip;
            alignas(4) int   videoLevels;
            alignas(4) int   invalidValues;
        };
    }
}
