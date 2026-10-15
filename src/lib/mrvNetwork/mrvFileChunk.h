#pragma once

#include <cstddef>
#include <cstdint>

namespace mrv
{

    // Every binary "file data" message sent on a file-transfer DataChannel
    // is prefixed with a fixed 8-byte, big-endian offset header giving the
    // byte offset (into the destination file) at which the payload that
    // follows belongs. This lets a receiver reassemble a file correctly
    // even when chunks:
    //
    //   - arrive out of order (unordered channel, used for movie files),
    //   - arrive interleaved with control messages ("size" / "done") that
    //     can race ahead of or behind data on an unordered channel,
    //   - or get replayed/duplicated to a late-joining peer — which is
    //     exactly what a future Read-Once-Send-Many broadcast (or full
    //     P2P swarm) backfill would need: a late joiner can simply be
    //     handed every chunk with its original offset and reassemble the
    //     file correctly, with no special-casing required on either side.
    //
    // Image sequences (OpenEXR) are still transferred as one complete file
    // per frame over an *ordered* channel, so for them this header is
    // technically redundant (chunks always land in order and at the write
    // cursor's current position). It's still applied uniformly so both
    // code paths speak the same wire format, which keeps this file simple
    // and means the unordered/ordered decision is purely a DataChannel
    // property rather than a protocol difference.
    constexpr size_t kChunkHeaderSize = sizeof(uint64_t);

    inline void packChunkOffset(uint64_t offset, std::byte* out)
    {
        for (int i = 0; i < 8; ++i)
        {
            out[i] = static_cast<std::byte>((offset >> ((7 - i) * 8)) & 0xff);
        }
    }

    inline uint64_t unpackChunkOffset(const std::byte* in)
    {
        uint64_t offset = 0;
        for (int i = 0; i < 8; ++i)
            offset = (offset << 8) | static_cast<uint64_t>(in[i]);
        return offset;
    }

}  // namespace mrv
