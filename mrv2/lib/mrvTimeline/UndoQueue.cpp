
#include "mrvTimeline/UndoQueue.h"

namespace tl
{
    namespace edit
    {

        struct UndoQueue::Private
        {
            std::vector< otio::SerializableObject::Retainer<otio::Timeline> >
                redo;
            std::vector< otio::SerializableObject::Retainer<otio::Timeline> >
                undo;
        };

        UndoQueue::UndoQueue() :
            _p(new Private)
        {
        }

        std::shared_ptr<UndoQueue>
        UndoQueue::create(const std::shared_ptr<system::Context>& context)
        {
            std::shared_ptr<UndoQueue> out =
                std::shared_ptr<UndoQueue>(new UndoQueue);
            out->_init(context);
            return out;
        }

        void UndoQueue::push_back(
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline)
        {
            TLRENDER_P();

            p.undo.push_back(timeline);
            p.redo.clear();
        }

        bool UndoQueue::hasUndo() const
        {
            TLRENDER_P();

            return !p.undo.empty();
        }

        bool UndoQueue::hasRedo() const
        {
            TLRENDER_P();

            return !p.redo.empty();
        }

        const otio::SerializableObject::Retainer<otio::Timeline>
        UndoQueue::undo()
        {
            TLRENDER_P();

            if (p.undo.empty())
                return otio::SerializableObject::Retainer<otio::Timeline>();

            auto timeline = p.undo.back();
            p.undo.pop_back();
            p.redo.push_back(timeline);
            return timeline;
        }

        const otio::SerializableObject::Retainer<otio::Timeline>
        UndoQueue::redo()
        {
            TLRENDER_P();

            if (p.redo.empty())
                return otio::SerializableObject::Retainer<otio::Timeline>();

            auto timeline = p.redo.back();
            p.redo.pop_back();
            p.redo.push_back(timeline);
            return timeline;
        }

    } // namespace edit
} // namespace tl
