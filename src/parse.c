#define JSMN_STATIC
#include "jsmn.h"
#include "string.h"
#include "parse.h"
#include "veo.h"

char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];


int parse(unsigned char *src, unsigned int src_len, tx_token_t *tx_tokens) {
  jsmn_parser parser;
  jsmntok_t tokens[11];

  unsigned int tokens_count = 0;

  jsmn_init(&parser);
  char r = jsmn_parse(&parser, src, src_len, tokens, 11);
  if (r < 0)
    return r;

  tx_token_t tx_token;

  for (int i=1; i<11; i++) {
    if (tokens[i].type > JSMN_PRIMITIVE || tokens[i].type < JSMN_OBJECT) {
      break;
    }

    *(src + tokens[i].end) = '\0';
    tx_token.start = src + tokens[i].start;
    tx_token.length = tokens[i].end - tokens[i].start;
    tx_token.is_number = tokens[i].type == JSMN_PRIMITIVE;

    tx_tokens[i-1] = tx_token;
    tokens_count++;
  }

  return tokens_count;
}

static void write_int(unsigned int value, unsigned char *out) {
    *out++ = 0xff & (value >> 24);
    *out++ = 0xff & (value >> 16);
    *out++ = 0xff & (value >> 8);
    *out++ = 0xff & value;
}

int serialize(tx_token_t *tokens, unsigned int tokens_count, unsigned char *result){
    unsigned char *p = result;

    *p++ = 2;
    p += 4 * sizeof(char); // skip space for int length

    unsigned char *token;
    unsigned int len;

    for (int i=0; i<tokens_count; i++) {
      token = tokens[i].start;

      PRINTF("Token:\n%.*h \n\n", tokens[i].length, token);

      if (i > 0) { // atom in the record
          // char first = token[0];
            if (tokens[i].is_number) {
                *p++ = 3; // int

                uint64_t num = atol(token);

                PRINTF("NUM: %.*h\n\n", 8, &num);

                for (char n=0; n<7; n++) {
                  write_int(0, p);
                  write_int(0, p + 4);
                  p += 8;
                }

                write_int((uint32_t)(num >> 32), p);
                write_int((uint32_t)(num), p + 4);
                p += 8;

            } else {
                *p++ = 0; // string

                char decoded[92];
                len = Base64decode(decoded, token);

                write_int(len, p);
                p += 4;

                os_memmove(p, decoded, len);
                p += len;
            }

        } else {
            *p++ = 4; // atom

            write_int(tokens[i].length, p);
            p += 4;

            os_memmove(p, token, tokens[i].length);
            p += tokens[i].length;
        }

        PRINTF("Len:\n%d \n\n", tokens[i].length);
    }

    len = p - result - 5;
    write_int(len, result + 1);

    PRINTF("OUT: %d\n", len + 5);

    PRINTF("Token:\n%.*h \n\n", len + 5, result);
    return len + 5;
}

static const char *tx_types[] = {"create_acc_tx", "spend"};
static const char not_supported[] = "tx type not supported";


void prepare_text_description(tx_token_t *tokens) {
  char txinfo[30];
  char address[50];
  char info[50];

  const unsigned char tx_type_len = tokens[0].length;
  if ( (os_memcmp(tokens[0].start, tx_types[0], tx_type_len) == 0) ||
       (os_memcmp(tokens[0].start, tx_types[1], tx_type_len) == 0) ) {

    char amount[20];
    adjust_decimals(tokens[5].start, tokens[5].length, amount, 20, 8);
    snprintf(txinfo, 30, "%s VEO", amount);

    char fee[20];
    adjust_decimals(tokens[3].start, tokens[3].length, fee, 20, 8);
    snprintf(info, 50, "%s VEO for %s", fee, tokens[0].start);

    char address_truncated[21];
    os_memmove(address_truncated, tokens[4].start, 20);
    address_truncated[20] = '\0';
    snprintf(address, 50, "%s...%s", address_truncated, tokens[4].start + 68);

  } else {
    os_memmove(txinfo, not_supported, strlen(not_supported));
    os_memmove(address, not_supported, strlen(not_supported));
    os_memmove(info, not_supported, strlen(not_supported));
  }

  os_memmove(curr_tx_desc[0], address, 50);
  os_memmove(curr_tx_desc[1], txinfo, 30);
  os_memmove(curr_tx_desc[2], info, 50);
}
