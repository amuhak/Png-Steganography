#include <stdio.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"
#include "stb_image.h"

void encode(const char *image_path, const char *str) {
    // x, y, and no of channels in the image
    int x, y, n;

    // Load the image into an 8-bit array so its easy to manipulate
    uint8_t *data = stbi_load(image_path, &x, &y, &n, 0);

    int data_size = x * y * n;

    // printf("x: %d y: %d n: %d \n", x, y, n);


    int len = strlen(str);
    int no_of_bits = 8 * len;

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

    // printf("no_of_bits: %d \n", no_of_bits);

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
        head++;
    }

    free(bits);

    // Writing the data to the image
    stbi_write_png("./out.png", x, y, n, data, x * n);

    // Freeing the data
    stbi_image_free(data);
}

char *decode(const char *image_path) {

    // reading the data from the image
    int x, y, n;

    uint8_t *data = stbi_load("./out.png", &x, &y, &n, 0);
    int data_size = x * y * n;
    uint8_t *head = data;

    // Find the length of the data (first 32 bits)
    int len_data = 0;
    for (int i = 0; i < 32; i++) {
        len_data = len_data << 1;
        len_data |= *head & 1;
        head++;
    }
    // printf("len_data: %d\n", len_data);

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
            decoded_data[i] = decoded_data[i] << 1;
            decoded_data[i] |= data_bits[i * 8 + j];
        }
    }

    decoded_data[len_data / 8] = '\0';

    free(data_bits);
    stbi_image_free(data);
    return decoded_data;
}

int main(void) {
    encode("./image.png", "Hello World");
    char *ans = decode("./out.png");
    printf("Decoded data: %s\n", ans);
    free(ans);
    return 0;
}
