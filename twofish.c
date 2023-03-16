#define MODULE_NAME "encryption"
#define MAKING_ENCRYPTION

#include "src/mod/module.h"
#include "twofish.h"

#define LittleEndian 1
#define ALIGN32 0
#undef CONST
#include "aes.h"

#undef global
static Function *global = NULL;

static keyInstance ki;
static cipherInstance ci;
static int key_size = 128;

// Yeah right. Maybe later!
static int twofish_expmem()
{
	return(0);
}

static void twofish_report(int idx, int details)
{
  if (details) {
    dprintf(idx, "    Twofish encryption module:\n"
                 "    Thanks for using Twofish! You rock!\n");
  }
}

/* Convert 64-bit encrypted password to text for userfile */
static char *base64 = "./0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static int base64dec(char c)
{
  int i;

  for (i = 0; i < 64; i++)
    if (base64[i] == c)
      return i;
  return 0;
}

// Initializes ki and ci (keyInstance and cipherInstance)
static void twofish_init (char *key, int keylen)
{
	memset(&ki, 0, sizeof(ki));
	memset(&ci, 0, sizeof(ci));
	makeKey(&ki, DIR_ENCRYPT, key_size, NULL);
	cipherInit(&ci, MODE_CBC, NULL);
	if (keylen > sizeof(ki.key32)) keylen = sizeof(ki.key32);
	memcpy(ki.key32, key, keylen);
	reKey(&ki);
}

static void twofish_encipher (char *input, int inputlen)
{
	char output[BLOCK_SIZE/8];

	//while (inputlen > 0) {
		blockEncrypt(&ci, &ki, (BYTE *)input, BLOCK_SIZE, (BYTE *)output);
		memcpy(input, output, BLOCK_SIZE/8);
	//	input += BLOCK_SIZE/8;
	//	inputlen -= BLOCK_SIZE/8;
	//}
}

static void twofish_decipher (char *input, int inputlen)
{
	char output[BLOCK_SIZE/8];

	//while (inputlen > 0) {
		blockDecrypt(&ci, &ki, (BYTE *)input, BLOCK_SIZE, (BYTE *)output);
		memcpy(input, output, BLOCK_SIZE/8);
	//	inputlen -= BLOCK_SIZE/8;
	//	input += BLOCK_SIZE/8;
	//}
}

static void twofish_encrypt_pass(char *text, char *new)
{
	int i, left, right;
	char *p, block[BLOCK_SIZE/8];

	i = strlen(text);
	if (i < BLOCK_SIZE/8) memset(block, 0, BLOCK_SIZE/8);
	else i = BLOCK_SIZE/8;
	memcpy(block, text, i);

	twofish_init(text, strlen(text));
	twofish_encipher(block, BLOCK_SIZE/8);

	p = new;
	*p++ = '+';			/* + means encrypted pass */
	memcpy(&right, block, sizeof(right));
	memcpy(&left, block+sizeof(right), sizeof(left));
	for (i = 0; i < 6; i++) {
		*p++ = base64[right & (int) 0x3f];
		right = (right >> 6);
		*p++ = base64[left & (int) 0x3f];
		left = (left >> 6);
	}
	*p = 0;
}

/* Returned string must be freed when done with it!
 */
static char *encrypt_string(char *key, char *str)
{
	char *input_, *input, *output, *output_, block[BLOCK_SIZE/8];
	int i, j, k;

	k = strlen(str);
	input_ = nmalloc(k+BLOCK_SIZE/8+1);
	memcpy(input_, str, k);
	memset(input_+k, 0, BLOCK_SIZE/8+1);
	if ((!key) || (!key[0])) return(input_);

	output_ = nmalloc((k + BLOCK_SIZE/8 + 1) * 2);

	input = input_;
	output = output_;

	twofish_init(key, strlen(key));

	while (*input) {
		memcpy(block, input, BLOCK_SIZE/8);
		input += BLOCK_SIZE/8;
		twofish_encipher(block, BLOCK_SIZE/8);
		for (i = 0; i < BLOCK_SIZE/8; i += 4) {
			memcpy(&k, block+i, 4);
			for (j = 0; j < 6; j++) {
				*output++ = base64[k & 0x3f];
				k = (k >> 6);
			}
		}
	}
	*output = 0;
	nfree(input_);
	return(output_);
}

/* Returned string must be freed when done with it!
 */
static char *decrypt_string(char *key, char *str)
{
	char *input, *input_, *output, *output_, block[BLOCK_SIZE/8];
	int i, j, k;

	i = strlen(str);
	input_ = nmalloc(i + BLOCK_SIZE/8+21);
	memcpy(input_, str, i);
	memset(input_+i, 0, BLOCK_SIZE/8+21);
	if ((!key) || (!key[0])) return(input_);

	output_ = nmalloc(i + BLOCK_SIZE/8+21);
	memset(output_, 0, i+BLOCK_SIZE/8+21);

	twofish_init(key, strlen(key));

	input = input_;
	output = output_;

	while (*input) {
		for (i = 0; i < BLOCK_SIZE/8; i += 4) {
			k = 0;
			for (j = 0; j < 6; j++)
				k |= (base64dec(*input++)) << (j * 6);
			memcpy(block+i, &k, 4);
		}
		twofish_decipher(block, BLOCK_SIZE/8);
		for (i = 0; i < BLOCK_SIZE/8; i += 4) {
			memcpy(&k, block+i, 4);
			for (j = 3; j >= 0; j--)
				*output++ = (k & (0xff << ((3 - j) * 8))) >> ((3 - j) * 8);
		}
	}
	*output = 0;
	nfree(input_);
	return(output_);
}

static int tcl_encrypt STDVAR
{
  char *p;

  BADARGS(3, 3, " key string");
  p = encrypt_string(argv[1], argv[2]);
  Tcl_AppendResult(irp, p, NULL);
  nfree(p);
  return TCL_OK;
}

static int tcl_decrypt STDVAR
{
  char *p;

  BADARGS(3, 3, " key string");
  p = decrypt_string(argv[1], argv[2]);
  Tcl_AppendResult(irp, p, NULL);
  nfree(p);
  return TCL_OK;
}

static int tcl_encpass STDVAR
{
  BADARGS(2, 2, " string");
  if (strlen(argv[1]) > 0) {
    char p[16];
    twofish_encrypt_pass(argv[1], p);
    Tcl_AppendResult(irp, p, NULL);
  } else
    Tcl_AppendResult(irp, "", NULL);
  return TCL_OK;
}

static int tcl_set_key_size STDVAR
{
	int new_key;

	BADARGS(2, 2, " new_key_size (128, 192, 256)");
	new_key = atoi(argv[1]);
	if (new_key != 128 && new_key != 192 && new_key != 256) {
		Tcl_AppendResult(irp, "new_key_size must be 128, 192, or 256", NULL);
		return TCL_ERROR;
	}
	else {
		key_size = new_key;
		return TCL_OK;
	}
}

static tcl_cmds mytcls[] =
{
  {"encrypt",	tcl_encrypt},
  {"decrypt",	tcl_decrypt},
  {"encpass",	tcl_encpass},
  {"set_key_size", tcl_set_key_size},
  {NULL,	NULL}
};

/* You CANT -module an encryption module , so -module just resets it.
 */
static char *twofish_close()
{
  return "You can't unload an encryption module";
}

char *twofish_start(Function *);

static Function twofish_table[] =
{
  /* 0 - 3 */
  (Function) twofish_start,
  (Function) twofish_close,
  (Function) twofish_expmem,
  (Function) twofish_report,
  /* 4 - 7 */
  (Function) encrypt_string,
  (Function) decrypt_string,
};

char *twofish_start(Function *global_funcs)
{
  /* `global_funcs' is NULL if eggdrop is recovering from a restart.
   *
   * As the encryption module is never unloaded, only initialise stuff
   * that got reset during restart, e.g. the tcl bindings.
   */
  if (global_funcs) {
    global = global_funcs;

    if (!module_rename("twofish", MODULE_NAME))
      return "Already loaded.";

    module_register(MODULE_NAME, twofish_table, 2, 4);
    if (!module_depend(MODULE_NAME, "eggdrop", 108, 4)) {
      module_undepend(MODULE_NAME);
      return "This module requires Eggdrop 1.8.4 or later.";
    }
    add_hook(HOOK_ENCRYPT_PASS, (Function) twofish_encrypt_pass);
    add_hook(HOOK_ENCRYPT_STRING, (Function) encrypt_string);
    add_hook(HOOK_DECRYPT_STRING, (Function) decrypt_string);
  }
  add_tcl_commands(mytcls);
  return NULL;
}
