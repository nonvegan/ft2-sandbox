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
#define FONT_HEIGHT_PX 512
#define GLYPH_TEXT "Hello world!"
#define GLYPH_DIR "glyphs"

void check_mkdir(const char* dir_name)
{
    struct stat st = {0};
    if (stat(dir_name, &st) == -1) {
        if (mkdir(dir_name, 0700) == -1) {
            fprintf(stderr, "ERROR: Could not create directory %s: %s\n", dir_name, strerror(errno));
        } else {
            printf("INFO: Created %s directory\n", dir_name);
        }
    } else {
        printf("INFO: Found %s directory\n", dir_name);
    }
}

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

    check_mkdir(GLYPH_DIR);

    char text[] = GLYPH_TEXT; 
    size_t len = strlen(text);

    for(size_t i = 0; i < len; i++) {
        char c = text[i];

        unsigned int glyph_index = FT_Get_Char_Index(face, c);
        if(!glyph_index) {
            fprintf(stderr, "ERROR: Could not find %c glyph index\n", c);
        }

        if(FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
            fprintf(stderr, "ERROR: Could not load %c with the glyph index %u\n", c, glyph_index);
        }

        if(FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) {
            fprintf(stderr, "ERROR: Could not render %c with the glyph index %u\n", c, glyph_index);
        }

        char file_name[64];
        sprintf(file_name, GLYPH_DIR"/glyph-%c.ppm", c);

        FILE *f = fopen(file_name, "w");
        if(f == NULL) {
            fprintf(stderr, "ERROR: Could not open file %s: %s\n", file_name, strerror(errno));
            break;
        }

        fprintf(f, "P5\n%u %u 255\n", face->glyph->bitmap.width, face->glyph->bitmap.rows);

        for(size_t y = 0; y < face->glyph->bitmap.rows; y++)
            fwrite(face->glyph->bitmap.buffer + y * face->glyph->bitmap.pitch, face->glyph->bitmap.width, 1, f);

        fclose(f);
        printf("INFO: Generated '%c' glyph to %s\n", c, file_name);
    }

    return 0;
}
