#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/gl.h>

typedef enum {
    TEX_RAT = 0,
    TEX_BRICK,
    TEX_THING,
    TEX_CEILING,
    TEX_FLOOR,
    TEX_COUNT   /* sentinel – also used as "none" placeholder */
} TexType;

typedef struct {
    GLuint id;
    unsigned int number;  /* texture unit index */
} Texture;

/* Load a BMP (24-bit, uncompressed) or PNG via libpng into a GL texture.
   number is the texture-unit index (0-based). */
void texture_load(Texture *tex, const char *path, unsigned int number);

void texture_bind(const Texture *tex);

/* How many times to tile this texture type per quad */
int textype_tiling(TexType t);

#endif /* TEXTURE_H */
