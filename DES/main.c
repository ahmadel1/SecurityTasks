#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

static const int ip[64] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

static const int expansion[48] = {
    32, 1, 2, 3, 4, 5,
    4, 5, 6, 7, 8, 9,
    8, 9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32, 1
};


static const int ip_inverse[64] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9, 49, 17, 57, 25
};

static const int pc_1[56] = {
    57, 49, 41, 33, 25, 17, 9,
    1, 58, 50, 42, 34, 26, 18,
    10, 2, 59, 51, 43, 35, 27,
    19, 11, 3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
    7, 62, 54, 46, 38, 30, 22,
    14, 6, 61, 53, 45, 37, 29,
    21, 13, 5, 28, 20, 12, 4
};

static const int shift_amount[16] = {1, 2, 4, 6, 8, 10, 12, 14, 15, 17, 19, 21, 23, 25, 27, 28};

static const int pc_2[48] = {
    14, 17, 11, 24, 1, 5,
    3, 28, 15, 6, 21, 10,
    23, 19, 12, 4, 26, 8,
    16, 7, 27, 20, 13, 2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

static const int sbox[8][4][16] = {
    {
        {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7},
        {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
        {4, 1, 14, 8, 13, 6, 2, 11, 15, 12, 9, 7, 3, 10, 5, 0},
        {15, 12, 8, 2, 4, 9, 1, 7, 5, 11, 3, 14, 10, 0, 6, 13}
    },
    {
        {15, 1, 8, 14, 6, 11, 3, 4, 9, 7, 2, 13, 12, 0, 5, 10},
        {3, 13, 4, 7, 15, 2, 8, 14, 12, 0, 1, 10, 6, 9, 11, 5},
        {0, 14, 7, 11, 10, 4, 13, 1, 5, 8, 12, 6, 9, 3, 2, 15},
        {13, 8, 10, 1, 3, 15, 4, 2, 11, 6, 7, 12, 0, 5, 14, 9}
    },
    {
        {10, 0, 9, 14, 6, 3, 15, 5, 1, 13, 12, 7, 11, 4, 2, 8},
        {13, 7, 0, 9, 3, 4, 6, 10, 2, 8, 5, 14, 12, 11, 15, 1},
        {13, 6, 4, 9, 8, 15, 3, 0, 11, 1, 2, 12, 5, 10, 14, 7},
        {1, 10, 13, 0, 6, 9, 8, 7, 4, 15, 14, 3, 11, 5, 2, 12}
    },
    {
        {7, 13, 14, 3, 0, 6, 9, 10, 1, 2, 8, 5, 11, 12, 4, 15},
        {13, 8, 11, 5, 6, 15, 0, 3, 4, 7, 2, 12, 1, 10, 14, 9},
        {10, 6, 9, 0, 12, 11, 7, 13, 15, 1, 3, 14, 5, 2, 8, 4},
        {3, 15, 0, 6, 10, 1, 13, 8, 9, 4, 5, 11, 12, 7, 2, 14}
    },
    {
        {2, 12, 4, 1, 7, 10, 11, 6, 8, 5, 3, 15, 13, 0, 14, 9},
        {14, 11, 2, 12, 4, 7, 13, 1, 5, 0, 15, 10, 3, 9, 8, 6},
        {4, 2, 1, 11, 10, 13, 7, 8, 15, 9, 12, 5, 6, 3, 0, 14},
        {11, 8, 12, 7, 1, 14, 2, 13, 6, 15, 0, 9, 10, 4, 5, 3}
    },
    {
        {12, 1, 10, 15, 9, 2, 6, 8, 0, 13, 3, 4, 14, 7, 5, 11},
        {10, 15, 4, 2, 7, 12, 9, 5, 6, 1, 13, 14, 0, 11, 3, 8},
        {9, 14, 15, 5, 2, 8, 12, 3, 7, 0, 4, 10, 1, 13, 11, 6},
        {4, 3, 2, 12, 9, 5, 15, 10, 11, 14, 1, 7, 6, 0, 8, 13}
    },
    {
        {4, 11, 2, 14, 15, 0, 8, 13, 3, 12, 9, 7, 5, 10, 6, 1},
        {13, 0, 11, 7, 4, 9, 1, 10, 14, 3, 5, 12, 2, 15, 8, 6},
        {1, 4, 11, 13, 12, 3, 7, 14, 10, 15, 6, 8, 0, 5, 9, 2},
        {6, 11, 13, 8, 1, 4, 10, 7, 9, 5, 0, 15, 14, 2, 3, 12}
    },
    {
        {13, 2, 8, 4, 6, 15, 11, 1, 10, 9, 3, 14, 5, 0, 12, 7},
        {1, 15, 13, 8, 10, 3, 7, 4, 12, 5, 6, 11, 0, 14, 9, 2},
        {7, 11, 4, 1, 9, 12, 14, 2, 0, 6, 10, 13, 15, 3, 5, 8},
        {2, 1, 14, 7, 4, 10, 8, 13, 15, 12, 9, 0, 3, 5, 6, 11}
    }
};

static const int p[] = {
    16, 7, 20, 21,
    29, 12, 28, 17,
    1, 15, 23, 26,
    5, 18, 31, 10,
    2, 8, 24, 14,
    32, 27, 3, 9,
    19, 13, 30, 6,
    22, 11, 4, 25
};


uint64_t applyInitP(uint64_t in) {
    uint64_t res = 0;
    for(int i = 0; i < 64; ++i)
        res |= (uint64_t)(in >> (64 - ip[i]) & 1) << (63 - i);
    return res;
}

uint64_t applyPc1(uint64_t in) {
    uint64_t res = 0;
    for(int i = 0; i < 56; ++i) {
        res |= (uint64_t)(in >> (64 - pc_1[i]) & 1) << (55 - i);
    }
    return res;
}

uint32_t* split(uint64_t in, int size) {
    static uint32_t arr[2];
    uint64_t mask_1 = (size == 56) ? 0xfffffff0000000 : 0xffffffff00000000;
    uint64_t mask_2 = (size == 56) ? 0xfffffff : 0xffffffff;
    arr[0] = (in & mask_1) >> (size / 2);
    arr[1] = (in & mask_2);
    return arr;
}

uint32_t circularShift(uint32_t in, int shift_amount) {
    uint32_t res = ((in << shift_amount) & 0xFFFFFFF) | (in >> (28 - shift_amount));
    return res;
}

uint64_t concatKey(uint32_t c, uint32_t d) {
    uint64_t res = 0x0;
    res |= ((uint64_t)c) << 28;
    res |= (uint64_t)d;
    return res;
}

uint64_t applyPc2(uint64_t in) {
    uint64_t res = 0;
    for(int i = 0; i < 48; ++i)
        res |= (uint64_t)(in >> (56 - pc_2[i]) & 1) << (47 - i);
    return res;
}

uint64_t expandRi(uint32_t in) {
    uint64_t res = 0;
    for(int i = 0; i < 48; i++)
        res |= (uint64_t)(in >> (32 - expansion[i]) & 1) << (47 - i);
    return res;
}

uint64_t sBoxSubstitute(uint32_t in, int i) {
    int row = (in & 0x1);
    row |= (in & 0x20) >> 4;
    int col = (in & 0x1e) >> 1;
    return (uint64_t)sbox[i][row][col];
}

uint32_t applyP(uint32_t in) {
    uint32_t res = 0;
    for(int i = 0; i < 32; i++) {
        res |= (uint32_t)(in >> (32 - p[i]) & 1) << (31 - i);
    }
    return res;
}

uint32_t processRi(uint32_t ri, uint32_t li, uint64_t ki) {
    uint32_t res = 0;
    uint64_t expanded_ri = expandRi(ri);
    uint64_t k_xor_r = expanded_ri ^ ki;
    uint32_t s_boxed = 0;
    for(int i = 0; i < 8; i++) {
        uint32_t bi = k_xor_r >> (7 - i) * 6;
        s_boxed |= sBoxSubstitute(bi, i) << (7 - i) * 4;
    }
    res = applyP(s_boxed) ^ li;
    return res;
}

uint64_t applyInvP(uint64_t in) {
    uint64_t res = 0;
    for(int i = 0; i < 64; i++) {
        res |= (uint64_t)(in >> (64 - ip_inverse[i]) & 1) << (63 - i);
    }
    return res;
}

uint64_t encrypt_decrypt(uint64_t plain, uint64_t key, int encrypt) {
    uint64_t ciphered = 0;
    plain = applyInitP(plain);
    key = applyPc1(key);

    uint32_t* splitted = split(key, 56);
    uint32_t c0 = splitted[0], d0 = splitted[1];

    splitted = split(plain, 64);
    uint32_t li = splitted[0], ri = splitted[1];

    for(int i = 0; i < 16; i++) {
        int index = encrypt ? i : 15 - i;
        uint32_t ci = circularShift(c0, shift_amount[index]);
        uint32_t di = circularShift(d0, shift_amount[index]);
        uint64_t ki = applyPc2(concatKey(ci, di));

        uint32_t tmp_l = li;
        li = ri;
        ri = processRi(ri, tmp_l, ki);
    }

    ciphered |= (uint64_t)(li);
    ciphered |= ((uint64_t)(ri) << 32);
    ciphered = applyInvP(ciphered);

    return ciphered;
}

unsigned char* load_file(const char *fn, int *len) {
    struct stat info = {0};
    int ret = stat(fn, &info);
    if(ret)
        return 0;
    FILE *fsrc = fopen(fn, "rb");
    if(!fsrc)
        return 0;
    unsigned char* data = (unsigned char*)malloc(info.st_size);
    if(!data) {
        exit(1);
        return 0;
    }
    size_t nread = fread(data, 1, info.st_size, fsrc);
    fclose(fsrc);
    *len = (int)nread;
    return data;
}

int save_file(const char *fn, unsigned char *data, int len) {
    FILE *fdst = fopen(fn, "wb");
    if(!fdst)
        return 0;
    fwrite(data, 1, len, fdst);
    fclose(fdst);
    return 1;
}

uint64_t bytes_to_uint64(unsigned char* bytes) {
    uint64_t result = 0;
    for(int i = 0; i < 8; i++) {
        result = (result << 8) | bytes[i];
    }
    return result;
}

void uint64_to_bytes(uint64_t val, unsigned char* bytes) {
    for(int i = 7; i >= 0; i--) {
        bytes[i] = val & 0xFF;
        val >>= 8;
    }
}

int main(int argc, char **argv) {
    if(argc != 5) {
        printf("Usage: %s <e/d> <key_file> <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    int encrypt = (strcmp(argv[1], "e") == 0) ? 1 : 0;

    int key_len;
    unsigned char* key_data = load_file(argv[2], &key_len);
    if(!key_data || key_len < 8) {
        printf("Error: Invalid key file\n");
        return 1;
    }
    uint64_t key = bytes_to_uint64(key_data);
    free(key_data);

    int input_len;
    unsigned char* input_data = load_file(argv[3], &input_len);
    if(!input_data) {
        printf("Error: Cannot read input file\n");
        return 1;
    }

    int output_len = ((input_len + 7) / 8) * 8;
    unsigned char* output_data = (unsigned char*)malloc(output_len);
    if(!output_data) {
        free(input_data);
        printf("Error: Memory allocation failed\n");
        return 1;
    }

    for(int i = 0; i < input_len; i += 8) {
        unsigned char block[8] = {0};
        int block_size = (input_len - i < 8) ? input_len - i : 8;
        memcpy(block, input_data + i, block_size);

        uint64_t block_val = bytes_to_uint64(block);
        uint64_t result = encrypt_decrypt(block_val, key, encrypt);
        uint64_to_bytes(result, output_data + i);
    }

    if(!save_file(argv[4], output_data, output_len)) {
        printf("Error: Cannot write output file\n");
        free(input_data);
        free(output_data);
        return 1;
    }

    free(input_data);
    free(output_data);

    return 0;
}
