#include <stdio.h>
#include <string.h>
#include <unistd.h>

void secret_function() {
    printf("[!] Exploit Success! Redirected code execution to secret_function.\n");
    _exit(0);
}

void vulnerable_function(char *str) {
    char buffer[8];
    strcpy(buffer, str);
    printf("Buffer contains: %s\n", buffer);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input_string>\n", argv[0]);
        return 1;
    }
    vulnerable_function(argv[1]);
    printf("Program exiting normally.\n");
    return 0;
}
