#include "c2.h"

char *mystrndup( char *src, long maxlen)
{
    if( strlen( src ) > maxlen )
    {
        char *retbuf;
        if( (retbuf = malloc( 1 + maxlen )) != NULL )
        {
            memcpy( retbuf, src, maxlen );
            retbuf[maxlen] = '\0';
        }
        return retbuf;
    }
    return strdup( src );
}

char *new_label() {
    static int c = 0;
    char buf[20];
    sprintf(buf, "$LNdata%d", c++);
    return mystrndup(buf, 20);
}
