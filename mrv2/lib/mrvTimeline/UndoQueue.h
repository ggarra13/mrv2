
#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/timeline.h>

namespace tl
{
    namespace edit
    {

        class UndoQueue
        {
        protected:
            void _init(const std::shared_ptr<system::Context>&);

            UndoQueue();

        public:
            static std::shared_ptr<UndoQueue>
            create(const std::shared_ptr<system::Context>&);

            //! Returns whether there's at least one item in undo queue.
            bool hasUndo() const;

            //! Returns whether there's at least one item in redo queue.
            bool hasRedo() const;

            //! Add a timeline to the undo queue.
            void push_back(
                const otio::SerializableObject::Retainer<otio::Timeline>&);

            //! Revert to previous timeline state.
            const otio::SerializableObject::Retainer<otio::Timeline> undo();

            //! Redo last timeline state.
            const otio::SerializableObject::Retainer<otio::Timeline> redo();

        private:
            TLRENDER_PRIVATE();
        };

    } // namespace edit
} // namespace tl
