
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
#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include "global.c"

static int zim_handle_closer(pdo_dbh_t *dbh)  {
    //zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "zim_handle_closer");
    zim_driver *driver = (zim_driver *)dbh->driver_data;
    if (driver) {
        ssh_disconnect(driver->my_ssh_session);
        ssh_free(driver->my_ssh_session);

        pefree(driver, dbh->is_persistent);
        dbh->driver_data = NULL;
    }
    //zend_throw_exception_ex(php_pdo_get_exception(), 0 TSRMLS_CC, "SQLSTATE[%s]: %s", "deu pau", "deupau");
    return 1;
}

int lower(int a) {
    if ((a >= 65) && (a <= 90))
        a = a + 32;
    return a;
}

static int identify( char * sql ){
    if(sql == NULL){
        return -1;
    }
    if(strlen(sql) < 6) {
        return -2;
    }
    switch (lower(sql[0])) {
        case 's':
            return ZIM_SELECT;
            break;
        case 'i':
            return ZIM_INSERT;
            break;
        case 'u':
            return ZIM_INSERT;
            break;
        case 'd':
            return ZIM_DELETE;
            break;
    }
}

int tablesName(char * subject, char *** tables, int *tables_qtd) {
    int result_index = 0;
    char ** result;

    result = malloc(sizeof(result)*1);
    int OVECCOUNT = 90;
    pcre *re;
    const char *error;
    char *pattern;
    int erroffset;
    int ovector[OVECCOUNT];
    int subject_length;
    int rc;

    pattern = "(?i)(?<=FROM|JOIN|INTO|UPDATE)(\\s*)(\\w+\\b)";
    subject_length = (int) strlen(subject);

    re = pcre_compile(pattern, 0, &error, &erroffset, NULL);

    if (re == NULL) {
        printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return 1;
    }

    rc = pcre_exec(re, NULL, subject, subject_length,  0, 0, ovector,  OVECCOUNT);

    while (rc > 0) {
        const char *match_result;
        if (pcre_get_substring(subject, ovector, rc, 2, &match_result) >= 0) {
            if(result_index > 0){
                char ** result_tmp = (char**) realloc(result, sizeof(*result)*(result_index+1));
                if(result_tmp) {
                    result = result_tmp;
                }
            }
            result[result_index] = malloc(sizeof(match_result));
            strcpy(result[result_index], match_result);
            result_index++;
        }
        rc = pcre_exec(re, NULL, subject,  subject_length, ovector[1], 0, ovector, OVECCOUNT);
    }

    pcre_free(re);

    *tables = result;
    *tables_qtd = result_index;

    return 0;
}

static int zim_handle_preparer(pdo_dbh_t *dbh, char *sql, int sql_len, pdo_stmt_t *stmt, zval *driver_options)  {
    //zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "zim_handle_preparer");
    zim_driver *H = (zim_driver *)dbh->driver_data;
    char *nsql = NULL;
    size_t nsql_len;

    pdo_zim_stmt *S = ecalloc(1, sizeof(*S));
    memset(S, '\0', sizeof (*S));
    S->driver = H;
    S->sql = sql;
    S->type = identify(sql);
    tablesName(sql, &S->tables, &S->tables_size);

    stmt->driver_data = S;
    stmt->methods = &zim_stmt_methods;
    stmt->supports_placeholders = PDO_PLACEHOLDER_NONE;
    /*stmt->supports_placeholders = PDO_PLACEHOLDER_POSITIONAL;*/
    int ret = pdo_parse_params(stmt, (char*)sql, sql_len, &nsql, &nsql_len);
    if( ret  == 1 ) {
        sql = nsql;
        sql_len = nsql_len;
    } else if( ret  == -1 ){
        strcpy(dbh->error_code, stmt->error_code);
        return 0;
    }

    return 1;
}

static long zim_handle_doer(pdo_dbh_t *dbh, const char *sql, long sql_len) {
    zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "zim_handle_doer");
    zim_driver *driver = (zim_driver *)dbh->driver_data;

    char * result = NULL;

    int rc = 1;

    if( exec(driver->my_ssh_session, sql, &result) == SSH_ERROR ) {
        rc = 0;
    }
    if(result != NULL) {
        free(result);
    }

    return rc;
}

static int zim_handle_quoter(pdo_dbh_t *dbh, const char *unquoted, int unquotedlen, char **quoted, int *quotedlen, enum pdo_param_type paramtype TSRMLS_DC) {
    zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "zim_handle_quoter");
    char *q;
    int l = 1;

    *quoted = q = emalloc(2 * unquotedlen + 3);
    *q++ = '\'';

    while (unquotedlen--) {
        if (*unquoted == '\'') {
            *q++ = '\'';
            *q++ = '\'';
            l += 2;
        } else {
            *q++ = *unquoted;
            ++l;
        }
        unquoted++;
    }

    *q++ = '\'';
    *q++ = '\0';
    *quotedlen = l+1;
    return 1;
}

static int zim_fetch_error(pdo_dbh_t *dbh, pdo_stmt_t *stmt, zval *info TSRMLS_DC) {
    /**
     * Implement Later, Error is for pussies
     */
    zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "zim_fetch_error");
    return 1;
}

static struct pdo_dbh_methods zim_dbh_methods = {
        zim_handle_closer,
        zim_handle_preparer,
        zim_handle_doer,
        zim_handle_quoter,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL, /* last insert */
        zim_fetch_error, /* fetch error */
        NULL, /* get attr */
        NULL, /* check aliveness */
};

static int pdo_zim_handle_factory(pdo_dbh_t *dbh, zval *driver_options TSRMLS_DC) {
    //zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "pdo_zim_handle_factory");

    int verbosity = SSH_LOG_NOLOG;
    zim_driver * driver;

    struct pdo_data_src_parser vars[] = {
            { "appname",		"PHP ZIM Driver",	0 },
            { "host",		"127.0.0.1", 0 },
            { "user",		NULL,	0 },
            { "password",		NULL,	0 }
    };

    php_pdo_parse_data_source(dbh->data_source, dbh->data_source_len, vars, 4);

    //zend_throw_exception_ex(php_pdo_get_exception(), 0 TSRMLS_CC, "SQLSTATE[%s]: %s", "deu pau", vars[1].optval);

    driver = pecalloc(1, sizeof(*driver), dbh->is_persistent);

    driver->my_ssh_session  = ssh_new();
    if (driver->my_ssh_session == NULL){
            pefree(driver, dbh->is_persistent);
            zend_throw_exception_ex(php_pdo_get_exception(), 0 TSRMLS_CC, "DRIVER [500]: %s", "Nao foi possivel inicializar");
            return 0;
    }
    ssh_options_set(driver->my_ssh_session, SSH_OPTIONS_HOST, vars[1].optval);
    ssh_options_set(driver->my_ssh_session, SSH_OPTIONS_USER, vars[2].optval);
    ssh_options_set(driver->my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
    ssh_options_set(driver->my_ssh_session, SSH_OPTIONS_CIPHERS_C_S, "aes128-ctr");
    int rc = ssh_connect(driver->my_ssh_session);
    if (rc != SSH_OK) {
        zend_throw_exception_ex(php_pdo_get_exception(), 0 TSRMLS_CC, "DRIVER [500]: %s", ssh_get_error(driver->my_ssh_session));
        ssh_free(driver->my_ssh_session);
        return 0;
    }
    rc = ssh_userauth_password(driver->my_ssh_session, NULL, vars[3].optval);
    if (rc != SSH_AUTH_SUCCESS) {
        zend_throw_exception_ex(php_pdo_get_exception(), 0 TSRMLS_CC, "DRIVER [550]: Error authenticating with password (%s)", ssh_get_error(driver->my_ssh_session));
        ssh_disconnect(driver->my_ssh_session);
        ssh_free(driver->my_ssh_session);
        return 0;
    }

    dbh->methods = &zim_dbh_methods;
    dbh->driver_data = driver;

    return 1;
}

pdo_driver_t pdo_zim_driver = {
        PDO_DRIVER_HEADER(zim),
        pdo_zim_handle_factory
};