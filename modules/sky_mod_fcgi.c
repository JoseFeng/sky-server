
#include "sky_mod_fcgi.h"

typedef struct sockaddr SA;
#define PARAMS_BUFF_LEN 1024
#define CONTENT_BUFF_LEN 65535



static FCGI_Header MakeHeader(int type, int requestId, int contentLength, int paddingLength);
static FCGI_BeginRequestBody MakeBeginRequestBody(int role, int keepConnection);
static void FCGI_BuildNameValueBody(char *name,int nameLen, char *value, int valueLen, unsigned char *bodyBuffPtr, int *bodyLenPtr);
int sendParam (char *name, char *value, int sockfd, int requestId);
/*
 *----------------------------------------------------------------------
 * MakeHeader --
 * Constructs an FCGI_Header struct.
 *----------------------------------------------------------------------
 */
static FCGI_Header MakeHeader(int type, int requestId, int contentLength, int paddingLength){

	FCGI_Header header;
	header.version = FCGI_VERSION_1;
	header.type	= (unsigned char) type;
	header.requestIdB1	= (unsigned char) ((requestId >> 8) & 0xff);
	header.requestIdB0	= (unsigned char) ((requestId) & 0xff);
	header.contentLengthB1	 = (unsigned char) ((contentLength >> 8) & 0xff);
	header.contentLengthB0	 = (unsigned char) ((contentLength) & 0xff);
	header.paddingLength	= (unsigned char) paddingLength;
	header.reserved	= 0;

	return header;
}

/*
 *----------------------------------------------------------------------
 * MakeBeginRequestBody --
 * Constructs an FCGI_BeginRequestBody record.
 *----------------------------------------------------------------------
 */
static FCGI_BeginRequestBody MakeBeginRequestBody(int role, int keepConnection){
	FCGI_BeginRequestBody body;
	body.roleB1 = (unsigned char) ((role >>  8) & 0xff);
	body.roleB0 = (unsigned char) (role   & 0xff);
	body.flags  = (unsigned char) ((keepConnection) ? FCGI_KEEP_CONN : 0);
	memset(body.reserved, 0, sizeof(body.reserved));
	return body;
}

static void FCGI_BuildNameValueBody(char *name,int nameLen, char *value, int valueLen, unsigned char *bodyBuffPtr, int *bodyLenPtr) {

	unsigned char *startBodyBuffPtr = bodyBuffPtr;

	if (nameLen < 0x80) {
		*bodyBuffPtr++ = (unsigned char) nameLen;
	} else {
	*bodyBuffPtr++ = (unsigned char) ((nameLen >> 24) | 0x80);
	*bodyBuffPtr++ = (unsigned char) (nameLen >> 16);
	*bodyBuffPtr++ = (unsigned char) (nameLen >> 8);
	*bodyBuffPtr++ = (unsigned char) nameLen;
	}

	if (valueLen < 0x80) {
		*bodyBuffPtr++ = (unsigned char) valueLen;
	} else {
		*bodyBuffPtr++ = (unsigned char) ((valueLen >> 24) | 0x80);
		*bodyBuffPtr++ = (unsigned char) (valueLen >> 16);
		*bodyBuffPtr++ = (unsigned char) (valueLen >> 8);
		*bodyBuffPtr++ = (unsigned char) valueLen;
	}

	while(*name != '\0') {
		*bodyBuffPtr++ = *name++;
	}

	while(*value != '\0') {
		*bodyBuffPtr++ = *value++;
	}

	*bodyLenPtr = bodyBuffPtr - startBodyBuffPtr;
}


int sendParam (char *name, char *value, int sockfd, int requestId){

	int nameLen, valueLen, bodyLen,valuenameRecordLen,count;
	unsigned char bodyBuff[PARAMS_BUFF_LEN];
	FCGI_Header nameValueHeader;


	bzero(bodyBuff,PARAMS_BUFF_LEN);
	nameLen = strlen(name);
	valueLen = strlen(value);

	FCGI_BuildNameValueBody(name, nameLen, value, valueLen, &bodyBuff[0], &bodyLen);
	nameValueHeader = MakeHeader(FCGI_PARAMS, requestId,bodyLen, 0);
	valuenameRecordLen = bodyLen+FCGI_HEADER_LEN;

	char myvaluenameRecord[valuenameRecordLen];
	memcpy(myvaluenameRecord, (char *)&nameValueHeader, FCGI_HEADER_LEN);
	memcpy(myvaluenameRecord+FCGI_HEADER_LEN, bodyBuff, bodyLen);

	count = write(sockfd, (char *)&myvaluenameRecord, valuenameRecordLen);
	if(count != valuenameRecordLen) {
		printf("write aluenameRecord error.len:%d,send:%d",valuenameRecordLen,count);
		perror("write");
		exit(1);
	}
	return 0;
}

int main() {

	int sockfd, count, requestId = 1, result;
	struct sockaddr_in serveraddr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
	}

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr.sin_port = htons((unsigned short) 9000);
	result = connect(sockfd ,(SA *)&serveraddr, sizeof(serveraddr));

	if(result < 0) {
		perror("bind");
		exit(1);
	}

	FCGI_BeginRequestRecord beginRecord;
	beginRecord.header = MakeHeader(FCGI_BEGIN_REQUEST, requestId, sizeof(beginRecord.body), 0);
	beginRecord.body = MakeBeginRequestBody(FCGI_RESPONDER, 0);
	count = write(sockfd, (char *)&beginRecord, sizeof(beginRecord));

	if(count != sizeof(beginRecord)) {
		printf("write error.len:%ld,send:%d",sizeof(beginRecord),count);
		perror("write");
		exit(1);
	}


	array_t array,*parray=&array;//声明数组
	initArray(&array);//初始化数组
	pushArray(&array, "SCRIPT_FILENAME", "/var/www/f.php");
	pushArray(&array, "REQUEST_METHOD", "GET");
	pushArray(&array, "SERVER_PROTOCOL", "HTTP/1.1");
	pushArray(&array, "HTTP_COOKIE", "a=b;c=b");

	while(parray->next != NULL){
		parray = parray->next;
		sendParam (parray->name, parray->value, sockfd, requestId);
	}

	//结束请求
	FCGI_Header endHeader;
	endHeader = MakeHeader(FCGI_PARAMS, requestId, 0, 0);
	count = write(sockfd, (char *)&endHeader, FCGI_HEADER_LEN);

	if(count != FCGI_HEADER_LEN) {
		perror("write");
		exit(1);
	}

	//读取返回头信息
	FCGI_Header responderHeader;
	char content[CONTENT_BUFF_LEN];
	int contenLen;
	char tmp[8];

	while(read(sockfd, &responderHeader, FCGI_HEADER_LEN)>0) {


		if(responderHeader.type == FCGI_STDOUT) {
			responderHeader.contentLengthB1 = 1;
			contenLen = (responderHeader.contentLengthB1<<8)+(responderHeader.contentLengthB0);
			bzero(content,CONTENT_BUFF_LEN);
			count = read(sockfd,content,contenLen);

			if(count != contenLen) {
				perror("read");
			}

			fprintf(stdout,"%s",content);
			//跳过填充部分
			if(responderHeader.paddingLength>0) {
				count = read(sockfd,tmp,responderHeader.paddingLength);
				if(count != responderHeader.paddingLength) {
					perror("read");
				}
			}
		} else if(responderHeader.type == FCGI_STDERR) {
			contenLen = (responderHeader.contentLengthB1<<8)+(responderHeader.contentLengthB0);
			bzero(content,CONTENT_BUFF_LEN);
			count = read(sockfd,content,contenLen);
			if(count != contenLen) {
				perror("read");
			}

			fprintf(stdout,"error:%s\n",content);
			//跳过填充部分

			if(responderHeader.paddingLength>0) {
				//long int n=lseek(sockfd,responderHeader.paddingLength,SEEK_CUR);
				count = read(sockfd,tmp,responderHeader.paddingLength);
				if(count != responderHeader.paddingLength) {
					perror("read");
				}
			}
		} else if(responderHeader.type == FCGI_END_REQUEST) {
			FCGI_EndRequestBody endRequest;
			count = read(sockfd,&endRequest,8);
			if(count != 8) {
				perror("read");
			}

			fprintf(stdout,"\nendRequest:appStatus:%d,protocolStatus:%d\n",(endRequest.appStatusB3<<24)+(endRequest.appStatusB2<<16)
					+(endRequest.appStatusB1<<8)+(endRequest.appStatusB0),endRequest.protocolStatus);
		}
	}

	close(sockfd);
	return 0;
}
