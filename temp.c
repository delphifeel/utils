// Online C compiler to run C program online
#include <stdio.h>

char* mystrcat(char* dest, const char* src )
{
     while (*dest != '\0') dest++;
     while ((*dest++ = *src++));
     return --dest;
}

int main() {
    char buff[300];
    buff[299] = 0;
    char *str = buff;
    str = mystrcat(str, "Hello");
    str = mystrcat(str, "World");
    str = mystrcat(str, "And Bob");

    printf("%s\n", buff);

    return 0;
}
