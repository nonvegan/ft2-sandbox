main: main.c
	gcc -o main main.c -Wall -Wextra -pedantic `pkg-config --cflags --libs freetype2`

run: main
	rm -f glyphs/* && ./main && feh glyphs/*.ppm
