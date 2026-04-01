#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>

#include "types.h" // Contains user defined types
#include <stdlib.h>
/*
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *src_image_fname; // To store the src image name
    FILE *fptr_src_image;  // To store the address of the src image

    /* Output File Info */ 
    char *secret_fname;       // To store the secret file name
    FILE *fptr_secret;        // To store the secret file address
    // char extn_secret_file[5]; // To store the Secret file extension
    char *extn_secret_file; 
    //char secret_data[100];    // To store the secret data
    long size_secret_file;    // To store the size of the secret data

} DecodeInfo;

/* Encoding function prototype */

/* Read and validate Decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *encInfo);

/* Perform the encoding */
Status do_decoding(DecodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
Status open_files_D(DecodeInfo *encInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *encInfo);

/*Decode extension size*/
Status decode_secret_file_extn_size(int size, DecodeInfo *encInfo);


/* Decode secret file size */
Status decode_secret_file_size( DecodeInfo *encInfo);

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *encInfo);

/* Decode a byte into LSB of image data array */
Status decode_byte_from_lsb(char* data, char *image_buffer);

// Decode a size to lsb
int decode_size_from_lsb(char *imageBuffer);


#endif
