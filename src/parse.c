#define JSMN_STATIC
#include "jsmn.h"
#include "parse.h"


static void write_int(unsigned int value, char *out) {
    *out++ = 0xff & (value >> 24);
    *out++ = 0xff & (value >> 16);
    *out++ = 0xff & (value >> 8);
    *out++ = 0xff & value;
}

int parse(char *src, unsigned int src_len, char *result){
    jsmn_parser parser;
    jsmntok_t tokens[11];
    char *p = result;

    jsmn_init(&parser);
    char r = jsmn_parse(&parser, src, src_len, tokens, 11);
    if (r < 0)
      return r;

    *p++ = 2;
    p += 4 * sizeof(char); // skip space for int length

    char *token;
    int len;

    for (int i=1; i<11; i++) {
        if (tokens[i].type > JSMN_PRIMITIVE || tokens[i].type < JSMN_OBJECT)
            break;

        *(src + tokens[i].end) = '\0';
        token = src + tokens[i].start;

        PRINTF("Token:\n%.*h \n\n", tokens[i].end - tokens[i].start, token);

        if (i > 1) { // atom in the record
          // char first = token[0];

            if (tokens[i].type == JSMN_PRIMITIVE) {
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

            len = tokens[i].end - tokens[i].start;

            write_int(len, p);
            p += 4;

            os_memmove(p, token, len);
            p += len;
        }

        PRINTF("Len:\n%d \n\n", tokens[i].end - tokens[i].start);
    }

    len = p - result - 5;
    write_int(len, result + 1);

    PRINTF("OUT: %d\n", len + 5);

    PRINTF("Token:\n%.*h \n\n", len + 5, result);
    return len + 5;
}
