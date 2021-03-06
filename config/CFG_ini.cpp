#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "CFG_ini.h" 

// printf("File = %s\nLine = %d\nFunc=%s\nDate=%s\nTime=%s\n", __FILE__, __LINE__, __FUNCTION__, __DATE__, __TIME__);
#define  PRINT_ERRMSG(STR) fprintf(stderr,"line:%d,msg:%s,eMsg:%s\n", __LINE__, STR, strerror(errno))
 
 
/**
* 判断字符串是否为空
* 为空返回true,不为空返回false
**/
bool str_empty(const char *string)
{
  return NULL == string || 0 == strlen(string);
}
 
/**
* 向链表添加section,key,value
* 如果添加时不存在section则新增一个
* 如果对应section的key不存在则新增一个
* 如果section已存在则不会重复创建
* 如果对应section的key已存在则只会覆盖key的值
**/
bool cnf_add_option(Config *cnf, const char *section, const char *key, const char *value)
{
  if (NULL == cnf || str_empty(section) || str_empty(key) || str_empty(value)) {
    return false; /* 参数不正确,返回false */
  }
  
  Data *p = cnf->data; /* 让变量p循环遍历data,找到对应section */
  while (NULL != p && 0 != strcmp(p->section, section)) {
    p = p->next;
  }
  
  if (NULL == p) { /* 说明没有找到section,需要加一个 */
    Data *ps = (Data*)malloc(sizeof(Data));
    if (NULL == ps) {
      exit(-1); /* 申请内存错误 */
    }
    strcpy(ps->section, section);
    ps->option = NULL;    /* 初始的option要为空 */
    ps->next = cnf->data; /* cnf->data可能为NULL */
    cnf->data = p = ps;   /* 头插法插入链表 */
  }
  
  Option *q = p->option;
  while (NULL != q && 0 != strcmp(q->key, key)) {
    q = q->next; /* 遍历option,检查key是否已经存在 */
  }
  
  if (NULL == q) { /* 不存在option,则新建一个 */
    q = (Option*)malloc(sizeof(Option));
    if (NULL == q) {
      exit(-1); /* 申请内存错误 */
    }
    strcpy(q->key, key);
    q->next = p->option; /*这里p->option可能为NULL,不过也没关系 */
    p->option = q; /* 头插法插入链表 */
  }
  strcpy(q->value, value); /* 无论如何要把值改了 */
  
  return true;
}
 
/**
* 去掉字符串内所有空白
* 且忽略注释部分
* 最终得到没有空白的字符串
**/
bool strip_comments(char *string, char comment)
{
  if (NULL == string || '\n' == *string || '\r' == *string) {
    return false; /* 第一个字符为回车或换行,表示空行 */
  }
  
  char *p, *q; /* 下面去掉字符串中所有空白字符 */
  for (p = q = string; *p != '\0' && *p != comment; p++) {
    if (0 == isspace(*p)) {
      *q++ = *p; /* 不是空白字符则重写字符串 */
    }
  }
  *q = '\0';
  
  return 0 != strlen(string); /* 字符串长度不为0,表示数据可用 */
}
 
/**
* 传递配置文件路径
* 参数有文件路径,注释字符,分隔符
* 返回Config结构体
**/
Config *cnf_read_config(const char *filename, char comment, char separator)
{
  Config *cnf     = (Config*)malloc(sizeof(Config));
  cnf->comment    = comment; /* 每一行该字符及以后的字符将丢弃 */
  cnf->separator  = separator; /* 用来分隔Section 和 数据 */
  cnf->data       = NULL; /* 初始数据为空 */
  
  if (str_empty(filename)) {
    return cnf; /* 空字符串则直接返回对象 */
  }
  
  char *p, sLine[MAX_VALUE];    /* 保存一行数据到字符串 */
  char section[MAX_VALUE], key[MAX_VALUE], value[MAX_VALUE]; /* 缓存section,key,value */
  FILE *fp = fopen(filename, "r");
  if(NULL == fp) {
    PRINT_ERRMSG("fopen");
    exit(errno); /* 读文件错误直接按照错误码退出 */
  }
  
  while (NULL != fgets(sLine, MAX_VALUE, fp)) {
    if (strip_comments(sLine, cnf->comment)) { /* 去掉字符串所有空白,注释也忽略 */
      if ('[' == sLine[0] && ']' == sLine[strlen(sLine)-1]) {
        memset(section, '\0', MAX_VALUE); /* 清空section,因为strncpy不追加'\0' */
        strncpy(section, sLine+1, strlen(sLine)-2);
      } else if (NULL != (p = strchr(sLine, cnf->separator))) {  /* 存在分隔符 */
        memset(key,   '\0', MAX_VALUE); /* 清空key,因为strncpy不追加'\0' */
        strncpy(key,  sLine, p - sLine);
        strcpy(value, p + 1); /* strcpy会追加'\0',所以妥妥哒 */
        cnf_add_option(cnf, section, key, value); /* 添加section,key,value */
      } /* 如果该行不存在分隔符则忽略这一行 */
    } /* end strip_comments */
  } /* end while */
  
  fclose(fp);
  return cnf;
}
 
/**
* 获取指定类型的值
* 根据不同类型会赋值给对应值
* 本方法需要注意,int和double的转换,不满足就是0
*     需要自己写代码时判断好
**/
bool cnf_get_value(Config *cnf, const char *section, const char *key)
{
  Data *p = cnf->data; /* 让变量p循环遍历data,找到对应section */
  while (NULL != p && 0 != strcmp(p->section, section)) {
    p = p->next;
  }
  
  if (NULL == p) {
    PRINT_ERRMSG("section not find!");
    return false;
  }
  
  Option *q = p->option;
  while (NULL != q && 0 != strcmp(q->key, key)) {
    q = q->next; /* 遍历option,检查key是否已经存在 */
  }
  
  if (NULL == q) {
    PRINT_ERRMSG("key not find!");
    return false;
  }
  
  strcpy(cnf->re_string, q->value);       /* 将结果字符串赋值 */
  cnf->re_int    = atoi(cnf->re_string);  /* 转换为整形 */
  cnf->re_bool   = 0 == strcmp ("true", cnf->re_string); /* 转换为bool型 */
  cnf->re_double = atof(cnf->re_string);  /* 转换为double型 */
  
  return true;
}
 
/**
* 判断section是否存在
* 不存在返回空指针
* 存在则返回包含那个section的Data指针
**/
Data *cnf_has_section(Config *cnf, const char *section)
{
  Data *p = cnf->data; /* 让变量p循环遍历data,找到对应section */
  while (NULL != p && 0 != strcmp(p->section, section)) {
    p = p->next;
  }
  
  if (NULL == p) { /* 没找到则不存在 */
    return NULL;
  }
  
  return p;
}
 
/**
* 判断指定option是否存在
* 不存在返回空指针
* 存在则返回包含那个section下key的Option指针
**/
Option *cnf_has_option(Config *cnf, const char *section, const char *key)
{
  Data *p = cnf_has_section(cnf, section);
  if (NULL == p) { /* 没找到则不存在 */
    return NULL;
  }
  
  Option *q = p->option;
  while (NULL != q && 0 != strcmp(q->key, key)) {
    q = q->next; /* 遍历option,检查key是否已经存在 */
  }
  if (NULL == q) { /* 没找到则不存在 */
    return NULL;
  }
  
  return q;
}
 
/**
* 将Config对象写入指定文件中
* header表示在文件开头加一句注释
* 写入成功则返回true
**/
bool cnf_write_file(Config *cnf, const char *filename, const char *header)
{
  FILE *fp = fopen(filename, "w");
  if(NULL == fp) {
    PRINT_ERRMSG("fopen");
    exit(errno); /* 读文件错误直接按照错误码退出 */
  }
  
  if (0 < strlen(header)) { /* 文件注释不为空,则写注释到文件 */
    fprintf(fp, "%c %s\n\n", cnf->comment, header);
  }
  
  Option *q;
  Data   *p = cnf->data;
  while (NULL != p) {
    fprintf(fp, "[%s]\n", p->section);
    q = p->option;
    while (NULL != q) {
      fprintf(fp, "%s %c %s\n", q->key, cnf->separator, q->value);
      q = q->next;
    }
    p = p->next;
  }
  
  fclose(fp);
  return true;
}
 
/**
* 删除option
**/
bool cnf_remove_option(Config *cnf, const char *section, const char *key)
{
  Data *ps = cnf_has_section(cnf, section);
  if (NULL == ps) { /* 没找到则不存在 */
    return NULL;
  }
  
  Option *p, *q;
  q = p = ps->option;
  while (NULL != p && 0 != strcmp(p->key, key)) {
    if (p != q) { q = q->next; } /* 始终让q处于p的上一个节点 */
    p = p->next;
  }
  
  if (NULL == p) { /* 没找到则不存在 */
    return NULL;
  }
  
  if (p == q) { /* 第一个option就匹配了 */
    ps->option = p->next;
  } else {
    q->next = p->next;
  }
  
  free(p);
  q = p = NULL; // 避免野指针
  
  return true;
}
 
/**
* 删除section
**/
bool cnf_remove_section(Config *cnf, const char *section)
{
  if (str_empty(section)) {
    return false;
  }
  
  Data *p, *q;
  q = p = cnf->data; /* 让变量p循环遍历data,找到对应section */
  while (NULL != p && 0 != strcmp(p->section, section)) {
    if (p != q) { q = q->next; } /* 始终让q处于p的上一个节点 */
    p = p->next;
  }
  
  if (NULL == p) { /* 没有找到section */
    return false;
  }
  
  if (p == q) { /* 这里表示第一个section,因此链表头位置改变 */
    cnf->data = p->next;
  } else { /* 此时是中间或尾部节点 */
    q->next = p->next;
  }
  
  Option *o = p->option;
  while (NULL != o) {
    free(o); /* 循环释放所有option */
    o = o->next;
  }
  p->option = NULL; // 避免野指针
  free(p); /* 释放删除的section */
  q = p = NULL;  // 避免野指针
  
  return true;
}
 
/**
* 打印当前Config对象
**/
void print_config(Config *cnf)
{
  Data *p = cnf->data; // 循环打印结果
  while (NULL != p) {
    printf("[%s]\n",p->section);
    
    Option *q = p->option;
    while (NULL != q) {
      printf("  %s %c %s\n", q->key, cnf->separator, q->value);
      q = q->next;
    }
    p = p->next;
  }
}