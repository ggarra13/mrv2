// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlCore/Quantize.h>

#include <algorithm>
#include <vector>

namespace tl
{
    namespace
    {
        // The colors are counted in cells five bits to a channel: 32768 of
        // them, few enough to sort and to keep a table of, and fine enough
        // that the palette is not noticeably coarser for it.
        constexpr int cellBits = 5;
        constexpr int cellShift = 8 - cellBits;
        constexpr int cellCount = 1 << (cellBits * 3);
        constexpr uint16_t noIndex = 0xFFFF;

        inline int getCell(int r, int g, int b)
        {
            return
                ((r >> cellShift) << (cellBits * 2)) |
                ((g >> cellShift) << cellBits) |
                (b >> cellShift);
        }

        inline int cellR(int cell) { return cell >> (cellBits * 2); }
        inline int cellG(int cell) { return (cell >> cellBits) & ((1 << cellBits) - 1); }
        inline int cellB(int cell) { return cell & ((1 << cellBits) - 1); }

        // The middle of a cell, as a color.
        inline void cellColor(int cell, int out[3])
        {
            const int half = (1 << cellShift) / 2;
            out[0] = (cellR(cell) << cellShift) + half;
            out[1] = (cellG(cell) << cellShift) + half;
            out[2] = (cellB(cell) << cellShift) + half;
        }

        // The ordered dither pattern: eight by eight, each value once.
        const uint8_t bayer[8][8] =
        {
            {  0, 32,  8, 40,  2, 34, 10, 42 },
            { 48, 16, 56, 24, 50, 18, 58, 26 },
            { 12, 44,  4, 36, 14, 46,  6, 38 },
            { 60, 28, 52, 20, 62, 30, 54, 22 },
            {  3, 35, 11, 43,  1, 33,  9, 41 },
            { 51, 19, 59, 27, 49, 17, 57, 25 },
            { 15, 47,  7, 39, 13, 45,  5, 37 },
            { 63, 31, 55, 23, 61, 29, 53, 21 }
        };

        // The most a pixel is moved by the dither, in either direction.
        constexpr int ditherMax = 16;

        // A color the palette does not serve: one further from the color it
        // is given than the colors that one was chosen for, by this much,
        // and by at least this distance a channel.
        constexpr double servedScale = 1.5;
        constexpr int servedMin = 8;

        // The part of the picture that may go unserved before the palette is
        // changed for it, and the part beyond which it is a new picture, a
        // cut, and the palette is chosen afresh.
        constexpr double unservedMax = 0.001;
        constexpr double unservedNew = 0.4;

        // The most colors replaced for one frame, and how many times the
        // pixels of the color it replaces a new color has to stand for.
        constexpr size_t replaceMax = 64;
        constexpr uint64_t replaceScale = 4;

        // The colors of an image, counted by cell.
        struct Histogram
        {
            std::vector<uint32_t> counts;
            // The sums of the colors in each cell: a palette color is the
            // mean of the pixels it stands for rather than the middle of the
            // cells they fell in.
            std::vector<uint64_t> sums;
            // The cells with pixels in them.
            std::vector<int> cells;
            uint64_t pixelCount = 0;

            void build(const uint8_t* rgb, ptrdiff_t rgbStride, int w, int h)
            {
                counts.assign(cellCount, 0);
                sums.assign(static_cast<size_t>(cellCount) * 3, 0);
                for (int y = 0; y < h; ++y)
                {
                    const uint8_t* p = rgb + y * rgbStride;
                    for (int x = 0; x < w; ++x, p += 3)
                    {
                        const int cell = getCell(p[0], p[1], p[2]);
                        ++counts[cell];
                        uint64_t* sum = sums.data() + static_cast<size_t>(cell) * 3;
                        sum[0] += p[0];
                        sum[1] += p[1];
                        sum[2] += p[2];
                    }
                }
                cells.clear();
                for (int i = 0; i < cellCount; ++i)
                {
                    if (counts[i])
                    {
                        cells.push_back(i);
                    }
                }
                pixelCount = static_cast<uint64_t>(w) * static_cast<uint64_t>(h);
            }
        };

        // A box of cells: a run of the list of cells in use, and what the
        // run spans.
        struct Box
        {
            size_t begin = 0;
            size_t end = 0;
            uint64_t count = 0;
            int min[3] = { 0, 0, 0 };
            int max[3] = { 0, 0, 0 };

            size_t getCellCount() const { return end - begin; }

            // In cells; what a split by volume is chosen with.
            uint64_t getVolume() const
            {
                return
                    static_cast<uint64_t>(max[0] - min[0] + 1) *
                    static_cast<uint64_t>(max[1] - min[1] + 1) *
                    static_cast<uint64_t>(max[2] - min[2] + 1);
            }
        };

        void measure(Box& box, const Histogram& histogram)
        {
            box.count = 0;
            for (int c = 0; c < 3; ++c)
            {
                box.min[c] = (1 << cellBits) - 1;
                box.max[c] = 0;
            }
            for (size_t i = box.begin; i < box.end; ++i)
            {
                const int cell = histogram.cells[i];
                const int v[3] = { cellR(cell), cellG(cell), cellB(cell) };
                for (int c = 0; c < 3; ++c)
                {
                    box.min[c] = std::min(box.min[c], v[c]);
                    box.max[c] = std::max(box.max[c], v[c]);
                }
                box.count += histogram.counts[cell];
            }
        }

        // A palette, and what is needed to give a pixel its color in it.
        struct Table
        {
            size_t colorCount = 0;
            std::array<uint8_t, quantizeColorsMax * 3> colors;
            // How far the dither moves a pixel of each color.
            std::array<int, quantizeColorsMax> ditherAmount;
            // How far, squared, the furthest cell a color was chosen for is
            // from it.
            std::array<int, quantizeColorsMax> radius;
            // How many pixels each color was chosen for.
            std::array<uint64_t, quantizeColorsMax> counts;
            // The color of each cell that had pixels in it when the palette
            // was chosen: the box it was in.
            std::vector<uint16_t> lookup;
            // The color nearest to each cell, found as it is asked for.
            std::vector<uint16_t> nearestLookup;

            uint16_t getNearest(int cell)
            {
                if (noIndex == nearestLookup[cell])
                {
                    int color[3];
                    cellColor(cell, color);
                    size_t out = 0;
                    int distance = -1;
                    for (size_t i = 0; i < colorCount; ++i)
                    {
                        const int dr = color[0] - colors[i * 3 + 0];
                        const int dg = color[1] - colors[i * 3 + 1];
                        const int db = color[2] - colors[i * 3 + 2];
                        const int d = 3 * dr * dr + 4 * dg * dg + 2 * db * db;
                        if (distance < 0 || d < distance)
                        {
                            distance = d;
                            out = i;
                        }
                    }
                    nearestLookup[cell] = static_cast<uint16_t>(out);
                }
                return nearestLookup[cell];
            }

            // The color of an undithered pixel: that of its box, and for a
            // cell that was in no box, the nearest.
            uint16_t get(int cell)
            {
                return lookup[cell] != noIndex ? lookup[cell] : getNearest(cell);
            }

            void getPalette(QuantizePalette& out) const
            {
                out.fill(0xFF000000);
                for (size_t i = 0; i < colorCount; ++i)
                {
                    out[i] =
                        0xFF000000 |
                        (static_cast<uint32_t>(colors[i * 3 + 0]) << 16) |
                        (static_cast<uint32_t>(colors[i * 3 + 1]) << 8) |
                        static_cast<uint32_t>(colors[i * 3 + 2]);
                }
            }
        };

        // Choose a palette for the colors counted: a median cut. A box is
        // divided along its longest side, where half of its pixels are to
        // either side. Which box is divided next goes by how many pixels it
        // holds, for most of the palette, and then by pixels times volume:
        // by count alone, a large box of few pixels -- a highlight, a thin
        // gradient -- is never divided and comes out as one flat color.
        void build(Histogram& histogram, size_t colorsMax, Table& table)
        {
            std::vector<Box> boxes;
            boxes.reserve(colorsMax);
            if (!histogram.cells.empty())
            {
                Box box;
                box.begin = 0;
                box.end = histogram.cells.size();
                measure(box, histogram);
                boxes.push_back(box);
            }
            const size_t byCountMax = colorsMax * 3 / 4;
            while (!boxes.empty() && boxes.size() < colorsMax)
            {
                const bool byCount = boxes.size() < byCountMax;
                size_t split = boxes.size();
                uint64_t splitKey = 0;
                for (size_t i = 0; i < boxes.size(); ++i)
                {
                    if (boxes[i].getCellCount() > 1)
                    {
                        const uint64_t key = byCount ?
                            boxes[i].count :
                            boxes[i].count * boxes[i].getVolume();
                        if (split == boxes.size() || key > splitKey)
                        {
                            split = i;
                            splitKey = key;
                        }
                    }
                }
                if (split == boxes.size())
                {
                    // Every box is one cell: the image has no more colors.
                    break;
                }

                Box& box = boxes[split];
                int axis = 1;
                {
                    // Green counts for most of what is seen and blue for
                    // least, so a side is longer for being green.
                    const int weights[3] = { 3, 4, 2 };
                    int length = -1;
                    for (int c = 0; c < 3; ++c)
                    {
                        const int l = (box.max[c] - box.min[c]) * weights[c];
                        if (l > length)
                        {
                            length = l;
                            axis = c;
                        }
                    }
                }
                const auto channel = [axis](int cell)
                {
                    return 0 == axis ? cellR(cell) : (1 == axis ? cellG(cell) : cellB(cell));
                };
                std::sort(
                    histogram.cells.begin() + box.begin,
                    histogram.cells.begin() + box.end,
                    [&channel](int a, int b)
                    {
                        const int ca = channel(a);
                        const int cb = channel(b);
                        return ca != cb ? ca < cb : a < b;
                    });
                size_t middle = box.begin + 1;
                {
                    uint64_t count = 0;
                    for (size_t i = box.begin; i + 1 < box.end; ++i)
                    {
                        count += histogram.counts[histogram.cells[i]];
                        middle = i + 1;
                        if (count * 2 >= box.count)
                        {
                            break;
                        }
                    }
                }
                Box other;
                other.begin = middle;
                other.end = box.end;
                box.end = middle;
                measure(box, histogram);
                measure(other, histogram);
                boxes.push_back(other);
            }

            // The palette, the box each cell is in, and how far the dither
            // moves a pixel of each box: a quarter of the box's longest
            // side. Half of it, which reaches the boxes to either side,
            // measured no closer to the picture once the eye has averaged
            // the pattern, and showed the pattern plainly and made the file
            // a tenth larger.
            table.colorCount = boxes.size();
            table.colors.fill(0);
            table.ditherAmount.fill(0);
            table.radius.fill(0);
            table.counts.fill(0);
            table.lookup.assign(cellCount, noIndex);
            table.nearestLookup.assign(cellCount, noIndex);
            for (size_t i = 0; i < boxes.size(); ++i)
            {
                const Box& box = boxes[i];
                uint64_t sum[3] = { 0, 0, 0 };
                for (size_t j = box.begin; j < box.end; ++j)
                {
                    const int cell = histogram.cells[j];
                    table.lookup[cell] = static_cast<uint16_t>(i);
                    const uint64_t* s = histogram.sums.data() + static_cast<size_t>(cell) * 3;
                    sum[0] += s[0];
                    sum[1] += s[1];
                    sum[2] += s[2];
                }
                const uint64_t count = std::max(box.count, static_cast<uint64_t>(1));
                for (int c = 0; c < 3; ++c)
                {
                    table.colors[i * 3 + c] = static_cast<uint8_t>((sum[c] + count / 2) / count);
                }
                int side = 0;
                for (int c = 0; c < 3; ++c)
                {
                    side = std::max(side, (box.max[c] - box.min[c] + 1) << cellShift);
                }
                table.ditherAmount[i] = std::min(side / 4, ditherMax);
                table.counts[i] = box.count;
                for (size_t j = box.begin; j < box.end; ++j)
                {
                    int color[3];
                    cellColor(histogram.cells[j], color);
                    const int dr = color[0] - table.colors[i * 3 + 0];
                    const int dg = color[1] - table.colors[i * 3 + 1];
                    const int db = color[2] - table.colors[i * 3 + 2];
                    table.radius[i] = std::max(table.radius[i], dr * dr + dg * dg + db * db);
                }
            }
        }

        // What became of the palette for a frame.
        enum class Update
        {
            Kept,
            Changed,
            New
        };

        // Fit the palette to the colors of the next frame. The colors the
        // frame uses stay where they are: it is their moving that makes a
        // still part of the picture wobble. Colors the frame has that the
        // palette does not serve are given the places of colors the frame
        // does not use, or hardly uses. Where much of the picture is
        // unserved it is another picture, and the palette is chosen afresh.
        Update update(Table& table, Histogram& histogram, size_t colorsMax)
        {
            std::array<bool, quantizeColorsMax> replaced;
            replaced.fill(false);
            std::array<uint64_t, quantizeColorsMax> used;
            used.fill(0);
            std::vector<int> unserved;
            uint64_t unservedCount = 0;
            const int servedMin2 = servedMin * servedMin * 3;
            for (const int cell : histogram.cells)
            {
                const uint16_t index = table.get(cell);
                int color[3];
                cellColor(cell, color);
                const int dr = color[0] - table.colors[index * 3 + 0];
                const int dg = color[1] - table.colors[index * 3 + 1];
                const int db = color[2] - table.colors[index * 3 + 2];
                const int d = dr * dr + dg * dg + db * db;
                const int served = std::max(
                    static_cast<int>(table.radius[index] * servedScale * servedScale),
                    servedMin2);
                if (d > served)
                {
                    unserved.push_back(cell);
                    unservedCount += histogram.counts[cell];
                }
                else
                {
                    used[index] += histogram.counts[cell];
                }
            }
            const double pixelCount = static_cast<double>(
                std::max(histogram.pixelCount, static_cast<uint64_t>(1)));
            const double unservedFraction = unservedCount / pixelCount;
            if (unservedFraction <= unservedMax)
            {
                return Update::Kept;
            }
            if (unservedFraction > unservedNew)
            {
                build(histogram, colorsMax, table);
                return Update::New;
            }

            // The places for new colors: those never filled, and then those
            // of the colors this frame uses least. A new color takes the
            // place of one in use only where it stands for several times as
            // many pixels: the pixels of the color replaced are given the
            // nearest of the rest, which is a change to a part of the
            // picture that had not changed.
            std::vector<size_t> places;
            for (size_t i = table.colorCount; i < colorsMax; ++i)
            {
                places.push_back(i);
            }
            const size_t emptyPlaces = places.size();
            {
                std::vector<size_t> byUse;
                for (size_t i = 0; i < table.colorCount; ++i)
                {
                    byUse.push_back(i);
                }
                std::stable_sort(
                    byUse.begin(),
                    byUse.end(),
                    [&used](size_t a, size_t b)
                    {
                        return used[a] < used[b];
                    });
                const size_t placesMax = std::max(emptyPlaces, replaceMax);
                for (size_t i = 0; i < byUse.size() && places.size() < placesMax; ++i)
                {
                    places.push_back(byUse[i]);
                }
            }
            if (places.empty())
            {
                return Update::Kept;
            }

            // The unserved colors, divided among the places as a palette is,
            // and given the places most pixels first.
            Histogram part;
            part.counts = histogram.counts;
            part.sums = histogram.sums;
            part.cells = unserved;
            part.pixelCount = unservedCount;
            Table partTable;
            build(part, places.size(), partTable);
            std::vector<size_t> byCount;
            for (size_t i = 0; i < partTable.colorCount; ++i)
            {
                byCount.push_back(i);
            }
            std::stable_sort(
                byCount.begin(),
                byCount.end(),
                [&partTable](size_t a, size_t b)
                {
                    return partTable.counts[a] > partTable.counts[b];
                });
            std::array<uint16_t, quantizeColorsMax> placed;
            placed.fill(noIndex);
            bool changed = false;
            for (size_t i = 0; i < byCount.size(); ++i)
            {
                const size_t color = byCount[i];
                const size_t place = places[i];
                const bool empty = i < emptyPlaces;
                if (!empty &&
                    partTable.counts[color] < used[place] * replaceScale)
                {
                    break;
                }
                placed[color] = static_cast<uint16_t>(place);
                replaced[place] = !empty;
                for (int c = 0; c < 3; ++c)
                {
                    table.colors[place * 3 + c] = partTable.colors[color * 3 + c];
                }
                table.ditherAmount[place] = partTable.ditherAmount[color];
                table.radius[place] = partTable.radius[color];
                table.counts[place] = partTable.counts[color];
                table.colorCount = std::max(table.colorCount, place + 1);
                changed = true;
            }
            if (!changed)
            {
                return Update::Kept;
            }
            // The cells of the colors replaced are no longer theirs, the new
            // colors have their own, and which color is nearest to a cell
            // has to be found again.
            for (int cell = 0; cell < cellCount; ++cell)
            {
                const uint16_t index = table.lookup[cell];
                if (index != noIndex && replaced[index])
                {
                    table.lookup[cell] = noIndex;
                }
            }
            for (const int cell : unserved)
            {
                const uint16_t index = partTable.lookup[cell];
                if (index != noIndex && placed[index] != noIndex)
                {
                    table.lookup[cell] = placed[index];
                }
            }
            table.nearestLookup.assign(cellCount, noIndex);
            return Update::Changed;
        }

        // The color of a pixel. A dithered pixel takes the color nearest to
        // where the dither put it, rather than the color of the box it
        // landed in: the boxes are long and thin where the colors are dense,
        // and the color of the next box along can be a long way off to the
        // side.
        inline uint8_t getIndex(Table& table, const uint8_t* p, int x, int y, bool dither)
        {
            uint16_t index = table.get(getCell(p[0], p[1], p[2]));
            if (dither && table.ditherAmount[index] > 0)
            {
                // From -amount to +amount, the pattern's 64 values spread
                // evenly across it.
                const int offset =
                    (table.ditherAmount[index] * (2 * static_cast<int>(bayer[y & 7][x & 7]) - 63)) / 64;
                index = table.getNearest(getCell(
                    std::clamp(p[0] + offset, 0, 255),
                    std::clamp(p[1] + offset, 0, 255),
                    std::clamp(p[2] + offset, 0, 255)));
            }
            return static_cast<uint8_t>(index);
        }
    }

    size_t quantize(
        const uint8_t* rgb,
        ptrdiff_t rgbStride,
        int w,
        int h,
        uint8_t* indices,
        ptrdiff_t indicesStride,
        QuantizePalette& palette,
        bool dither)
    {
        palette.fill(0xFF000000);
        if (!rgb || !indices || w <= 0 || h <= 0)
        {
            return 0;
        }
        Histogram histogram;
        histogram.build(rgb, rgbStride, w, h);
        Table table;
        build(histogram, quantizeColorsMax, table);
        for (int y = 0; y < h; ++y)
        {
            const uint8_t* p = rgb + y * rgbStride;
            uint8_t* out = indices + y * indicesStride;
            for (int x = 0; x < w; ++x, p += 3)
            {
                out[x] = getIndex(table, p, x, y, dither);
            }
        }
        table.getPalette(palette);
        return table.colorCount;
    }

    struct Quantizer::Private
    {
        int w = 0;
        int h = 0;
        Histogram histogram;
        Table table;
        bool hasTable = false;
        size_t paletteCount = 0;
    };

    Quantizer::Quantizer() :
        _p(new Private)
    {}

    Quantizer::~Quantizer()
    {}

    size_t Quantizer::quantize(
        const uint8_t* rgb,
        ptrdiff_t rgbStride,
        int w,
        int h,
        uint8_t* indices,
        ptrdiff_t indicesStride,
        QuantizePalette& palette,
        bool dither)
    {
        TLRENDER_P();
        palette.fill(0xFF000000);
        if (!rgb || !indices || w <= 0 || h <= 0)
        {
            return 0;
        }
        if (w != p.w || h != p.h)
        {
            p.w = w;
            p.h = h;
            p.hasTable = false;
        }

        // The palette of the frame before, fitted to this one.
        p.histogram.build(rgb, rgbStride, w, h);
        // One color fewer than the palette holds: the last entry is the
        // transparent one.
        const size_t colorsMax = quantizeColorsMax - 1;
        if (!p.hasTable)
        {
            build(p.histogram, colorsMax, p.table);
            p.hasTable = true;
            ++p.paletteCount;
        }
        else if (update(p.table, p.histogram, colorsMax) != Update::Kept)
        {
            ++p.paletteCount;
        }

        // Each pixel is given its color from the picture alone, and not
        // from what it had in the frame before. Keeping the color of a pixel
        // that has hardly changed steadies a noisy sky further, and leaves
        // a trail where an edge has passed over flat color.
        for (int y = 0; y < h; ++y)
        {
            const uint8_t* in = rgb + y * rgbStride;
            uint8_t* out = indices + y * indicesStride;
            for (int x = 0; x < w; ++x, in += 3)
            {
                out[x] = getIndex(p.table, in, x, y, dither);
            }
        }

        p.table.getPalette(palette);
        palette[quantizeColorsMax - 1] = 0x00000000;
        return p.table.colorCount;
    }

    size_t Quantizer::getPaletteCount() const
    {
        return _p->paletteCount;
    }
}
