/**************************************************************************//**
 * @file avmc_ops.h
 *
 * @brief List of supported ops
 *
 * @details
 * <em>Copyright (C) 2017, Andrew Kephart.  All rights reserved.</em>
 * */
#ifndef _AVMC_OPS_H_
#define _AVMC_OPS_H_

#include "avmc.h"
#include "avmm_data.h"
#include "avmlib_table.h"


/* Basic canonical definition of an instruction */
typedef struct opdef_s {
    char *i_token; /* Canonical name */
    uint32_t i_opcode; /* The 32-bit opcode */
    uint32_t i_argc; /* Minimum # of args */
    struct op_s *(*i_create)(struct opdef_s*); /* Create new */
    char *(*i_validate)(struct opdef_s*, struct op_s *op); /* Validate */
} opdef_t;

/* A full instruction instance during parsing/compilation. */
typedef struct op_s {
    opdef_t *i_ref; /* Reference to basics */
    char *i_source_file; /* Source file */
    int i_source_line; /* Line # in source file */
    uint32_t i_paramc; /* Number of parameters */
    param_t *i_params[64]; /* Most possible params */
} op_t;


/**
 * Prototypes for op processing
 */
void avmc_ops_init(void);
op_t *avmc_op_new(opdef_t *def); /* Default */
char *avmc_op_validate(opdef_t *def, op_t *op); /* Default */
opdef_t *avmc_op_lookup(char *token);
    /* Table support */
int avmc_opdef_cmp(table_t self, opdef_t *def, intptr_t test);


#endif /* _AVMC_OPS_H_ */