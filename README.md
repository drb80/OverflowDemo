``` bash
sudo sysctl -w kernel.randomize_va_space=0
sudo apt update
sudo apt install gcc-multilib gdb
gcc -m32 -fno-stack-protector -z execstack -no-pie vuln.c -o vuln
```

