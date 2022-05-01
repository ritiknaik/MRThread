#include <stdio.h>
#include <setjmp.h>

//function to encrypt the values before assigning to jmpbuf
//Reference link: "https://sites.cs.ucsb.edu/~chris/teaching/cs170/projects/proj2.html"
long int mangle(long int p){
        long int ret;
        asm(" mov %1, %%rax;\n"
            " xor %%fs:0x30, %%rax;"
            " rol $0x11, %%rax;"
            " mov %%rax, %0;"
        : "=r"(ret)
        : "r"(p)
        : "%rax"
        );
        return ret;
}