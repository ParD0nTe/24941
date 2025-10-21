#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/resource.h>

// #define int long long
#define ll long long
#define elif else if
#define forn(i, n) for (int i=0;i<n;i++)

extern char **environ;

void* swap(void* a, void* b, int size) {
    void* t = (void*) malloc(size);
    memcpy(t, a, size);
    memcpy(a, b, size);
    memcpy(b, t, size);
    free(t);
}

void print_cwd() {
	char cwd[1024]; // Buffer to store the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s\n", cwd);
	else
		fprintf(stderr, "getcwd() Error\n");
}

void print_ulimit() {
	struct rlimit rlim;
    
    if (getrlimit(RLIMIT_FSIZE, &rlim) == 0) {
        printf("ulimit fsize: %ld\n", (long)rlim.rlim_cur);
    } else {
        perror("getrlimit failed");
		return;
    }
    if (getrlimit(RLIMIT_DATA, &rlim) == 0) {
        printf("ulimit data: %ld\n", (long)rlim.rlim_cur);
    }
    if (getrlimit(RLIMIT_STACK, &rlim) == 0) {
        printf("ulimit stack: %ld\n", (long)rlim.rlim_cur);
    }
    if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        printf("ulimit core: %ld\n", (long)rlim.rlim_cur);
    }
    if (getrlimit(RLIMIT_NOFILE, &rlim) == 0) {
        printf("ulimit nofile: %ld\n", (long)rlim.rlim_cur);
    }
}

void set_ulimit(char *value) {
    struct rlimit rl;
    long new_limit;
    if (getrlimit(RLIMIT_FSIZE, &rl) != 0) {
		perror("Get ulimits failed\n");
		return;
	}
    if (strcmp(value, "unlimited") == 0) {
		rl.rlim_max = RLIM_INFINITY;
        rl.rlim_cur = RLIM_INFINITY;
    } else {
        new_limit = atol(value);
        if (new_limit < 0) {
            fprintf(stderr, "Wrong value of ulimit: %s\n", value);
            return;
        }
		rl.rlim_max = new_limit;
        rl.rlim_cur = new_limit;
    }
    if (setrlimit(RLIMIT_FSIZE, &rl) == -1) {
        perror("Error with setting new ulimit\n");
        return;
    }
    
    printf("New ulimit = %ld\n", rl.rlim_cur);
}

void print_core_size() {
	struct rlimit rlim;
	if (getrlimit(RLIMIT_CORE, &rlim) == 0) {
        printf("Core file size: %ld\n", (long)rlim.rlim_cur);
    }
	else {
		perror("Get core size failed\n");
	}
}

void set_core_size(char* nsize) {
	struct rlimit rl;
    long size;
    
    if (strcmp(nsize, "unlimited") == 0) {
        rl.rlim_cur = RLIM_INFINITY;
        rl.rlim_max = RLIM_INFINITY;
    }
	size = atol(nsize);
	printf("size = %ld, nsize = %s\n", size, nsize);
	if (size < 0) {
		perror("Wrong new core size, ignored\n");
		return;
	}
	if (getrlimit(RLIMIT_CORE, &rl) == 0) {
		rl.rlim_cur = size;
		rl.rlim_max = size;
		setrlimit(RLIMIT_CORE, &rl);
		printf("New core size set to %ld\n", size);    
	}
	else {
		perror("Set core size failed\n");
	}

}

void print_env() {
	char **env = environ;
	printf("Env vars:\n");
	while (*env != NULL) {
		printf("%s\n", *env);
		env++;
	}
}

void set_env(const char *assignment) {
    char *name = strdup(assignment);
    char *value = strchr(name, '=');
    if (value == NULL) {
        perror("Wrong new env var format. Use: name=value\n");
        free(name);
		return;
    }
    *value = '\0';
    value++;
    printf("Value = %s\n", value);
    if (setenv(name, value, 1) == -1) {
        perror("Error while setting new env var");
        free(name);
        return;
    }
    
    printf("Env var set: %s=%s\n", name, value);
    free(name);
}

void print_ids() {
	printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID: %d\n", getgid());
    printf("Effective GID: %d\n", getegid());
}

void make_group_leader(){
	if (setpgid(0, 0) == 0) 
		printf("Become group leader successful. Now: %d\n", getpgrp());
	else
		perror("Failed to become group leader\n");
}

void print_pids() {
    printf("Process ID (PID): %d\n", getpid());
    printf("Parent Process ID (PPID): %d\n", getppid());
    printf("Process Group ID (PGID): %d\n", getpgrp());
}

signed main(int argc, char* argv[]){
	opterr = 0;
    char options[ ] = "dcuC:vV:U:ips";
	int c;
	while ((c = getopt(argc, argv, options)) != EOF) {    
		switch (c) {
			case 'd':
				print_cwd();
				break;
			case 'c':
				print_core_size();
				break;
			case 'u':
				print_ulimit();
				break;
			case 'U':
				char* nlimit = optarg;
				set_ulimit(nlimit);
				break;
			case 'C':
				char* nsize = optarg;
				set_core_size(nsize);
				break;
			case 'v':
				print_env();
				break;
			case 'V':
				set_env(optarg);
				break;
			case 'i':
				print_ids();
				break;
			case 'p':
				print_pids();
				break;
			case 's':
				make_group_leader();
				break;
			default:
				break;
		}	
	}	
	return 0;
}