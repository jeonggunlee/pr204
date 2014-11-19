#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char const *argv[])
{
	char c;
	int count = 0;
	FILE * file = fopen("machine_file", "r");

	if (file)
	{
		do
		{
			c = getc(file);
			if (c == '\n')
				count++;
		}
		while (c != EOF);
	}

	printf("%i\n", count);

	fclose(file);

	return 0;
}