/**************************************************************************//**
 * @file avmm_data.h
 *
 * @brief Define the different entities that the machine can deal with.
 *
 * @details
 * <em>Copyright (C) 2017, Andrew Kephart.  All rights reserved.</em>
 * */

#ifndef _AVMM_DATA_H_
#define _AVMM_DATA_H_

#include "avmlib_table.h"
#include <stdio.h>

/**
 * Core entity type
 */
typedef uint32_t entity_t;

/**
 * Enumerate the supported entity types.  Each type's value defined
 * here may be an index into a table of tables.  All entity references
 * of each type will have the type as the high 8-bits of the
 * 32-bit entity value.
 */
typedef enum {
    AVM_CLASS_RESERVED = 0xFF, /* System-reserved values */
    AVM_CLASS_INSTRUCTION = 0x00, /* Basic OP */
    AVM_CLASS_ERROR = 0x01, /* Errors/exceptions */
    AVM_CLASS_GROUP = 0x02, /* Grouping of other elements */
    AVM_CLASS_REGISTER = 0x03, /* A register of (nearly) arbitrary width */
    AVM_CLASS_BUFFER = 0x04, /* A memory buffer. Seekable. */
    AVM_CLASS_PORT = 0x05, /* An I/O port. (File, socket, etc.) */
    AVM_CLASS_STRING = 0x06, /* A character string reference */
    AVM_CLASS_LABEL = 0x07, /* A named code location (for jumps, gotos, etc.) */
    AVM_CLASS_PROCESS = 0x08, /* A thread ID */
    AVM_CLASS_NUMBER = 0x09, /* A numeric reference (basically an 'int' variable) */
    AVM_CLASS_IMMEDIATE = 0x0A, /* Lower 16 bits are an immediate value. */
    AVM_CLASS_SEGMENT = 0x0B, /* A program segment */
    AVM_CLASS_UNRESOLVED = 0x0C, /* Unresolved marker */
    AVM_CLASS_MAX

} avm_class_e; 


/**
 * From the AVM_CLASS_RESERVED, some entities have 
 * predefined meanings.
 */
#define ENTITY_INVALID ((uint32_t)(0xFFFFFFFF))


/**
 * Enumerate the supported opcodes
 */
typedef enum {
    /* Runtime OPs */
    AVM_OP_NOP = 0x00,
    AVM_OP_STOR = 0x01,
    AVM_OP_INS = 0x02,
    AVM_OP_GOTO = 0x03,
    AVM_OP_JZ = 0x04,
    AVM_OP_JE = 0x05,
    AVM_OP_JNZ = 0x06,
    AVM_OP_FORK = 0x07,
    AVM_OP_KILL = 0x08,
    AVM_OP_PUSH = 0x09,
    AVM_OP_POP = 0x0A,
    AVM_OP_LABEL = 0x0B,

    AVM_OP_ADD = 0x0C,
    AVM_OP_SUB = 0x0D,
    AVM_OP_MUL = 0x0E,
    AVM_OP_DIV = 0x0F,
    AVM_OP_POW = 0x10,
    AVM_OP_OR = 0x11,
    AVM_OP_AND = 0x12,
    AVM_OP_CMP = 0x13,
    AVM_OP_INC = 0x14,
    AVM_OP_DEC = 0x15,

    AVM_OP_FILE = 0x16,
    AVM_OP_IN = 0x17,
    AVM_OP_OUT = 0x18,

    /* Compiler or linker instructions */
    AVM_OP_DEF = 0xA0,
    AVM_OP_SIZE = 0xA1,
    AVM_OP_INVALID = 0xFF

} avm_opcode_t;

/**
 * Common struct for all entity stores
 */
typedef struct {
    char symname[64]; /* Name of this entity */
} class_header_t;

    
/**
 * Storage for error entity
 */
typedef struct {
    class_header_t header; /* Generic common header */
    entity_t segment; /* Entity of segment containing handler */
    entity_t label;   /* Label to jump to */
} class_error_t;

/**
 * Storage for a group entity
 */
typedef struct {
    class_header_t header; /* Generic common header */
    table_t members; /* Table of member entities */
} class_group_t;


typedef enum register_mode_e {
    REGMODE_INVALID = 0, 
    REGMODE_READ    = 0x01,
    REGMODE_WRITE   = 0x02,
    REGMODE_RW      = 0x03,

} register_mode_t;
/**
 * Storage for a register entity
 *
 * Registers represent u32 values, but handle them 
 * differently from number entities.
 */
typedef struct _class_register_s {
    class_header_t header; /* Generic common header */
    /* Register mode */
    register_mode_t mode;
    /* Register-specific private data */
    intptr_t private_data;
    /* Register initializer */
    void (*reset)(struct _class_register_s *reg);
    /* If a register can be read, assign a getter */
    uint32_t (*get)(struct _class_register_s *reg);
    /* If a register can be written, assign a setter */
    uint32_t (*set)(struct _class_register_s *reg, uint32_t value);
} class_register_t;

/**
 * Storage for a buffer entity
 *
 */
typedef struct {
    class_header_t header; /* Generic common header */
    void *buf; /* The actual buffer */
    uint32_t capacity; /* Bytes available in buf */
    uint32_t size; /* How many bytes actually stored */
    uint32_t cursor; /* Position within buffer of cursor */
} class_buffer_t;

/**
 * Storage for a port entity 
 *
 * CLEAN: TODO: Flesh this out more.
 */
typedef struct _class_port_s {
    class_header_t header; /* Generic common header */
    /* Base values for common case */
    char *path; /* Reference path, if meaningful */
    int fd; /* File descriptor */
    FILE *file; /* File pointer */
    /* Generic reset/flush */
    int (*reset)(struct _class_port_s *port);
    /* If a port can be read, assign a getter */
    int (*read)(struct _class_port_s *port, void *buffer, int size);
    /* If a port can be written, assign a setter */
    int (*write)(struct _class_port_s *port, void *buffer, int size);
} class_port_t;

/**
 * Storage for a string entity
 */
typedef struct {
    class_header_t header; /* Generic common header */
    char *text;
    uint32_t capacity;
} class_string_t;

/**
 * Storage for an undefined entity
 */
typedef struct {
    class_header_t header; /* Generic common header */
} class_undefined_t;

/**
 * Storage for a label entity
 */
typedef struct {
    class_header_t header; /* Generic common header */
    uint8_t segment; /* Which segment this label references */
    uint32_t offset; /* Instruction offset into reference segment's code */
} class_label_t;

/**
 * Storage for a process/thread/core entity
 */
typedef struct {
    class_header_t header; /* Generic common header */
    table_t registers; /* Table of per-core registers */
    table_t stack;     /* Entity stack */
} class_process_t;

/**
 * Storage for a numeric entity 
 */
typedef struct {
    class_header_t header; /* Generic common header */
    uint32_t bitwidth;
    int64_t value;
} class_number_t;

/**
 * Supported entity flags
 *
 * These should all be 32-bit values suitable for ORing into
 * an exiting entity.
 */
#define OP_FLAG_CONSTANT ((uint32_t)0x00C00000)

/**
 * Storage for an unresolved reference.
 */
typedef struct {
    class_header_t header;
} class_unresolved_t;

/**
 * Storage for a virtual machine
 */
typedef struct {
    class_header_t header; /* Generic common header */
    table_t tables; /* Table of tables */
    entity_t entrypoint; /* Segment entrypoint */
} avm_t;

/**
 * Storage for a program segment
 */
typedef struct {
    class_header_t header; /* Generic common header */
    table_t tables; /* Table of tables */
    uint8_t id; /* Segment number */
    avm_t *avm; /* Machine we're building for */
} class_segment_t;

/**
 * @brief Segment ID of the machine itself (globals)
 */
#define AVMM_SEGMENT_GLOBAL ((uint8_t)(0))
/**
 * @brief Segment ID of the local segment.
 */
#define AVMM_SEGMENT_UNLINKED ((uint8_t)(0xFF))

#define avmm_entity_name(__entity) \
    ((class_header_t *)__entity)->symname

#define AVMM_DEFAULT_ENTRYPOINT ((entry_t)(0))

#define AVM_CLASS_TABLE(__this, __class)  \
    ((table_t *)(((avm_t *)__this)->tables.entries[(int)__class]))


#endif /* _AVMM_DATA_H_ */
