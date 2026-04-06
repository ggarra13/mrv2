// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlIO/USD.h>
#include <tlIO/USDPlugin.h>

#ifndef NOMINMAX
#    define NOMINMAX
#endif // NOMINMAX

#include <pxr/usd/usd/stage.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>

namespace tl
{
    namespace usd
    {
        //! USD renderer.
        class Render : public std::enable_shared_from_this<Render>
        {
        protected:
            void _init(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            Render();

        public:
            ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create(
                const std::shared_ptr<io::Cache>&,
                const std::weak_ptr<log::System>&);

            //! Get information.
            std::future<io::Info> getInfo(int64_t id, const file::Path& path);

            //! Render an image.
            std::future<io::VideoData> render(
                int64_t id, const file::Path& path,
                const otime::RationalTime& time, const io::Options&);

            //! Cancel requests.
            void cancelRequests(int64_t id);

        private:
            void _open(
                const std::string&, PXR_NS::UsdStageRefPtr&,
                std::shared_ptr<PXR_NS::UsdImagingGLEngine>&);
            void _run();
            void _finish();

            TLRENDER_PRIVATE();
        };
    } // namespace usd
} // namespace tl
