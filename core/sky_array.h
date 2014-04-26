#include "common.h"


#ifndef _SKY_CORE_ARRAY_H

#define _SKY_CORE_ARRAY_H

#define LIST_INIT_SIZE 10 // 线性表存储空间的初始分配量
#define LIST_INCREMENT 2 // 线性表存储空间的分配增量



typedef struct sky_array * sky_array_p;//定义节点指针
typedef struct sky_array{
	char *name;
	char *value;
	sky_array_p next;

}sky_array_t;


int sky_array_init(sky_array_t *);
int sky_array_push(sky_array_t *, char *name, char *value);
char * sky_array_get_value(sky_array_t, char *name);
char * sky_array_del(sky_array_t *, char *name);
void sky_array_print(sky_array_t );


#endif
