/**************************************************************************//**
 * @file avmc_main.c 
 *
 * @brief Main body of AVM compiler.
 *
 * @details
 * <em>Copyright (C) 2017, Andrew Kephart. All rights reserved.</em>
 * */
#ifndef _AVMC_MAIN_C_
#define _AVMC_MAIN_C_

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "avmc.h"
#include "avmc_ops.h"
#include "avmlib_table.h"
char *avmc_input_file = NULL;

/* Globals */
static op_t *cur_op = NULL;

/* Define our internal tables */
table_t *param_table = NULL;
extern table_t *avmc_opdef_table;

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
    if (NULL == (param_table = avmlib_table_new(8))) {
        fprintf(stderr, "Failed to init parameter table.\n");
        return 3;
    }
    avmc_ops_init();

    /* For now, just parse all command line args as input files */
    for (i=1;(i<=(argc-1));i++) {
        avmc_input_file = strdup(argv[i]);
        printf("PARSING: %s\n",avmc_input_file);
        yylineno = 1; /* Reset line number */
        yyin = fopen(argv[i],"r");
        yyparse();
        if (yyin) fclose(yyin);
        free(avmc_input_file);
    }
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
        fprintf(stdout,"OP: %s (%s)\n",instruction, i_def->i_token);
    } else {
        fprintf(stdout,"OP: %s\n",i_def->i_token);
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
    printf("   param: %s\n",p_text);
    return NULL;
}
#endif /* _AVMC_MAIN_C */
