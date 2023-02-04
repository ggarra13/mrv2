
namespace mrv
{
    inline bool EnvironmentMapOptions::operator==(
        const EnvironmentMapOptions& b ) const
    {
        return ( type == b.type &&
                 horizontalAperture == b.horizontalAperture &&
                 verticalAperture == b.verticalAperture &&
                 focalLength == b.focalLength &&
                 rotateX && b.rotateX &&
                 rotateY && b.rotateX ) ;
    }

    inline bool EnvironmentMapOptions::operator!=(
        const EnvironmentMapOptions& b ) const
    {
        return !(*this == b);
    }
}
