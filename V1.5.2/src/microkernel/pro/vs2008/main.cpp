#include <Windows.h>
#include <stdio.h>
#include "mbedtls/aes.h"

void main(void)
{
	char a[17] = "1234567891234567";
	unsigned char aes_key[16];
	unsigned char input_buf[16];
	unsigned char output_buf[16];
	int aes_result = 0;
	unsigned char iv[16];
	mbedtls_aes_context aes_ctx;
	do 
	{
		memset(aes_key, 0, 16);
		memset(input_buf, 0, 16);
		memset(output_buf, 0, 16);
		memcpy(aes_key, a, 16);
		memcpy(input_buf, "1111111111111111", 16);

		
		mbedtls_aes_init( &aes_ctx );
		memset(iv, 0, 16);
		for(int i=0;i<8;i++)
		{
			iv[i]=i + 0x30;
		}
		aes_result = mbedtls_aes_setkey_enc(&aes_ctx, aes_key, 128);
		if (aes_result != 0)
		{
			break;
		}
		aes_result = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, 16, iv, input_buf, output_buf);
		if (aes_result != 0)
		{
			break;
		}
		mbedtls_aes_free(&aes_ctx);
	} while (0);
	
	printf("result:");
	for (int i=0; i<16; i++)
	{
		printf("%2X", output_buf[i]);
	}
	printf("\n");
	
	
	while(1)
	{
		Sleep(1000);
	}

	return;
}