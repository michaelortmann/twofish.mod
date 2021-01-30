#define LittleEndian 1
#define ALIGN32 1

#include "aes.h"

keyInstance ki;
cipherInstance ci;
int key_size = 128;

static void twofish_init (char *key, int keylen)
{
        memset(&ki, 0, sizeof(ki));
        memset(&ci, 0, sizeof(ci));
        makeKey(&ki, DIR_ENCRYPT, key_size, NULL);
        cipherInit(&ci, MODE_ECB, NULL);
        if (keylen > sizeof(ki.key32)) keylen = sizeof(ki.key32);
        //memcpy(ki.key32, key, keylen);
        reKey(&ki);
}

static void twofish_encipher (char *input, int inputlen)
{
        char *output;
        int nbits;

        if (inputlen % 8) return;
        output = (char *)malloc(inputlen+8);
        memset(output, 0, inputlen+8);
        nbits = blockEncrypt(&ci, &ki, input, inputlen*8, output);
        memcpy(input, output, inputlen);
	free(output);
}

static char *base64 = "./0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void twofish_encrypt_pass(char *text, char *new)
{
        int i, left, right;
        char *p, input[8];

        //i = strlen(text);
        //if (i < 8) memset(input, 0, 8);
        //else i = 8;
        //memcpy(input, text, i);
        memset(input, 0, 8);
	twofish_init(text, strlen(text));
        twofish_encipher(input, 8);

        p = new;
        *p++ = '+';                     /* + means encrypted pass */
        memcpy(&right, input, sizeof(right));
        memcpy(&left, input+sizeof(right), sizeof(left));
        for (i = 0; i < 6; i++) {
                *p++ = base64[right & (int) 0x3f];
                right = (right >> 6);
                *p++ = base64[left & (int) 0x3f];
                left = (left >> 6);
        }
        *p = 0;
}


main ()
{
	char buf[16];
	twofish_encrypt_pass("dragon", buf);
	printf("%s\n", buf);
}
