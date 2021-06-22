
// JULIO 2020. CÉSAR BORAO MORATINOS: xpaths.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

enum {
	Linesize = 150,
	Maxlist = 1024,
};

struct Cell {
	char path[Linesize];
	int counter;
};

typedef struct Cell Cell;

int
addtolist(char *line, Cell *stack) {

	int i = 0;
	while (strcmp(stack[i].path,"\0") != 0) {
		if (strcmp(stack[i].path,line) == 0) {
			stack[i].counter += 1;
			return 1;
		}
		i++;
	}

	if (i < Maxlist) {
		strncpy(stack[i].path,line,strlen(line)+1);
		stack[i].counter = 1;
		stack[i+1].path[0] = '\0';
		return 1;
	}
	return -1;
}

char *
mostpopular(Cell *stack)
{
	int i = 0, higher = 0, index = 0;
	while (strcmp(stack[i].path,"\0") != 0) {

		if (stack[i].counter > higher) {
			higher = stack[i].counter;
			index = i;
		}
		i++;
	}
	return stack[index].path;
}

void
redirection(char *path)
{
	int fout;
	if ((fout = open(path, O_WRONLY | O_TRUNC)) < 0)
		errx(EXIT_FAILURE,"cannot open the file");

	if (dup2(fout,1) < 0)
		errx(EXIT_FAILURE,"dup2 failed");

	if (close(fout) < 0)
		errx(EXIT_FAILURE,"error: cannot close fd %d", fout);

}

int
readpaths(FILE *file, Cell stack[Maxlist]) {

	char line[Linesize];
	char *fileline;

	if ((fileline = fgets(line,Linesize,file)) == NULL)
		execl("/bin/ps","/bin/ps",NULL);

	while (fileline != NULL) {
		line[strlen(line)-1] = '\0';
		if(access(line, W_OK) < 0)
			return -1;

		if (addtolist(line,stack) < 0)
			errx(EXIT_FAILURE,"error: full stack");

		fileline = fgets(line,Linesize,file);
	}
	return 1;
}

int
readfiles(int argc, char *argv[], Cell stack[Maxlist]) {

	FILE *file;
	for (int i = 0; i < argc; i++) {
		if (access(argv[i], R_OK) < 0)
			return -1;

		if ((file = fopen(argv[i], "r")) == NULL)
			errx(EXIT_FAILURE,"fatal error = cannot open %s file", argv[i]);

		if (readpaths(file,stack) < 1)
			return -1;

		if (fclose(file) < 0)
			errx(EXIT_FAILURE,"error: cannot close");
	}
	return 1;
}

int
main (int argc, char *argv[])
{

	Cell stack[Maxlist];
	stack[0].path[0] = '\0';

	++argv;
	--argc;

	if (argc == 0)
		execl("/bin/ps","/bin/ps",NULL);

	if (readfiles(argc,argv,stack) < 0) {
		if (dup2(2,1) < 0)
			errx(EXIT_FAILURE,"dup2 failed");
		execl("/bin/ps","/bin/ps",NULL);
	}

	if (strcmp(stack[0].path,"\0") != 0)
		redirection(mostpopular(stack));

	execl("/bin/ps","/bin/ps",NULL);
}
