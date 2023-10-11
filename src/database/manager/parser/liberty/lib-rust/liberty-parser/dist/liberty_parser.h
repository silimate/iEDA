/*
Gutengerg Post Parser, the C bindings.

Warning, this file is autogenerated by `cbindgen`.
Do not modify this manually.

*/

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * The complex attribute statement.
 * # Example
 * index_1 ("0.0010,0.0020,0.0030");
 */
typedef struct LibertyComplexAttrStmt LibertyComplexAttrStmt;

/**
 * The group statement.
 * # Example
 *
 * wire_load("5K_hvratio_1_1") {
 * capacitance : 1.774000e-01;
 * resistance : 3.571429e-03;
 * slope : 5.000000;
 * fanout_length( 1, 1.7460 );
 * fanout_length( 2, 3.9394 );
 * fanout_length( 3, 6.4626 );
 * fanout_length( 4, 9.2201 );
 * fanout_length( 5, 11.9123 );
 * fanout_length( 6, 14.8358 );
 * fanout_length( 7, 18.6155 );
 * fanout_length( 8, 22.6727 );
 * fanout_length( 9, 25.4842 );
 * fanout_length( 11, 27.0320 );
 * }
 */
typedef struct LibertyGroupStmt LibertyGroupStmt;

/**
 * The simple attribute statement.
 * # Example
 * capacitance : 1.774000e-01;
 */
typedef struct LibertySimpleAttrStmt LibertySimpleAttrStmt;

typedef struct RustVec {
    void *data;
    uintptr_t len;
    uintptr_t cap;
} RustVec;

typedef struct RustLibertyStringValue {
    char *value;
} RustLibertyStringValue;

typedef struct RustLibertyFloatValue {
    double value;
} RustLibertyFloatValue;

typedef struct RustLibertyGroupStmt {
    char *file_name;
    uint32_t line_no;
    char *group_name;
    struct RustVec attri_values;
    struct RustVec stmts;
} RustLibertyGroupStmt;

typedef struct RustLibertySimpleAttrStmt {
    char *file_name;
    uint32_t line_no;
    char *attri_name;
    const void *attri_value;
} RustLibertySimpleAttrStmt;

typedef struct RustLibertyComplexAttrStmt {
    char *file_name;
    uint32_t line_no;
    char *attri_name;
    struct RustVec attri_values;
} RustLibertyComplexAttrStmt;

void *rust_parse_lib(const char *s);

uintptr_t rust_vec_len(const struct RustVec *vec);

void free_c_char(char *s);

struct RustLibertyStringValue *rust_convert_string_value(void *string_value);

struct RustLibertyFloatValue *rust_convert_float_value(void *float_value);

bool rust_is_float_value(void *c_attribute_value);

bool rust_is_string_value(void *c_attribute_value);

struct RustLibertyGroupStmt *rust_convert_group_stmt(struct LibertyGroupStmt *group_stmt);

struct RustLibertySimpleAttrStmt *rust_convert_simple_attribute_stmt(struct LibertySimpleAttrStmt *simple_attri_stmt);

struct RustLibertyComplexAttrStmt *rust_convert_complex_attribute_stmt(struct LibertyComplexAttrStmt *complex_attri_stmt);
