#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

//path variable
char *path = NULL;

//Error Log Display Function
void errorLog(){
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
}

/*
Function to tokenize command read using getline
return string array containing the tokenized commands/arguments
*/
char **tokenizing_cmd(char *ln, char *delim){
	//char *token1 = NULL;
	char *token = NULL;

	//taking a copy of line to use : for finding number of commands in read line
	char *ln_copy = NULL; 
	ln_copy = strdup(ln);


	//finding the number of tokens in the read command line
	token = strtok(ln_copy,delim);
	int tokenCount = 1;
	while(token != NULL) {
		token = strtok(NULL,delim);
		tokenCount++;
	}
	
	char **list_cmd = (char **)malloc(tokenCount*sizeof(char*));
	
	//taking copy for tokenizing
	char *ln_copy1 = NULL;
	ln_copy1 = strdup(ln);

	token = strtok(ln_copy1,delim);
	int i = 0;
	
	while(token!=NULL){
		list_cmd[i] = token;
		token = strtok(NULL,delim);
		i++;
	}
	list_cmd[i] = NULL;
	return list_cmd;
}

//Function to check if the given list of strings have any character other than blank space
int checkIfListEmpty(char **chk_list) {
	int i = 0;
	int emptyFlag = 1;
	char **emptyList = NULL;
	while(chk_list[i] != NULL) {
		emptyList = tokenizing_cmd(chk_list[i]," ");
		if(emptyList[0] != NULL) {
			emptyFlag = 0;
			break;
		}
		i++;
	}
	return emptyFlag;
}

/*
Function to check if there is a path in Path list with the command
returns filepath(path/cmd) if there is proper path for the command from the list
returns NULL if proper path is not found
*/
char *checkPath(char *cmd) {
	char *filepath = NULL;
	char **list_path = NULL;
	//tokenizing path variable
	char *path_copy = strdup(path);
	list_path = tokenizing_cmd(path_copy, " \t\r\n");
	
	int i=0;
	while(list_path[i] != NULL) {
		filepath = malloc((sizeof(list_path[i]) + sizeof(cmd) + 1)*sizeof(char*));
		strcpy(filepath, list_path[i]);
		strcat(filepath,"/");
		strcat(filepath, cmd);
		//checking if the command exists in the path
		if(access(filepath,F_OK)==0) {
			return filepath;	
		}
		i++;
	}
	return filepath;
}

/*
Function to execute the commands
First check if the command is one of the built in commands if so it is executed
Secondly if the command is not built-in command execution is done using fork and execv
*/
int commandExecution(char **list_cmd, char **outName) {
	char *cmdpath = NULL;


	
	//buit-in exit command
	if(strcmp(list_cmd[0],"exit")==0) {
		
		//error : if argument is passed to exit
		if(list_cmd[1]!=NULL) {
			errorLog();	
		}
		else {
			exit(0);	
		}
	}

	//built-in cd command
	else if(strcmp(list_cmd[0],"cd")==0) {
		
		//error if 0 or more than 1 arguments passed
		if((list_cmd[1]==NULL) || (list_cmd[2]!=NULL)) {
			errorLog();	
		}

		//calling chdir
		//error : if chdir returns a value other than 0
		else if(chdir(list_cmd[1])!=0) {
			errorLog();	
		}
	
	}
	
	//built-in path command
	else if(strcmp(list_cmd[0],"path")==0) {
	
		//empty the existing path
		strcpy(path,"");
		
		//finding the length of path
		int token_l = 1;
		int tk_size = 1;
		while(list_cmd[token_l] != NULL) {
			tk_size += (strlen(list_cmd[token_l])+1);
			token_l++;
		}
		//checking if more memory needs to be allocated to PATH variable
		if( tk_size  > sizeof(path)) {
			path = realloc(path, tk_size*sizeof(char));	
		}			

		int i = 1;
		//adding user path arguments
		if (list_cmd[i] != NULL)
			strcpy(path, list_cmd[i++]);

		while(list_cmd[i]!=NULL) {	
			strcat(path," ");
			strcat(path,list_cmd[i++]);
		}

	
	}

	//system calls
	else {
		//check for the proper path of the command
		cmdpath = checkPath(list_cmd[0]);
		
		//calling fork to create child process
		int id = fork();
	
		//inside child process
		if(id==0) {
			
			//if rediction is not specified
			if(outName==NULL) {
				execv(cmdpath, list_cmd);
				//if exec return : error
				errorLog();
				exit(1);
			}

			//if redirection is specified
			else {
				//opening the output file
				int newFD;
				if((newFD =open(outName[0], O_CREAT|O_RDWR|O_TRUNC, S_IRWXU))<0) {
					errorLog();
					exit(1);
				}
				//redirecting output
				dup2(newFD, 1);
				//redirecting error
				dup2(newFD, 2);

				//calling exec
				execv(cmdpath, list_cmd);
				//if exec returns : Error
				exit(1);
			}

		}
		
	}

	return 0;
}

//Function to check for parallel commands and redirection 
void para_redir_cmd(char *line) {
	char **outName = NULL;
	char **redir_tokens = NULL;
	char **list_cmd = NULL;
	int cmdNum  = 0;
	int wpid, status;

	//tokenizing at delimiter & to find out if parallel commands exists
	char **para_tokens = NULL; 
	para_tokens = tokenizing_cmd(line, "&\n");
	
	
	//check for solo parallel command symbol with no commands
	if((strchr(line,'&') !=NULL) && ((para_tokens[0] == NULL) || (checkIfListEmpty(para_tokens) == 1)))
		errorLog();
	
	//loop through commands
	//executed once if no parallel commands
	while(para_tokens[cmdNum] != NULL) {
		
		//check for redirection
		redir_tokens = tokenizing_cmd(para_tokens[cmdNum],">\n");
		
		if((strchr(para_tokens[cmdNum],'>') != NULL) && ((redir_tokens[0] == NULL) || (redir_tokens[1] == NULL)))
			errorLog();

		//No redirection specified
		else if(redir_tokens[1]==NULL) {
			
			list_cmd = tokenizing_cmd(redir_tokens[0], " \t\r\n");
			if(list_cmd[0] != NULL)
				commandExecution(list_cmd, outName);
	
		}

		//Mulitple redirection inputs : Error
		else if(redir_tokens[2]!=NULL) {
			errorLog();
			break;
		}
		
		//if redirection is specified
		else if(redir_tokens[1]!=NULL) {
			
			//tokenizing cmds
			list_cmd = tokenizing_cmd(redir_tokens[0], " \t\r\n");
			
			
			//retreiving redirection filename
			outName = tokenizing_cmd(redir_tokens[1], " \t\r\n");
			//checking if multiple names are specified : Error
			if(outName[1]!=NULL) {
				errorLog();
				break;
			}
			
			//calling execution of the command
			if(list_cmd[0] != NULL)
				commandExecution(list_cmd, outName);

		}
		cmdNum++;
	}

	//wait for all children to finish
	while((wpid=wait(&status))>0);
	
}

//Function handles the interactive mode of dash
int interactive_mode() {
	char *line = NULL;
	size_t len = 0;
	ssize_t ln_read = 0;
	
	while (1){
		
		//shell prompt
		printf("dash> ");

		//reading the command
		ln_read = getline(&line, &len, stdin);
	
		//error handling
		if (ln_read==-1){
			errorLog();
		}

		para_redir_cmd(line);
	}
				
	
	return 0;
}

/*
Function handles the batch mode of dash
returns 1 if bad batch file is inputed
*/
int batch_mode(char *filename) {

	FILE *fileptr;
	char *line = NULL;
	size_t len = 0;
	ssize_t ln_read = 0;
	//file opened in read mode
	fileptr = fopen(filename,"r");

	//error if file ptr returns NULL
	if(fileptr != NULL) {
		
		//reading
		while((ln_read = getline(&line,&len,fileptr)) != -1) {
			
			para_redir_cmd(line);
			
		} 
	
	}
	else {
	
		errorLog();
		return 1;
	}
	
	fclose(fileptr);
	return 0;

}

/*
main function
initial path list is added to the path variable
checks the argument number to see if the dash is supposed to work in interactive or batch mode
*/
int main(int argc, char *argv[]) {
	
	path = strdup("/bin /usr/bin");

	//check if the call is interactive
	if(argc==1){
		interactive_mode();	
	}

	//check if the call is batch
	else if(argc==2) {
		batch_mode(argv[1]);
	}

	//error if called with more than 1 argument
	else if(argc>2) {
		errorLog();	
	}
	return 0;

}
