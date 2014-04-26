#include "common.h"



#ifndef _SKY_CORE_STRING_H //防止多次加载
#define _SKY_CORE_STRING_H




#define SKY_SIZE_CHAR(n) sizeof(char)*n

char * sky_string_sub(char *, int , int);
void sky_string_to_array(char *, char *, char [][1000]);

#endif
