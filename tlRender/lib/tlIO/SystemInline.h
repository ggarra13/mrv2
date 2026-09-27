// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

namespace tl
{
    namespace io
    {
        inline const std::vector<std::shared_ptr<IReadPlugin> >&
        ReadSystem::getPlugins() const
        {
            return _plugins;
        }

        template<typename T>
        inline std::shared_ptr<T> ReadSystem::getPlugin() const
        {
            for (const auto& i : _plugins)
            {
                if (auto plugin = std::dynamic_pointer_cast<T>(i))
                {
                    return plugin;
                }
            }
            return nullptr;
        }

        inline const std::vector<std::shared_ptr<IWritePlugin> >&
        WriteSystem::getPlugins() const
        {
            return _plugins;
        }

        template<typename T>
        inline std::shared_ptr<T> WriteSystem::getPlugin() const
        {
            for (const auto& i : _plugins)
            {
                if (auto plugin = std::dynamic_pointer_cast<T>(i))
                {
                    return plugin;
                }
            }
            return nullptr;
        }
    }
}
