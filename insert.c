#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // output to stdout
    printf("HTTP/1.1 200 OK\r\n");
    printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");
    printf("<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n");

    // auto redirect to homepage in 3 seconds
    printf("<meta http-equiv='refresh' content='3; url=/'>");

    printf("<TITLE>Insert</TITLE>\n");

    // open database.txt to append
    FILE * fPtr = fopen ("database.txt","a+");
    
    // file open failed
    if (!fPtr) {    
        printf("File open failed, please try again.<br>");
    }
    else {
        // append insert text to newline
        fprintf (fPtr, "%s\n", argv[1]);
        // close file
        fclose (fPtr);

        printf("<BODY>Insert success: %s<br>\n", argv[1]);
    }

    printf("Redirect to homepage in 3 seconds...<br>\n");
    printf("</BODY></HTML>\n");

    return 0;
}