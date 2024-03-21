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
 * Store each line of Port section
 * Port entry example: *37 I *C 633.84 0.242
 * name: "37"
 * direction: ConnectionType::INPUT
 * coordinate: (633.84, 0.242)
 */
typedef enum ConnectionDirection {
    INPUT,
    OUTPUT,
    INOUT,
    Internal,
    UNITIALIZED,
} ConnectionDirection;

/**
 * Store each line of Conn section
 * Conn entry example: *I *33272:Q O *C 635.66 405.835 *L 0 *D sky130_fd_sc_hd__dfxtp_1
 * name: "37"
 * direction: ConnectionType::INPUT
 * coordinate: (633.84, 0.242)
 * driving_cell: "sky130_fd_sc_hd__dfxtp_1"
 */
typedef enum ConnectionType {
    INTERNAL,
    EXTERNAL,
    UNITIALIZED,
} ConnectionType;

typedef struct SpefConnEntry SpefConnEntry;

/**
 * Spef Exchange data structure with cpp
 */
typedef struct SpefExchange SpefExchange;

/**
 * Store everthing about a net
 * Conn entry example: 3 *1:2 0.000520945
 * name: "1:2"
 * direction: ConnectionType::INPUT
 * coordinate: (633.84, 0.242)
 * driving_cell: "sky130_fd_sc_hd__dfxtp_1"
 */
typedef struct SpefNet SpefNet;

typedef struct SpefResCap SpefResCap;

typedef struct RustVec {
    void *data;
    uintptr_t len;
    uintptr_t cap;
    uintptr_t type_size;
} RustVec;

typedef struct RustSpefFile {
    char *file_name;
    struct RustVec header;
    struct RustVec ports;
    struct RustVec nets;
} RustSpefFile;

typedef struct RustNetItem {
    char *name;
    double lcap;
    struct RustVec conns;
    struct RustVec caps;
    struct RustVec ress;
} RustNetItem;

typedef struct RustPair_f64 {
    double first;
    double second;
} RustPair_f64;

typedef struct RustConnItem {
    enum ConnectionType conn_type;
    enum ConnectionDirection conn_direction;
    char *pin_port_name;
    char *driving_cell;
    double load;
    uint32_t layer;
    struct RustPair_f64 coordinate;
    struct RustPair_f64 ll_coordinate;
    struct RustPair_f64 ur_coordinate;
} RustConnItem;

typedef struct RustResCapItem {
    char *node1;
    char *node2;
    double res_cap;
} RustResCapItem;

void spef_free_c_char(char *s);

void *rust_parser_spef(const char *spef_path);

void rust_free_spef_data(struct SpefExchange *c_spef_data);

void *rust_covert_spef_file(struct SpefExchange *c_spef_data);

void rust_free_spef_file(struct RustSpefFile *c_spef_data);

void *rust_convert_spef_net(struct SpefNet *c_spef_net);

void rust_free_spef_net(struct RustNetItem *c_spef_net);

void *rust_convert_spef_conn(struct SpefConnEntry *c_spef_net);

void rust_free_spef_conn(struct RustConnItem *c_spef_net);

void *rust_convert_spef_net_cap_res(struct SpefResCap *c_spef_net_cap_res);

void rust_free_spef_net_cap_res(struct RustResCapItem *c_spef_net_cap_res);

char *rust_expand_name(struct SpefExchange *c_spef_data, uintptr_t index);

void rust_expand_all_name(struct SpefExchange *c_spef_data);

char *rust_get_spef_cap_unit(struct SpefExchange *c_spef_data);

char *rust_get_spef_res_unit(struct SpefExchange *c_spef_data);
