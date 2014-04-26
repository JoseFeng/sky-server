#include "sky_string.h"


char * sky_string_sub(char *str, int start, int len){
		char *s = (char *)malloc(SKY_SIZE_CHAR(len));
		strncpy(s, str+start, len);
		return s;
}


void sky_string_to_array(char *str, char *delim, char arr[][1000]){

		int index = strcspn(str, delim);
		int slen  = strlen(str);
		int slim  = strlen(delim);
		int start = index+slim;

		char *s;

		s = sky_string_sub(str, 0, index);
		strcpy(arr[0], s);
		free(s);

		s = sky_string_sub(str, start, slen - start);
		strcpy(arr[1], s);
		free(s);
}

/*
int main (){

	char str[] = "Host: localhost:10000";
	char arr[2][1000];

	sky_string_to_array(str, ": ", arr);
	printf("%s\n", arr[0]);
	printf("%s\n", arr[1]);

	//

}
*/
