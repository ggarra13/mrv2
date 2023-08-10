
#ifdef TLRENDER_USD

#    include <tlIO/USD.h>

namespace mrv
{
    namespace usd
    {
        //! Return the current USD Render Options
        tl::usd::RenderOptions renderOptions();

        //! Set the current USD Render Options
        bool setRenderOptions(const tl::usd::RenderOptions& o);
    } // namespace usd
} // namespace mrv

#endif
