// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>
#include <tlUI/IWidgetOptions.h>

#include <list>

namespace tl
{
    namespace ui
    {
        class IWindow;

        //! Base class for widgets.
        class IWidget : public std::enable_shared_from_this<IWidget>
        {
            TLRENDER_NON_COPYABLE(IWidget);

        protected:
            void _init(
                const std::string& objectName,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            IWidget();

        public:
            virtual ~IWidget() = 0;

            //! Get the object name.
            const std::string& getObjectName() const;

            //! Set the object name.
            void setObjectName(const std::string&);

            //! Set the background role.
            void setBackgroundRole(ColorRole);

            //! Get whether updates are needed.
            int getUpdates() const;

            //! Hierarchy
            ///@{

            //! Get the parent widget.
            const std::weak_ptr<IWidget>& getParent() const;

            //! Set the parent widget.
            void setParent(const std::shared_ptr<IWidget>&);

            //! Get the children widgets.
            const std::list<std::shared_ptr<IWidget> >& getChildren() const;

            //! Get the first parent widget of the given type.
            template <typename T> std::shared_ptr<T> getParentT() const;

            //! Move the child widget to the front of the drawing order.
            void moveToFront(const std::shared_ptr<IWidget>&);

            //! Move the child widget to the back of the drawing order.
            void moveToBack(const std::shared_ptr<IWidget>&);

            //! Get the window.
            std::shared_ptr<IWindow> getWindow();

            ///@}

            //! Geometry
            ///@{

            //! Get the size hint.
            const math::Size2i& getSizeHint() const;

            //! Get the horizontal layout stretch.
            Stretch getHStretch() const;

            //! Set the horizontal layout stretch.
            void setHStretch(Stretch);

            //! Get the vertical layout stretch.
            Stretch getVStretch() const;

            //! Set the vertical layout stretch.
            void setVStretch(Stretch);

            //! Set the horizontal and vertical layout stretch.
            void setStretch(Stretch horizontal, Stretch vertical);

            //! Set the horizontal and vertical layout stretch.
            void setStretch(Stretch);

            //! Get the horizontal layout alignment.
            HAlign getHAlign() const;

            //! Set the horizontal layout alignment.
            void setHAlign(HAlign);

            //! Get the vertical layout alignment.
            VAlign getVAlign() const;

            //! Set the vertical layout alignment.
            void setVAlign(VAlign);

            //! Set the horizontal and vertical layout alignment.
            void setAlign(HAlign, VAlign);

            //! Get the geometry.
            const math::Box2i& getGeometry() const;

            //! Set the geometry.
            virtual void setGeometry(const math::Box2i&);

            ///@}

            //! Visibility
            ///@{

            //! Is the widget visible?
            bool isVisible(bool andParentsVisible = true) const;

            //! Set whether the widget is visible.
            virtual void setVisible(bool);

            //! Show the widget.
            void show();

            //! Hide the widget.
            void hide();

            //! Is the widget clipped?
            bool isClipped() const;

            //! Get the clipping rect applied to the child widgets. By
            //! default this is the same as the widget geometry.
            virtual math::Box2i getChildrenClipRect() const;

            ///@}

            //! Enabled
            ///@{

            //! Is the widget enabled?
            bool isEnabled(bool andParentsEnabled = true) const;

            //! Set whether the widget is enabled.
            virtual void setEnabled(bool);

            ///@}

            //! Mouse
            ///@{

            //! Does the widget support mouse enter and leave events?
            bool hasMouseHover();

            ///@}

            //! Key Focus
            ///@{

            //! Does this widget accept key focus?
            bool acceptsKeyFocus() const;

            //! Set whether the widget accepts key focus.
            void setAcceptsKeyFocus(bool);

            //! Does this widget have key focus?
            bool hasKeyFocus() const;

            //! Take the key focus.
            virtual void takeKeyFocus();

            //! Release the key focus.
            void releaseKeyFocus();

            ///@}

            //! Tool tips.
            ///@{

            //! Get the tool tip.
            const std::string& getToolTip() const;

            //! Set the tool tip.
            void setToolTip(const std::string&);

            ///@}

            //! Events
            ///@{

            //! Child added event.
            virtual void childAddedEvent(const ChildEvent&);

            //! Child removed event.
            virtual void childRemovedEvent(const ChildEvent&);

            //! Tick event.
            virtual void tickEvent(
                bool parentsVisible, bool parentsEnabled, const TickEvent&);

            //! Size hint event.
            virtual void sizeHintEvent(const SizeHintEvent&);

            //! Clip event.
            virtual void clipEvent(const math::Box2i&, bool clipped);

            //! Draw event.
            virtual void drawEvent(const math::Box2i&, const DrawEvent&);

            //! Draw overlay event.
            virtual void drawOverlayEvent(const math::Box2i&, const DrawEvent&);

            //! Mouse enter event.
            virtual void mouseEnterEvent();

            //! Mouse leave event.
            virtual void mouseLeaveEvent();

            //! Mouse move event.
            virtual void mouseMoveEvent(MouseMoveEvent&);

            //! Mouse press event.
            virtual void mousePressEvent(MouseClickEvent&);

            //! Mouse release event.
            virtual void mouseReleaseEvent(MouseClickEvent&);

            //! Scroll event.
            virtual void scrollEvent(ScrollEvent&);

            //! Key focus event.
            virtual void keyFocusEvent(bool);

            //! Key press event.
            virtual void keyPressEvent(KeyEvent&);

            //! Key release event.
            virtual void keyReleaseEvent(KeyEvent&);

            //! Text event.
            virtual void textEvent(TextEvent&);

            //! Drag enter event.
            virtual void dragEnterEvent(DragAndDropEvent&);

            //! Drag leave event.
            virtual void dragLeaveEvent(DragAndDropEvent&);

            //! Drag move event.
            virtual void dragMoveEvent(DragAndDropEvent&);

            //! Drop event.
            virtual void dropEvent(DragAndDropEvent&);

            ///@}

        protected:
            void _setMouseHover(bool);
            void _setMousePress(bool, int button = -1, int modifiers = -1);
            virtual void _releaseMouse();

            std::weak_ptr<system::Context> _context;
            std::string _objectName;
            ColorRole _backgroundRole = ColorRole::None;
            float _displayScale = 1.F;
            int _updates = 0;
            std::weak_ptr<IWidget> _parent;
            std::list<std::shared_ptr<IWidget> > _children;
            math::Size2i _sizeHint;
            Stretch _hStretch = Stretch::Fixed;
            Stretch _vStretch = Stretch::Fixed;
            HAlign _hAlign = HAlign::Center;
            VAlign _vAlign = VAlign::Center;
            math::Box2i _geometry;
            bool _visible = true;
            bool _parentsVisible = true;
            bool _clipped = false;
            bool _enabled = true;
            bool _parentsEnabled = true;
            struct MouseData
            {
                bool inside = false;
                math::Vector2i pos;
                bool press = false;
                math::Vector2i pressPos;
            };
            MouseData _mouse;
            bool _acceptsKeyFocus = false;
            bool _keyFocus = false;
            std::string _toolTip;

        private:
            bool _mouseHoverEnabled = false;
            bool _mousePressEnabled = false;
            int _mousePressButton = -1;
            int _mousePressModifiers = -1;
        };
    } // namespace ui
} // namespace tl

#include <tlUI/IWidgetInline.h>
