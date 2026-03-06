
namespace tl
{
    namespace timeline
    {
        
        inline bool
        ShaderOptions::operator==(const ShaderOptions& other) const
        {
            return debanding == other.debanding;
        }

        inline bool
        ShaderOptions::operator!=(const ShaderOptions& other) const
        {
            return !(*this == other);
        }
        
    } // namespace timeline
} // namespace tl
