# DirtyFrag

# dirtyfrag

Linux kernel research framework for testing file corruption primitives.

## Features

- ARM64 support
- x86_64 support
- Safe mode checks
- Module detection
- Namespace detection
- Backup/restore support

## Compile

```bash
gcc -O2 -Wall dirtyfrag.c -o dirtyfrag

x86_64-linux-gnu-gcc -O2 -Wall dirtyfrag.c -o dirtyfrag
```

## ARM64

```bash
aarch64-linux-gnu-gcc -O2 -Wall dirtyfrag.c -o dirtyfrag_arm64
```
