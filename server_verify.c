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


void usage(const char *program)
{
    fprintf(stderr, "Usage: %s [flag value | flag] ...\n\n", program);
    fprintf(stderr, "Flags:\n");
    fprintf(stderr, "-a addr : the IP address of the KMIP server\n");
    fprintf(stderr, "-c path : path to client certificate file\n");
    fprintf(stderr, "-h      : print this help info\n");
    fprintf(stderr, "-i id   : the ID of the symmetric key to retrieve\n");
    fprintf(stderr, "-k path : path to client key file\n");
    fprintf(stderr, "-p port : the port number of the KMIP server\n");
    fprintf(stderr, "-r path : path to CA certificate file\n");
    fprintf(stderr, "-s pass : the password for KMIP server authentication\n");
    fprintf(stderr, "-u user : the username for KMIP server authentication\n");
}


void *check_calloc(void *state, size_t count, size_t size)
{
    debug_printf("state=%p, count=%zu, size=%zu\n", state, count, size);
    return calloc(count, size);
}


void *check_realloc(void *state, void *address, size_t size)
{
    debug_printf("state=%p, address=%p, size=%zu\n", state, address, size);
    return realloc(address, size);
}


void check_free(void *state, void *address)
{
    debug_printf("state=%p, address=%p\n", state, address);
    free(address);
}


int use_high_level_api(const char *server_address,
                       const char *server_port,
                       const char *client_certificate,
                       const char *client_key,
                       const char *ca_certificate,
                       char *id)
{

    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    SSL_library_init();
    ctx = SSL_CTX_new(SSLv23_client_method());

    fprintf(stdout, "\n");
    fprintf(stdout, "Loading the client certificate: %s\n", client_certificate);
    if (SSL_CTX_use_certificate_file(ctx, client_certificate, SSL_FILETYPE_PEM) != 1) {
        fprintf(stderr, "Loading the client certificate failed\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return -1;
    }

    fprintf(stdout, "Loading the client key: %s\n", client_key);
    if (SSL_CTX_use_PrivateKey_file(ctx, client_key, SSL_FILETYPE_PEM) != 1) {
        fprintf(stderr, "Loading the client key failed\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return -1;
    }

    fprintf(stdout, "Loading the CA certificate: %s\n", ca_certificate);
    if (SSL_CTX_load_verify_locations(ctx, ca_certificate, NULL) != 1) {
        fprintf(stderr, "Loading the CA file failed\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return -1;
    }

    BIO *bio = NULL;
    bio = BIO_new_ssl_connect(ctx);
    if (bio == NULL) {
        fprintf(stderr, "BIO_new_ssl_connect failed\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return -1;
    }

    BIO_get_ssl(bio, &ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
    BIO_set_conn_hostname(bio, server_address);
    BIO_set_conn_port(bio, server_port);
    if (BIO_do_connect(bio) != 1) {
        fprintf(stderr, "BIO_do_connect failed\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return -1;
    }

    char *key = NULL;
    int key_size = 0;

    int result = kmip_bio_get_symmetric_key(bio, id, kmip_strnlen_s(id, 50), &key, &key_size);

    BIO_free_all(bio);
    SSL_CTX_free(ctx);

    fprintf(stdout, "\n");
    if (result < 0) {
        fprintf(stdout, "An error occurred while deleting object: %s\n", id);
        fprintf(stdout, "Error Code: %d\n", result);
    } else {

        fprintf(stdout, "The KMIP operation was executed with no errors.\n");
        fprintf(stdout, "Result: ");
        kmip_print_result_status_enum(result);
        fprintf(stdout, " (%d)\n", result);

        if (result == 0) {
            fprintf(stdout, "Symmetric Key ID: %s\n", id);
            fprintf(stdout, "Symmetric Key Size: %d bits\n", key_size * 8);
            fprintf(stdout, "Symmetric Key:");
            kmip_print_buffer(key, key_size);
            fprintf(stdout, "\n");
        }

    }

    return result;

}


int
main(int argc, char **argv)
{

    char *server_address = NULL;
    char *server_port = NULL;
    char *client_certificate = NULL;
    char *client_key = NULL;
    char *ca_certificate = NULL;
    char *id = NULL;
    int c;
    int error = 0;
    int result = EXIT_SUCCESS;

    while ((c = getopt(argc, argv, "a:c:hi:k:p:r:")) != -1) {
        switch (c) {
        case 'a':
            server_address = optarg;
            break;
        case 'c':
            client_certificate = optarg;
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
        default:
            fprintf(stderr, "Invalid option: '-%c'\n", optopt);
            error = 1;
        }
    }

    if (error) {
        usage(argv[0]);
        result = EXIT_FAILURE;
    } else {
        error = use_high_level_api(server_address, server_port,
                                   client_certificate, client_key,
                                   ca_certificate,
                                   id);
       result = (error < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    exit(result);

}
