#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DATA_LENGTH 100

int main(int argc, char *argv[]){

    FILE *fPtr;
    char line[DATA_LENGTH+1];

    // output to stdout
    printf("HTTP/1.1 200 OK\r\n");
    printf("Content-Type: text/html; charset=UTF-8\r\n\r\n");
    printf("<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n");
    printf("<TITLE>View</TITLE>\n");
    printf("<BODY>");

    // open database.txt to stdout
    fPtr = fopen("database.txt", "r");

    // file open failed
    if (!fPtr) {    
        printf("File not exist, please insert first.<br>");
    }
    // start to print content in database.txt
    else            
    {
        // printf("Following data in database.txt: <br>");
        while (fgets(line, sizeof(line), fPtr) != NULL) {
            printf("%s<br>\n", line);
        }
        printf("\n");
        // close file
        fclose(fPtr);
    }
    printf("</BODY></HTML>\n");

    return 0;
}