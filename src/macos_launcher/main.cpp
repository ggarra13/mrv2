#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <mach-o/dyld.h>

int main(int argc, char *argv[]) {
    char path[PATH_MAX];
    uint32_t size = sizeof(path);

    // Find the actual path of this executable inside Contents/MacOS/
    if (_NSGetExecutablePath(path, &size) == 0) {
        char *dir = dirname(path); // points to Contents/MacOS
        
        char resources_dir[PATH_MAX];
        snprintf(resources_dir, sizeof(resources_dir), "%s/../Resources", dir);

        // Construct absolute paths to bundled Vulkan configs
        char icd_path[PATH_MAX];
        snprintf(icd_path, sizeof(icd_path), "%s/etc/vulkan/icd.d/MoltenVK_icd.json", resources_dir);
        
        char layer_path[PATH_MAX];
        snprintf(layer_path, sizeof(layer_path), "%s/etc/vulkan", resources_dir);

        // Set environment variables before launching the core engine
        setenv("VK_ICD_FILENAMES", icd_path, 1);
        setenv("VK_LAYER_PATH", layer_path, 1);
        setenv("MVK_CONFIG_LOG_LEVEL", "0", 1);

        // Path to your real core engine binary
        char core_path[PATH_MAX];
        snprintf(core_path, sizeof(core_path), "%s/bin/mrv2", dir);

        // Replace launcher process with the core binary, preserving args and env
        execve(core_path, argv, environ);
        perror("Failed to launch vmrv2_core");
    }
    return 1;
}
