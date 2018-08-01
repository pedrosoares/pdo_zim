//
// Created by pedrosoares on 7/30/18.
//

#ifndef PDO_ZIM_PHP_PDO_ZIM_INT_FILE_H
#define PDO_ZIM_PHP_PDO_ZIM_INT_FILE_H

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/pdo/php_pdo.h"
#include "ext/pdo/php_pdo_driver.h"
#include "php_pdo_zim.h"
#include "php_pdo_zim_int.h"
#include "zend_exceptions.h"
#include <libssh/libssh.h>

extern pdo_driver_t pdo_zim_driver;
extern struct pdo_stmt_methods zim_stmt_methods;

#define ZIM_SELECT 0
#define ZIM_UPDATE 1
#define ZIM_DELETE 2
#define ZIM_INSERT 3

typedef struct  {
    ssh_session my_ssh_session;
    const int port;// = 22;
    const char *username;
    const char *password;
    const char *cipher;// = "aes128-ctr";
} zim_driver;

typedef struct {
    char *name;
    char * value;
} column;

typedef struct {
    column *col;
} line;

typedef struct {
    //SQL
    char ** tables;
    int tables_size;

    //RESPONSE
    char * response;
    //DRIVER
    zim_driver 	*driver;
    char * sql;

    int executed;
    int type;

    line *lines;

} pdo_zim_stmt;

#endif //PDO_ZIM_PHP_PDO_ZIM_INT_FILE_H
