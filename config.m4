dnl $Id$
dnl config.m4 for extension pdo_zim


PHP_ARG_WITH(pdo_zim, for pdo_zim support,
dnl Make sure that the comment is aligned:
 [  --with-pdo_zim             Include pdo_zim support])


if test "$PHP_PDO_ZIM" != "no"; then

  PHP_ADD_LIBRARY_WITH_PATH(ssh, /usr/lib)
  PHP_ADD_LIBRARY_WITH_PATH(pcre, /usr/lib)

  PHP_SUBST(PDO_ZIM_SHARED_LIBADD)

  PHP_NEW_EXTENSION(pdo_zim,pdo_zim.c zim_driver.c zim_statement.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
