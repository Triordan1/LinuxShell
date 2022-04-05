#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

/*
 * A slightly more complicated shell designed to take linux commands and their arguments
 * now also supports file command arguments
 * Created by Tracy Riordan
 */

#define MAX_BUF 160
#define MAX_TOKS 100
void getTokens(char *toks[],char s[]) {
	char *token;
	const char space[2] = " ";
	int i=0;
	token = strtok(s,space);
	while(token!=NULL) {
 		toks[i] = token;
 		token = strtok(NULL,space);
 		i++;
	}
}
void parseTokens(char *toks[]) {
	if(strcmp(toks[0],"exit")==0) {
		exit(0);
	}
	else if(strcmp(toks[0],"today")==0) {
		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		printf("%0.2d/%0.2d/%d\n",tm.tm_mon+1,tm.tm_mday,tm.tm_year + 1900);
	}
	else if(strcmp(toks[0],"help")==0) {
		printf("enter Linux commands, or 'exit' to exit\n");
	}
	else if(strcmp(toks[0],"cd")==0) {
		int err;
		if(toks[1]==NULL) {
			char *homePath = getenv("HOME");
			chdir(homePath);
		}
		else {
			err = chdir(toks[1]);
			if (err == -1) {
				printf("msh: %s: %s: %s\n", toks[0], toks[1],  strerror(errno));
			}
		}
	} 
	else {
		int i;
		int j=0;
		for (i = 0; i < strlen(toks); i++) {
			if(toks[i]!=NULL) {
				j++;		
			}
		}
		int rc = fork();
		if(rc < 0) {
			fprintf(stderr, "fork failed\n");
			exit(1);
		}
		else if(rc == 0) {
			int k;
			char *myargs[j+1];
			for (k = 0; k < j; k++) {
				myargs[k] = toks[k];
				//printf("%s\n",myargs[k]);
			}
			myargs[j] = NULL;
			execvp(myargs[0],myargs);
			printf("msh: %s: %s\n", toks[0], strerror(errno));
			exit(1);
			
		}
		else {
			int rc_wait = wait(NULL);
			//printf("parent done waiting\n");
		}
	}
	return;

}
int main( int argc, char *argv[]) {
	int ch;
	char *pos;
	char s[MAX_BUF+2];   // 2 extra for the newline and ending '\0'
	static const char prompt[] = "msh> ";
	char *toks[MAX_TOKS];
	while (1) {
		if(argc == 2) {
			//printf("argument supplied\n");
			FILE *fp;
			
			fp = fopen(argv[1],"r");
			while(1) {
				char *status = fgets(s,MAX_BUF+2, (FILE*)fp);
				if(feof(fp)) { 
					break;
				}
				if((pos = strchr(s,'\n')) == NULL) {
					printf("error: input too long\n");
					while ((ch = getchar()) != '\n' && ch != EOF);
					continue;
				}
				*pos = '\0';
				getTokens(toks,s);
				parseTokens(toks);
			}
			fclose(fp);
			break;
		}
		if (isatty(fileno(stdin))) {
			printf(prompt);
		}

		// read input
		char *status = fgets(s, MAX_BUF+2, stdin);

		// exit if ^d entered
		if (status == NULL) {
			printf("\n");
			break;
		}

		// input is too long if last character is not newline 
		if ((pos = strchr(s, '\n')) == NULL) {
			printf("error: input too long\n");
			// clear the input buffer
			while ((ch = getchar()) != '\n' && ch != EOF) ;
			continue; 
		}

		// remove trailing newline
		*pos = '\0';
                getTokens(toks,s); // tokenizes input
                parseTokens(toks); // parses tokenized input and does commands based on input given.
	}
	exit(EXIT_SUCCESS);
}
