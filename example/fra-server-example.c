#include <fcgi_stdio.h>

int main() {
	 while( FCGI_Accept() >= 0 ) {
		 printf("Content-type: text/html\r\nStatus: 200 OK\r\n\r\nHello world :)\r\n\r\n");
	 }
	 return 0;
}
