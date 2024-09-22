CC = gcc  # we are using gcc compiler, but clang should work too

# debug flags: -g -O0 -g3 -fsanitize=address,undefined,leak
CFLAGS = -lm -Wall -Wextra -Ofast -march=native -pipe
BIN_DIR = bin
DATA_DIR = profiling_data

# Source and header files
SRC = main.c stb_image.h stb_image_write.h sha256.h sha256.c

# Output binaries
ENCODE_BIN = $(BIN_DIR)/encode
DECODE_BIN = $(BIN_DIR)/decode

# Default target
all: $(ENCODE_BIN) $(DECODE_BIN) PROFILE REBUILD_ENCODE REBUILD_DECODE REBUILD_CLEAN

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Encode binary
$(ENCODE_BIN): $(SRC) | $(BIN_DIR)
	$(CC) $(SRC) -o $(ENCODE_BIN) -fprofile-generate=$(DATA_DIR) $(CFLAGS) -D ENCODE

# Decode binary
$(DECODE_BIN): $(SRC) | $(BIN_DIR)
	$(CC) $(SRC) -o $(DECODE_BIN) -fprofile-generate=$(DATA_DIR) $(CFLAGS)

# Generate profiling data
PROFILE: $(SRC) | $(BIN_DIR) $(ENCODE_BIN) $(DECODE_BIN)
	$(ENCODE_BIN) ./image.png "A very very long string that I am using for profile guided optimization. This should tell the compiler what an average program run is doing, and fingers crossed, if everything goes well, the compiler will be able to make the final binary faster than it would ever be able to without all of this extra information. At worst? It should be about the same, so there is no real reason not to do this. Compile time you say. Well, that’s valid, you have to compile twice, so it takes 2x as long. I guess if you are in a hurry to compile this is a bad idea. 575 characters should do." ./out.png
	$(DECODE_BIN) ./out.png

REBUILD_ENCODE: $(SRC) | PROFILE
	$(CC) $(SRC) -o $(ENCODE_BIN) -fprofile-use=$(DATA_DIR) $(CFLAGS) -D ENCODE

REBUILD_DECODE: $(SRC) | PROFILE
	$(CC) $(SRC) -o $(DECODE_BIN) -fprofile-use=$(DATA_DIR) $(CFLAGS)

REBUILD_CLEAN:  $(SRC) | REBUILD_ENCODE REBUILD_DECODE
	rm -rf $(DATA_DIR)

# Clean up generated files
clean:
	rm -rf $(BIN_DIR)
	rm -rf $(DATA_DIR)

.PHONY: all clean