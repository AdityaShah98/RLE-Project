#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 TODO:
 encoding/decoding bugs
 test on linux with command line compiler
 valgrind to check for memory leaks
 write test cases
 */

void WriteFile(unsigned const char* buf, const long size, char* filename, const int mode){
    /*
     * Function:  WriteFile
     * --------------------
     * Writes the given char buffer to a file with a modified filename.
     *
     *  buf: encoded or decoded character array
     *  size: size of the given character array
     *  filename: original name of the inputted file
     *  mode: 0 for writing a compressed file, 1 for writting a decompressed file
     *
     */
    
    const char* extention = ".rle\0";
    char * newname = (char *) malloc(1 + strlen(filename) + strlen(extention));
    if (!mode){ //add .rle file extention
        strcat(newname, filename);
        strcat(newname, extention);
    } else { // remove .rle file extention if exists
        char* point = filename;
        if((point = strrchr(filename,'.')) != NULL ) {
            if(strcmp(point,".rle") == 0) { //if .rle extention exists
                filename[strlen(filename)-4] = '\0'; //delete .rle extention
            }
        }
        strcat(newname, filename);
    }

    // Open File for writing
    FILE *f_dst = fopen(newname, "wb");
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

int Encode(unsigned char* source, const long size,  char* filename){
    /* Maximum size of encoded file is 2 * original size, considering worst-case
     scenario of no repetition among characters. +1 for null-termination. */
    unsigned char *enc = malloc(sizeof (char) * (size*2 + 1));
    
    long new_size = 0;
    long i = 0;
    long j = 0;
    
    while (i < size){
        signed int count = 1;
        if (i < size - 1){ //if not on the last char of file (we can compare to next)
            if (source[i] == source[i+1]){ //if equal to the next character on the file (POSITIVE RUN)
                while (i < size - 1 && source[i] == source[i + 1]) {
                    count++; //count number of repeating charactrs in a row Until last character)
                    i++;
                }
                // Add count for each > 127
                while (count > 127){
                    enc[j] = 127;
                    enc[j+1] = source[i];
                    count -= 127;
                    j+=2;
                    new_size += 2;
                }
                // Add final < 127 count
                enc[j] = count;
                enc[j+1] = source[i];
                j+=2;
                new_size += 2;
            } else { //if not equal to the next character on the file (NEGATIVE RUN)
                //2 cases, next char = last char or next char is part of a positive run
//                if (i < size -2 && source[i+1] != source[i+2]){ //if next char is not part of a positive run
                    count = -1;
                    long count_index = j;
                    j++;
                    while (i < size && source[i] != source[i+1]) {
                        count--;
                        enc[j] = source[i];
                        j++;
                        i++;
                        new_size++;
                        if (count < -128){
                            /* If only 1 repeated character left in unique run (after reset),
                             it is added as a solo NEGATIVE run. Not a problem for decoding.*/
                            i-=1;
                            enc[count_index] = -128;
                            count = 0;
                            count_index = j;
                        }
                    }
                    //backtrack one (when starting a positive run or when last char)
                    count += 1;
                    i-=1;
                    j--;
                
                    enc[count_index] = count;
                    new_size++;
                    j++;
//                } else { //if char is between 2 positive runs, keep it a positive run
//                    enc[j] = count;
//                    enc[j+1] = source[i];
//                    j+=2;
//                    new_size += 2;
//                }
                
            }
        } else { //if on the last character of the file
            enc[j] = count;
            enc[j+1] = source[i];
            j+=2;
            new_size += 2;
        }
        i++; //move forward
    }
    enc[j++] = '\0'; //null terminate encoded buffer
    
    
    WriteFile(enc, new_size, filename, 0);
    //free buffer memory
    free(enc);
    printf("Size of file after compression: %ld Bytes\n", new_size);
    
    return(1);
}

int Decode(unsigned char* source, const long size,  char* filename){
    /* Max decoded file size is 2 times the source size (+1 for null termination).
     Consider the compressed string "1a". Decompressed, it would be "a", 0.5 * original size */
    unsigned char *dec = malloc(sizeof (char) * (size*2 + 1));
//    long tempsize = sizeof (char) * (size*2 + 1);
    long new_size = 0;
    
    long i = 0;
    long j = 0;
    while (i < size){
        signed char c = source[i]; //convert to signed char
        int count = c;
        
        if (count > 0){ //positive run
            while (count > 0){
                dec[j] = source[i+1]; //set charcter
                j++;
                new_size++; //increment size of decoded file
                count--;
            }
            i+=2;
        } else { //negative run /*TODO: SOMETHING BAD HAPPENING HERE*/
            while (count < 0){
                i++;
                dec[j] = source[i];
                j++;
                count++;
                new_size++;
            }
            i++;
        }
    }

    WriteFile(dec, new_size, filename, 1);
    
    //free buffer memory
    free(dec);
    
    return (1);
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
    /*
     TODO: handle cases of very large (>INT_MAX) runs
     2's complement range: -128 to 127
     */
    
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
    
    printf("Size of file to compress: %ld Bytes\n", size);

    float entropy = ShannonEntropy(sourcebuf, size);
    printf("Shannon Entropy: %2.5f\n", entropy);
    
    printf("Smallest possible size after lossless compression: %ld Bytes\n", (long)(size * entropy/8.0f));

    Encode(sourcebuf, size, filename);
    
    //free buffer memory
    free (sourcebuf);
    
    return 1;
}

int Decompress(FILE *fp, const char* filename){
    
    // Get file size
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
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
    
    printf("Size of file to decompress: %ld Bytes\n", size);
    
    Decode(sourcebuf, size, filename);
    // free buffer memory
    free(sourcebuf);
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
            if (Decompress(file, argv[2]) == 1){
                printf("File decompression suceeded\n");
            } else {
                printf("File decompression failed\n");
            }
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

