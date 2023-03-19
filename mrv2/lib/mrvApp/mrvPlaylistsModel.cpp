
#include "mrvApp/mrvPlaylistsModel.h"

namespace mrv
{
    struct PlaylistsModel::Private
    {
        std::weak_ptr<system::Context> context;
        std::shared_ptr<observer::List<std::shared_ptr<Playlist> > >
            playlists;
        std::shared_ptr<observer::Value<std::shared_ptr<Playlist> > > a;
        std::shared_ptr<observer::Value<int> > aIndex;
    };

    void PlaylistsModel::_init(const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();

        p.context = context;

        p.playlists = observer::List<std::shared_ptr<Playlist> >::create();
        p.a = observer::Value<std::shared_ptr<Playlist> >::create();
        p.aIndex = observer::Value<int>::create();
        
    }

    PlaylistsModel::PlaylistsModel() :
        _p(new Private)
    {
    }

    PlaylistsModel::~PlaylistsModel() {}

    std::shared_ptr<PlaylistsModel>
    PlaylistsModel::create(const std::shared_ptr<system::Context>& context)
    {
        auto out = std::shared_ptr<PlaylistsModel>(new PlaylistsModel);
        out->_init(context);
        return out;
    }

    std::shared_ptr<observer::IList<std::shared_ptr<Playlist> > >
    PlaylistsModel::observePlaylists() const
    {
        return _p->playlists;
    }

    void PlaylistsModel::add(const std::shared_ptr<Playlist>& item)
    {
        TLRENDER_P();

        p.playlists->pushBack(item);

        p.a->setIfChanged(p.playlists->getItem(p.playlists->getSize() - 1));
        p.aIndex->setIfChanged(_index(p.a->get()));
    }
    
    void PlaylistsModel::set(int index)
    {
    }
    
    void PlaylistsModel::close()
    {
    }

    int PlaylistsModel::_index(const std::shared_ptr<Playlist>& item) const
    {
        TLRENDER_P();
        size_t index = p.playlists->indexOf(item);
        return index != observer::invalidListIndex ? index : -1;
    }
    
} // namspace mrv
