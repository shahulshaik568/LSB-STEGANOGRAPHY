/*Documentation
Date:12/11/25
sk shahul
project:steganography
to encode the secret file into the beautiful.bmp
and to decode from beautiful.bmp*/
#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{

    // Step 1 : Check the argc >= 4 true - > step 2
    // Step 2 : Call the check_operation_type(argv[1]) == e_encode )) true - > step 3 
    // Step 3 : Declare structure variable EncodeInfo enc_info    
    // Step 4 : Call the read_and_validate_encode_args(argv,&enc_info)== e_success)    
    //  true -> Step 5 , false - > terminate the program    
    // Step 5 : Call the do_encoding (&encInfo);

    if(argc>=3)
    {
        if(check_operation_type(argv[1])==e_encode)
        {
            EncodeInfo enc_info;
            if(read_and_validate_encode_args(argv,&enc_info)==e_success)
            {
                do_encoding(&enc_info);
            }
            
            return 0;
        }
        else if(check_operation_type(argv[1])==e_decode)
            {
                DecodeInfo dec_info;
                if(read_and_validate_decode_args(argv,&dec_info)==e_success)
                do_decoding(&dec_info);
            }
        return 0;
    }
    else
    {
        printf("Error!!!\n");
        return 0;
    }
}

OperationType check_operation_type(char *symbol)
{
    // Step 1 : Check whether the symbol is -e or not true - > return e_encode false -> Step 2
    // Step 2 : Check whether the symbol is -d or not true - > return e_decode false -> return e_unsupported
    if(strcmp(symbol,"-e")==0)
    {
       return e_encode;
    }
   else if(strcmp(symbol,"-d")==0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported ;
    }
}
