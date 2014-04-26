
#include "array.h"
#include "array.h"


#define SERVER_NAME "web_server"
#define PROTOCOL "HTTP/1.0"
#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
//根目录
#define ROOT "/var/www/"

static char* get_mime_type(char *);
static void send_headers( int , char*, int, char* , off_t, time_t);
static void send_test( int fd);
static void get_path(char*, char[]);
static int read_file(char[], char[], time_t *);





void * handle(void * arg){

	int fd,ret,fd_log,len,status,i=0;
	FILE *file=NULL;
	char line[1000],method[100],path[1000],protocol[1000],content[10000],abs_path[1000];
	char *file_path,* mime_type, *delim = ": ",*key, *val;
	time_t mtime;

	array_t params;
	initArray(&params);


	pthread_detach(pthread_self());//回收线程
	fd_log = open("log",O_APPEND|O_RDWR);

	fd = *((int *) arg);//输入的文件描述符
	file = fdopen(fd, "r");//转成文件指针

	//获取文件地址，http协议等
	fgets( line, sizeof(line), file );
	write(fd_log, line, strlen(line));
	sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol);

 	while (1){
	 	fgets( line, sizeof(line), file );
	 	write(fd_log, line, strlen(line));


      	if (strcmp( line, "\r\n" ) == 0){
      		//printArray(params );
      		/*
      		if(strcmp( method, "POST" ) == 0){
      			fgets( line, getValue(params, "Content-Length"), file );
      			write(fd_log, line, strlen(line));
      		}*/
      		//break;
		}else{

		 	//key = strtok(line, delim);
	 		//val = strtok(NULL, delim);
	 		//printf("%s\n%s\n", key, val);
	 		//pushArray(&params , key, val);
		}
	}

	if(path[0] != '/'){
		//TODO 路径不对
	}

	file_path = &(path[1]);
	get_path(file_path, abs_path);

	mime_type = get_mime_type(abs_path);

	status = read_file(abs_path, content, &mtime);
	send_headers(status, "Ok", fd, mime_type, strlen(content), mtime);

	write(fd, content, strlen(content));
	close(fd);
	pthread_exit(NULL);
	exit(0);
}


int main(){

	int fd,fd2,ret;
	struct sockaddr_in addr;
	int opt =1;
 	char s[100];
	pthread_t tid;

	fd = socket(PF_INET,SOCK_STREAM, 0);
 	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	memset(&addr, 0, sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(10000);
	addr.sin_addr.s_addr=htons(INADDR_ANY);
	ret = bind(fd,  (struct sockaddr *)&addr, sizeof(addr));

	listen(fd, 20);
	while(1){
		fd2 = accept(fd, NULL, NULL);
		printf("fd2= %d\n", fd2);
		pthread_create(&tid, NULL, handle, (void*)&fd2);
	}
}


static void send_headers( int status, char* title, int fd, char* mime_type, off_t length, time_t mod ){
    time_t now;
    char timebuf[100];
	char head[10000];
	char tmp[1000];
	now = time( (time_t*) 0 );

    (void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );


	sprintf(tmp, "%s %d %s\r\n", PROTOCOL, status, title);
	sprintf(head, "%s", tmp);
	sprintf(tmp, "Server:%s\r\n", SERVER_NAME);
	sprintf(head, "%s%s", head, tmp);
	sprintf(tmp, "Date:%s\r\n", timebuf);
	sprintf(head, "%s%s", head, tmp);


    if ( mime_type != (char*) 0 ){
    	sprintf(tmp, "Content-Type: %s\r\n", mime_type );
		sprintf(head, "%s%s", head, tmp);
	}
    if ( length >= 0 ){
    	sprintf(tmp, "Content-Length: %ld\r\n", (int64_t) length );
		sprintf(head, "%s%s", head, tmp);
	}
    if ( mod != (time_t) -1 ){
    	(void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &mod ) );
    	sprintf(tmp, "Last-Modified: %s\015\012", timebuf );
		sprintf(head, "%s%s", head, tmp);
    }

	sprintf(head, "%sConnection: close\r\n\r\n", head);

	write(fd, head, strlen(head));
}

static int read_file(char abs_path[1000], char content[10000],  time_t *mtime){
	int fd,status;
	struct stat stat_buf;

	fd = open(abs_path, O_RDONLY);
	if(fd<0){
		if(errno == ENOENT){//404
			strcpy(content, "404");
			status = 404;
		}else if(errno == EACCES){//没有权限
			strcpy(content, "403");
			status = 403;
		}
	}else{
		stat(abs_path, &stat_buf);
		mtime = &(stat_buf.st_mtime);
		read(fd, content, 10000);
		status=200;
	}

	close(fd);
	return status;
}

static void get_path(char* file_path, char* abs_path){

	char *s;

	s = strtok(file_path, "?");

	abs_path[0]='\0';
	abs_path = strcat(abs_path, ROOT);
	abs_path = strcat(abs_path, s);
}

static char* get_mime_type(char *name){

	char* dot;

    dot = strrchr( name, '.' );
    if ( dot == (char*) 0 )
	    return "text/plain; charset=iso-8859-1";
    if ( strcmp( dot, ".html" ) == 0 || strcmp( dot, ".htm" ) == 0 )
    	return "text/html; charset=iso-8859-1";
    if ( strcmp( dot, ".jpg" ) == 0 || strcmp( dot, ".jpeg" ) == 0 )
    	return "image/jpeg";
    if ( strcmp( dot, ".gif" ) == 0 )
    	return "image/gif";
    if ( strcmp( dot, ".png" ) == 0 )
    	return "image/png";
    if ( strcmp( dot, ".css" ) == 0 )
    	return "text/css";
	if ( strcmp( dot, ".js" ) == 0 )
    	return "text/javascript";

	return "text/plain; charset=iso-8859-1";
}


//TODO
static void write_log (){
	int fd;
	fd = open("access.log", O_WRONLY|O_APPEND|O_CREAT);

}
