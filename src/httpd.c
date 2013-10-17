#include "stdinclude.h"
#include "appinclude.h"




void http_headers(Socket * s, int clength)
{
	socket_write(s, "HTTP/1.0 200 OK\r\n");
	socket_write(s, "Content-Type: text/html\r\n");
	socket_write(s, "Host: localhost\r\n");
	socket_write(s, "Content-Length: %d\r\n", clength);
	socket_write(s, "\r\n");
}


void http_parser(Socket * s) {
	
}