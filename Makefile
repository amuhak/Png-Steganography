all: main.c
	mkdir -p "bin"
	gcc main.c stb_image.h stb_image_write.h -o ./bin/SSRL -lm -fsanitize=address