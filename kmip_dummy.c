/* Copyright (c) 2018 The Johns Hopkins University/Applied Physics Laboratory
 * All Rights Reserved.
 *
 * This file is dual licensed under the terms of the Apache 2.0 License and
 * the BSD 3-Clause License. See the LICENSE file in the root of this
 * repository for more information.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kmip.h"
#include "kmip_bio.h"
#include "kmip_memset.h"


void kmip_debug(uint32_t flags, FILE *dest)
{
    (void)flags;
    (void)dest;
}


size_t
kmip_strnlen_s(const char *str, size_t strsz)
{

    (void)str;
    (void)strsz;

    return 0;

}


void
kmip_init(KMIP *ctx, void *buffer, size_t buffer_size, enum kmip_version v)
{

    (void)ctx;
    (void)buffer;
    (void)buffer_size;
    (void)v;

    return;

}


int
kmip_add_credential(KMIP *ctx, Credential *cred)
{

    (void)ctx;
    (void)cred;

    return 0;

}


void
kmip_stack_trace(KMIP *ctx, void (*handler)(const char *, void *), void *user)
{
    (void)ctx;
    (void)handler;
    (void)user;
}


const char *
kmip_error_string(int value)
{
    (void)value;
    return NULL;
}


void
kmip_dump_buffer(void *buffer, int size,
                 void (*handler)(const char *, void *), void *user)
{
    (void)buffer;
    (void)size;
    (void)handler;
    (void)user;
}


void
kmip_destroy(KMIP *ctx)
{
    (void)ctx;
}


void *
kmip_memset(void *ptr, int value, size_t size)
{

    (void)ptr;
    (void)value;
    (void)size;

    return NULL;

}


void
kmip_free(void *state, void *ptr)
{
    (void)state;
    (void)ptr;
}


void
kmip_print_result_status_enum(enum result_status value)
{

    (void)value;

    return;

}


const char *
kmip_result_status_enum(enum result_status value)
{

    (void)value;

    return NULL;

}


int kmip_bio_get_symmetric_key_with_context(KMIP *ctx, BIO *bio,
                                            char *uuid, int uuid_size,
                                            char **key, int *key_size)
{

    (void)ctx;
    (void)bio;
    (void)uuid;
    (void)uuid_size;
    (void)key;
    (void)key_size;

    return KMIP_ARG_INVALID;

}
