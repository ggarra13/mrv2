// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvApp/mrvApp.h"

#include "mrvWidgets/mrvVersion.h"

#include "mrvCore/mrvBackend.h"
#include "mrvCore/mrvGPU.h"
#include "mrvOS/mrvI8N.h"

namespace mrv
{
    namespace gpu
    {
        const std::string getGPUVendor()
        {
            std::string out = _("Unknown");
        
#ifdef OPENGL_BACKEND
            tl::gl::initGLAD();

        // Get OpenGL information
            const char* vendorString = (char*)glGetString(GL_VENDOR);
            if (vendorString)
                out = vendorString;
            out = "GPU: " + out;
#endif
            
#ifdef VULKAN_BACKEND
            out = gpu_list(App::ui);
#endif
        
            return out;
        }
    }
    
}
