#ifndef ___REQUEST_H___
#define ___REQUEST_H___


typedef struct { 	
	dlink_list headers;
	dlink_list cookies;
} Request;

typedef struct { 
	dlink_list headers;
	char * content;
	size_t content_len
} Response;

typedef struct { 
	dlink_list session;
	Response * resp;
	Request  * req;
} Client;


#endif