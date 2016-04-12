#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char default_name[] = "crc_table";

#define CRCFACTORY_CRC_TYPE uint64_t
#define CRCFACTORY_CRCTABLE_TYPE uint64_t
#include "crcfactory.h"

typedef CRCFACTORY_CRCTABLE_TYPE crctable_t;

crctable_t crctable[256];

void dump_table(crctable_t *table, const char *type, const char *name, int cols, const char *fmt) {
    int i, j;

    printf("%s %s[256] = {\n", type, name);
    for (i = 0; i < 256; i += cols) {
        printf("    ");
        for (j = 0; j < cols; j++) {
            if (i+j >= 256) break;
            printf(fmt, table[i+j]);
        }
        printf("\n");
    }
    printf("};\n");
}

int main(int argc, char *argv[]) {
    int width;
    uint64_t poly;
    bool reflected;
    char *name;
    char *endp;

    if (argc < 4 || argc > 5) {
        fprintf(stderr, "Usage: %s width reflected poly [table-name]\n", argv[0]);
        return 1;
    }
    width = strtol(argv[1], &endp, 0);
    if (*endp || width < 1 || width > 64) {
        fprintf(stderr, "Invalid CRC width (bits) specified (\"%s\").  Must be a number between 1 and 64.\n", argv[1]);
        return 1;
    }
    if (!strcmp(argv[2], "true")) {
        reflected = true;
    } else if (!strcmp(argv[2], "false")) {
        reflected = false;
    } else {
        fprintf(stderr, "Bad value for 'reflected' setting (\"%s\").  Must be either 'true' or 'false'.\n", argv[2]);
        return 1;
    }
    errno = 0;
    poly = strtoull(argv[3], &endp, 0);
    if (errno || *endp) {
        fprintf(stderr, "Bad CRC polynomial specified (\"%s\").\n", argv[3]);
        return 1;
    }

    if (argc > 4) {
        name = argv[4];
    } else {
        name = default_name;
    }

    crcfactory_table_init(width, reflected, poly, crctable);

    if (width > 32) {
        dump_table(crctable, "uint64_t", name, 4, "0x%016lx,");
    } else if (width > 16) {
        dump_table(crctable, "uint32_t", name, 6, "0x%08x, ");
    } else if (width > 8) {
        dump_table(crctable, "uint16_t", name, 8, "0x%04x, ");
    } else {
        dump_table(crctable, "uint8_t", name, 12, "0x%02x, ");
    }

    return 0;
}
