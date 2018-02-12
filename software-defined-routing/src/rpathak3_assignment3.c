/**
 * @swetankk_assignment3
 * @author  Swetank Kumar Saha <swetankk@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Read the CONTROL_PORT and start
 * the connection manager.
 */


/** REFERENCES:

1) For Linux Kernel Linked List operations I have consulted the FreeBSD - https://www.freebsd.org/cgi/man.cgi?apropos=0&sektion=3&query=LIST_INSERT_BEFORE&manpath=FreeBSD+7.0-current&format=html
2) For packing I have used code from Beej - https://beej.us/guide/bgnet/examples/pack2.c
3) For bit manipulations I have used the following - https://graphics.stanford.edu/~seander/bithacks.html
4) Got idea of timerfd from Stack Overflow - https://stackoverflow.com/questions/2328127/select-able-timers
5) check_msb() - http://codeforwin.org/2016/01/c-program-to-check-most-significant-bit-of-number-is-set-or-not.html
6) getIP() - https://stackoverflow.com/questions/1680365/integer-to-ip-address-c

*/

#include "../include/allinclude.h"

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
    /*Start Here*/

    sscanf(argv[1], "%" SCNu16, &CONTROL_PORT);
    init_globals();
    init(); // Initialize connection manager; This will block

    return 0;
}
