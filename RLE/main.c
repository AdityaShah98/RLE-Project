#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*
 TODO:
 test on linux with command line compiler
 valgrind to check for memory leaks (free())
 
 */

void WriteFile(unsigned const char* buf, const long size, const char* filename){
    // Open File for writing
    FILE *f_dst = fopen("/Users/AdityaShah/Documents/Xcode/RLE/cheryl.jpeg.rl2", "wb");
    if(f_dst == NULL)
    {
        printf("ERROR - Failed to open file for writing\n");
        exit(1);
    }

    // Write Buffer
    if(fwrite(buf, 1, size, f_dst) != size)
    {
        printf("ERROR - Failed to write %ld bytes to file\n", size);
        exit(1);
    }

    // Close File
    fclose(f_dst);
    f_dst = NULL;
}

int Encode(unsigned char* source, const long size, const char* filename){
    /* Maximum size of encoded file is 2 * original size, considering worst-case
     scenario of no repetition among characters. +1 for null-termination. */
    unsigned char *enc = malloc(sizeof (char) * (size*2 + 1));
    
    long new_size = 0;
    int i = 0;
    int j = 0;
    while (i < size){
        unsigned int count = 0x01;
        while (i < size - 1 && source[i] == source[i + 1]) {
            count++;
            i++;
        }
        
        enc[j] = source[i];
        enc[j+1] = count;
        j+=2;
        
        i++;
        new_size += 2;
    }
    enc[j++] = '\0';
    
    
    WriteFile(enc, new_size, filename);
    return(1);
}

float ShannonEntropy(unsigned char* buf, const long size){
    float entropy = 0;
    float count = 0;
    long c = 0;
    long byte_count[256] = {0}; //initialize byte counts to 0

    for (int i = 0; i < size; i++){ //count different bytes
        c = (int) buf[i];
        byte_count[c] += 1;
    }

    for (int i = 0; i < 256; i++)
    {
        if (byte_count[i] > 0)
        {
            count = (float) byte_count[i] / (float) size;
            entropy += -count * log2f(count);
        }
    }
    return entropy;
}

int Compress(FILE *fp, const char* filename){
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp); //get file size
    if (size == -1){ perror("Failed: "); return 0;} //error
    rewind(fp); //seek back to file's beginning

    //Allocate approrpiately sized buffer
    unsigned char *sourcebuf = malloc(sizeof (char) * (size + 1)); //allocate size + 1 for null-termination

    //Read the entire file into memory
    size_t newLen = fread(sourcebuf, sizeof(char), size, fp);
    if ( ferror( fp ) != 0 ) {
        fputs("Error reading file", stderr);
    } else {
        sourcebuf[newLen++] = '\0'; /* Just to be safe. */
    }
    
    printf("Size of file: %ld Bytes\n", size);

    float entropy = ShannonEntropy(sourcebuf, size);
    printf("Shannon Entropy: %2.5f\n", entropy);
    
    printf("Smallest possible size after lossless compression: %ld Bytes\n", (long)(size * entropy/8.0f));

    Encode(sourcebuf, size, filename);
    
    
    
    return 1;
}

int Decompress(FILE *f, char* filename){
    return 1;
}

void ProcessCommandArgs(int argc, const char* argv[])
{
    if (argc == 3){ //if both mode and filename are supplied
        FILE *file = fopen(argv[2], "rb");
        if (file == NULL){
            perror("Failed: ");
            return;
        }
        if (atoi(argv[1]) == 1){ //if in compression mode
            if (Compress(file, argv[2]) == 1){
                printf("File compression suceeded\n");
            } else {
                printf("File compression failed\n");
            }
        } else if (atoi(argv[1]) == 2){ //if in decompression mode

        } else {
            printf("Enter either 1 or 2 for first argument. 1) Compression 2) Decompression.");
        }
        fclose(file);
    }
}

int main(int argc, const char* argv[])
{
    ProcessCommandArgs(argc, argv);
    return 0;
}

