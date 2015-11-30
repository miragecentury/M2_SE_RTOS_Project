#include <stdio.h>

int _write(int file, char *ptr, int len)
{
	int i;

	if(file == 1)
	{
		for(i = 0; i < len; i++)
		fputc(ptr[i], stdout);
	}
	else if(file == 2)
	{
		for(i = 0; i < len; i++)
		fputc(ptr[i], stderr);
	}

	return len;
}
