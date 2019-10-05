#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAXLINE 80 /* 80 chars per line, per command, should be enough. */
#define HISTLENGTH 10 //number of the history
char history[HISTLENGTH][MAXLINE];
int hCount = 0;
int isHistory = 0;
int isRR = 0;

void addHistory(char input[]);
void printHistory();
void runHistory(char input[], int length, int *bkgd);

/** The setup() routine reads in the next command line string storing it in the input buffer.
The line is separated into distinct tokens using whitespace as delimiters.  Setup also 
modifies the args parameter so that it holds points to the null-terminated strings which 
are the tokens in the most recent user command line as well as a NULL pointer, indicating the
end of the argument list, which comes after the string pointers that have been assigned to
args. ***/

void setup(char inBuffer[], char *args[],int *bkgd)
{
    int length,  /* #  characters in the command line */
        start,   /* Beginning of next command parameter           */
        i,       /* Index for inBuffer arrray          */
        j,       /* Where to place the next parameter into args[] */
		temp;

    /* Read what the user enters */
    length = read(STDIN_FILENO, inBuffer, MAXLINE);  

    start = -1;
    j = 0;

    if (length == 0)
        exit(0);            /* Cntrl-d was entered, end of user command stream */

    if (length < 0){
        perror("error reading command");
		exit(-1);           /* Terminate with error code of -1 */
    }

    inBuffer[length] = '\0';

    /* Check if the history command is entered*/
    if (strncmp(inBuffer, "history", 7) == 0 || strncmp(inBuffer, "h", 1) == 0){
		
		isHistory = 1;
		return;	
	}else{
		isHistory = 0;
	}

    
	if (!isHistory){
    	addHistory(inBuffer);
    }

    /* This part is used to run the most recent command and run a selected command from the history list*/
    if (inBuffer[0] == 'r'){
		if (inBuffer[1] == 'r'){
			if (hCount == 0){
				printf("No Command can be found in the history");
				return;
			}
			
			strcpy(inBuffer, history[hCount-2]);
			
		}else{
			temp = atoi(&inBuffer[1]);
			if(temp < 1 || temp > hCount || temp <= hCount - HISTLENGTH){
				printf("Number cannot be found");
				return;
			}
			strcpy(inBuffer, history[temp-1]);
		}
		length = strlen(inBuffer);
	}

	

    //printf("%s\n", inBuffer);
    /* Examine every character in the input buffer */
    for (i = 0; i < length; i++) {
 
        switch (inBuffer[i]){
        case ' ':
        case '\t' :          /* Argument separators */

            if(start != -1){
                args[j] = &inBuffer[start];    /* Set up pointer */
                j++;
            }

            inBuffer[i] = '\0'; /* Add a null char; make a C string */
            start = -1;
            break;

        case '\n':             /* Final char examined */
            if (start != -1){
                args[j] = &inBuffer[start];     
                j++;
            }

            inBuffer[i] = '\0';
            args[j] = NULL; /* No more arguments to this command */
            
            break;

        case '&':
            *bkgd = 1;
            inBuffer[i] = '\0';
            break;
            
        default :             /* Some other character */
            if (start == -1)
                start = i;
		}
    }

        
    args[j] = NULL; /* Just in case the input line was > 80 */
} 

/**
Add history to array
**/
void addHistory(char input[]){
	if (hCount >= 10){
		for (int i = 0; i < HISTLENGTH-1; i++){
			strcpy(history[i],history[i+1]);

		}
		strcpy(history[HISTLENGTH-1], input);
	}else{
		strcpy(history[hCount], input);
		hCount++;
	}
}


/**
Print history on command line
**/
void printHistory(){
	printf("Most Recent Entered Command:\n");
	for (int i = 0; i < HISTLENGTH; i++){
		printf("%i:\t", i+1);
		printf("%s\n", history[i]);
	}
}

int main(void)
{
    char inBuffer[MAXLINE]; /* Input buffer  to hold the command entered */
    char *args[MAXLINE/2+1];/* Command line arguments */
    int bkgd;             /* Equals 1 if a command is followed by '&', else 0 */
    

    while (1){            /* Program terminates normally inside setup */

		bkgd = 0;

		printf("MyShell: ");  /* Shell prompt */
        fflush(0);



        setup(inBuffer, args, &bkgd);       /* Get next command */

         
	 	/*Fork a child process using fork()*/
		pid_t child;
		child = fork();	
		int status;

	 	//The child process will invoke execvp(),
		if (child < 0){
			fprintf(stderr, "Fork Failed");
			exit(-1);
		}	
		else if (child == 0){
			if (isHistory){
    			printHistory();
    			return 0;
    		}
    		
			status = execvp(inBuffer, args);
			if (status != 0){
				printf("%s: command not found. \n", inBuffer);
				return 0;
			}
			
		}
		else{
			//If bkgd == 0, the parent will wait, o/w returns to the setup() function.
			if (bkgd == 0){
				waitpid(child, &bkgd, 0);
				
			}
		}

    }
}

