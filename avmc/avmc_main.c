/**************************************************************************//**
 * @file avmc_main.c 
 *
 * @brief Main body of AVM compiler.
 *
 * @details The AVM compiler takes a single input file and generates
 * a program block (aka segment) that represents the programming
 * in that file.  It then emits that segment as an AVM object file,
 * which may be linked using the AVM linker into an AVM executable.
 * <em>Copyright (C) 2017, Andrew Kephart. All rights reserved.</em>
 * */
#ifndef _AVMC_MAIN_C_
#define _AVMC_MAIN_C_

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "avmc.h"
#include "avmc_ops.h"
#include "avmlib_table.h"
#include "avmlib_log.h"

/* Convenience logging */
#define avmc_log(__format_and_args...) \
    avm_log("avmc",__format_and_args)

#define avmc_err(__format_and_args...) \
    avm_err("avmc",__format_and_args)

/* Prototypes */
void avmc_seg_init(class_segment_t *seg);

char *avmc_source_file = NULL; /* Input. */
char *avmc_object_file = NULL; /* Output */

/* Globals */
static op_t *cur_op = NULL; 

/* Named parameters */
typedef struct {
    char *name;
    uint32_t entity;
} entity_map_t;

/**
 * The entity_map is a map of _all_ named symbols defined
 * by this program, with associated entity coding.
 */
static table_t entity_map;

/* The segment we're constructing */
static class_segment_t cur_seg;

/**
 * Command line options.
 * The only non-option argument we support is the input file.
 */
static struct option opts[] = {
        /* "name" is the segment (program) name.  Default is the basename 
         * of the input .avma file.
         */
    { "name", 1, NULL, 'n' },
        /* "entrypoint" is the label to which the machine should branch on
         * program load.  If no entrypoint is specified, execution begins
         * at offset zero in the program stream.
         */
    { "entrypoint", 1, NULL, 'e' },
    { NULL },

};
 

/**************************************************************************//**
 * @brief Main.
 *
 * */
int
main(
    int argc,
    char **argv
)
{
    int i;
    parser_init(argc,argv);

    /* Init global tables */
    avmc_ops_init();
    avmc_seg_init(&cur_seg);

    /* For now, just parse all command line args as input files */
    for (i=1;(i<=(argc-1));i++) {
        avmc_source_file = strdup(argv[i]);
        avmc_log("PARSING: %s\n",avmc_source_file);
        yylineno = 1; /* Reset line number */
        yyin = fopen(argv[i],"r");
        yyparse();
        if (yyin) fclose(yyin);
        free(avmc_source_file);
    }

    /* Now, emit the segment as object code */
}

/**************************************************************************//**
 * @brief Start decoding/assembling an instruction line
 *
 * @details This method is called when the parser identifies the begginning
 * of a new instruction or alias.
 *
 * @param instruction The instruction that's beginning
 * @param file The file currently being compiled
 * @param lineno The line number (in <file>) of the instruction.
 *
 * @returns NULL on success, or error description on failure.
 *
 * @remarks Parsing is always global and single-threaded, so we use
 * statics and globals liberally.
 * */
char *
avmc_inst_start(
    char *instruction,
    char *file,
    int lineno
)
{
    opdef_t *i_def;
    /* Step 1:  Lookup instruction */
    if (NULL == (i_def = avmc_op_lookup(instruction))) {
        sprintf(avmc_errstr,"ERROR: Instruction \"%s\" is not a supported opcode or alias.\n",instruction); 
        return avmc_errstr;
    }

    /* Step 2: Create new container */
    if (NULL == (cur_op = avmc_op_new(i_def))) {
        sprintf(avmc_errstr,"ERROR: Failed to generate instruction for \"%s\" op.\n", instruction);
        return avmc_errstr;
    }

    /* Step 3: Fill in loction bits */
    cur_op->i_source_file = file;
    cur_op->i_source_line = lineno;

    /* Step 4: Tell user */
        /* For macros or aliases, the actual token that will be rendered
         * into the machine code may be different from the text in the
         * file....
         */
    if (strcmp(instruction,i_def->i_token)) {
        avmc_log("OP: %s (%s)\n",instruction, i_def->i_token);
    } else {
        avmc_log("OP: %s\n",i_def->i_token);
    }

    return NULL; /* Success! */
}

/**************************************************************************//**
 * @brief Finish an in-process instruction
 *
 * @details This performs final validation and adds to our instruction stream.
 *
 * @param
 *
 * @returns Result code indicating success or failure mode
 *
 * @remarks
 * */
char *
avmc_inst_finish(void)
{
    if (!cur_op) {
        /* This should never ever happen. */
        return "ERROR: No instruction processing in progress.";
    }
    return NULL;
}

char *
avmc_inst_param(
    param_type_t p_type,
    char *p_text
)
{
    avmc_log("   param: %s\n",p_text);
    return NULL;
}

/**************************************************************************//**
 * @brief Lookups for in-process symbol table
 *
 * @details The in-process map table is searched by string name.
 *
 * @param this The map table
 * @param entry An entity map from the table
 * @param test String name to check
 *
 * @returns 0 if it's a match, nonzero otherwise
 *
 * @remarks
 * */
int
avmc_entity_map_compare(
    table_t *this,
    entry_t entry,
    intptr_t test
)
{
    entity_map_t *map = (entity_map_t *)entry;
    char *name = (char *)test;

    if (!strcmp(name,map->name)) return 0;
    return -1;
}

/**************************************************************************//**
 * @brief Initialize an object segment
 *
 * */
void
avmc_seg_init(
    class_segment_t *seg
)
{
    /* Internal entity map */
    avmlib_table_init(&entity_map,64);
    entity_map.compare = avmc_entity_map_compare;

    /* Code Stream */
    avmlib_table_init(&seg->code,256);
    seg->code.alloc_count = 32; /* Expect quite a bit */

    avmlib_table_init(&seg->groups,10);
    avmlib_table_init(&seg->buffers,10);
    avmlib_table_init(&seg->ports,10);
    avmlib_table_init(&seg->strings,10);
}

#endif /* _AVMC_MAIN_C */
