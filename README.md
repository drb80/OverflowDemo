``` C
#include <stdio.h>
#include <string.h>

void secret_function() {
    printf("[!] Exploit Success! Redirected code execution to secret_function.\n");
}

void vulnerable_function(char *str) {
    char buffer[64];
    // Vulnerable function: No bounds checking
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
```

``` bash
sudo sysctl -w kernel.randomize_va_space=0
sudo apt update
sudo apt install gcc-multilib gdb
gcc -m32 -fno-stack-protector -z execstack -no-pie vuln.c -o vuln
```

