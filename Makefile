CC = gcc  # we are using gcc compiler, but clang should work too
CFLAGS = -lm -Wall -Wextra -g -O0 -g3 -fsanitize=address,undefined,leak
BIN_DIR = bin

# Source and header files
SRC = main.c stb_image.h stb_image_write.h

# Output binaries
ENCODE_BIN = $(BIN_DIR)/encode
DECODE_BIN = $(BIN_DIR)/decode

# Default target
all: $(ENCODE_BIN) $(DECODE_BIN)

# Create bin directory if it doesn't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Encode binary
$(ENCODE_BIN): $(SRC) | $(BIN_DIR)
	$(CC) $(SRC) -o $(ENCODE_BIN) $(CFLAGS) -D ENCODE

# Decode binary
$(DECODE_BIN): $(SRC) | $(BIN_DIR)
	$(CC) $(SRC) -o $(DECODE_BIN) $(CFLAGS)

# Clean up generated files
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
