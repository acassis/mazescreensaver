#include "texture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <png.h>

/* ── Minimal uncompressed 24-bit BMP loader ──────────────────────────── */

static unsigned char *load_bmp(const char *path,
                                int *out_w, int *out_h) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open BMP: %s\n", path); return NULL; }

    /* Read BMP file header (14 bytes) */
    unsigned char hdr[54];
    if (fread(hdr, 1, 54, f) != 54) { fclose(f); return NULL; }

    if (hdr[0] != 'B' || hdr[1] != 'M') {
        fprintf(stderr, "Not a BMP file: %s\n", path);
        fclose(f); return NULL;
    }

    int data_offset = *(int *)(hdr + 10);
    int width       = *(int *)(hdr + 18);
    int height      = *(int *)(hdr + 22);
    short bpp       = *(short*)(hdr + 28);
    int compression = *(int *)(hdr + 30);

    if (bpp != 24 || compression != 0) {
        fprintf(stderr, "Only uncompressed 24-bit BMP supported: %s\n", path);
        fclose(f); return NULL;
    }

    fseek(f, data_offset, SEEK_SET);

    /* BMP rows are bottom-up and padded to 4 bytes */
    int row_stride = (width * 3 + 3) & ~3;
    unsigned char *raw  = (unsigned char *)malloc(row_stride * abs(height));
    unsigned char *rgba = (unsigned char *)malloc(width * abs(height) * 3);
    if (!raw || !rgba) { free(raw); free(rgba); fclose(f); return NULL; }

    fread(raw, 1, row_stride * abs(height), f);
    fclose(f);

    /* Flip vertically and convert BGR → RGB */
    int h = abs(height);
    for (int y = 0; y < h; y++) {
        unsigned char *src = raw + (h - 1 - y) * row_stride;
        unsigned char *dst = rgba + y * width * 3;
        for (int x = 0; x < width; x++) {
            dst[x*3+0] = src[x*3+2]; /* R */
            dst[x*3+1] = src[x*3+1]; /* G */
            dst[x*3+2] = src[x*3+0]; /* B */
        }
    }
    free(raw);
    *out_w = width;
    *out_h = h;
    return rgba;
}

/* ── libpng loader ────────────────────────────────────────────────────── */

static unsigned char *load_png(const char *path,
                                int *out_w, int *out_h, int *has_alpha) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open PNG: %s\n", path); return NULL; }

    png_structp png  = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                              NULL, NULL, NULL);
    png_infop   info = png_create_info_struct(png);
    if (!png || !info || setjmp(png_jmpbuf(png))) {
        fclose(f); return NULL;
    }
    png_init_io(png, f);
    png_read_info(png, info);

    int w    = png_get_image_width (png, info);
    int h    = png_get_image_height(png, info);
    int ct   = png_get_color_type (png, info);
    int bd   = png_get_bit_depth  (png, info);

    if (bd == 16) png_set_strip_16(png);
    if (ct == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if (ct == PNG_COLOR_TYPE_GRAY && bd < 8) png_set_expand_gray_1_2_4_to_8(png);
    if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if (ct == PNG_COLOR_TYPE_GRAY ||
        ct == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(png);

    png_read_update_info(png, info);

    *has_alpha = (png_get_color_type(png, info) == PNG_COLOR_TYPE_RGBA);
    int channels = *has_alpha ? 4 : 3;
    int stride   = png_get_rowbytes(png, info);

    unsigned char *data = (unsigned char *)malloc(stride * h);
    png_bytep *rows = (png_bytep *)malloc(h * sizeof(png_bytep));
    for (int y = 0; y < h; y++)
        rows[y] = data + y * stride;

    png_read_image(png, rows);
    free(rows);
    png_destroy_read_struct(&png, &info, NULL);
    fclose(f);

    *out_w = w; *out_h = h;
    (void)channels;
    return data;
}

/* ── Public API ──────────────────────────────────────────────────────── */

void texture_load(Texture *tex, const char *path, unsigned int number) {
    int w = 0, h = 0;
    unsigned char *data = NULL;
    GLenum fmt = GL_RGB;

    /* Choose loader by extension */
    const char *ext = strrchr(path, '.');
    int is_png = ext && (strcmp(ext, ".png") == 0 || strcmp(ext, ".PNG") == 0);

    if (is_png) {
        int has_alpha = 0;
        data = load_png(path, &w, &h, &has_alpha);
        fmt  = has_alpha ? GL_RGBA : GL_RGB;
    } else {
        /* Assume BMP */
        data = load_bmp(path, &w, &h);
        fmt  = GL_RGB;
    }

    if (!data) { fprintf(stderr, "Failed to load texture: %s\n", path); exit(1); }

    glGenTextures(1, &tex->id);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)fmt, w, h, 0,
                 fmt, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    free(data);
    tex->number = number;
}

void texture_bind(const Texture *tex) {
    glActiveTexture(GL_TEXTURE0 + tex->number);
    glBindTexture(GL_TEXTURE_2D, tex->id);
}

int textype_tiling(TexType t) {
    switch (t) {
        case TEX_RAT:
        case TEX_BRICK:
        case TEX_THING:    return 1;
        case TEX_CEILING:
        case TEX_FLOOR:    return 4;
        default:           return 1;
    }
}
