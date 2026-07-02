
#include <mrvNetwork/xxhash.h>


#include <FL/fl_utf8.h>

#include <string>

namespace mrv
{
    inline std::string hashFile(const std::string& fileName)
    {
        char result[64];

        FILE* f = fl_fopen(fileName.c_str(), "r");
        if (!f)
            return "";

        // Allocate a state struct. Do not just use malloc() or new.
        XXH3_state_t* state = XXH3_createState();

        assert(state != NULL && "Out of memory!");

        // Reset the state to start a new hashing session.
        XXH3_128bits_reset(state);

        // Read the file in chunks
        char buffer[4096];
        size_t count;

        while ((count = fread(buffer, 1, sizeof(buffer), f)) != 0) {
            // Run update() as many times as necessary to process the data
            XXH3_128bits_update(state, buffer, count);
        }

        fclose(f);

        // Retrieve the finalized hash. This will not change the state.
        XXH128_hash_t h = XXH3_128bits_digest(state);
        // Free the state. Do not use free().
        XXH3_freeState(state);

        snprintf(result, 64, "%016llx%016llx\n",
                 (unsigned long long)h.high64,
                 (unsigned long long)h.low64);
        return result;
    }
}
