# Png Steganography

## Features
``
- Encode ASCII messages into PNG images
- Decode messages from encoded PNG images
- Error detection and correction using Hamming(7,4) code
- SHA256 hash for message integrity verification

## Requirements

- C compiler (Tested on Ubuntu with gcc 14.1.0 built from source)
- Make

## Libraries Used

- stb_image.h and stb_image_write.h: These libraries are used for reading and writing PNG images.
  They were chosen becuase they are simple and header-only libraries, which simplifies compilation and distribution.
- sha256.h and sha256.c: Used for generating SHA256 hashes for message integrity verification. Used because efficiently
  calculating SHA256 hashes is a non-trivial task and this library provides a simple interface for this purpose.

## Compilation

To compile the project, run the following commands in the project directory:

```bash
make clean
make
```

This will create two executables in the `bin` directory:

- `encode`: Used for encoding messages into PNG images
- `decode`: Used for decoding messages from encoded PNG images

## Usage

### Encoding

```
./bin/encode <input_image> <message> <output_image>
```

Example:

```
./bin/encode image.png "Hello, World!" encoded_image.png
```

### Decoding

```
./bin/decode <input_image>
```

Example:

```
./bin/decode encoded_image.png
```

## Implementation Details

1. **Encoding Method**: The project uses the Least Significant Bit (LSB) steganography to embed data in the
   image.
   The format the data is encoded in is:
    - Length of the message to read (32 bits)
    - SHA256 hash of the original message (256 bits)
    - Encoded message data

2. **Error Correction**: A Hamming(7,4) code is implemented for error detection and correction.

3. **Data Integrity**: A SHA256 hash of the original message is prepended to the encoded data.
   During decoding, this hash is used to verify the integrity of the decoded message.

4. **Capacity**: The first 32 bits of the encoded data store the length of the message, allowing for dynamic message
   sizes up to the capacity of the image.

## Design Decisions

1. **Choice of Encoding Method**: LSB steganography was chosen for its simplicity and minimal impact on the visual
   quality of the image.

2. **Error Correction**: Hamming(7,4) code provides a good balance between error correction capability and data
   overhead.
   It can correct single-bit errors evey 7 bits, which are the most common in this context.

3. **Data Integrity**: SHA256 hash was chosen for its strong cryptographic properties, ensuring that any tampering with
   the encoded message can be detected.

4. **PNG Library**: The stb_image and stb_image_write libraries were chosen for their simplicity and being header-only,
   which simplifies the build process of the project.

5. **Profile-Guided Optimization**: PGO is used to improve the performance of the binaries based on typical
   usage patterns.
   The downside is that the build process is more complex, and longer, but this is an acceptable trade-off.

## Limitations and Future Improvements

- The current implementation does not encrypt the message, only encodes it. Adding encryption could make this more
  useful.
- The current format of the encoded data has a fatal flaw, if the length of the message is tampered with, the decoding
  will likely fail.
- The error correction method can only correct single-bit errors. More advanced error correction codes could be
  implemented for higher reliability.
  Or even a different image encoding method such as fourier-transform-based steganography.

## Platform

This project was developed and tested on Ubuntu 24.04.1. While it may work on other platforms, cross-platform
compatibility is not guaranteed.

GCC 14.1.0 was used for development, and was built from
source [using this script](https://github.com/amuhak/make-gcc/blob/main/build.sh).