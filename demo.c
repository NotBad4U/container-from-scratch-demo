#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <seccomp.h>
#include <libcgroup.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
			} while (0)

#define CGROUP_DIR "/sys/fs/cgroup/"

char *const child_cmd[] = {
    "/bin/bash",
    NULL
};

//int network_sync_veth[2];

static void print_pids();
static void print_nodename();
static void setup_network_veth_child();
static void setup_network_veth_host(pid_t child_pid);
static void change_rootfs_by_chroot();
static void init_seccomp();
static void setup_cgroup(pid_t child_pid);

static int childFunc(void *arg)
{
    //char sync_veth;
    //close(network_sync_veth[1]);

    sethostname(arg, strlen(arg));

    print_pids();
    print_nodename();

    //wait for network veth setup in parent
    //setup_network_veth_child();
    //read(network_sync_veth[0], &sync_veth, 1);

    change_rootfs_by_chroot();

    //Needed by commands like ps & top
    mount("proc", "/proc", "proc", 0, NULL);

    //init_seccomp();
    
    execvp(child_cmd[0], child_cmd);
    return 0;
}




#define STACK_SIZE (1024 * 1024)

static char child_stack[STACK_SIZE];

int main(int argc, char *argv[])
{
    int child_pid;
    //pipe(network_sync_veth);

    child_pid = clone(childFunc,
		      child_stack + STACK_SIZE,
		      CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC |
              CLONE_NEWNS | CLONE_NEWNET | SIGCHLD,
		      argv[1]
		    );

   /*  if (child_pid == -1)
	errExit("clone"); */

    printf("PID of child created by clone() is %ld\n", (long) child_pid);

    //setup_network_veth_host(child_pid);
    //close(network_sync_veth[1]); // send "done" setup veth
    
    setup_cgroup(child_pid);

    sleep(1);

    waitpid(child_pid, NULL, 0);
    printf("child has terminated\n");
    exit(EXIT_SUCCESS);
}

static void print_pids()
{
    printf("PID = %ld\n", (long) getpid());
    printf("PPID = %ld\n", (long) getppid());
}

static void print_nodename()
{
    struct utsname uts;

    if (uname(&uts) == -1)
	    errExit("uname");

    printf("uts.nodename in child:  %s\n", uts.nodename);
}

static void change_rootfs_by_chroot()
{
    chroot("rootfs");
    chdir("/");
}


static void setup_cgroup(pid_t child_pid)
{
    mkdir("/sys/fs/cgroup/memory/demo_cg", 0755);

   FILE *memory_fd, *cgroup_procs_fd;

   sleep(2);

    if ((memory_fd = fopen("/sys/fs/cgroup/memory/demo_cg/memory.limit_in_bytes","w")) == NULL) {
       fprintf(stderr,"error opening memory.limit_in_bytes\n");
       strerror(errno);
       exit(1);
    }

    fprintf(memory_fd, "%d", 300000);
    fclose(memory_fd);

    if ((cgroup_procs_fd = fopen("/sys/fs/cgroup/memory/demo_cg/cgroup.procs", "w")) == NULL) {
       fprintf(stderr,"error opening cgroup.procs\n");
       exit(1);
    }

    fprintf(cgroup_procs_fd, "%d", (int) child_pid);
    fclose(cgroup_procs_fd);
}


static void setup_network_veth_child()
{
    system("ip link set lo up");
    system("ip link set veth1 up");
    system("ip addr add 169.254.1.2/30 dev veth1");
}

static void setup_network_veth_host(int child_pid)
{
    char *cmd;
    asprintf(&cmd, "ip link set veth1 netns %d", child_pid);
    system("ip link add veth0 type veth peer name veth1");
    system(cmd);
    system("ip link set veth0 up");
    system("ip addr add 169.254.1.1/30 dev veth0");
    free(cmd);
}

static void init_seccomp()
{
    scmp_filter_ctx scmp = seccomp_init(SCMP_ACT_ALLOW);

    if (!scmp) {
        fprintf(stderr, "Failed to initialize libseccomp\n");
        exit(EXIT_FAILURE);
    }

    seccomp_rule_add(scmp, SCMP_ACT_KILL, SCMP_SYS(fork), 0);

    if (seccomp_load(scmp) != 0) {
        fprintf(stderr, "Failed to load the filter in the kernel\n");
        exit(EXIT_FAILURE);
    }
}
