#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "sha256.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"
#include "stb_image.h"

void print(const char *image_path, const char *str, const char *out_path) {
    // x, y, and no of channels in the image
    int x, y, n;

    // Load the image into an 8-bit array so its easy to manipulate
    uint8_t *data = stbi_load(image_path, &x, &y, &n, 0);

    int data_size = x * y * n;

    // printf("x: %d y: %d n: %d \n", x, y, n);

    int len = (int) strlen(str);
    int no_of_bits = 8 * len;

    if (no_of_bits + 32 > data_size) {
        printf("Data is too large for the image\n could print only %d bytes\n", (data_size - 32) / 8);
        return;
    }

    // Converting the data into binary for easy reading
    bool *bits = malloc(no_of_bits * sizeof(bool));

    // Converting the data into binary
    for (int i = 0; i < len; i++) {
        for (int j = 7; j >= 0; j--) {
            bits[i * 8 + 7 - j] = (str[i] >> j) & 1;
        }
    }

    // printf("len: %d\n", len);

    // Encoding the data into the image (LSB)
    // first 32 bits are the length of the data

    bool len_bits[32];
    for (int i = 31; i >= 0; i--) {
        len_bits[31 - i] = (no_of_bits >> i) & 1;
    }

    // Encoding the length of the data
    uint8_t *head = data;

    for (int i = 0; i < 32; i++) {
        if (len_bits[i]) {
            *head |= 1;
        } else {
            *head &= 0b11111110;
        }
        head++;
    }

    // Encoding the data
    for (int i = 0; i < no_of_bits; i++) {
        if (bits[i]) {
            *head |= 1;
        } else {
            *head &= 0b11111110;
        }
        ++head;
    }

    // Padding the rest of the image with 0
    while (head - data < data_size) {
        *head &= 0b11111110;
        ++head;
    }

    free(bits);

    // Writing the data to the image
    stbi_write_png(out_path, x, y, n, data, x * n);

    // Freeing the data
    stbi_image_free(data);
}

char *read(const char *image_path) {

    // reading the data from the image
    int x, y, n;

    uint8_t *data = stbi_load(image_path, &x, &y, &n, 0);
    uint8_t *head = data;

    // Find the length of the data (first 32 bits)
    int len_data = 0;
    for (int i = 0; i < 32; i++) {
        len_data <<= 1;
        len_data |= *head & 1;
        head++;
    }

    // Reading the data
    bool *data_bits = malloc(len_data * sizeof(bool));
    for (int i = 0; i < len_data; i++) {
        data_bits[i] = *head & 1;
        head++;
    }

    // Decoding the data 1 char = 1 byte
    char *decoded_data = calloc(len_data / 8 + 1, sizeof(char));
    for (int i = 0; i < len_data / 8; i++) {
        for (int j = 0; j < 8; j++) {
            decoded_data[i] <<= 1;
            decoded_data[i] |= data_bits[i * 8 + j];
        }
    }

    decoded_data[len_data / 8] = '\0';

    free(data_bits);
    stbi_image_free(data);
    return decoded_data;
}

void print_help() {
    printf("Usage:\n");
#ifdef ENCODE
    printf("  Encode: ./encode <input_image> <message> <output_image>\n");
#else
    printf("  Decode: ./read <input_image>\n");
#endif
}

int main(int argc, char *argv[]) {
#ifdef ENCODE
    if (argc != 4) {
        print_help();
        return 1;
    }
    char *text = argv[2];
    char hex[SHA256_HEX_SIZE];
    sha256_hex(text, strlen(text), hex);
    // Add the hash to the text
    char *new_text = malloc(strlen(text) + SHA256_HEX_SIZE + 1);
    strcpy(new_text, text);
    strcat(new_text, hex);
    print(argv[1], new_text, argv[3]);
    free(new_text);
#else
    if (argc != 2) {
        print_help();
        return 1;
    }
    char *ans = read(argv[1]);
    int len = strlen(ans);
    char hash[SHA256_HEX_SIZE + 1];
    strcpy(hash, ans + len + 1 - SHA256_HEX_SIZE);
    ans[len + 1 - SHA256_HEX_SIZE] = 0;
    char real_hash[SHA256_HEX_SIZE];
    sha256_hex(ans, len + 1 - SHA256_HEX_SIZE, real_hash);
    if (strcmp(real_hash, hash) != 0) {
        printf("Hash mismatch\n");
        free(ans);
        return 1;
    }
    printf("Decoded data: %s\n", ans);
    free(ans);
#endif
    return 0;
}
