/**************************************************************************//**
 * @file avmlib_data.c
 *
 * @brief Implementation for entity/object utilities
 *
 * @details
 * <em>Copyright (C) 2012, Andrew Kephart. All rights reserved.</em>
 * */
#ifndef _AVMLIB_DATA_C_
#define _AVMLIB_DATA_C_

#include "avmlib.h"
#include <stdarg.h>
#include <stdbool.h>


/**************************************************************************//**
 * @brief Determine if an entity is one of a set of classes.
 *
 * @details This is a simple comparison between all of the provided classes
 * and the given entity.
 *
 * @param e The entity to test
 * @param num The number of classes to check
 * @param ... List of classes.
 *
 * @returns false if the entity is not a member of any of the given classes,
 * true if it is.
 *
 * @remarks
 * */
int
avmlib_entity_assert_class(
    entity_t e,
    int num,
    ...
)
{
    va_list vclasses;
    int i;
    int c = avmlib_entity_class(e);
    int retval = false;

    va_start(vclasses,num);
    for (i=0; i<num;i++) {
        if (c == va_arg(vclasses,int)) {
            retval = true;
            break;
        }
    }
    va_end(vclasses);

    return retval;
}

/**************************************************************************//**
 * @brief Create a new instruction entity from components.
 *
 * @param opcode The operation to encode
 * @param flags The flags modifying operation
 * @param argc Number of entities to follow.
 *
 * @returns Newly-created entity on success error entity on failure.
 *
 * @remarks
 * */
entity_t 
avmlib_instruction_new(
    avm_opcode_t opcode,
    uint8_t flags,
    uint8_t argc
)
{
    return (AVM_CLASS_INSTRUCTION << 24 ) |
           ((opcode) << 16 ) |
           ((flags) << 8) |
           (argc);
}


/**************************************************************************//**
 * @brief Create a new immediate number entity
 *
 * @param val The value to encode
 *
 * @returns New entity on success, ENTITY_INVALID on failure
 *
 * @remarks
 * */
entity_t 
avmlib_immediate_new(
    int64_t val
)
{
    int64_t v = val;
    uint32_t code = (AVM_CLASS_IMMEDIATE << 24);

    if (v < 0) {
        v = -v;
        code |= (1<<20);
    }
    if (v > 0xFFFFF)  return ENTITY_INVALID;

    code |= (uint32_t)(v & 0xFFFFF);

    return (entity_t)code; 
}

/**************************************************************************//**
 * @brief Common non-instruction entity creator
 *
 * @details Most entities are composed of a class and a table index.
 *
 * @param class Class of entity to create
 * @param index Index into relevant table.
 *
 * @returns Result code indicating success or failure mode
 *
 * @remarks Entities with special values may be created with this and
 * then modified.
 * */
entity_t 
avmlib_entity_new(
    avm_class_e class,
    int table_index
)
{
    return ((uint32_t)class << 24) | (((uint32_t)table_index) & 0xFFFF) ;

}

/**************************************************************************//**
 * @brief Create an unresolved reference
 *
 * @details
 *
 * @param
 *
 * @returns New register object on success, NULL on failure.
 *
 * @remarks
 * */
class_unresolved_t *
avmlib_unresolved_new(
    char *name
)
{
    /* Base declaration/alloc */
    class_unresolved_t *obj = calloc(1,sizeof(*obj));
    if (NULL == obj) return NULL;

    strncpy(obj->header.symname, name, sizeof(obj->header.symname));
    return obj;
}

/**************************************************************************//**
 * @brief Create a register object from parameters
 *
 * @details
 *
 * @param
 *
 * @returns New register object on success, NULL on failure.
 *
 * @remarks
 * */
class_register_t *
avmlib_register_new(
    char *name,
    register_mode_t mode,
    intptr_t priv,
    void (*reset)(class_register_t *),
    uint32_t (*get)(class_register_t *),
    uint32_t (*set)(class_register_t *,uint32_t value)
)
{
    /* Base declaration/alloc */
    class_register_t *obj = calloc(1,sizeof(*obj));
    if (NULL == obj) return NULL;

    strncpy(obj->header.symname, name, sizeof(obj->header.symname));
    obj->mode = mode;
    obj->private_data = priv;
    obj->reset = reset;
    obj->get = get;
    obj->set = set;
    return obj;
}

/**************************************************************************//**
 * @brief Create a numeric object from parameters
 *
 * @details
 *
 * @param name The name of this object.  If NULL, this is an anonymous constant
 * @param value The value for this object.
 *
 * @returns New numeric object on success, NULL on failure.
 *
 * @remarks
 * */
class_number_t *
avmlib_number_new(
    char *name,
    int width,
    int64_t value
)
{
    /* Base declaration/alloc */
    class_number_t *obj = calloc(1,sizeof(*obj));

    /* Save name */
    if (NULL != name) {
        strncpy(obj->header.symname, name, sizeof(obj->header.symname));
    }
    obj->bitwidth = width;
    obj->value = value;
    return obj;
}


/**************************************************************************//**
 * @brief Dump the contents of a program segment
 *
 * */
void
avmlib_dump_seg(
    avm_t *avm,
    class_segment_t *seg
)
{
    int i,j;
    table_t *ti;

    printf("MACHINE: %s\n",avmm_entity_name(seg));
    for (i=0; i < avmlib_table_size(&avm->tables); i++) {
        table_t *t = AVM_CLASS_TABLE(avm,i);
        switch(i) {
            case AVM_CLASS_REGISTER: {
                for (j=0; j < avmlib_table_size(t); j++) {
                    class_register_t *reg = (class_register_t *)t->entries[j];
                    printf("\tREGISTER: %s (%c%c)\n",avmm_entity_name(reg),
                           (reg->mode & REGMODE_READ)?'R':'-',
                           (reg->mode & REGMODE_WRITE)?'W':'-');
                }
                break;
            }
        }
    }
    printf("SEGMENT: %s\n",avmm_entity_name(seg));
    for (i=0; i < avmlib_table_size(&seg->tables); i++) {
        table_t *t = AVM_CLASS_TABLE(seg,i);
        switch(i) {
            case AVM_CLASS_STRING: {
                for (j=0;j<avmlib_table_size(t);j++) {
                    class_string_t *obj = (class_string_t *)t->entries[j];
                    printf("\tSTRING: %s = \"%s\"\n",avmm_entity_name(obj),
                           obj->text?obj->text:NULL);
                }
                break;
            }
        }
    }
    printf("CODE: \n");
    ti = AVM_CLASS_TABLE(seg,AVM_CLASS_INSTRUCTION);
    for (i=0;i<avmlib_table_size(ti);i++) {
        if (i && !(i%4)) { 
            printf("   ");
            if (!(i%8)) {
                printf("\n");
            }
        }
        printf("%08X ",(uint32_t)ti->entries[i]);
    }

    printf("\n");
}


#endif /* _AVMLIB_DATA_C_ */
