#include <stdio.h>
#include <seccomp.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv, char **envp)
{
    scmp_filter_ctx scmp = seccomp_init(SCMP_ACT_KILL);

    if (!scmp) {
        fprintf(stderr, "Failed to initialize libseccomp\n");
        return -1;
    }

    // Allow all needed syscalls
    seccomp_rule_add(scmp, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(scmp, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
    seccomp_rule_add(scmp, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(scmp, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);

    seccomp_rule_add(scmp, SCMP_ACT_KILL, SCMP_SYS(fork), 0);
    seccomp_rule_add(scmp, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1,
		     SCMP_A2(SCMP_CMP_EQ, O_RDONLY));


    seccomp_rule_add(scmp, SCMP_ACT_ALLOW, SCMP_SYS(dup2), 2, 
                        SCMP_A0(SCMP_CMP_EQ, 1),
                        SCMP_A1(SCMP_CMP_EQ, 2));
    
    // Load in the kernel
    if (seccomp_load(scmp) != 0) {
        fprintf(stderr, "Failed to load the filter in the kernel\n");
        return -1;
    }

    

    printf("before opening in read mode");
    open("Makefile", O_RDONLY);

    //printf("before opening in write mode");
    open("Makefile", O_WRONLY);

    //fork();

    //dup2(1, 2);

    //dup2(1, 42);
    
    printf("Hello world\n");
    return 0;
}

