#include <stdio.h>
#include <stdlib.h>

int main( int argc, char *argv[] ) {

    unsigned long val;
    unsigned int a,b,c;

//    if( argc < 2 ) exit(1);
//    val = strtoul( argv[1], NULL, 16 );
//    printf( "val = 0x%lx \n", val );

    a=2; b=0;
    c = a & b + 1;
    printf( "a & b + 1 = %u \n", c );
    c = (a & b) + 1;
    printf( "(a & b) + 1 = %u \n", c );


    exit(0);
}