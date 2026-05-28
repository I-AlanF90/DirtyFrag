/*
 * dirtyfrag improvements template
 * safer / cleaner / architecture-aware framework
 *
 * goals:
 * - ARM + x86_64 support
 * - safer testing
 * - better debugging
 * - cleaner GitHub repo
 * - easier maintenance
 *
 * compile:
 * gcc -O2 -Wall dirtyfrag.c -o dirtyfrag
 *
 * ARM64:
 * aarch64-linux-gnu-gcc -O2 -Wall dirtyfrag.c -o dirtyfrag_arm64
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/utsname.h>
#include <sys/stat.h>

/* ============================================================
 * logging
 * ============================================================ */

#define INFO(fmt, ...) \
    fprintf(stderr, "[+] " fmt "\n", ##__VA_ARGS__)

#define WARN(fmt, ...) \
    fprintf(stderr, "[-] " fmt "\n", ##__VA_ARGS__)

#define GOOD(fmt, ...) \
    fprintf(stderr, "[*] " fmt "\n", ##__VA_ARGS__)

/* ============================================================
 * architecture detection
 * ============================================================ */

void print_architecture(void)
{
    struct utsname u;

    if (uname(&u) < 0) {
        WARN("uname failed: %s", strerror(errno));
        return;
    }

    INFO("System: %s", u.sysname);
    INFO("Kernel: %s", u.release);
    INFO("Architecture: %s", u.machine);

    if (!strcmp(u.machine, "x86_64"))
        GOOD("Detected x86_64");

    else if (!strcmp(u.machine, "aarch64"))
        GOOD("Detected ARM64");

    else
        WARN("Unknown architecture");
}

/* ============================================================
 * kernel/module checks
 * ============================================================ */

int module_loaded(const char *name)
{
    FILE *f = fopen("/proc/modules", "r");

    if (!f)
        return 0;

    char line[256];

    while (fgets(line, sizeof(line), f)) {

        if (strstr(line, name)) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

void check_modules(void)
{
    INFO("Checking kernel modules...");

    if (module_loaded("xfrm_user"))
        GOOD("xfrm_user loaded");
    else
        WARN("xfrm_user NOT loaded");

    if (module_loaded("rxrpc"))
        GOOD("rxrpc loaded");
    else
        WARN("rxrpc NOT loaded");
}

/* ============================================================
 * apparmor detection
 * ============================================================ */

void check_apparmor(void)
{
    if (access("/sys/module/apparmor", F_OK) == 0)
        WARN("AppArmor detected");
    else
        GOOD("AppArmor not detected");
}

/* ============================================================
 * namespace support
 * ============================================================ */

void check_namespaces(void)
{
    INFO("Checking namespace support...");

    if (access("/proc/self/ns/user", F_OK) == 0)
        GOOD("User namespaces supported");
    else
        WARN("User namespaces unavailable");

    if (access("/proc/self/ns/net", F_OK) == 0)
        GOOD("Network namespaces supported");
    else
        WARN("Network namespaces unavailable");
}

/* ============================================================
 * backup system
 * ============================================================ */

int backup_file(const char *src, const char *dst)
{
    int in = open(src, O_RDONLY);

    if (in < 0) {
        WARN("open(%s): %s", src, strerror(errno));
        return -1;
    }

    int out = open(dst, O_CREAT | O_WRONLY | O_TRUNC, 0644);

    if (out < 0) {
        WARN("open(%s): %s", dst, strerror(errno));
        close(in);
        return -1;
    }

    char buf[4096];
    ssize_t n;

    while ((n = read(in, buf, sizeof(buf))) > 0) {

        if (write(out, buf, n) != n) {
            WARN("write failed");
            close(in);
            close(out);
            return -1;
        }
    }

    close(in);
    close(out);

    GOOD("Backup created: %s -> %s", src, dst);

    return 0;
}

/* ============================================================
 * restore system
 * ============================================================ */

int restore_file(const char *backup, const char *target)
{
    INFO("Restoring file...");

    return backup_file(backup, target);
}

/* ============================================================
 * sanity checks
 * ============================================================ */

void check_target(const char *path)
{
    struct stat st;

    if (stat(path, &st) < 0) {
        WARN("Target not found: %s", path);
        return;
    }

    INFO("Target exists: %s", path);
    INFO("Size: %ld bytes", st.st_size);
}

/* ============================================================
 * dry-run / safe mode
 * ============================================================ */

void safe_mode(void)
{
    INFO("========== SAFE MODE ==========");

    print_architecture();

    check_modules();

    check_apparmor();

    check_namespaces();

    check_target("/usr/bin/su");

    INFO("No exploitation performed");
}

/* ============================================================
 * corruption primitive placeholder
 * ============================================================ */

int test_write_primitive(const char *path)
{
    INFO("Testing write primitive...");
    INFO("Target: %s", path);

    /*
     * put your primitive here
     *
     * keep this generic:
     * - no shellcode
     * - no ELF overwrite
     * - no PTY spawning
     * - no auto-root shell
     */

    GOOD("Primitive completed");

    return 0;
}

/* ============================================================
 * exploitability checks
 * ============================================================ */

int environment_supported(void)
{
    if (!module_loaded("xfrm_user")) {
        WARN("Missing xfrm_user");
        return 0;
    }

    if (access("/proc/self/ns/user", F_OK) != 0) {
        WARN("User namespaces disabled");
        return 0;
    }

    return 1;
}

/* ============================================================
 * usage
 * ============================================================ */

void usage(const char *prog)
{
    printf("\n");
    printf("dirtyfrag research framework\n\n");

    printf("usage:\n");
    printf("  %s --check\n", prog);
    printf("  %s --backup\n", prog);
    printf("  %s --restore\n", prog);
    printf("  %s --test-write\n", prog);

    printf("\n");
}

/* ============================================================
 * main
 * ============================================================ */

int main(int argc, char **argv)
{
    if (argc < 2) {
        usage(argv[0]);
        return 0;
    }

    /*
     * SAFE MODE
     */

    if (!strcmp(argv[1], "--check")) {

        safe_mode();

        return 0;
    }

    /*
     * BACKUP
     */

    if (!strcmp(argv[1], "--backup")) {

        backup_file("/usr/bin/su", "/tmp/su.bak");

        return 0;
    }

    /*
     * RESTORE
     */

    if (!strcmp(argv[1], "--restore")) {

        restore_file("/tmp/su.bak", "/usr/bin/su");

        return 0;
    }

    /*
     * TEST PRIMITIVE
     */

    if (!strcmp(argv[1], "--test-write")) {

        if (!environment_supported()) {
            WARN("Environment unsupported");
            return 1;
        }

        test_write_primitive("/usr/bin/su");

        return 0;
    }

    usage(argv[0]);

    return 0;
}
