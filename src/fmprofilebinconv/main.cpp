
#include <stdio.h>

#if defined(_MSC_VER)
#include "../wingetopt/wingetopt.h"
#else
#include <unistd.h>
#endif

#include "../include/oasis.h"

#if !defined(_MSC_VER) && !defined(__MINGW32__)
#include <signal.h>
#include <string.h>
#endif

namespace fmprofilebinconv {
    void doit();
}

char *progname;

#if !defined(_MSC_VER) && !defined(__MINGW32__)
void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
    fprintf(stderr, "%s: Segment fault at address: %p\n", progname,
            si->si_addr);
    exit(0);
}
#endif

void help() { fprintf(stderr, "-h help\n-v version\n"); }

int main(int argc, char *argv[]) {

    progname = argv[0];
    int opt;

    while ((opt = getopt(argc, argv, "vh")) != -1) {
        switch (opt) {
        case 'v':
            fprintf(stderr, "%s : version: %s\n", argv[0], VERSION);
            exit(EXIT_FAILURE);
            break;
        case 'h':
        default:
            help();
            exit(EXIT_FAILURE);
        }
    }

#if !defined(_MSC_VER) && !defined(__MINGW32__)
    struct sigaction sa;

    memset(&sa, 0, sizeof(struct sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = segfault_sigaction;
    sa.sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, &sa, NULL);
#endif

    try {
        initstreams();
        fmprofilebinconv::doit();
    } catch (std::bad_alloc) {
        fprintf(stderr, "%s: Memory allocation failed\n", progname);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
