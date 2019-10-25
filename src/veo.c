#include "os.h"

#include "veo.h"
#include "utils.h"

typedef unsigned int uint;

unsigned char raw_tx[MAX_TX_RAW_LENGTH];


void derive_amoveo_keys(unsigned char *bip44_in,
                        cx_ecfp_private_key_t *privateKey,
                        cx_ecfp_public_key_t *publicKey) {
  /** BIP44 path, used to derive the private key from the
      mnemonic by calling os_perso_derive_node_bip32. */

  unsigned int bip44_path[BIP44_PATH_LEN];
  bip44_path[0] = 0x8000002c;  // `m` in derivation path
  bip44_path[1] = 0x800001e8;  // VEO id in SLIP-44
  uint32_t i;
  for (i = 2; i < BIP44_PATH_LEN; i++) {
    bip44_path[i] = (bip44_in[0] << 24) | (bip44_in[1] << 16) | (bip44_in[2] << 8) | (bip44_in[3]);
    bip44_in += 4;
  }
  unsigned char privateKeyData[32];
  os_perso_derive_node_bip32(CX_CURVE_256K1, bip44_path, BIP44_PATH_LEN, privateKeyData, NULL);
  cx_ecdsa_init_private_key(CX_CURVE_256K1, privateKeyData, 32, privateKey);

  if (publicKey != NULL) {
    // generate the public key.
    cx_ecdsa_init_public_key(CX_CURVE_256K1, NULL, 0, publicKey);
    cx_ecfp_generate_pair(CX_CURVE_256K1, publicKey, privateKey, 1);
  }
}
