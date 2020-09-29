#include <stdlib.h>
#include <stdio.h>

main()
{
		char *c = (char *)malloc(10);
		free(c);
		return(0);		
}
