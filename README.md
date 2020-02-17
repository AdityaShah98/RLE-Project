<img src="https://image.flaticon.com/icons/svg/1387/1387554.svg" title = "RLE Logo" width="160" align="right">

# Command Line RLE Compressor/Decompressor
>Run length encoder/decoder written in C.

## Description
This compressor uses positive- and negative-run RLE to lossleslly encode files.
* **Positive counts** represent the run length of repeated characters (i.e. `aaaa` encodes to `4a`).
* **Negative counts** represent the run length of non-repeating characters (i.e. `abcd` encodes to `-4abcd`). These negative counts minimize the number of bytes needed to encode a long non-repeating run (only 1 byte needed per 127 characters instead of the 1 byte needed for each character if only positive runs were used).

All counts are represented in encoded files as **signed, 8-bit integers** (-128 to 127).

**Note:** Compressed files will be saved with a `.rle` extention. `foo.txt` compresses to `foo.txt.rle`. Decompressed files will be saved with the original filename. **If a** `.rle` **extention is found, it will be removed, potentially overwriting a file with the same name**. All files will be written to the same folder as the source file.

Information entropy and optimal size after compression is calculated using Shannon's Theorem.

## Usage
1. `make`
2. `./RLE MODE FILENAME`: Mode is 1 (for compression) or 2 (for decompression)
