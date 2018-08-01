#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/pdo/php_pdo.h"
#include "ext/pdo/php_pdo_driver.h"
#include "php_pdo_zim.h"
#include "php_pdo_zim_int.h"
#include "zend_exceptions.h"
#include "zend_exceptions.h"
#include <libssh/libssh.h>

static void die_(const char *err){
    if (errno){
        perror(err);
    } else {
        fprintf(stderr,"%s\n",err);
    }
    exit(1);
}

static void *xmalloc(size_t len){
    void *ptr = malloc(len);
    if(ptr == NULL)
        die_("malloc error");
    return ptr;
}

static void xfree(void **ptr){
    assert(ptr);
    if(ptr != NULL){
        free(*ptr);
        *ptr = NULL;
    }
}

static char *addslashes(const char *str){
    size_t i = 0, x = 0, j = 0, size_x = 0;
    char *str2 = NULL;

    for(i=0; str[i]; i++){
        if(str[i] == 0x5c)
            x++;
        else if(str[i] == 0x27)
            x++;
        else if(str[i] == 0x22)
            x++;
    }

    size_x = x+i+1;

    str2 = (char*) malloc(size_x*sizeof(char));

    if(str2 == NULL) {
        return NULL;
    }

    for(i=0;str[i];i++){
        char c = str[i];

        if(c == 0x27){
            str2[j] = 0x5c;
            j++;
            str2[j] = c;
        }

        else if(c == 0x5c){
            str2[j] = c;
            j++;
            str2[j] = c;
        }

        else if(c == 0x22){
            str2[j] = 0x5c;
            j++;
            str2[j] = c;
        }

        else {
            str2[j] = c;
        }

        j++;
    }

    str2[j] = 0x0;
    return str2;
}

static void merge(char **s1, const char *s2) {
    size_t len = 0;
    if(*s1 != NULL){
        len = strlen(*s1);
    }
    int stringSize = len+ strlen(s2);
    char *result = malloc(stringSize + 1); // +1 for the null-terminator

    if (result == NULL) {
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "merge");
        return;
    }

    if(*s1 != NULL){
        strcpy(result, *s1);
        strcat(result, s2);
        free(*s1);
    } else {
        strcpy(result, s2);
    }
    //result[stringSize - 1] = '\0';
    *s1 = result;
}

static int exec(ssh_session session, const char* sql, char **result) {
    if (session == NULL || sql == NULL || result == NULL) {
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "Dados NULLOS");
        return SSH_ERROR;
    }
    //PART1
    ssh_channel channel;
    int rc;
    channel = ssh_channel_new(session);
    if (channel == NULL) {
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "Nao conectado");
        return SSH_ERROR;
    }
    rc = ssh_channel_open_session(channel);
    if (rc != SSH_OK) {
        ssh_channel_free(channel);
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "Sem Sessao");
        return rc;
    }

    char * nsql = addslashes(sql);
    if (nsql == NULL) {
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "addslashes");
        return SSH_ERROR;
    }
    char * mysql = NULL;
    merge(&mysql, "/trans/net/pquery \"cmndnet('");
    merge(&mysql, nsql);
    merge(&mysql, "')\"");
    xfree((void **)&nsql);

    //PART2
    rc = ssh_channel_request_exec(channel, mysql);
    //rc = ssh_channel_request_exec(channel, "/trans/net/pquery \"cmndnet('find etrpnonu')\"");

    if (rc != SSH_OK) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "Nao executou comando");
        return rc;
    }

    //PART3
    char buffer[256];
    int nbytes = 0;
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    while (nbytes > 0) {
        //zend_throw_exception_ex(php_pdo_get_exception(), 0, "pdo_zim_stmt[%d]", nbytes);
        char * tmp = malloc(nbytes + 1);
        if(tmp == NULL) {
            break;
        }
        strcpy(tmp, buffer);
        tmp[nbytes] = 0;
        merge(result, tmp);
        free(tmp);

        nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
    }

    if (nbytes < 0) {
        ssh_channel_close(channel);
        ssh_channel_free(channel);
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "Leitura Invalida");
        return SSH_ERROR;
    }

    //PART4
    ssh_channel_send_eof(channel);
    ssh_channel_close(channel);
    ssh_channel_free(channel);

    return SSH_OK;
}