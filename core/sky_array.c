#include "sky_array.h"

int sky_array_init(sky_sky_array_t *L){

	L=(sky_array_t*)malloc(sizeof(sky_sky_array_t));
	L->next = NULL;
	L->name = NULL;
	L->value = NULL;

	return 1;
}


int sky_array_push(sky_sky_array_t *L, char *name, char *value){

	sky_array_t *data, *pdata;

	data = (sky_array_t*)malloc(sizeof(sky_array_t));
	data->name = name;
	data->value = value;
	data->next = NULL;

	pdata = L;

	while(pdata->next!=NULL){
		pdata = pdata->next;
	}

	pdata->next = data;

	return 1;
}


char * sky_array_get_value(sky_array_t L, char *name){

	sky_array_t  *pdata;
	pdata = &L;

	while(pdata->next!=NULL){
		pdata = pdata->next;
		if(strcmp(pdata->name, name) == 0){
			return pdata->value;
		}
	}

	return NULL;
}

char * sky_array_del(sky_array_t *L, char *name){

	sky_array_t  *pdata, *p;
	char *value=NULL;
	pdata = L;
	while(pdata->next!=NULL){
		if(strcmp(pdata->next->name, name) == 0){
			p = pdata->next;
			pdata->next = p->next;
			value = p->value;
			free(p);
			p=NULL;
			break;
		}
		pdata = pdata->next;
	}

	return value;
}

void sky_array_print(sky_array_t array){

	sky_array_t *pdata;
	pdata = &array;
	while(pdata->next != NULL){
		pdata = pdata->next;
		printf("name:%s, value:%s\n",pdata->name,pdata->value);

	}
}

/*
int main (){

	sky_array_t *pdata;

	pdata = &L;
	initArray(&L);
	pushArray(&L, "name", "fengwei");
	pushArray(&L, "age", "18");
	printArray(L);

	printf("%s\n", delValue(&L, "name"));
	delValue(&L, "age");
	pushArray(&L, "a", "18");
	printArray(L);

	//printf("value:%s\n", getValue(L, "age"));

	return 0;
}

*/
