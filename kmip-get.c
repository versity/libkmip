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
#include <unistd.h>
#include <getopt.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "kmip.h"
#include "kmip_bio.h"
#include "kmip_memset.h"


#define DEBUG 0
#define debug_printf(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, \
                                __FILE__, __LINE__, __func__,\
                                __VA_ARGS__); } while (0)


static inline int min(int x, int y)
{
    return x < y ? x : y;
}


void usage(const char *program)
{
    fprintf(stderr, "Usage: %s [flag value | flag] ...\n\n", program);
    fprintf(stderr, "Flags:\n");
    fprintf(stderr, "-a address  : the IP address of the KMIP server\n");
    fprintf(stderr, "-c path     : path to client certificate file\n");
    fprintf(stderr, "-h          : print this help info\n");
    fprintf(stderr, "-i id       : the ID of the symmetric key to retrieve\n");
    fprintf(stderr, "-k path     : path to client key file\n");
    fprintf(stderr, "-p port     : the port number of the KMIP server\n");
    fprintf(stderr, "-r path     : path to CA certificate file\n");
    fprintf(stderr, "-s password : the password for KMIP server authentication\n");
    fprintf(stderr, "-u user     : the user name for KMIP server authentication\n");
    fprintf(stderr, "-v version  : KMIP Version protocol\n");
}


static void *_calloc(void *state, size_t count, size_t size)
{
    debug_printf("state=%p, count=%zu, size=%zu\n", state, count, size);
    return calloc(count, size);
}


static void *_realloc(void *state, void *address, size_t size)
{
    debug_printf("state=%p, address=%p, size=%zu\n", state, address, size);
    return realloc(address, size);
}


static void _free(void *state, void *address)
{
    debug_printf("state=%p, address=%p\n", state, address);
    free(address);
}


static int print_OpenSSL_errors(const char *string, size_t length, void *user)
{

    const char *tag = (user != NULL) ? (const char *)user : "";
    char buffer[256];
    size_t count = min(length, sizeof (buffer) - 1);

    strncpy(buffer, string, count);
    buffer[count] = '\0';

    fprintf(stderr, "%s%s", tag, buffer);

    return 1;

}


static void print_libkmip_stack(const char *string, void *user)
{

    const char *tag = (user != NULL) ? (const char *)user : "";

    fprintf(stderr, "%s%s", tag, string);

}


static void print_libkmip_buffer(const char *string, void *user)
{

    const char *tag = (user != NULL) ? (const char *)user : "";

    fprintf(stdout, "%s%s\n", tag, string);

}


int use_mid_level_api(char *server_address,
                      char *server_port,
                      char *client_certificate,
                      char *client_key,
                      char *ca_certificate,
                      char *username,
                      char *password,
                      char *id,
                      enum kmip_version version)
{

    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    SSL_library_init();
    ctx = SSL_CTX_new(SSLv23_client_method());

    fprintf(stdout, "\n");
    fprintf(stdout, "Loading the client certificate: %s\n", client_certificate);
    if (SSL_CTX_use_certificate_file(ctx, client_certificate, SSL_FILETYPE_PEM) != 1) {
        fprintf(stderr, "Loading the client certificate failed\n");
        ERR_print_errors_cb(print_OpenSSL_errors, NULL);
        SSL_CTX_free(ctx);
        return -1;
    }

    fprintf(stdout, "Loading the client key: %s\n", client_key);
    if (SSL_CTX_use_PrivateKey_file(ctx, client_key, SSL_FILETYPE_PEM) != 1) {
        fprintf(stderr, "Loading the client key failed\n");
        ERR_print_errors_cb(print_OpenSSL_errors, NULL);
        SSL_CTX_free(ctx);
        return -1;
    }

    fprintf(stdout, "Loading the CA certificate: %s\n", ca_certificate);
    if (SSL_CTX_load_verify_locations(ctx, ca_certificate, NULL) != 1) {
        fprintf(stderr, "Loading the CA file failed\n");
        ERR_print_errors_cb(print_OpenSSL_errors, NULL);
        SSL_CTX_free(ctx);
        return -1;
    }

    BIO *bio = NULL;
    bio = BIO_new_ssl_connect(ctx);
    if (bio == NULL) {
        fprintf(stderr, "BIO_new_ssl_connect failed\n");
        SSL_CTX_free(ctx);
        return -1;
    }

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    BIO_set_conn_hostname(bio, server_address);
    BIO_set_conn_port(bio, server_port);

    if (BIO_do_connect(bio) != 1) {
        fprintf(stderr, "BIO_do_connect failed\n");
        ERR_print_errors_cb(print_OpenSSL_errors, NULL);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return -1;
    }

    char *key = NULL;
    int key_size = 0;
    size_t id_size = kmip_strnlen_s(id, 50);

    KMIP kmip_context = {0};

    kmip_context.calloc_func = &_calloc;
    kmip_context.realloc_func = &_realloc;
    kmip_context.free_func = &_free;

    kmip_init(&kmip_context, NULL, 0, version);

    TextString u = {0};
    u.value = username;
    u.size = kmip_strnlen_s(username, 50);

    TextString p = {0};
    p.value = password;
    p.size = kmip_strnlen_s(password, 50);

    UsernamePasswordCredential upc = {0};
    upc.username = &u;
    upc.password = &p;

    Credential credential = {0};
    credential.credential_type = KMIP_CRED_USERNAME_AND_PASSWORD;
    credential.credential_value = &upc;

    int result = kmip_add_credential(&kmip_context, &credential);

    if (result != KMIP_OK) {
        fprintf(stderr, "Failed to add credential to the KMIP context.\n");
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        kmip_destroy(&kmip_context);
        return result;
    }

    result = kmip_bio_get_symmetric_key_with_context(&kmip_context, bio,
                                                     id, id_size,
                                                     &key, &key_size);

    BIO_free_all(bio);
    SSL_CTX_free(ctx);

    fprintf(stdout, "\n");
    if (result < 0) {
        fprintf(stdout, "An error occurred while getting the symmetric key.");
        fprintf(stdout, "Error Code: %d\n", result);
        fprintf(stdout, "Error Name: %s", kmip_error_string(result));
        fprintf(stdout, "\n");
        fprintf(stdout, "Context Error: %s\n", kmip_context.error_message);
        fprintf(stdout, "Stack trace:\n");
        kmip_stack_trace(&kmip_context, print_libkmip_stack, NULL);
    } else if (result >= 0) {

        fprintf(stdout, "The KMIP operation was executed with no errors.\n");
        fprintf(stdout, "Result: %s (%d)\n", kmip_result_status_enum(result), result);

        if (result == KMIP_STATUS_SUCCESS) {
            const char *padding = "               ";
            fprintf(stdout, "Symmetric Key ID: %s\n", id);
            fprintf(stdout, "Symmetric Key Size: %d bits\n", key_size * 8);
            fprintf(stdout, "Symmetric Key:");
            kmip_dump_buffer(key, key_size, print_libkmip_buffer, (void *)padding);
            fprintf(stdout, "\n");
        }

    }

    fprintf(stdout, "\n");

    if (key != NULL) {
        kmip_memset(key, 0x00, key_size);
        kmip_free(NULL, key);
    }

    kmip_destroy(&kmip_context);

    return result;

}


int
main(int argc, char **argv)
{

    char *server_address = "127.0.0.1";
    char *server_port = "5696";
    char *client_certificate = NULL;
    char *client_key = NULL;
    char *ca_certificate = NULL;
    char *username = NULL;
    char *password = NULL;
    char *id = NULL;
    enum kmip_version version = KMIP_1_0;
    int c;
    int error = 0;
    int result = EXIT_SUCCESS;

    while ((c = getopt(argc, argv, "a:c:dhi:k:p:r:s:u:v:")) != -1) {
        switch (c) {
        case 'a':
            server_address = optarg;
            break;
        case 'c':
            client_certificate = optarg;
            break;
        case 'd':
            kmip_debug(KMIP_DEBUG_REQUEST | KMIP_DEBUG_RESPONSE, NULL);
            break;
        case 'h':
            error = 1;
            break;
        case 'i':
            id = optarg;
            break;
        case 'k':
            client_key = optarg;
            break;
        case 'p':
            server_port = optarg;
            break;
        case 'r':
            ca_certificate = optarg;
            break;
        case 's':
            password = optarg;
            break;
        case 'u':
            username = optarg;
            break;
        case 'v':
            if (strcmp(optarg, "1.0") == 0) {
                version = KMIP_1_0;
            } else if (strcmp(optarg, "1.1") == 0) {
                version = KMIP_1_1;
            } else if (strcmp(optarg, "1.2") == 0) {
                version = KMIP_1_2;
            } else if (strcmp(optarg, "1.3") == 0) {
                version = KMIP_1_3;
            } else if (strcmp(optarg, "1.4") == 0) {
                version = KMIP_1_4;
            } else if (strcmp(optarg, "2.0") == 0) {
                version = KMIP_2_0;
            } else {
                fprintf(stderr, "Invalid KMIP protocol: '%s'\n", optarg);
                error = 1;
            }
            break;
        default:
            fprintf(stderr, "Invalid option: '-%c'\n", optopt);
            error = 1;
        }
    }

    if (error) {
        usage(argv[0]);
        result = EXIT_FAILURE;
    } else {
        error = use_mid_level_api(server_address, server_port,
                                  client_certificate, client_key,
                                  ca_certificate,
                                  username, password,
                                  id, version);
       result = (error < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    exit(result);

}
