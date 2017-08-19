/*
* This file is part of mipOS
* Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
* All rights reserved.
* Licensed under the MIT License.
* See COPYING file in the project root for full license information.
*/


/* --------------------------------------------------------------------------- */

#include "mipos_kernel.h"
#include "mipos_bsp.h"
#include <stdint.h>


/* --------------------------------------------------------------------------- */

#ifdef WIN32
#include <conio.h>
#endif

/* --------------------------------------------------------------------------- */

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif


/* --------------------------------------------------------------------------- */

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif


/* --------------------------------------------------------------------------- */

#if defined(ENVIRONMENT64)
#warning platform or compiler not supported
#else // ENVIRONMENT32
#if (defined(_MSC_VER) || defined(__BORLANDC__))
unsigned int mipos_get_sp() {
    unsigned int stack_ptr;

    __asm { mov eax, esp }
    __asm { mov[stack_ptr], eax }

    return stack_ptr;
}
#elif defined (__GNUC__) 
unsigned int mipos_get_sp() {
    unsigned int stack_ptr;
    __asm__ __volatile__("movl %%esp, %0\n\t"
        : "=&r"(stack_ptr) /* output */
        : 
        );
    return stack_ptr;
}
#else
#warning platform or compiler not supported
#endif
#endif


/* --------------------------------------------------------------------------- */

#if _WIN32 || _WIN64
#include <conio.h>
#else
#include <sys/select.h>
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
 
static int _kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
}

static int _getch(void) {
    int ch = getchar();

    switch (ch) {
       case 0xA:
          return 0xD;
       case 0x7f:
          return '\b';
    }

    return ch;
}

#endif


/* --------------------------------------------------------------------------- */

unsigned char mipos_console_get_char(void)
{
    fflush(stdout);

    while (!_kbhit()) {
        mipos_tm_wkafter(0);
    }

    char ch = _getch();

    return ch;
}


/* --------------------------------------------------------------------------- */

void mipos_bsp_rs232_putc(unsigned char c) {
    putc(c, stdout);
}


/* --------------------------------------------------------------------------- */

void mipos_bsp_configure_rs232(unsigned int baud_rate) {
}


/* --------------------------------------------------------------------------- */

unsigned int mipos_bsp_rs232_recv_byte(unsigned char *key) {
    return 0;
}


/* --------------------------------------------------------------------------- */

