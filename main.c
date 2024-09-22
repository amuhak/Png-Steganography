#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "sha256.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"
#include "stb_image.h"

bool *encode(bool *in, int *in_len) {
    // 7,4 Hamming code
    int blocks = (*in_len + 3) / 4;
    *in_len = ((*in_len + 3) / 4) * 7;
    bool *out = (bool *) malloc((*in_len) * sizeof(bool));

    for (int i = 0; i < blocks; i++) {
        bool *data = out + i * 7;  // 7 bits for each block

        data[3 - 1] = in[i * 4];
        data[5 - 1] = in[i * 4 + 1];
        data[6 - 1] = in[i * 4 + 2];
        data[7 - 1] = in[i * 4 + 3];
        data[1 - 1] = data[3 - 1] ^ data[5 - 1] ^ data[7 - 1];
        data[2 - 1] = data[3 - 1] ^ data[6 - 1] ^ data[7 - 1];
        data[4 - 1] = data[5 - 1] ^ data[6 - 1] ^ data[7 - 1];
    }
    return out;
}

char *decode(bool *in, int in_len) {
    int out_len = in_len / 7 * 4;
    bool *out = calloc(out_len, sizeof(bool));

    for (int i = 0; i < in_len / 7; i++) {
        bool *data = in + i * 7;
        bool *decoded = out + i * 4;

        int problem = 0;
        for (int j = 0; j < 7; j++) {
            if (data[j]) {
                problem ^= (j + 1);
            }
        }

        // If there is a problem, correct it
        if (problem != 0) {
            data[problem - 1] = !data[problem - 1];
        }

        decoded[0] = data[3 - 1];
        decoded[1] = data[5 - 1];
        decoded[2] = data[6 - 1];
        decoded[3] = data[7 - 1];
    }
    char *result = calloc(out_len + 10, sizeof(char));
    for (int i = 0; i < out_len / 8; i++) {
        int c = 0;
        for (int j = 0; j < 8; j++) {
            c = (c << 1) | (out[i * 8 + j] ? 1 : 0);
        }
        result[i] = (char) c;
    }

    free(out);
    return result;
}

bool *char_to_bits(const char *str, int len) {
    bool *bits = malloc(len * 8 * sizeof(bool));
    for (int i = 0; i < len; i++) {
        for (int j = 7; j >= 0; j--) {
            bits[i * 8 + 7 - j] = (str[i] >> j) & 1;
        }
    }
    return bits;
}

void print(const char *image_path, bool *input, int no_of_bits, char *out_path) {
    // x, y, and no of channels in the image
    int x, y, n;

    // Load the image into an 8-bit array so its easy to manipulate
    uint8_t *data = stbi_load(image_path, &x, &y, &n, 0);

    int data_size = x * y * n;

    if (no_of_bits + 32 > data_size) {
        printf("Data is too large for the image\n could print only %d bytes\n", (data_size - 32) / 8);
        return;
    }

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
        if (input[i]) {
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

    // Writing the data to the image
    stbi_write_png(out_path, x, y, n, data, x * n);

    // Freeing the data
    stbi_image_free(data);
}

bool *read(const char *image_path, int *len) {

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

    *len = len_data;

    // Reading the data
    bool *data_bits = malloc(len_data * sizeof(bool));
    for (int i = 0; i < len_data; i++) {
        data_bits[i] = *head & 1;
        head++;
    }

    stbi_image_free(data);
    return data_bits;
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
    char text_hash[SHA256_HEX_SIZE];
    sha256_hex(text, strlen(text), text_hash);

    // Add the hash to the text Hash then text
    char *new_text = malloc(strlen(text) + SHA256_HEX_SIZE + 1);
    strcpy(new_text, text_hash);
    strcat(new_text, text);
    int len[1];
    *len = strlen(new_text);
    bool *bits = char_to_bits(new_text, *len);
    *len *= 8;
    bool *temp = encode(bits, len);
    print(argv[1], temp, *len, argv[3]);
    free(new_text);
    free(bits);
    free(temp);
#else
    if (argc != 2) {
        print_help();
        return 1;
    }

    int len;
    bool *encoded_data = read(argv[1], &len);
    encoded_data[0] = !encoded_data[0];
    char *decoded_data = decode(encoded_data, len);
    len = strlen(decoded_data);
    decoded_data[len] = '\0';

    if (len < SHA256_HEX_SIZE) {
        fprintf(stderr, "Decoded data is too short to contain a valid hash.\n");
        free(encoded_data);
        free(decoded_data);
        return 1;
    }

    char hash[SHA256_HEX_SIZE];
    char real_hash[SHA256_HEX_SIZE];

    // Extract the hash from the decoded data
    strncpy(hash, decoded_data, SHA256_HEX_SIZE - 1);
    hash[SHA256_HEX_SIZE - 1] = '\0';

    // The real text starts after the hash
    char *real_text = decoded_data + SHA256_HEX_SIZE - 1;

    // Calculate the hash of the real text
    sha256_hex(real_text, strlen(real_text), real_hash);

    // Compare the hash
    if (strcmp(real_hash, hash) != 0) {
        printf("Hash mismatch\n");
        printf("This means the data is corrupted\n");
        printf("Expected hash: %s\n", hash);
        printf("Actual hash: %s\n", real_hash);
        printf("Decoded data: %s\n", real_text);
    } else {
        printf("Decoded data: %s\n", real_text);
        printf("Hash matches\n");
        printf("It worked \n");
    }

    free(decoded_data);
    free(encoded_data);
#endif
    return 0;
}