// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/IWidgetPopup.h>

#include <tlUI/DrawUtil.h>
#include <tlUI/IWindow.h>

namespace tl
{
    namespace ui
    {
        namespace
        {
            class ContainerWidget : public IWidget
            {
                TLRENDER_NON_COPYABLE(ContainerWidget);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ContainerWidget();

            public:
                virtual ~ContainerWidget();

                static std::shared_ptr<ContainerWidget> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const math::Box2i&) override;
                void sizeHintEvent(const SizeHintEvent&) override;
            };

            void ContainerWidget::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init("tl::ui::ContainerWidget", context, parent);
                _setMouseHover(true);
                _setMousePress(true);
            }

            ContainerWidget::ContainerWidget() {}

            ContainerWidget::~ContainerWidget() {}

            std::shared_ptr<ContainerWidget> ContainerWidget::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out =
                    std::shared_ptr<ContainerWidget>(new ContainerWidget);
                out->_init(context, parent);
                return out;
            }

            void ContainerWidget::setGeometry(const math::Box2i& value)
            {
                IWidget::setGeometry(value);
                if (!_children.empty())
                {
                    _children.front()->setGeometry(value);
                }
            }

            void ContainerWidget::sizeHintEvent(const SizeHintEvent& event)
            {
                IWidget::sizeHintEvent(event);
                if (!_children.empty())
                {
                    _sizeHint = _children.front()->getSizeHint();
                }
            }
        } // namespace

        struct IWidgetPopup::Private
        {
            ColorRole popupRole = ColorRole::Window;
            math::Box2i buttonGeometry;
            bool open = false;
            std::function<void(void)> closeCallback;
            std::shared_ptr<IWidget> widget;
            std::shared_ptr<ContainerWidget> containerWidget;

            struct SizeData
            {
                int border = 0;
                int shadow = 0;
            };
            SizeData size;
        };

        void IWidgetPopup::_init(
            const std::string& objectName,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IPopup::_init(objectName, context, parent);
            TLRENDER_P();
            p.containerWidget =
                ContainerWidget::create(context, shared_from_this());
        }

        IWidgetPopup::IWidgetPopup() :
            _p(new Private)
        {
        }

        IWidgetPopup::~IWidgetPopup() {}

        void IWidgetPopup::open(
            const std::shared_ptr<IWindow>& window,
            const math::Box2i& buttonGeometry)
        {
            TLRENDER_P();
            p.buttonGeometry = buttonGeometry;
            p.open = true;
            setParent(window);
            takeKeyFocus();
        }

        bool IWidgetPopup::isOpen() const
        {
            return _p->open;
        }

        void IWidgetPopup::close()
        {
            TLRENDER_P();
            p.open = false;
            setParent(nullptr);
            if (p.closeCallback)
            {
                p.closeCallback();
            }
        }

        void
        IWidgetPopup::setCloseCallback(const std::function<void(void)>& value)
        {
            _p->closeCallback = value;
        }

        void IWidgetPopup::setPopupRole(ColorRole value)
        {
            TLRENDER_P();
            if (value == p.popupRole)
                return;
            p.popupRole = value;
            _updates |= Update::Draw;
        }

        void IWidgetPopup::setWidget(const std::shared_ptr<IWidget>& value)
        {
            TLRENDER_P();
            if (p.widget)
            {
                p.widget->setParent(nullptr);
            }
            p.widget = value;
            if (p.widget)
            {
                p.widget->setParent(p.containerWidget);
            }
        }

        void IWidgetPopup::setGeometry(const math::Box2i& value)
        {
            IPopup::setGeometry(value);
            TLRENDER_P();
            math::Size2i sizeHint = p.containerWidget->getSizeHint();
            std::list<math::Box2i> boxes;
            boxes.push_back(math::Box2i(
                p.buttonGeometry.min.x, p.buttonGeometry.max.y + 1, sizeHint.w,
                sizeHint.h));
            boxes.push_back(math::Box2i(
                p.buttonGeometry.max.x + 1 - sizeHint.w,
                p.buttonGeometry.max.y + 1, sizeHint.w, sizeHint.h));
            boxes.push_back(math::Box2i(
                p.buttonGeometry.min.x, p.buttonGeometry.min.y - sizeHint.h,
                sizeHint.w, sizeHint.h));
            boxes.push_back(math::Box2i(
                p.buttonGeometry.max.x + 1 - sizeHint.w,
                p.buttonGeometry.min.y - sizeHint.h, sizeHint.w, sizeHint.h));
            struct Intersect
            {
                math::Box2i original;
                math::Box2i intersected;
            };
            std::vector<Intersect> intersect;
            for (const auto& box : boxes)
            {
                intersect.push_back({box, box.intersect(value)});
            }
            std::stable_sort(
                intersect.begin(), intersect.end(),
                [](const Intersect& a, const Intersect& b)
                {
                    return a.intersected.getSize().getArea() >
                           b.intersected.getSize().getArea();
                });
            math::Box2i g = intersect.front().intersected;
            p.containerWidget->setGeometry(g);
        }

        void IWidgetPopup::sizeHintEvent(const SizeHintEvent& event)
        {
            IPopup::sizeHintEvent(event);
            TLRENDER_P();
            p.size.border =
                event.style->getSizeRole(SizeRole::Border, _displayScale);
            p.size.shadow =
                event.style->getSizeRole(SizeRole::Shadow, _displayScale);
        }

        void IWidgetPopup::drawEvent(
            const math::Box2i& drawRect, const DrawEvent& event)
        {
            IPopup::drawEvent(drawRect, event);
            TLRENDER_P();
            // event.render->drawRect(
            //     _geometry,
            //     image::Color4f(0.F, 0.F, 0.F, .2F));
            const math::Box2i g =
                p.containerWidget->getGeometry().margin(p.size.border);
            if (g.isValid())
            {
                const math::Box2i g2(
                    g.min.x - p.size.shadow, g.min.y, g.w() + p.size.shadow * 2,
                    g.h() + p.size.shadow);
                event.render->drawColorMesh(
                    shadow(g2, p.size.shadow), math::Vector2i(),
                    image::Color4f(1.F, 1.F, 1.F));

                event.render->drawMesh(
                    border(g, p.size.border), math::Vector2i(),
                    event.style->getColorRole(ColorRole::Border));

                event.render->drawRect(
                    g.margin(-p.size.border),
                    event.style->getColorRole(p.popupRole));
            }
        }
    } // namespace ui
} // namespace tl
