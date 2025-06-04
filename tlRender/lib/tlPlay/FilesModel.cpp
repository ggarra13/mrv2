// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlay/FilesModel.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace play
    {
        struct FilesModel::Private
        {
            std::weak_ptr<system::Context> context;
            std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > >
                files;
            std::shared_ptr<observer::Value<std::shared_ptr<FilesModelItem> > >
                a;
            std::shared_ptr<observer::Value<int> > aIndex;
            std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > >
                b;
            std::shared_ptr<observer::List<int> > bIndexes;
            std::shared_ptr<observer::List<std::shared_ptr<FilesModelItem> > >
                active;
            std::shared_ptr<observer::List<int> > layers;
            std::shared_ptr<observer::Value<timeline::CompareOptions> >
                compareOptions;
            std::shared_ptr<observer::Value<timeline::CompareTimeMode> >
                compareTime;
        };

        void FilesModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;

            p.files =
                observer::List<std::shared_ptr<FilesModelItem> >::create();
            p.a = observer::Value<std::shared_ptr<FilesModelItem> >::create();
            p.aIndex = observer::Value<int>::create();
            p.b = observer::List<std::shared_ptr<FilesModelItem> >::create();
            p.bIndexes = observer::List<int>::create();
            p.active =
                observer::List<std::shared_ptr<FilesModelItem> >::create();
            p.layers = observer::List<int>::create();
            p.compareOptions =
                observer::Value<timeline::CompareOptions>::create();
            p.compareTime =
                observer::Value<timeline::CompareTimeMode>::create();
        }

        FilesModel::FilesModel() :
            _p(new Private)
        {
        }

        FilesModel::~FilesModel() {}

        std::shared_ptr<FilesModel>
        FilesModel::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<FilesModel>(new FilesModel);
            out->_init(context);
            return out;
        }

        const std::vector<std::shared_ptr<FilesModelItem> >&
        FilesModel::getFiles() const
        {
            return _p->files->get();
        }

        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
        FilesModel::observeFiles() const
        {
            return _p->files;
        }

        const std::shared_ptr<FilesModelItem>& FilesModel::getA() const
        {
            return _p->a->get();
        }

        std::shared_ptr<observer::IValue<std::shared_ptr<FilesModelItem> > >
        FilesModel::observeA() const
        {
            return _p->a;
        }

        int FilesModel::getAIndex() const
        {
            return _p->aIndex->get();
        }

        std::shared_ptr<observer::IValue<int> >
        FilesModel::observeAIndex() const
        {
            return _p->aIndex;
        }

        const std::vector<std::shared_ptr<FilesModelItem> >&
        FilesModel::getB() const
        {
            return _p->b->get();
        }

        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
        FilesModel::observeB() const
        {
            return _p->b;
        }

        const std::vector<int>& FilesModel::getBIndexes() const
        {
            return _p->bIndexes->get();
        }

        std::shared_ptr<observer::IList<int> >
        FilesModel::observeBIndexes() const
        {
            return _p->bIndexes;
        }

        const std::vector<std::shared_ptr<FilesModelItem> >&
        FilesModel::getActive() const
        {
            return _p->active->get();
        }

        std::shared_ptr<observer::IList<std::shared_ptr<FilesModelItem> > >
        FilesModel::observeActive() const
        {
            return _p->active;
        }

        void FilesModel::add(const std::shared_ptr<FilesModelItem>& item)
        {
            TLRENDER_P();

            p.files->pushBack(item);

            p.a->setIfChanged(p.files->getItem(p.files->getSize() - 1));
            p.aIndex->setIfChanged(_index(p.a->get()));

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::close()
        {
            TLRENDER_P();
            if (p.a->get())
            {
                auto files = p.files->get();
                const auto i =
                    std::find(files.begin(), files.end(), p.a->get());
                if (i != files.end())
                {
                    const int aPrevIndex = _index(p.a->get());

                    files.erase(i);
                    p.files->setIfChanged(files);

                    const int aNewIndex = math::clamp(
                        aPrevIndex, 0, static_cast<int>(files.size()) - 1);
                    p.a->setIfChanged(
                        aNewIndex != -1 ? files[aNewIndex] : nullptr);
                    p.aIndex->setIfChanged(_index(p.a->get()));

                    auto b = p.b->get();
                    auto j = b.begin();
                    while (j != b.end())
                    {
                        const auto k =
                            std::find(files.begin(), files.end(), *j);
                        if (k == files.end())
                        {
                            j = b.erase(j);
                        }
                        else
                        {
                            ++j;
                        }
                    }
                    p.b->setIfChanged(b);
                    p.bIndexes->setIfChanged(_bIndexes());

                    p.active->setIfChanged(_getActive());
                    p.layers->setIfChanged(_getLayers());
                }
            }
        }

        void FilesModel::closeAll()
        {
            TLRENDER_P();

            p.files->clear();

            p.a->setIfChanged(nullptr);
            p.aIndex->setIfChanged(-1);

            p.b->clear();
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::reload()
        {
            TLRENDER_P();

            p.a->setAlways(p.a->get());
            p.aIndex->setAlways(p.aIndex->get());

            p.b->setAlways(p.b->get());
            p.bIndexes->setAlways(_bIndexes());

            p.active->setAlways(_getActive());
            p.layers->setAlways(_getLayers());
        }

        void FilesModel::setA(int index)
        {
            TLRENDER_P();
            const int prevIndex = _index(p.a->get());
            if (index >= 0 && index < p.files->getSize() && index != prevIndex)
            {
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::setB(int index, bool value)
        {
            TLRENDER_P();
            if (index >= 0 && index < p.files->getSize())
            {
                auto b = p.b->get();
                int removedIndex = -1;
                const auto bIndexes = _bIndexes();
                const auto i =
                    std::find(bIndexes.begin(), bIndexes.end(), index);
                if (value && i == bIndexes.end())
                {
                    b.push_back(p.files->getItem(index));
                    switch (p.compareOptions->get().mode)
                    {
                    case timeline::CompareMode::A:
                    case timeline::CompareMode::B:
                    case timeline::CompareMode::Wipe:
                    case timeline::CompareMode::Overlay:
                    case timeline::CompareMode::Difference:
                    case timeline::CompareMode::Horizontal:
                    case timeline::CompareMode::Vertical:
                        if (b.size() > 1)
                        {
                            removedIndex = _index(b.front());
                            b.erase(b.begin());
                        }
                        break;
                    default:
                        break;
                    }
                }
                else if (!value && i != bIndexes.end())
                {
                    b.erase(b.begin() + (i - bIndexes.begin()));
                }
                p.b->setIfChanged(b);
                p.bIndexes->setIfChanged(_bIndexes());

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::toggleB(int index)
        {
            TLRENDER_P();
            if (index >= 0 && index < p.files->getSize())
            {
                const auto& item = p.files->getItem(index);
                setB(index, p.b->indexOf(item) == observer::invalidListIndex);
            }
        }

        void FilesModel::clearB()
        {
            TLRENDER_P();
            if (!p.b->isEmpty())
            {
                p.b->clear();
                p.bIndexes->setIfChanged(_bIndexes());

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::first()
        {
            TLRENDER_P();
            const int prevIndex = _index(p.a->get());
            if (!p.files->isEmpty() && prevIndex != 0)
            {
                p.a->setIfChanged(p.files->getItem(0));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::last()
        {
            TLRENDER_P();
            const int index = static_cast<int>(p.files->getSize()) - 1;
            const int prevIndex = _index(p.a->get());
            if (!p.files->isEmpty() && index != prevIndex)
            {
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::next()
        {
            TLRENDER_P();
            if (!p.files->isEmpty())
            {
                const int prevIndex = _index(p.a->get());
                int index = prevIndex + 1;
                if (index >= p.files->getSize())
                {
                    index = 0;
                }
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::prev()
        {
            TLRENDER_P();
            if (!p.files->isEmpty())
            {
                const int prevIndex = _index(p.a->get());
                int index = prevIndex - 1;
                if (index < 0)
                {
                    index = p.files->getSize() - 1;
                }
                p.a->setIfChanged(p.files->getItem(index));
                p.aIndex->setIfChanged(_index(p.a->get()));

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::firstB()
        {
            TLRENDER_P();

            p.b->clear();
            if (!p.files->isEmpty())
            {
                p.b->pushBack(p.files->getItem(0));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::lastB()
        {
            TLRENDER_P();

            p.b->clear();
            if (!p.files->isEmpty())
            {
                p.b->pushBack(p.files->getItem(p.files->getSize() - 1));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::nextB()
        {
            TLRENDER_P();

            int index = 0;
            const auto bIndexes = _bIndexes();
            if (!bIndexes.empty())
            {
                index = bIndexes[bIndexes.size() - 1];
                ++index;
            }
            if (index >= p.files->getSize())
            {
                index = 0;
            }
            p.b->clear();
            if (index >= 0 && index <= p.files->getSize())
            {
                p.b->pushBack(p.files->getItem(index));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        void FilesModel::prevB()
        {
            TLRENDER_P();

            int index = 0;
            const auto bIndexes = _bIndexes();
            if (!bIndexes.empty())
            {
                index = bIndexes[0];
                --index;
            }
            if (index < 0)
            {
                index = static_cast<int>(p.files->getSize()) - 1;
            }
            p.b->clear();
            if (index >= 0 && index <= p.files->getSize())
            {
                p.b->pushBack(p.files->getItem(index));
            }
            p.bIndexes->setIfChanged(_bIndexes());

            p.active->setIfChanged(_getActive());
            p.layers->setIfChanged(_getLayers());
        }

        std::shared_ptr<observer::IList<int> > FilesModel::observeLayers() const
        {
            return _p->layers;
        }

        void FilesModel::setLayer(
            const std::shared_ptr<FilesModelItem>& item, int layer)
        {
            TLRENDER_P();
            const int index = _index(item);
            if (index != -1 &&
                layer < p.files->getItem(index)->videoLayers.size() &&
                layer != p.files->getItem(index)->videoLayer)
            {
                p.files->getItem(index)->videoLayer = layer;
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::nextLayer()
        {
            TLRENDER_P();
            const int index = _index(p.a->get());
            if (index != -1)
            {
                auto item = p.files->getItem(index);
                int layer = item->videoLayer + 1;
                if (layer >= item->videoLayers.size())
                {
                    layer = 0;
                }
                item->videoLayer = layer;
                p.layers->setIfChanged(_getLayers());
            }
        }

        void FilesModel::prevLayer()
        {
            TLRENDER_P();
            const int index = _index(p.a->get());
            if (index != -1)
            {
                auto item = p.files->getItem(index);
                int layer = item->videoLayer - 1;
                if (layer < 0)
                {
                    layer = static_cast<int>(item->videoLayers.size()) - 1;
                }
                item->videoLayer = std::max(layer, 0);
                p.layers->setIfChanged(_getLayers());
            }
        }

        const timeline::CompareOptions& FilesModel::getCompareOptions() const
        {
            return _p->compareOptions->get();
        }

        std::shared_ptr<observer::IValue<timeline::CompareOptions> >
        FilesModel::observeCompareOptions() const
        {
            return _p->compareOptions;
        }

        void
        FilesModel::setCompareOptions(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (p.compareOptions->setIfChanged(value))
            {
                switch (p.compareOptions->get().mode)
                {
                case timeline::CompareMode::A:
                case timeline::CompareMode::B:
                case timeline::CompareMode::Wipe:
                case timeline::CompareMode::Overlay:
                case timeline::CompareMode::Difference:
                case timeline::CompareMode::Horizontal:
                case timeline::CompareMode::Vertical:
                {
                    auto b = p.b->get();
                    while (b.size() > 1)
                    {
                        b.pop_back();
                    }
                    if (p.b->setIfChanged(b))
                    {
                        p.bIndexes->setIfChanged(_bIndexes());
                    }
                    break;
                }
                default:
                    break;
                }

                p.active->setIfChanged(_getActive());
                p.layers->setIfChanged(_getLayers());
            }
        }

        timeline::CompareTimeMode FilesModel::getCompareTime() const
        {
            return _p->compareTime->get();
        }

        std::shared_ptr<observer::IValue<timeline::CompareTimeMode> >
        FilesModel::observeCompareTime() const
        {
            return _p->compareTime;
        }

        void FilesModel::setCompareTime(timeline::CompareTimeMode value)
        {
            TLRENDER_P();
            p.compareTime->setIfChanged(value);
        }

        int
        FilesModel::_index(const std::shared_ptr<FilesModelItem>& item) const
        {
            TLRENDER_P();
            size_t index = p.files->indexOf(item);
            return index != observer::invalidListIndex ? index : -1;
        }

        std::vector<int> FilesModel::_bIndexes() const
        {
            TLRENDER_P();
            std::vector<int> out;
            for (const auto& b : p.b->get())
            {
                out.push_back(_index(b));
            }
            return out;
        }

        std::vector<std::shared_ptr<FilesModelItem> >
        FilesModel::_getActive() const
        {
            TLRENDER_P();
            std::vector<std::shared_ptr<FilesModelItem> > out;
            if (p.a->get())
            {
                out.push_back(p.a->get());
            }
            switch (p.compareOptions->get().mode)
            {
            case timeline::CompareMode::A:
                if (!p.b->isEmpty())
                {
                    out.push_back(p.b->getItem(0));
                }
                break;
            case timeline::CompareMode::B:
            case timeline::CompareMode::Wipe:
            case timeline::CompareMode::Overlay:
            case timeline::CompareMode::Difference:
            case timeline::CompareMode::Horizontal:
            case timeline::CompareMode::Vertical:
            case timeline::CompareMode::Tile:
                for (const auto& b : p.b->get())
                {
                    out.push_back(b);
                }
                break;
            default:
                break;
            }
            return out;
        }

        std::vector<int> FilesModel::_getLayers() const
        {
            TLRENDER_P();
            std::vector<int> out;
            for (const auto& f : p.files->get())
            {
                out.push_back(f->videoLayer);
            }
            return out;
        }
    } // namespace play
} // namespace tl
