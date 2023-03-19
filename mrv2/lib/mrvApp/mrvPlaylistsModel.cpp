
#include "mrvApp/mrvPlaylistsModel.h"

namespace mrv
{
    struct PlaylistModel::Private
    {
        std::weak_ptr<system::Context> context;
        std::shared_ptr<observer::List<std::shared_ptr<Playlist> > >
            playlists;
    };

    void PlaylistModel::_init(const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();

        p.context = context;

        p.playlists = observer::List<std::shared_ptr<Playlist> >::create();
        
    }

    PlaylistModel::PlaylistModel() :
        _p(new Private)
    {
    }

    PlaylistModel::~PlaylistModel() {}

    std::shared_ptr<PlaylistModel>
    PlaylistModel::create(const std::shared_ptr<system::Context>& context)
    {
        auto out = std::shared_ptr<PlaylistModel>(new PlaylistModel);
        out->_init(context);
        return out;
    }

    std::shared_ptr<observer::IList<std::shared_ptr<Playlist> > >
    PlaylistModel::observePlaylists() const
    {
        return _p->playlists;
    }

} // namspace mrv
