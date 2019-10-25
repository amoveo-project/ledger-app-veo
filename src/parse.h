#ifndef _PARSE_H_
#define _PARSE_H_

#include "os.h"

#include "base64.h"
#include "utils.h"
#include "config.h"

typedef struct {
  char *start;
  unsigned int length;
  bool is_number;
} tx_token_t;

extern char curr_tx_desc[MAX_TX_TEXT_LINES][MAX_TX_TEXT_WIDTH];

void prepare_text_description(tx_token_t *);
int serialize(tx_token_t *, unsigned int, unsigned char *);
int parse(unsigned char *, unsigned int, tx_token_t *);


#endif
