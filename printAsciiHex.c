
#include <stdio.h>
#include "printAsciiHex.h"

void printAsciiHex(char* buffer, int nread)
{
	int i,j;
	for(i = 1; i <= nread; i++)
	{
		printf("%02x", buffer[i - 1] & (0xFF));
		if(i%2==0 && i!=0)
			printf(" ");
		if(i%COLUMN_SIZE == 0)
		{
			putchar('\t');
			for(j=i-COLUMN_SIZE; j<i; j++)
			{
				if(buffer[j] >= 32 && buffer[j] <= 126)
					putchar(buffer[j]);
				else
					putchar('.');
			}
			putchar('\n');
		}
		else if(i==nread)
		{

			for(j=0; j<(COLUMN_SIZE-(nread%COLUMN_SIZE))*3; j++)
				putchar(' ');
			putchar('\t');
			for(j=(nread/COLUMN_SIZE)*COLUMN_SIZE; j<nread; j++)
			{
				if(buffer[j] >= 32 && buffer[j] <= 126)
					putchar(buffer[j]);
				else
					putchar('.');
			}
			putchar('\n');
		}
	}
	putchar('\n');
}
