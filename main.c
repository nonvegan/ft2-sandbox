#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#define FONT_PATH "/usr/share/fonts/truetype/freefont/FreeMono.ttf"
#define FONT_HEIGHT_PX 128
#define GLYPH_TEXT "Hello world!"

#define CANVAS_WIDTH_PX 900
#define CANVAS_HEIGHT_PX 300

int main(void)
{
    FT_Library library;
    if(FT_Init_FreeType(&library)) {
        fprintf(stderr, "ERROR: Could not Initialize FreeType2\n");
        exit(1);
    }
    printf("INFO: Initialized FreeType2\n");

    FT_Face face;
    if(FT_New_Face(library, FONT_PATH, 0, &face)) {
        fprintf(stderr, "ERROR: Could not load font from %s\n", FONT_PATH);
        exit(1);
    }
    printf("INFO: Loaded %s - %s font from %s\n", face->family_name, face->style_name, FONT_PATH);

    if(FT_Set_Pixel_Sizes(face, 0, FONT_HEIGHT_PX)) {
        fprintf(stderr, "ERROR: Could not set font pixel sizes\n");
        exit(1);
    }
    printf("INFO: Set font pixel height to %d\n", FONT_HEIGHT_PX);

    char text[] = GLYPH_TEXT;
    size_t text_len = strlen(text);

    FT_Bitmap text_bitmap;
    text_bitmap.pitch = text_bitmap.width = CANVAS_WIDTH_PX;
    text_bitmap.rows = CANVAS_HEIGHT_PX;
    text_bitmap.buffer = calloc(1, text_bitmap.pitch * text_bitmap.rows);
    text_bitmap.num_grays = 255;

    size_t pen_x = 0;
    size_t pen_y = CANVAS_HEIGHT_PX / 2;

    for(size_t i = 0; i < text_len; i++) {
        char c = text[i];

        if(FT_Load_Char(face, text[i], FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR: Could not render %c glyph\n", c);
        }

        FT_GlyphSlot glyph = face->glyph;

        size_t start_x = pen_x + glyph->bitmap_left;
        size_t start_y = pen_y - glyph->bitmap_top;

        printf("pen_x -> %zu, pen_y -> %zu\n", pen_x, pen_y);
        printf("start_x -> %zu, start_y -> %zu\n", start_x, start_y);

        for(size_t x = 0; x < glyph->bitmap.width; x++)
            for(size_t y = 0; y < glyph->bitmap.rows; y++)
                if((start_x + x) <= text_bitmap.width && (start_y + y) <= text_bitmap.rows)
                    text_bitmap.buffer[(start_y + y) * text_bitmap.pitch + start_x + x] =
                        glyph->bitmap.buffer[y * glyph->bitmap.pitch + x];

        memset(text_bitmap.buffer + text_bitmap.rows / 2 * text_bitmap.pitch, 255, text_bitmap.width);

        pen_x += glyph->advance.x >> 6;
    }

    const char* file_name = "test.ppm";
    FILE *f = fopen(file_name, "w");
    if(f == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", file_name, strerror(errno));
        exit(1);
    }

    fprintf(f, "P5\n%u %u %u\n", text_bitmap.width, text_bitmap.rows, text_bitmap.num_grays);
    for(size_t y = 0; y < text_bitmap.rows; y++)
        fwrite(text_bitmap.buffer + y * text_bitmap.pitch, text_bitmap.width, 1, f);

    fclose(f);
    printf("INFO: Generated %s\n", file_name);

    free(text_bitmap.buffer);
    return 0;
}
