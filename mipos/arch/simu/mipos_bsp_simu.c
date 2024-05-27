/*
 * This file is part of mipOS
 * Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
 * All rights reserved.
 * Licensed under the MIT License.
 * See COPYING file in the project root for full license information.
 */


/* ---------------------------------------------------------------------------
 */

#include "mipos_bsp.h"
#include "mipos_kernel.h"
#include <stdint.h>


/* ---------------------------------------------------------------------------
 */

#ifdef WIN32
#include <conio.h>
#endif

/* ---------------------------------------------------------------------------
 */

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define MIPOS64
#else
#define MIPOS32
#endif
#endif


/* ---------------------------------------------------------------------------
 */

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define MIPOS64
#else
#define MIPOS32
#endif
#endif


/* ---------------------------------------------------------------------------
 */

#if defined(MIPOS64)
// functions defined as static inline in the .h file
#else // MIPOS32
#if (defined(_MSC_VER) || defined(__BORLANDC__))
unsigned int mipos_get_sp()
{
    unsigned int stack_ptr;

    __asm { mov eax, esp }
    __asm
    {
        mov[stack_ptr], eax
    }

    return stack_ptr;
}
#elif defined(__GNUC__)
unsigned int mipos_get_sp()
{
    unsigned int stack_ptr;
    __asm__ __volatile__("movl %%esp, %0\n\t"
                         : "=&r"(stack_ptr) /* output */
                         :);
    return stack_ptr;
}
#else
#warning platform or compiler not supported
#endif
#endif


/* ---------------------------------------------------------------------------
 */

#if _WIN32 || _WIN64
#include <conio.h>
static int _peek_input(void)
{
    if (_kbhit()) {
        return _getch();
    }

    return 0;
}
#else
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>

static int _peek_input(void)
{
    static int termios_cnf = 0;
    static struct termios oldt, newt = { 0 };
    int ch = 0, oldf = 0;

    if (!termios_cnf) {
        termios_cnf = 1;

        oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

        tcgetattr(STDIN_FILENO, &oldt);
        memcpy(&newt, &oldt, sizeof(struct termios));

        newt.c_lflag &= ~(ICANON | ECHO);

        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }

    ch = getchar();

    if (ch == EOF) {
        return 0;
    }

    switch (ch) {
        case 0xA:
            return 0xD;
        case 0x7f:
        case 0x27:
            return '\b';
        case 3: // CTRL+C
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            fcntl(STDIN_FILENO, F_SETFL, oldf);
            exit(0);
            return 0;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    termios_cnf = 0;

    return ch;
}

#endif


/* ---------------------------------------------------------------------------
 */

void mipos_bsp_rs232_putc(unsigned char c)
{
    putc(c, stdout);
    fflush(stdout);
}


/* ---------------------------------------------------------------------------
 */

void mipos_bsp_configure_rs232(unsigned int baud_rate) {}


/* ---------------------------------------------------------------------------
 */

unsigned int mipos_bsp_rs232_recv_byte(unsigned char* key)
{
    char ch = _peek_input();

    if (ch) {
        *key = ch;
        return 1;
    }

    return 0;
}


/* ---------------------------------------------------------------------------
 */
