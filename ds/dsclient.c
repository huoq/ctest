#include <stdio.h>
#include "dsa/include/dsa_export.h"
#include "dsa/include/dsa_incl.h"

void test()
{
	printf("test() called from dsserver!\n");
	DS_PRINTF("test() called from dsserver!\n");
}

void main()
{
	printf("main() started from dsclient!\n");
	dsa_init();
	while(1);
}
