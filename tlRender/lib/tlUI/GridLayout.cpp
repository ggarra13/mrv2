// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/GridLayout.h>

#include <tlUI/LayoutUtil.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            struct GridPos
            {
                int row = 0;
                int column = 0;
            };
        } // namespace

        struct GridLayout::Private
        {
            std::map<std::shared_ptr<IWidget>, GridPos> gridPos;
            SizeRole marginRole = SizeRole::None;
            SizeRole spacingRole = SizeRole::Spacing;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int spacing = 0;
            };
            SizeData size;

            GridPos getSize() const;

            void getSizeHints(
                std::vector<int>& rows, std::vector<int>& columns) const;

            void getStretch(
                std::vector<bool>& rows, std::vector<bool>& columns) const;

            void getVisible(int& rows, int& columns) const;

            void getVisible(
                std::vector<bool>& rows, std::vector<bool>& columns) const;
        };

        void GridLayout::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::GridLayout", context, parent);
        }

        GridLayout::GridLayout() :
            _p(new Private)
        {
        }

        GridLayout::~GridLayout() {}

        std::shared_ptr<GridLayout> GridLayout::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<GridLayout>(new GridLayout);
            out->_init(context, parent);
            return out;
        }

        void GridLayout::setGridPos(
            const std::shared_ptr<IWidget>& child, int row, int column)
        {
            TLRENDER_P();
            p.gridPos[child].row = row;
            p.gridPos[child].column = column;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GridLayout::setMarginRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GridLayout::setSpacingRole(SizeRole value)
        {
            TLRENDER_P();
            if (value == p.spacingRole)
                return;
            p.spacingRole = value;
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        void GridLayout::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            TLRENDER_P();

            const math::Box2i g = _geometry.margin(-p.size.margin);

            // Get the child size hints.
            std::vector<int> rowSizeHints;
            std::vector<int> columnSizeHints;
            p.getSizeHints(rowSizeHints, columnSizeHints);

            // Get the total size.
            math::Vector2i totalSize;
            for (const auto& i : rowSizeHints)
            {
                totalSize.y += i;
            }
            for (const auto& i : columnSizeHints)
            {
                totalSize.x += i;
            }
            std::vector<bool> rowsVisible;
            std::vector<bool> columnsVisible;
            p.getVisible(rowsVisible, columnsVisible);
            int rowsVisibleCount = 0;
            int columnsVisibleCount = 0;
            p.getVisible(rowsVisibleCount, columnsVisibleCount);

            // Get the layout stretch.
            std::vector<bool> rowStretch;
            std::vector<bool> columnStretch;
            p.getStretch(rowStretch, columnStretch);

            // Get the layout stretch size.
            size_t rowStretchCount = 0;
            size_t columnStretchCount = 0;
            for (bool i : rowStretch)
            {
                if (i)
                {
                    ++rowStretchCount;
                }
            }
            for (bool i : columnStretch)
            {
                if (i)
                {
                    ++columnStretchCount;
                }
            }
            math::Vector2i stretchSize;
            if (rowStretchCount > 0)
            {
                stretchSize.y =
                    (g.h() - (rowsVisibleCount - 1) * p.size.spacing -
                     totalSize.y) /
                    rowStretchCount;
            }
            if (columnStretchCount > 0)
            {
                stretchSize.x =
                    (g.w() - (columnsVisibleCount - 1) * p.size.spacing -
                     totalSize.x) /
                    columnStretchCount;
            }

            // Get the sizes.
            std::vector<int> rowSizes;
            std::vector<int> columnSizes;
            for (int i = 0; i < rowSizeHints.size(); ++i)
            {
                int size = rowSizeHints[i];
                if (rowStretch[i])
                {
                    size += stretchSize.y;
                }
                rowSizes.push_back(size);
            }
            for (int i = 0; i < columnSizeHints.size(); ++i)
            {
                int size = columnSizeHints[i];
                if (columnStretch[i])
                {
                    size += stretchSize.x;
                }
                columnSizes.push_back(size);
            }

            // Layout the children.
            for (const auto& i : p.gridPos)
            {
                auto widget = i.first;
                const bool visible = widget->isVisible(false);
                math::Vector2i pos = g.min;
                for (int j = 0; j < i.second.row; ++j)
                {
                    if (rowsVisible[j])
                    {
                        pos.y += rowSizes[j] + (visible ? p.size.spacing : 0);
                    }
                }
                for (int j = 0; j < i.second.column; ++j)
                {
                    if (columnsVisible[j])
                    {
                        pos.x +=
                            columnSizes[j] + (visible ? p.size.spacing : 0);
                    }
                }
                const math::Vector2i size(
                    columnSizes[i.second.column], rowSizes[i.second.row]);
                widget->setGeometry(align(
                    math::Box2i(pos.x, pos.y, size.x, size.y),
                    widget->getSizeHint(), widget->getHStretch(),
                    widget->getVStretch(), widget->getHAlign(),
                    widget->getVAlign()));
            }
        }

        void GridLayout::sizeHintEvent(const SizeHintEvent& event)
        {
            const bool displayScaleChanged =
                event.displayScale != _displayScale;
            IWidget::sizeHintEvent(event);
            TLRENDER_P();

            if (displayScaleChanged || p.size.sizeInit)
            {
                p.size.margin =
                    event.style->getSizeRole(p.marginRole, _displayScale);
                p.size.spacing =
                    event.style->getSizeRole(p.spacingRole, _displayScale);
            }
            p.size.sizeInit = false;

            // Get size hints.
            std::vector<int> rowSizeHints;
            std::vector<int> columnSizeHints;
            p.getSizeHints(rowSizeHints, columnSizeHints);
            _sizeHint = math::Size2i();
            for (int i : rowSizeHints)
            {
                _sizeHint.h += i;
            }
            for (int i : columnSizeHints)
            {
                _sizeHint.w += i;
            }

            // Add spacing.
            int rowsVisible = 0;
            int columnsVisible = 0;
            p.getVisible(rowsVisible, columnsVisible);
            if (rowsVisible > 0)
            {
                _sizeHint.h += (rowsVisible - 1) * p.size.spacing;
            }
            if (columnsVisible > 0)
            {
                _sizeHint.w += (columnsVisible - 1) * p.size.spacing;
            }

            // Add the margin.
            _sizeHint.w += p.size.margin * 2;
            _sizeHint.h += p.size.margin * 2;
        }

        void GridLayout::childRemovedEvent(const ChildEvent& event)
        {
            TLRENDER_P();
            const auto i = p.gridPos.find(event.child);
            if (i != p.gridPos.end())
            {
                p.gridPos.erase(i);
            }
            _updates |= Update::Size;
            _updates |= Update::Draw;
        }

        GridPos GridLayout::Private::getSize() const
        {
            GridPos out;
            for (const auto& i : gridPos)
            {
                out.row = std::max(out.row, i.second.row);
                out.column = std::max(out.column, i.second.column);
            }
            if (!gridPos.empty())
            {
                out.row += 1;
                out.column += 1;
            }
            return out;
        }

        void GridLayout::Private::getSizeHints(
            std::vector<int>& rows, std::vector<int>& columns) const
        {
            const GridPos size = getSize();
            rows = std::vector<int>(size.row, false);
            columns = std::vector<int>(size.column, false);
            for (const auto& i : gridPos)
            {
                const bool visible = i.first->isVisible(false);
                if (visible)
                {
                    const math::Size2i& sizeHint = i.first->getSizeHint();
                    rows[i.second.row] =
                        std::max(rows[i.second.row], sizeHint.h);
                    columns[i.second.column] =
                        std::max(columns[i.second.column], sizeHint.w);
                }
            }
        }

        void GridLayout::Private::getStretch(
            std::vector<bool>& rows, std::vector<bool>& columns) const
        {
            const GridPos size = getSize();
            rows = std::vector<bool>(size.row, false);
            columns = std::vector<bool>(size.column, false);
            for (const auto& i : gridPos)
            {
                const bool visible = i.first->isVisible(false);
                if (visible)
                {
                    if (Stretch::Expanding == i.first->getVStretch())
                    {
                        rows[i.second.row] = true;
                    }
                    if (Stretch::Expanding == i.first->getHStretch())
                    {
                        columns[i.second.column] = true;
                    }
                }
            }
        }

        void GridLayout::Private::getVisible(int& rows, int& columns) const
        {
            rows = 0;
            columns = 0;
            std::vector<bool> rowsVisible;
            std::vector<bool> columnsVisible;
            getVisible(rowsVisible, columnsVisible);
            for (bool i : rowsVisible)
            {
                if (i)
                {
                    ++rows;
                }
            }
            for (bool i : columnsVisible)
            {
                if (i)
                {
                    ++columns;
                }
            }
        }

        void GridLayout::Private::getVisible(
            std::vector<bool>& rows, std::vector<bool>& columns) const
        {
            const GridPos size = getSize();
            rows = std::vector<bool>(size.row, false);
            columns = std::vector<bool>(size.column, false);
            for (const auto& i : gridPos)
            {
                const bool visible = i.first->isVisible(false);
                rows[i.second.row] = rows[i.second.row] || visible;
                columns[i.second.column] = columns[i.second.column] || visible;
            }
        }
    } // namespace ui
} // namespace tl
