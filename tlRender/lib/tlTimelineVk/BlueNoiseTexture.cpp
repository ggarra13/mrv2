// bluenoise.cpp
// Blue noise texture generation using the void-and-cluster algorithm.
// Based on: https://gist.github.com/kajott/d9f9bb93043040bfe2f48f4f499903d8
// and Alan Wolfe's description: https://blog.demofox.org/2019/06/25/generating-blue-noise-textures-with-void-and-cluster/


#define CREATE_TEXTURE 0

#if !CREATE_TEXTURE
#include "tlTimelineVk/BlueNoiseData.h"
#endif

#include "tlVk/Texture.h"



#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>

// Log2 image size; set to 7 for 128x128, 8 for 256x256
#define LOG2_SIZE 7

// Energy function falloff (Gaussian sigma value); 1.9 is common for good blue noise
#define SIGMA 1.9

// Fraction of the image area to "seed" with initial random points (1/10 is typical)
#define INITIAL_POINT_DENSITY 10

#define SIZE (1 << LOG2_SIZE)              // Image size (width = height)
#define AREA (SIZE * SIZE)                 // Image area (width * height)
#define POINT_SHIFT ((2 * LOG2_SIZE) - 8)  // Shift amount between point ID and grayscale value (for 8-bit output)
#define XY_MASK ((1 << LOG2_SIZE) - 1)     // Coordinate mask

#define I2X(i) ((i) & XY_MASK)                // Extract X coordinate from point index
#define I2Y(i) ((i) >> LOG2_SIZE)             // Extract Y coordinate from point index
#define XY2I(x,y) (((y) << LOG2_SIZE) + (x))  // Build point index from coordinates

// Structure representing the current processing state
struct map_state {
    uint32_t bitmap[AREA / 32];  // Dot occupancy bitmap
    float emap[AREA];            // Energy map
    int imin, imax;              // Index of minimum and maximum energy spots
    int points;                  // Number of active points in the bitmap
} state1, state2, *state = &state1;

// Get/set/clear individual bits in the current state bitmap
#define BMP_GET(i) ((state->bitmap[((i) >> 5)] >> ((i) & 31)) & 1u)
#define BMP_SET(i) do { state->bitmap[(i) >> 5] |=   1u << ((i) & 31);  } while (0)
#define BMP_CLR(i) do { state->bitmap[(i) >> 5] &= ~(1u << ((i) & 31)); } while (0)

// Energy distribution LUT (twice the image dimensions for toroidal wrapping)
float etab[(SIZE * 2) * (SIZE * 2)];

// Resulting noise image (grayscale, 0-255)
uint8_t noise[AREA];

// Update the current energy map (and imax/imin) after adding (sign=+1.0f)
// or removing (sign=-1.0f) a point at index
void update_map(int index, float sign) {
    int px = I2X(index);
    int py = I2Y(index);
    const float* pt = &etab[((SIZE - py) * (SIZE * 2)) + (SIZE - px)];
    float* pm = state->emap;
    float emin = 1e9f, emax = 0.0f;
    int i = 0;
    for (int y = 0; y < SIZE; ++y) {
        for (int x = 0; x < SIZE; ++x) {
            float e = (*pm++ += *pt++ * sign);
            if (BMP_GET(i)) {
                if (e > emax) { emax = e; state->imax = i; }
            } else {
                if (e < emin) { emin = e; state->imin = i; }
            }
            ++i;
        }
        pt += SIZE;
    }
}

namespace tl
{
    namespace vlk
    {
        std::shared_ptr<vlk::Texture>
        create_blue_noise_texture(Fl_Vk_Context& ctx)
        {
            
#if CREATE_TEXTURE
            srand(time(NULL));

            // Initialize the noise image to white (saves minor work)
            memset(noise, 0xFF, AREA);

            // Prepare energy look-up table using symmetries and repetitions:
            // Create upper-left 1/16th explicitly, mirror to upper-left quarter, then repeat.
            float *row = etab;
            for (int y = 0; y < (SIZE / 2); ++y) {
                for (int x = 0; x < (SIZE / 2); ++x) {
                    row[x] = row[SIZE - x] = (float)exp((x * x + y * y) * (-0.5 / (SIGMA * SIGMA)));
                }
                memcpy(&row[SIZE], row, SIZE * sizeof(float));
                row += SIZE * 2;
            }
            for (int y = 0; y < (SIZE / 2); ++y) {
                memcpy(&etab[(SIZE - y) * (SIZE * 2)], &etab[y * (SIZE * 2)], (SIZE * 2) * sizeof(float));
            }
            memcpy(&etab[SIZE * (SIZE * 2)], etab, SIZE * (SIZE * 2) * sizeof(float));

            // Set initial points (sparse random seeding)
            while (state->points < (AREA / INITIAL_POINT_DENSITY)) {
                int i = XY2I(rand() & XY_MASK, rand() & XY_MASK);
                if (!BMP_GET(i)) {
                    BMP_SET(i); update_map(i, +1.0f);
                    ++state->points;
                }
            }

            // Re-distribute initial points (remove clusters, fill voids)
            int redist = 0;
            int last_point = AREA;
            while (state->imax != last_point) {
                BMP_CLR(state->imax); update_map(state->imax, -1.0f);
                last_point = state->imin;
                BMP_SET(state->imin); update_map(state->imin, +1.0f);
                ++redist;
            }
            // Serialize initial points (Phase 1: assign ranks by removing clusters)
            memcpy(&state2, &state1, sizeof(struct map_state));
            while (state->points) {
                noise[state->imax] = (--state->points) >> POINT_SHIFT;
                BMP_CLR(state->imax); update_map(state->imax, -1.0f);
            }
            state = &state2;

            // Create points until half full (Phase 2: fill voids)
            while (state->points < (AREA / 2)) {
                noise[state->imin] = (state->points++) >> POINT_SHIFT;
                BMP_SET(state->imin); update_map(state->imin, +1.0f);
            }

            // Invert energy map for Phase 3 (treat 0s as clusters)
            for (int i = 0; i < (AREA / 32); ++i) {
                state->bitmap[i] ^= (uint32_t)(-1);
            }
            memset(state->emap, 0, sizeof(state->emap));
            for (int i = 0; i < AREA; ++i) {
                if (BMP_GET(i)) { update_map(i, +1.0f); }
            }
            // Create points until full (Phase 3: remove "clusters" of 0s by filling them)
            while (state->points < (255 << POINT_SHIFT)) {
                noise[state->imax] = (state->points++) >> POINT_SHIFT;
                BMP_CLR(state->imax); update_map(state->imax, -1.0f);
            }

            // Write noise image as PGM (grayscale portable graymap)
            FILE* f = fopen("bluenoise.pgm", "wb");
            fprintf(f, "P5\n%d %d\n255\n", SIZE, SIZE);
            fwrite(noise, SIZE, SIZE, f);
            fclose(f);
            
            auto info = image::Info(SIZE, SIZE, image::PixelType::L_U8);
    
            vlk::TextureOptions options;
            options.filters.minify = timeline::ImageFilter::Nearest;
            options.filters.magnify = timeline::ImageFilter::Nearest;
            auto texture = vlk::Texture::create(ctx, info, options);
            texture->copy(noise, AREA);
            return texture;

#else
            auto info = image::Info(kBluenoise_width,
                                    kBluenoise_height, image::PixelType::L_U8);
    
            vlk::TextureOptions options;
            options.filters.minify = timeline::ImageFilter::Nearest;
            options.filters.magnify = timeline::ImageFilter::Nearest;
            auto texture = vlk::Texture::create(ctx, info, options);
            texture->copy(kBluenoise, kBluenoise_width * kBluenoise_height);
            return texture;
#endif
                        
        }
    }
}
