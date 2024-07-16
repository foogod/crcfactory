/* Copyright (c) 2016 Alex Stewart
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __CRCFACTORY_H__
#define __CRCFACTORY_H__

#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define CRCFACTORY_INLINE __attribute__ ((always_inline)) static inline

#ifndef CRCFACTORY_CRC_TYPE
  #define CRCFACTORY_CRC_TYPE uint_fast32_t
  #ifndef CRCFACTORY_CRCTABLE_TYPE
    #define CRCFACTORY_CRCTABLE_TYPE uint32_t
  #endif
#endif
#ifndef CRCFACTORY_CRCTABLE_TYPE
  #define CRCFACTORY_CRCTABLE_TYPE CRCFACTORY_CRC_TYPE
#endif

// Note: The use of "_8", "_16", "_32" arguments in the following functions
// instead of literal values is a hack to workaround the fact that gcc
// complains if we try to shift more than the number of bits in
// CRCFACTORY_CRC_TYPE (and there's no way to disable that warning with a
// pragma, etc).  Since the user may specify any size for that datatype, some
// of the following routines may not be used but will still be compiled and
// generate spurious warnings.
//
// We work around this by passing the numbers as arguments/variables instead of
// using inline constants, so they will ultimately be optimized down to the
// same thing when inlining, but the compiler can't be sure enough at
// compilation time to generate a warning, and so keeps quiet.

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE _crcfactory_reflect8(int width, CRCFACTORY_CRC_TYPE value) {
    value = ((value & 0x55) << 1) | ((value & 0xaa) >> 1);
    value = ((value & 0x33) << 2) | ((value & 0xcc) >> 2);
    value = (value << 4)          | (value >> 4);
    return (value & 0xff) >> (8 - width);
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE _crcfactory_reflect16(int width, CRCFACTORY_CRC_TYPE value, int _8) {
    value = ((value & 0x5555) << 1) | ((value & 0xaaaa) >> 1);
    value = ((value & 0x3333) << 2) | ((value & 0xcccc) >> 2);
    value = ((value & 0x0f0f) << 4) | ((value & 0xf0f0) >> 4);
    value = (value << _8)            | (value >> _8);
    return (value & 0xffff) >> (16 - width);
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE _crcfactory_reflect32(int width, CRCFACTORY_CRC_TYPE value, int _8, int _16) {
    value = ((value & 0x55555555) << 1) | ((value & 0xaaaaaaaa) >> 1);
    value = ((value & 0x33333333) << 2) | ((value & 0xcccccccc) >> 2);
    value = ((value & 0x0f0f0f0f) << 4) | ((value & 0xf0f0f0f0) >> 4);
    value = ((value & 0x00ff00ff) << _8) | ((value & 0xff00ff00) >> _8);
    value = (value << _16)               | (value >> _16);
    return (value & 0xffffffff) >> (32 - width);
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE _crcfactory_reflect64(int width, CRCFACTORY_CRC_TYPE value, int _8, int _16, int _32) {
    value = ((value & 0x5555555555555555L) << 1) | ((value & 0xaaaaaaaaaaaaaaaaL) >> 1);
    value = ((value & 0x3333333333333333L) << 2) | ((value & 0xccccccccccccccccL) >> 2);
    value = ((value & 0x0f0f0f0f0f0f0f0fL) << 4) | ((value & 0xf0f0f0f0f0f0f0f0L) >> 4);
    value = ((value & 0x00ff00ff00ff00ffL) << _8) | ((value & 0xff00ff00ff00ff00L) >> _8);
    value = ((value & 0x0000ffff0000ffffL) << _16) | ((value & 0xffff0000ffff0000L) >> _16);
    value = (value << _32)               | (value >> _32);
    return (value & 0xffffffffffffffffL) >> (64 - width);
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_reflect(int width, CRCFACTORY_CRC_TYPE value) {
    if (width > 32) {
        return _crcfactory_reflect64(width, value, 8, 16, 32);
    } else if (width > 16) {
        return _crcfactory_reflect32(width, value, 8, 16);
    } else if (width > 8) {
        return _crcfactory_reflect16(width, value, 8);
    } else {
        return _crcfactory_reflect8(width, value);
    }
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE _crcfactory_mask(int width, bool reflected, CRCFACTORY_CRC_TYPE value) {
    if (!reflected && width <= 8) {
        return value & 0xff;
    }

    // Avoid undefined behavior if a shift to generate the mask would exceed
    // the size of the type. Just return the value unmasked, since the mask
    // would cover all bits anyway. Strictly speaking we probably want a
    // limits-based check here in case CRC_TYPE is signed, but signed types
    // would likely break other things anyway.
    if ((size_t)width >= (CHAR_BIT * sizeof(CRCFACTORY_CRC_TYPE))) return value;
    return value & ((1ULL << width) - 1);
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_setup_state(int width, bool reflected, CRCFACTORY_CRC_TYPE iv) {
    CRCFACTORY_CRC_TYPE state;

    if (reflected) {
        state = crcfactory_reflect(width, iv);
    } else if (width < 8) {
        state = iv << (8 - width);
    } else {
        state = iv;
    }
    return state;
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_setup_poly(int width, bool reflected, CRCFACTORY_CRC_TYPE poly) {
    if (reflected) {
        poly = crcfactory_reflect(width, poly);
    } else if (width < 8) {
        poly <<= (8 - width);
    }
    return poly;
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_update(int width, bool reflected, CRCFACTORY_CRC_TYPE poly, CRCFACTORY_CRC_TYPE state, uint8_t data) {
    int bit;

    if (reflected) {
        state ^= data;
        for (bit = 8; bit > 0; bit--) {
            if (state & 1) {
                state = (state >> 1) ^ poly;
            } else {
                state = (state >> 1);
            }
        }
    } else if (width < 8) {
        state ^= data;
        for (bit = 8; bit > 0; bit--) {
            if (state & 0x80) {
                state = (state << 1) ^ poly;
            } else {
                state = (state << 1);
            }
        }
    } else {
        state ^= ((CRCFACTORY_CRC_TYPE)data << (width - 8));
        for (bit = 8; bit > 0; bit--) {
            if (state & (1ULL << (width - 1))) {
                state = (state << 1) ^ poly;
            } else {
                state = (state << 1);
            }
        }
    }
    return state;
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_result(int width, bool reflected, CRCFACTORY_CRC_TYPE xorout, CRCFACTORY_CRC_TYPE state) {
    CRCFACTORY_CRC_TYPE crc;

    if (reflected) {
        crc = state;
    } else if (width < 8) {
        crc = (state & 0xff) >> (8 - width);
    } else {
        crc = _crcfactory_mask(width, reflected, state);
    }
    return crc ^ xorout;
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_crc(int width, bool reflected, CRCFACTORY_CRC_TYPE poly, CRCFACTORY_CRC_TYPE iv, CRCFACTORY_CRC_TYPE xorout, const uint8_t *data, size_t data_len) {
    CRCFACTORY_CRC_TYPE state;
    size_t i;

    poly = crcfactory_setup_poly(width, reflected, poly);
    state = crcfactory_setup_state(width, reflected, iv);
    for (i = 0; i < data_len; i++) {
        state = crcfactory_update(width, reflected, poly, state, data[i]);
    }
    return crcfactory_result(width, reflected, xorout, state);
}

CRCFACTORY_INLINE void crcfactory_table_init(int width, bool reflected, CRCFACTORY_CRC_TYPE poly, CRCFACTORY_CRCTABLE_TYPE *table) {
    int i;

    poly = crcfactory_setup_poly(width, reflected, poly);
    for (i = 0; i < 256; i++) {
        table[i] = _crcfactory_mask(width, reflected, crcfactory_update(width, reflected, poly, 0, i));
    }
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_table_update(int width, bool reflected, const CRCFACTORY_CRCTABLE_TYPE *table, CRCFACTORY_CRC_TYPE state, uint8_t data) {
    if (reflected) {
        state = table[(data ^ state) & 0xff] ^ (state >> 8);
    } else if (width <= 8) {
        state = table[data ^ state];
    } else {
        state = table[(data ^ (state >> (width - 8))) & 0xff] ^ (state << 8);
    }
    return state;
}

CRCFACTORY_INLINE CRCFACTORY_CRC_TYPE crcfactory_table_crc(int width, bool reflected, const CRCFACTORY_CRCTABLE_TYPE *table, CRCFACTORY_CRC_TYPE iv, CRCFACTORY_CRC_TYPE xorout, const uint8_t *data, size_t data_len) {
    CRCFACTORY_CRC_TYPE state;
    size_t i;

    state = crcfactory_setup_state(width, reflected, iv);
    for (i = 0; i < data_len; i++) {
        state = crcfactory_table_update(width, reflected, table, state, data[i]);
    }
    return crcfactory_result(width, reflected, xorout, state);
}

#define CRCFACTORY_CRCFUNC(_funcname, _width, _reflected, _poly, _iv, _xor) \
    CRCFACTORY_CRC_TYPE _funcname(uint8_t *buffer, size_t buf_size) { \
        return crcfactory_crc(_width, _reflected, _poly, _iv, _xor, buffer, buf_size); \
    }

#define CRCFACTORY_CTABLE_CRCFUNC(_funcname, _width, _reflected, _poly, _iv, _xor) \
    CRCFACTORY_CRC_TYPE _funcname(const CRCFACTORY_CRCTABLE_TYPE *table, uint8_t *buffer, size_t buf_size) { \
        return crcfactory_table_crc(_width, _reflected, table, _iv, _xor, buffer, buf_size); \
    }

#define CRCFACTORY_TABLE_INITFUNC(_funcname, _width, _reflected, _poly, _iv, _xor) \
    void _funcname(CRCFACTORY_CRCTABLE_TYPE *table) { \
        crcfactory_table_init(_width, _reflected, _poly, table); \
    }

#define CRCFACTORY_TABLE_CRCFUNC(_funcname, _width, _reflected, _poly, _iv, _xor) \
    CRCFACTORY_TABLE_INITFUNC(_funcname##_init, _width, _reflected, _poly, _iv, _xor); \
    CRCFACTORY_CTABLE_CRCFUNC(_funcname, _width, _reflected, _poly, _iv, _xor);

#endif /* __CRCFACTORY_H__ */
