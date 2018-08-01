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
#include "global.c"

static int pdo_zim_stmt_dtor(pdo_stmt_t *stmt)  {
    //pdo_zim_stmt *S = (pdo_zim_stmt*)stmt->driver_data;
    //zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "pdo_zim_stmt_dtor");

    //efree(S);

    return 1;
}

static int pdo_zim_stmt_execute(pdo_stmt_t *stmt)  {
    //zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "pdo_zim_stmt_execute");
    pdo_zim_stmt *S = (pdo_zim_stmt*)stmt->driver_data;

    if(S == NULL || S->driver == NULL || S->sql == NULL || S->driver->my_ssh_session == NULL){
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "pdo_zim_stmt = [%d]\nzim_driver =[%d]\nsql =[%d]\nmy_ssh_session =[%d]\n",  S == NULL,  S->driver == NULL,  S->sql == NULL,  S->driver->my_ssh_session == NULL);
        return 0;
    }

    S->response = NULL;
    int result = exec(S->driver->my_ssh_session, S->sql, &S->response);
    if (  result == SSH_ERROR ) {
        zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "Erro ao executar Query");
    } else if( result == SSH_OK  ) {
        if (S->response != NULL) {
            return 1;
        } else {
            zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "Sem Resultado!");
        }
    }

    return 1;
}

static int pdo_zim_stmt_fetch(pdo_stmt_t *stmt, enum pdo_fetch_orientation ori, long offset TSRMLS_DC)  {
    //pdo_zim_stmt *S = (pdo_zim_stmt*)stmt->driver_data;
    zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "pdo_zim_stmt_fetch");


    return 0;
}

/*
 * get meta data for a column
 * INPUT: statement structure, column number
 * OUTPUT: 0: not a valid column
 *	   1: successful
 */
static int pdo_zim_stmt_describe(pdo_stmt_t *stmt, int colno TSRMLS_DC)
{
    //pdo_zim_stmt *S = (pdo_zim_stmt*)stmt->driver_data;
    zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "pdo_zim_stmt_describe");
   /* struct pdo_column_data *col = &stmt->columns[colno];

    if (!S->rows) {
        return 0;
    }

    col->maxlen = S->cols[colno].maxlen;
    col->namelen = strlen(S->cols[colno].name);
    col->name = estrdup(S->cols[colno].name);
    col->param_type = PDO_PARAM_STR;*/

    return 1;
}

/*
 * get data for a specific column
 * INPUT: statement structure, column number, char pointer, len pointer
 * OUTPUT:   1: successful
 */
static int pdo_zim_stmt_get_col(pdo_stmt_t *stmt, int colno, char **ptr,  unsigned long *len, int *caller_frees TSRMLS_DC)  {
    //pdo_zim_stmt *S = (pdo_zim_stmt*)stmt->driver_data;
    zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "pdo_zim_stmt_get_col");
    /*pdo_zim_colval *val = &S->rows[S->current * S->ncols + colno];

    *ptr = val->data;
    *len = val->len;*/
    return 1;
}


static int pdo_zim_stmt_param_hook(pdo_stmt_t *stmt, struct pdo_bound_param_data *param, enum pdo_param_event event_type TSRMLS_DC)  {
    //pdo_zim_stmt *S = (pdo_zim_stmt*)stmt->driver_data;
    zend_throw_exception_ex(php_pdo_get_exception(), 0, "SQLSTATE[%s]", "pdo_zim_stmt_param_hook");
    return 1;
}

struct pdo_stmt_methods zim_stmt_methods = {
        pdo_zim_stmt_dtor,
        pdo_zim_stmt_execute,
        pdo_zim_stmt_fetch,
        pdo_zim_stmt_describe,
        pdo_zim_stmt_get_col,
        pdo_zim_stmt_param_hook,
        NULL, /* set attr */
        NULL, /* get attr */
        NULL, /* meta */
};

