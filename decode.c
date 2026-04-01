#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"
#include <stdlib.h>

uint Dget_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);

    return size;
}

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Source Image
      if (argv[2] == NULL)//No Source File Found
    {
        printf("ERROR: Missing source image filename\n");
        return e_failure;
    }

    char *src_dot = strrchr(argv[2], '.');//check whether the file name is there or not(before .)
    if (src_dot == NULL || src_dot == argv[2]) // dot missing or at beginning
    {
        printf("ERROR: Invalid source image file name (must have name before .)\n");
        return e_failure;
    }
    //If there is dot in the Source File src_dot will return the address start from (.)

    if (strcmp(src_dot, ".bmp") != 0)//check the src file is having (.bmp) or not
    {
        printf("ERROR: Source image must be a .bmp file\n");
        return e_failure;
    }

    decInfo->src_image_fname = argv[2];

    // Output file name
    if (argv[3] != NULL)
    {
        // user provided output file
        decInfo->secret_fname = argv[3];
        printf("INFO: Output file specified as %s\n", decInfo->secret_fname);
    }
    else
    {
        // user did not provide output file — assign later after decoding extension
        decInfo->secret_fname = NULL;
        printf("INFO: No output file provided. Will generate automatically after reading extension.\n");
    }
    
    return e_success;
}

Status open_files_D(DecodeInfo *decInfo)
{
    // Src Image file
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname, "r");
    printf("INFO: Opened SkeletonCode/beautiful.bmp\n");
    // Do Error handling
    if (decInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->src_image_fname);

        return e_failure;
    }
    printf("INFO: Opened stego image file successfully\n");
    return e_success;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{   rewind(decInfo->fptr_src_image);
    printf("INFO: Decoding Magic String Signature\n");  
    fseek(decInfo->fptr_src_image, 54, SEEK_SET); // Skip BMP header
    char buffer[8];//To temporarily hold 8 bytes (8 pixels’ LSBs) that represent 1 hidden character.
    char ch;//To store the decoded character after extracting bits from the buffer.
    char decoded_char;//
    char magic_buf[10];//To store the entire decoded magic string (max 10 characte

    for (int i = 0; i < strlen(magic_string); i++)
    {
        //Reads 8 bytes (8 pixels) from the image file and stores them into buffer.
        fread(buffer, 8, 1, decInfo->fptr_src_image);
        decode_byte_from_lsb(&ch,buffer);
        magic_buf[i]=ch;
    }
    magic_buf[strlen(magic_string)] = '\0';
    
    if (strcmp(magic_buf, magic_string) == 0)
    {
        printf("INFO: Magic string verified successfully ✅\n");
        return e_success;
    }
    else
    {
        printf("ERROR: Magic String Mismatch ❌\n");
        return e_failure;
    }
}

Status decode_secret_file_extn_size(int size, DecodeInfo *decInfo)
{
    if (decInfo == NULL || decInfo->fptr_src_image == NULL)
        return e_failure;

    printf("INFO: Decoding Output File Extension\n");

    char size_buffer[32];
    if (fread(size_buffer, 32, 1, decInfo->fptr_src_image) != 1)
    {
        fprintf(stderr, "ERROR: Unable to read extension size bytes\n");
        return e_failure;
    }

    int extn_size = decode_size_from_lsb(size_buffer);
    if (extn_size <= 0)
    {
        fprintf(stderr, "ERROR: Invalid extension size decoded: %d\n", extn_size);
        return e_failure;
    }

    // printf("INFO: Extension size decoded = %d\n", extn_size);

    /* allocate extn_secret_file (+1 for null) */
    decInfo->extn_secret_file = malloc(extn_size + 1);
    if (decInfo->extn_secret_file == NULL)
    {
        perror("malloc");
        fprintf(stderr, "ERROR: Memory allocation for extension failed\n");
        return e_failure;
    }

    char buffer[8];
    char ch;
    for (int i = 0; i < extn_size; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_src_image) != 1)
        {
            fprintf(stderr, "ERROR: Unexpected EOF while reading extension bytes\n");
            free(decInfo->extn_secret_file);
            decInfo->extn_secret_file = NULL;
            return e_failure;
        }
        if (decode_byte_from_lsb(&ch, buffer) == e_failure)
        {
            free(decInfo->extn_secret_file);
            decInfo->extn_secret_file = NULL;
            return e_failure;
        }
        decInfo->extn_secret_file[i] = ch;
    }
    decInfo->extn_secret_file[extn_size] = '\0';

    printf("INFO: Decoded extension: %s\n", decInfo->extn_secret_file);

    /* prepare output filename */
    char output_fname[256] = "decoded";
    if ((int)strlen(output_fname) + extn_size + 1 < (int)sizeof(output_fname))
    {
        strcat(output_fname, decInfo->extn_secret_file);
    }
    else
    {
        fprintf(stderr, "ERROR: output filename too long\n");
        free(decInfo->extn_secret_file);
        decInfo->extn_secret_file = NULL;
        return e_failure;
    }

    printf("INFO: Output file will be %s\n", output_fname);
    decInfo->fptr_secret = fopen(output_fname, "w");
    if (decInfo->fptr_secret == NULL)
    {
        perror("fopen");
        free(decInfo->extn_secret_file);
        decInfo->extn_secret_file = NULL;
        return e_failure;
    }

    printf("DONE\n");
    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{   
    printf("INFO: Decoding secret.txt File Size\n");
    int file_size=0;
    decInfo->size_secret_file=0;
    
    char buffer[32];
    if(fread(buffer, 32, 1, decInfo->fptr_src_image)!=1)
    {   fprintf(stderr, "ERROR: Failed to read file size bits\n");
        return e_failure;
    }
    file_size=decode_size_from_lsb(buffer);
    decInfo->size_secret_file = file_size;
    // printf("INFO: Secret file size = %ld bytes\n", decInfo->size_secret_file);
       
    printf("DONE\n");
    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{   printf("INFO: Decoding secret.txt File Data\n");
    char buffer[8];
    char ch; 

    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_src_image);
        decode_byte_from_lsb(&ch,buffer);
        fputc(ch, decInfo->fptr_secret);
        //fputc(ch,stdout);
    }   printf("INFO: Secret data written successfully ✅\n");
        printf("DONE\n");
    return e_success;
}

Status decode_byte_from_lsb(char *data, char *image_buffer)
{   *data=0;
    for (int i= 0;i < 8; i++)
        {
            *data = (*data) | ((image_buffer[i] & 1)<<i);
        }
}

int decode_size_from_lsb(char *imageBuffer)
{   DecodeInfo *decInfo;
    
    int size=0;
    for (int i = 0; i < 32; i++)
    {
        size = size  | ((imageBuffer[i] & 1)<<i);
    }     
    
    return size;
}

Status do_decoding(DecodeInfo *decInfo)
{   
     if (open_files_D(decInfo) == e_failure)
        return e_failure;
    printf("INFO: ## DECODING PROCEDURE STARTED ##\n");

    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure) 
       return e_failure;

    if (decode_secret_file_extn_size(32, decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_size(decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_data(decInfo) == e_failure)
        return e_failure;

    printf("INFO: Decoding completed successfully!!!\n");

    fclose(decInfo->fptr_src_image);
    fclose(decInfo->fptr_secret);
    return e_success;

}
