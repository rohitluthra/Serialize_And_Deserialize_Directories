#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

/*
Following comments are for understanding:

argc: contianes the number of arguments coming in.
argv: is a array which contains the string value of the arguments typed through command line.

*/

int main(int argc, char **argv)
{
    int ret = validargs(argc, argv);
    //debug("%d", ret);

    //debug("%d", 2);

    if(ret == -1)
    {
        USAGE(*argv, EXIT_FAILURE);
    }

    if(global_options == 1)
    {
        //debug("eee");
        USAGE(*argv, EXIT_SUCCESS);
    }

    else if(global_options == 2)
    {
        // if i get 0 -1 ........ take care of such issuew.
        int returned = serialize();
        if(returned == 0)
        {
            exit(EXIT_SUCCESS);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }

    // represents : 0100 (wo c) and 1100(with c)
    else if (global_options == 4 || global_options == 12)
    {
       // debug("i am in deserialize: %d", global_options);
        int returned = deserialize();
        //debug("%d", returned);
        if(returned == 0)
        {
            exit(EXIT_SUCCESS);
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else
        USAGE(*argv, EXIT_FAILURE);

}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
