#ifndef CFG_INI_H_
#define CFG_INI_H_
#ifdef __cplusplus
extern "C"
 {
#endif

#include <stdbool.h>

#define  MAX_VALUE  1024 /* 定义section,key,value字符串最大长度 */

typedef struct _option {
  char    key[MAX_VALUE];   /* 对应键 */
  char    value[MAX_VALUE]; /* 对应值 */
  struct  _option *next;    /* 链表连接标识 */
}Option;
 
typedef struct _data {
  char    section[MAX_VALUE]; /* 保存section值          */
  Option  *option;            /* option链表头           */
  struct  _data *next;        /* 链表连接标识           */
}Data;

typedef struct {
  char    comment;              /* 表示注释的符号    */
  char    separator;            /* 表示分隔符        */
  char    re_string[MAX_VALUE]; /* 返回值字符串的值  */
  int     re_int;               /* 返回int的值       */
  bool    re_bool;              /* 返回bool的值      */
  double  re_double ;           /* 返回double类型    */
  Data    *data;                /* 保存数据的头      */
}Config;

bool str_empty(const char *string);
bool cnf_add_option(Config *cnf, const char *section, const char *key, const char *value);
bool strip_comments(char *string, char comment);
Config *cnf_read_config(const char *filename, char comment, char separator);
bool cnf_get_value(Config *cnf, const char *section, const char *key);
Data *cnf_has_section(Config *cnf, const char *section);
Option *cnf_has_option(Config *cnf, const char *section, const char *key);
bool cnf_write_file(Config *cnf, const char *filename, const char *header);
bool cnf_remove_option(Config *cnf, const char *section, const char *key);
bool cnf_remove_section(Config *cnf, const char *section);
void print_config(Config *cnf);

#ifdef __cplusplus
 }
#endif
#endif