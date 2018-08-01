#ifndef PDO_ZIM_GLOBAL_FILE_H
#define PDO_ZIM_GLOBAL_FILE_H

/*static void merge(char * text, char *extra) ;*/
static void merge(char **s1, const char *s2);
static int exec(ssh_session session, const char* sql, char **result);

#endif