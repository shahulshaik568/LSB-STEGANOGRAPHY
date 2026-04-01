#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "types.h"
#include"common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */ 

uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);
    
    // Return image capacity
    return width * height * 3;

}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr, 0, SEEK_END);
    uint size = ftell(fptr);
    rewind(fptr);
    return size;
    
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
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

    encInfo->src_image_fname = argv[2];
 
// Secret file
    if (argv[3] == NULL)
    {
        printf("ERROR: Missing secret file\n");
        return e_failure;
    }

    char *dot = strrchr(argv[3], '.');// Find last dot in filename
    if (dot == NULL || dot == argv[3])// Check if dot is missing or at the beginning (no name before dot)
    {
        printf("ERROR: Invalid secret file name (must have name before .)\n");
        return e_failure;
    }
    // Check for valid extension secret file is having (.txt or .c or .h or .sh) or not
    if (!(strcmp(dot, ".txt")==0 || strcmp(dot, ".c")==0 || strcmp(dot, ".h")==0 || strcmp(dot, ".sh")==0))
    {
        printf("ERROR: Secret file extension must be .txt, .c, .h, .sh\n");
        return e_failure;
    }

    strcpy(encInfo->extn_secret_file, dot);
    encInfo->secret_fname = argv[3];

// Stego image (optional)    
    if (argv[4] == NULL)//check whether the arg[4] is having NULL or not
    {
        printf("INFO: Output File not mentioned. Creating _default.bmp as default\n");
        encInfo->stego_image_fname = "default.bmp";  // Default output file
    }
    else
    {
        // Extract file extension
        char *dot = strrchr(argv[4], '.');

        // Check whether the file name is there or not (before '.')
        if (dot == NULL || dot == argv[4])
        {
            printf("ERROR: Invalid stego image file name (must have name before .)\n");
            return e_failure;
        }

        // Check if the file has .bmp extension
        if (strcmp(dot, ".bmp") != 0)
        {
            printf("ERROR: Stego image file must have .bmp extension\n");
            return e_failure;
        }

        // Valid case → store the filename
        encInfo->stego_image_fname = argv[4];
    }
    return e_success;
}

Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    printf("INFO: Opened SkeletonCode/beautiful.bmp\n");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    printf("INFO: Opened secret.txt\n");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    printf("INFO: Opened default.bmp\n");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

        return e_failure;
    }
    // No failure return e_success
    return e_success;
}

Status check_capacity(EncodeInfo *encInfo)
{   printf("INFO: Checking for SkeletonCode/beautiful.bmp capacity to handle secret.txt\n");
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    long total_bits_needed = 54 + (strlen(MAGIC_STRING)*8) + 32 + (strlen(encInfo->extn_secret_file) *8) + 32 + 32;
    // copy_bmp_header=54
    // magic_string=2*8=16
    // Source_file_extension_size=4*8=32
    // secret_file_extension=4*8=32(if.txt),3*8=24(if .sh),2*8=16(if .c or .h)
    // secret_file_size=32
    // secret_data=36*8=288
    // total=454

    if (encInfo->image_capacity > total_bits_needed)//image capacity->2359351 bytes
    {
        printf("DONE\n");
        return e_success;
    }
    else 
    {
        printf("ERROR: Image size insufficient for encoding\n");
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{   printf("INFO: Copying Image Header\n");

    rewind(fptr_src_image);
    char header[54];
    fread(header, 54, 1, fptr_src_image);
    fwrite(header, 54, 1, fptr_dest_image);
    printf("DONE\n");
    return e_success;
}
//        Style	                            Meaning	                                             Common Usage
// fread(header, 54, 1, file)	Read one structure (like a 54-byte BMP header)	    When reading a single block
// fread(header, 1, 54, file)	Read many small bytes	                            When reading raw bytes or text data

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{   printf("INFO: Encoding Magic String Signature\n");  

    char buffer[8];
     for(int i=0;i<strlen(magic_string);i++)
     {
           fread(buffer, 8, 1, encInfo ->fptr_src_image);
           encode_byte_to_lsb(magic_string[i],buffer);
           fwrite(buffer,8,1,encInfo-> fptr_stego_image);
         }
    printf("DONE\n");
    return e_success;
}
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{   printf("INFO: Checking for secret.txt size\n");
     char buffer[32];
     fread(buffer,32,1, encInfo->fptr_src_image);
     encode_size_to_lsb(size,buffer);
     fwrite(buffer,32,1,encInfo->fptr_stego_image);
   //  printf("INFO: Secret file extn size = %d bytes\n", size);
     printf("DONE\n"); 
    return e_success;
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{   printf("INFO: Encoding secret.txt File Extension\n");
     char buffer[8];
     for(int i=0;i<strlen(file_extn);i++)
     {
           fread(buffer, 8, 1,encInfo ->fptr_src_image);
           encode_byte_to_lsb(file_extn[i],buffer);
           fwrite(buffer,8,1,encInfo-> fptr_stego_image);
     }
        printf("DONE\n");
    return e_success;
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{   printf("INFO: Encoding secret.txt File Size\n");
    char buffer[32];
          fread(buffer, 32, 1, encInfo ->fptr_src_image);
          encode_size_to_lsb(file_size,buffer);
          fwrite(buffer,32,1,encInfo-> fptr_stego_image);
        //  printf("INFO: Secret file size = %ld bytes\n", encInfo->size_secret_file);
          printf("DONE\n");
    return e_success;
}

Status encode_secret_file_data(EncodeInfo *encInfo)
{   printf("INFO: Encoding secret.txt File Data\n");
    char buffer[8];
    char ch; 
    
    rewind(encInfo->fptr_secret);
    // Read each byte from the secret file 
    
    for(int i=0;i<(encInfo->size_secret_file);i++)
    {   // Read 1 byte from secret file
        if (fread(&ch, 1, 1, encInfo->fptr_secret) != 1)
        {
            fprintf(stderr, "ERROR: Failed to read from secret file\n");
            return e_failure;
        }
        // Read 8 bytes from source image
        if (fread(buffer, 8, 1, encInfo->fptr_src_image) != 1)
        {
            fprintf(stderr, "ERROR: Failed to read from source image\n");
            return e_failure;
        }
        // Encode 1 byte (8 bits) of secret data into 8 LSBs
        encode_byte_to_lsb(ch, buffer);

        // Write modified 8 bytes to stego image
        fwrite(buffer, 8, 1, encInfo->fptr_stego_image);
    }
        printf("DONE\n");
    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{   printf("INFO: Copying Left Over Data\n");
    char ch;
    while (fread(&ch, 1, 1, fptr_src))
    fwrite(&ch, 1, 1, fptr_dest);
    printf("DONE\n");
    return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{   
    for(int i=0;i<8;i++)
    {
        image_buffer[i]=image_buffer[i] &(~1) | (data>>i)&1;
    }
    
    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{   
    for(int i=0;i<32;i++)
    {
        imageBuffer[i]=imageBuffer[i] &(~1) | (size>>i)&1;
    }
    
    return e_success;
}

Status do_encoding(EncodeInfo *encInfo)
{   
     if (open_files(encInfo) == e_failure)
        return e_failure;
    printf("INFO: ## ENCODING PROCEDURE STARTED ##\n");

    if (check_capacity(encInfo) == e_failure)
        return e_failure;

     if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        return e_failure;

    if (encode_magic_string(MAGIC_STRING, encInfo) == e_failure) 
        return e_failure;

    if (encode_secret_file_extn_size((int) strlen(encInfo->extn_secret_file), encInfo) == e_failure)
        return e_failure;

    if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_failure)
        return e_failure;

    if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_failure)
        return e_failure;

    if (encode_secret_file_data(encInfo) == e_failure)
        return e_failure;

    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
        return e_failure;

    printf("INFO: Encoding completed successfully!!!\n");
    return e_success;

}
