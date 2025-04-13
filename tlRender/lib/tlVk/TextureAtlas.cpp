// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlVk/TextureAtlas.h>

#include <tlVk/Texture.h>

namespace tl
{
    namespace vk
    {
        namespace
        {
            //! Timestamp.
            typedef uint64_t Timestamp;

            //! Timestamp manager.
            class TimestampManager
            {
            private:
                TimestampManager();

            public:
                static std::shared_ptr<TimestampManager> create();

                Timestamp getNext();

            private:
                Timestamp _time = 0;
            };

            TimestampManager::TimestampManager() {}

            std::shared_ptr<TimestampManager> TimestampManager::create()
            {
                return std::shared_ptr<TimestampManager>(new TimestampManager);
            }

            Timestamp TimestampManager::getNext()
            {
                ++_time;
                return _time;
            }

            //! Box packing node.
            //!
            //! References:
            //! - http://blackpawn.com/texts/lightmaps/
            class BoxPackingNode
                : public std::enable_shared_from_this<BoxPackingNode>
            {
            private:
                BoxPackingNode(
                    int border, const std::shared_ptr<TimestampManager>&);

            public:
                static std::shared_ptr<BoxPackingNode>
                create(int border, const std::shared_ptr<TimestampManager>&);

                TextureAtlasID id = 0;
                std::shared_ptr<TimestampManager> timestampManager;
                uint64_t timestamp = 0;

                math::Box2i box;
                uint8_t border = 0;
                uint8_t textureIndex = 0;
                std::shared_ptr<BoxPackingNode> children[2];

                bool isBranch() const { return children[0].get(); }
                bool isOccupied() const { return id != 0; }

                std::shared_ptr<BoxPackingNode>
                insert(const std::shared_ptr<image::Image>&);
            };

            BoxPackingNode::BoxPackingNode(
                int border,
                const std::shared_ptr<TimestampManager>& timestampManager) :
                timestampManager(timestampManager),
                timestamp(timestampManager->getNext()),
                border(border)
            {
            }

            std::shared_ptr<BoxPackingNode> BoxPackingNode::create(
                int border,
                const std::shared_ptr<TimestampManager>& timestampManager)
            {
                auto out = std::shared_ptr<BoxPackingNode>(
                    new BoxPackingNode(border, timestampManager));
                return out;
            }

            std::shared_ptr<BoxPackingNode>
            BoxPackingNode::insert(const std::shared_ptr<image::Image>& image)
            {
                if (isBranch())
                {
                    if (auto node = children[0]->insert(image))
                    {
                        return node;
                    }
                    return children[1]->insert(image);
                }
                else if (isOccupied())
                {
                    return nullptr;
                }
                else
                {
                    const math::Size2i dataSize =
                        math::Size2i(image->getWidth(), image->getHeight()) +
                        border * 2;
                    const math::Size2i boxSize = box.getSize();
                    if (dataSize.w > boxSize.w || dataSize.h > boxSize.h)
                    {
                        return nullptr;
                    }
                    if (dataSize == boxSize)
                    {
                        return shared_from_this();
                    }
                    children[0] = create(border, timestampManager);
                    children[1] = create(border, timestampManager);
                    const int dw = boxSize.w - dataSize.w;
                    const int dh = boxSize.h - dataSize.h;
                    if (dw > dh)
                    {
                        children[0]->box.min.x = box.min.x;
                        children[0]->box.min.y = box.min.y;
                        children[0]->box.max.x = box.min.x + dataSize.w - 1;
                        children[0]->box.max.y = box.max.y;
                        children[1]->box.min.x = box.min.x + dataSize.w;
                        children[1]->box.min.y = box.min.y;
                        children[1]->box.max.x = box.max.x;
                        children[1]->box.max.y = box.max.y;
                    }
                    else
                    {
                        children[0]->box.min.x = box.min.x;
                        children[0]->box.min.y = box.min.y;
                        children[0]->box.max.x = box.max.x;
                        children[0]->box.max.y = box.min.y + dataSize.h - 1;
                        children[1]->box.min.x = box.min.x;
                        children[1]->box.min.y = box.min.y + dataSize.h;
                        children[1]->box.max.x = box.max.x;
                        children[1]->box.max.y = box.max.y;
                    }
                    children[0]->textureIndex = textureIndex;
                    children[1]->textureIndex = textureIndex;
                    return children[0]->insert(image);
                }
            }
        } // namespace

        struct TextureAtlas::Private
        {
            size_t textureCount = 0;
            int textureSize = 0;
            image::PixelType textureType = image::PixelType::None;
            int border = 0;
            TextureAtlasID id = 0;
            std::vector<std::shared_ptr<Texture> > textures;
            std::vector<std::shared_ptr<BoxPackingNode> > boxPackingNodes;
            std::map<TextureAtlasID, std::shared_ptr<BoxPackingNode> > cache;
            std::shared_ptr<TimestampManager> timestampManager;

            void getAllNodes(
                const std::shared_ptr<BoxPackingNode>&,
                std::vector<std::shared_ptr<BoxPackingNode> >&);
            void getLeafNodes(
                const std::shared_ptr<BoxPackingNode>&,
                std::vector<std::shared_ptr<BoxPackingNode> >&) const;
            void toTextureAtlasItem(
                const std::shared_ptr<BoxPackingNode>&, TextureAtlasItem&);
            void removeFromAtlas(const std::shared_ptr<BoxPackingNode>&);
        };

        void TextureAtlas::_init(
            size_t textureCount, int textureSize, image::PixelType textureType,
            timeline::ImageFilter filter, int border)
        {
            TLRENDER_P();

            p.textureCount = textureCount;
            p.textureSize = textureSize;
            p.textureType = textureType;
            p.border = border;
            p.timestampManager = TimestampManager::create();

            for (uint8_t i = 0; i < p.textureCount; ++i)
            {
                TextureOptions textureOptions;
                textureOptions.filters.minify = filter;
                textureOptions.filters.magnify = filter;
                auto texture = Texture::create(
                    image::Info(textureSize, textureSize, textureType),
                    textureOptions);
                p.textures.push_back(texture);

                auto node = BoxPackingNode::create(border, p.timestampManager);
                node->box.min.x = 0;
                node->box.min.y = 0;
                node->box.max.x = textureSize - 1;
                node->box.max.y = textureSize - 1;
                node->textureIndex = i;
                p.boxPackingNodes.push_back(node);
            }
        }

        TextureAtlas::TextureAtlas() :
            _p(new Private)
        {
        }

        TextureAtlas::~TextureAtlas() {}

        std::shared_ptr<TextureAtlas> TextureAtlas::create(
            size_t textureCount, int textureSize, image::PixelType textureType,
            timeline::ImageFilter filter, int border)
        {
            auto out = std::shared_ptr<TextureAtlas>(new TextureAtlas);
            out->_init(textureCount, textureSize, textureType, filter, border);
            return out;
        }

        size_t TextureAtlas::getTextureCount() const
        {
            return _p->textureCount;
        }

        int TextureAtlas::getTextureSize() const
        {
            return _p->textureSize;
        }

        image::PixelType TextureAtlas::getTextureType() const
        {
            return _p->textureType;
        }

        std::vector<unsigned int> TextureAtlas::getTextures() const
        {
            TLRENDER_P();
            std::vector<unsigned int> out;
            for (const auto& i : p.textures)
            {
                out.push_back(i->getID());
            }
            return out;
        }

        bool TextureAtlas::getItem(TextureAtlasID id, TextureAtlasItem& out)
        {
            TLRENDER_P();
            const auto& i = p.cache.find(id);
            if (i != p.cache.end())
            {
                p.toTextureAtlasItem(i->second, out);
                i->second->timestamp = p.timestampManager->getNext();
                return true;
            }
            return false;
        }

        TextureAtlasID TextureAtlas::addItem(
            const std::shared_ptr<image::Image>& image, TextureAtlasItem& out)
        {
            TLRENDER_P();

            for (uint8_t i = 0; i < p.textureCount; ++i)
            {
                if (auto node = p.boxPackingNodes[i]->insert(image))
                {
                    // The data has been added to the atlas.
                    p.id = p.id + 1;
                    node->id = p.id;
                    p.textures[node->textureIndex]->copy(
                        image, node->box.min.x + p.border,
                        node->box.min.y + p.border);
                    p.cache[node->id] = node;
                    p.toTextureAtlasItem(node, out);
                    return node->id;
                }
            }

            // The atlas is full, over-write older data.
            std::vector<std::shared_ptr<BoxPackingNode> > nodes;
            for (uint8_t i = 0; i < p.textureCount; ++i)
            {
                p.getAllNodes(p.boxPackingNodes[i], nodes);
            }
            std::sort(
                nodes.begin(), nodes.end(),
                [](const std::shared_ptr<BoxPackingNode>& a,
                   const std::shared_ptr<BoxPackingNode>& b)
                {
                    const int aArea = a->box.getSize().getArea();
                    const int bArea = b->box.getSize().getArea();
                    return std::tie(aArea, a->timestamp) <
                           std::tie(bArea, b->timestamp);
                });
            const math::Size2i dataSize =
                math::Size2i(image->getWidth(), image->getHeight()) +
                p.border * 2;
            for (auto node : nodes)
            {
                const math::Size2i nodeSize = node->box.getSize();
                if (dataSize.w <= nodeSize.w && dataSize.h <= nodeSize.h)
                {
                    auto old = node;
                    p.removeFromAtlas(old);
                    if (old->isBranch())
                    {
                        for (uint8_t i = 0; i < 2; ++i)
                        {
                            old->children[i].reset();
                        }
                    }
                    old->id = 0;
                    old->timestamp = p.timestampManager->getNext();
                    if (auto node2 = old->insert(image))
                    {
                        p.id = p.id + 1;
                        node2->id = p.id;

                        //! \todo Do we need to zero out the old data?
                        // auto zero =
                        // Image::Data::create(Image::Info(data->getSize() +
                        // p.border * 2, p.textureType)); zero->zero();
                        // p.textures[node2->texture]->copy(zero,
                        // node2->box.min);

                        p.textures[node2->textureIndex]->copy(
                            image, node2->box.min.x + p.border,
                            node2->box.min.y + p.border);
                        p.cache[node2->id] = node2;
                        p.toTextureAtlasItem(node2, out);

                        return node2->id;
                    }
                }
            }
            return 0;
        }

        float TextureAtlas::getPercentageUsed() const
        {
            TLRENDER_P();
            float out = 0.F;
            if (p.textureCount && p.textureSize)
            {
                for (uint8_t i = 0; i < p.textureCount; ++i)
                {
                    size_t used = 0;
                    std::vector<std::shared_ptr<BoxPackingNode> > leafs;
                    p.getLeafNodes(p.boxPackingNodes[i], leafs);
                    for (const auto& j : leafs)
                    {
                        if (j->isOccupied())
                        {
                            used += j->box.getSize().getArea();
                        }
                    }
                    out += static_cast<float>(used);
                }
                out /= static_cast<float>(p.textureSize * p.textureSize);
                out /= static_cast<float>(p.textureCount);
                out *= 100.F;
            }
            return out;
        }

        void TextureAtlas::Private::getAllNodes(
            const std::shared_ptr<BoxPackingNode>& node,
            std::vector<std::shared_ptr<BoxPackingNode> >& out)
        {
            out.push_back(node);
            if (node->isBranch())
            {
                getAllNodes(node->children[0], out);
                getAllNodes(node->children[1], out);
            }
        }

        void TextureAtlas::Private::getLeafNodes(
            const std::shared_ptr<BoxPackingNode>& node,
            std::vector<std::shared_ptr<BoxPackingNode> >& out) const
        {
            if (node->isBranch())
            {
                getLeafNodes(node->children[0], out);
                getLeafNodes(node->children[1], out);
            }
            else
            {
                out.push_back(node);
            }
        }

        void TextureAtlas::Private::toTextureAtlasItem(
            const std::shared_ptr<BoxPackingNode>& node, TextureAtlasItem& out)
        {
            out.w = node->box.w();
            out.h = node->box.h();
            out.textureIndex = node->textureIndex;
            out.textureU = math::FloatRange(
                (node->box.min.x + static_cast<float>(border)) /
                    static_cast<float>(textureSize),
                (node->box.max.x - static_cast<float>(border) + 1.F) /
                    static_cast<float>(textureSize));
            out.textureV = math::FloatRange(
                (node->box.min.y + static_cast<float>(border)) /
                    static_cast<float>(textureSize),
                (node->box.max.y - static_cast<float>(border) + 1.F) /
                    static_cast<float>(textureSize));
        }

        void TextureAtlas::Private::removeFromAtlas(
            const std::shared_ptr<BoxPackingNode>& node)
        {
            auto i = cache.find(node->id);
            if (i != cache.end())
            {
                node->id = 0;
                cache.erase(i);
            }
            if (node->isBranch())
            {
                removeFromAtlas(node->children[0]);
                removeFromAtlas(node->children[1]);
            }
        }
    } // namespace vk
} // namespace tl
