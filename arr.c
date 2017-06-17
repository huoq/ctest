#include <stdio.h>

void main()
{
	int a[10]={0,1};
	int i = 0;
	printf("%d %d\n",a[i++],a[i]);
	printf("%d %d\n",a[0],a[1]);
}
