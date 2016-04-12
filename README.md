The CRC Factory
===============

Easily create optimized and correct CRC generation routines for any type of CRC, with the inclusion of a single header file!

The CRC Factory takes advantage of inline functions and compiler optimizations to allow easy construction of highly optimized calculation functions for nearly any CRC routine.  It supports customization of datatypes used and algorithm choices (fast vs small, etc), and is implemented as a single header file, with no additional library dependencies.

Basic Usage
-----------

To quickly create a CRC generation function for a particular width, polynomial, etc, simply include the `crcfactory.h` header file and then use the `CRCFACTORY_CRCFUNC` macro with the appropriate parameters:

```c
#include "crcfactory.h"

CRCFACTORY_CRCFUNC(my_crc, 32, true, 0x04c11db7, 0xffffffff, 0xffffffff);

/* ... */

    crc = my_crc(buffer, buf_len);

/* ... */
```

(The above will create a function named `my_crc` which calculates a standard CRC-32 result)

The parameters to `CRCFACTORY_CRCFUNC` (in order) are:
 * `funcname` -- The name of the function to create
 * `width` -- CRC width (in bits)
 * `reflected` -- Whether CRC is "reflected"/"reversed" (`true` or `false`)
 * `poly` -- CRC Polynomial
 * `iv` -- CRC Initial Value
 * `xorout` -- XOR mask to apply to the final result

The width can be any number of bits from 1 up to the maximum supported by the datatype in use (see below).  (The CRC Factory has been successfully tested with CRCs ranging from 4 to 64 bits)

The "CRC polynomial" value should be represented in standard binary form with the least significant bit corresponding to the "x^0" component and the high-order bit omitted (sometimes referred to as "normal" or "canonical" form).  It should always be in non-reflected form (even for CRCs for which the reflected parameter is true).

As an example, the standard "CRC-32" algorithm would have a polynomial of `0x04c11db7`.  (Polynomials and other relevant parameters for a very wide range of CRC algorithms can be found at http://reveng.sourceforge.net/crc-catalogue/)

### Fast CRC Calculation

Note that `CRCFACTORY_CRCFUNC` will create a function which uses the basic "compact but slower", iterative approach for generating CRCs.  This is useful if code/data space is at a premium, but if instead speed is a primary concern, a faster (but larger), table-based approach can be used via the `CRCFACTORY_TABLE_CRCFUNC` macro, instead:

```c
#include "crcfactory.h"

CRCFACTORY_TABLE_CRCFUNC(my_crc, 32, true, 0x04c11db7, 0xffffffff, 0xffffffff);

static CRCFACTORY_TABLE_TYPE my_crc_table[256];

/* ... */

    my_crc_init(my_crc_table);

/* ... */

    crc = my_crc(my_crc_table, buffer, buf_len);

/* ... */
```

(`CRCFACTORY_TABLE_CRCFUNC` will create two new functions: `my_crc`, like before, calculates the CRC of some data (but now accepts an additional "crc table" argument).  A function named `my_crc_init` is also created which initializes the CRC table with the correct values, and should be called once prior to any call to `my_crc`)

### Fast CRCs with statically-defined tables

If you do not want to have to initialize the table every time your program runs, you can alternately encode it directly into your C code as a const array.  If you wish to do this and want to save a small bit of code space, you can use the `CRCFACTORY_CTABLE_CRCFUNC` macro instead, which does the same thing as `CRCFACTORY_TABLE_CRCFUNC`, but doesn't create the `*_init` function:

```c
#include "crcfactory.h"

CRCFACTORY_CTABLE_CRCFUNC(my_crc, 32, true, 0x04c11db7, 0xffffffff, 0xffffffff);

static const CRCFACTORY_TABLE_TYPE my_crc_table[256] = {
    /* (table values go here) */
};

/* ... */

    crc = my_crc(my_crc_table, buffer, buf_len);

/* ... */
```

NOTE: In some cases, the CRC tables used by CRC Factory functions may not be the same as those used by other code which implement the same CRC (so you should not just cut-and-paste the table from another program).  To generate a correct table matching this implementation, given a set of parameters, you can use the `crctable` program (found in the `utils` subdirectory of the CRC Factory source distribution).

### Choosing Datatypes

By default, `crcfactory.h` uses the `uint_fast32_t` datatype to hold and work with CRCs, and `uint32_t` for CRC table entries.  If you want or need to use different datatypes, you can do so by defining `CRCFACTORY_CRC_TYPE` and/or `CRCFACTORY_CRCTABLE_TYPE` *before* including `crcfactory.h` in your C program, like so:

```c
#define CRCFACTORY_CRC_TYPE uint16_t
#define CRCFACTORY_CRCTABLE_TYPE uint8_t
#include "crcfactory.h"
```

(If you set `CRCFACTORY_CRC_TYPE` but do not set `CRCFACTORY_CRCTABLE_TYPE`, `CRCFACTORY_CRCTABLE_TYPE` will default to being the same as `CRCFACTORY_CRC_TYPE`)

When choosing these datatypes, you must choose unsigned integer types.  It is recommended to choose from the `uint` datatypes provided by `stdint.h` (i.e. `uint32_t`, `uint_fast32_t`, etc).  Also, this should be obvious, but both `CRCFACTORY_CRC_TYPE` and `CRCFACTORY_CRCTABLE_TYPE` should be large enough to hold any CRC values you will be working with (that is, at least as large as the largest `width` value you will be defining CRC routines with).

Advanced Usage
--------------

For some specialized applications, you may want more fine-grained control over the CRC calculation process (for example, updating the CRC one byte at a time, etc).  You can therefore also use many of the CRC Factory inline functions directly yourself:

### Updating the CRC a byte at a time

The following functions can be used to construct or update a CRC a byte at a time:

* `crcfactory_setup_poly(width, reflected, poly)`
* `crcfactory_setup_state(width, reflected, iv)`
* `crcfactory_update(width, reflected, poly, state, data)`
* `crcfactory_result(width, reflected, xorout, state)`

For an example, one can look at the complete CRC calculation routine (`crcfactory_crc`), which actually just calls these routines to do its work, like so:

```c
    CRCFACTORY_CRC_TYPE state;
    size_t i;

    poly = crcfactory_setup_poly(width, reflected, poly);
    state = crcfactory_setup_state(width, reflected, iv);
    for (i = 0; i < data_len; i++) {
        state = crcfactory_update(width, reflected, poly, state, data[i]);
    }
    return crcfactory_result(width, reflected, xorout, state);
```

It is essential that you start by transforming the polynomial to meet the requirements of the CRC being calculated by calling `crcfactory_setup_poly`, and then initialize the state with `crcfactory_setup_state`.

Note also that unlike some other implementations, the intermediate values produced by these routines are not themselves CRCs, but simply a "working state" value.  To turn `state` into an actual CRC you *must* call `crcfactory_result`.  You can, however, call `crcfactory_result` at any time without disturbing the state, if you want to get the value of the CRC at a particular point but still continue on.

**NOTE:** While it is possible to use variables to hold any/all of the parameters passed to these functions, in order for the C compiler to be able to perform many useful optimizations (and thus produce a reasonably fast CRC implementation), you should make sure that the `width`, `reflected`, and `poly` values passed to these functions are always passed as compile-time constants (that is, #defines or literal values).  If this is not done, the resulting code may be substantially slower than it could be.

### Table-based functions

There are, of course, corresponding functions for the above when working with table-based CRC implementations:

* `crcfactory_table_init(width, reflected, poly, table)`
* `crcfactory_setup_state(width, reflected, iv)`
* `crcfactory_table_update(width, reflected, table, state, data)`
* `crcfactory_result(width, reflected, xorout, state)`

When using the table-based functions, it is not necessary to call `crcfactory_setup_poly`, but it is still necessary to call `crcfactory_setup_state` to set the initial state and then use `crcfactory_result` to get the final CRC:

```c
    CRCFACTORY_CRC_TYPE state;
    size_t i;

    state = crcfactory_setup_state(width, reflected, iv);
    for (i = 0; i < data_len; i++) {
        state = crcfactory_table_update(width, reflected, table, state, data[i]);
    }
    return crcfactory_result(width, reflected, xorout, state);
```

### Miscellaneous other routines

The following routine can also be useful in some rare situations:

* `crcfactory_reflect(width, value)`

Known Limitations
-----------------

* `crcfactory.h` cannot currently create routines to calculate CRCs with widths larger than the native datatypes supported by the compiler (so, for example, it cannot calculate CRCs larger than 64 bits on most platforms).
* `crcfactory.h` relies heavily on good compiler optimizations to perform well, and thus should be compiled with the highest optimization level available (i.e. `-O3` under GCC).  However, this may not be desirable for other parts of some programs (particularly when debugging), so it may be useful to place all uses of `crcfactory.h` in a separate C source file which can be compiled with separate compilation options than the rest of the project.
* `crcfactory.h` has thus far only been tested with GCC.  It should theoretically work with any reasonably recent optimizing C compiler, but may not work right (or may be slower than it could be) without some additional tweaking.
* `CRCFACTORY_CRC_TYPE` and `CRCFACTORY_CRCTABLE_TYPE` must be the same for all CRC routines defined in the same C file.  This can be inconvenient if you want to, for example, define both a table-based CRC-32 routine and a table-based CRC-8 routine (as the CRC-8 table would be much larger than actually required due to the larger datatype).  To work around this, you can put definitions which require different datatype settings into separate C files so that each can set `CRCFACTORY_CRCTABLE_TYPE` differently before including `crcfactory.h`.

Acknowledgements
----------------

Many thanks to Greg Cook for creating the excellent "Catalog of parametrised CRC algorithms" (http://reveng.sourceforge.net/crc-catalogue/).  It has been an invaluable tool in making sure that this code produces correct results across a wide variety of use cases.
