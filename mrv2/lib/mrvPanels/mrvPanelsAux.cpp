
#include "mrvPanelsAux.h"

#include "mrViewer.h"

namespace mrv
{

    std::string getLayerName(int layerId, ViewerUI* ui)
    {
        std::string layer;
        if (layerId >= 0 && layerId < ui->uiColorChannel->children())
        {
            layer = "\n";
            assert(ui->uiColorChannel->child(layerId)->label() != nullptr);
            layer += ui->uiColorChannel->child(layerId)->label();
        }
        return layer;
    }

} // namespace mrv
