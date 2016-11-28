/* Zakery Fyke
CS 4352 -- 01: Operating Systems
Project 2

This implements the first 3 parts of 2nd project for Operating Systems. It emulates some of 
the functions that are found in the find function built into UNIX/Linux. 

Compile as: gcc -o proj2-two project2-two.c

Input: A directory, given after a -w flag, criteria, given after either an -i, -m, or -n flag, and an action, given
by the existance of a -d flag. 
Example: ./find -w testdir -n test3 -d will delete all files named "test3" in testdir and its subdirectories.
./find -mmin -10 will list all file paths in the current working directory and its subdirectories to files which have
been modified in the last 10 minutes. 
Note: This function will only consider one criteria for search, e.g. ./find -w testdir -n test3 -mmin -10 will search for a file named test3 in testdir and its subdirectories, but will not consider the modification time. 


Output: If Action is not given: The file path to all files which match the given criteria within the file path of the given directory or its subdirectories. 
If Action is given: All files matching criteria will be removed from the given directory and all its sub directories.
A message will also print confirming which file has been deleted. 
Note: project2-two.c cannot be removed by itself, an ignore message will print.

If no directory is specified, then the current working directory will be used. If no criteria is specified, all file paths will be listed or deleted.
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

void get_output(char * file_path, struct dirent* sub_dirp, int delete){
	if(delete == 0){ //We don't want to delete anything
		printf("%s \n", file_path);
	}else{ //We are to delete this. 
		if(sub_dirp != NULL && strcmp(sub_dirp -> d_name, "project2-two.c") != 0){ //Taken as a precaution so this file doesn't get deleted (again)
			remove(file_path);
			printf("%s has been removed. \n", file_path);
		}else{
			printf("Ignoring %s. \n", file_path);
		}
	}
}

void time_handler(char * file_path, struct dirent* sub_dirp, int delete, char *crit, char * criteria){
	long val;
	val = atoi(crit); //This is the integer value of the criteria passed, e.g. 10 instead of +10 or -10
	struct stat buf; //define a file status structure
	stat(file_path, &buf);
	
	time_t modified_time = buf.st_mtime;
	time_t current_time = time(NULL); //Current time in seconds since the 1970 epoch
	long elapsed_time = difftime(current_time, modified_time)/60; //get the time since the file was modified in minutes
	
	
	if(strchr(criteria, '+') != NULL){ //If criteria begins with '+', print all paths of files which were modified more than criteria minutes ago
		if(elapsed_time > val){
			get_output(file_path, sub_dirp, delete);
		}
	}else if(strchr(criteria,'-')!= NULL){ //If criteria begins with '-', print all paths of files which were modified less than criteria minutes ago
		if(elapsed_time < val){
			get_output(file_path, sub_dirp, delete);
		}
	}else{
		if(elapsed_time == atoi(criteria)){ //otherwise rpint all paths of files which were modified exactly criteria minutes ago
			get_output(file_path, sub_dirp, delete);
		}
	}
}

void find(char * sub_dir, char * criteria, char * field, int delete){

	DIR *sub_dp = opendir(sub_dir);//open a directory stream
	struct dirent * sub_dirp;//define
	char temp1[PATH_MAX]=".";
	char temp2[PATH_MAX]="..";
	char temp3[PATH_MAX]="/";
	char *temp_sub;
	char inode[16];
	long val;
	char *crit= criteria; //Used in the case of time based search
	crit++; //Drop the first character in criteria (+ or -)
	
	
	if(sub_dp!=NULL){ //Check if the directory opened successfully 

		while((sub_dirp=readdir(sub_dp))!=NULL){ //until we've read every entry one by one
					
			char * temp = sub_dirp -> d_name; //get the name
			 
			if(strcmp(temp, temp1)!=0 && strcmp(temp, temp2)!=0){ //Ignores . and .. in the directory
				
				char *temp_sub = malloc(sizeof(char)*2000);
				temp_sub = strcpy(temp_sub, temp3); // Adds '/' before the name of the entry
				temp_sub = strcat(temp_sub, temp); 

				
				//now you can add the '/' in front of the entry's name
				char* temp_full_path=malloc(sizeof(char)*2000); //Create a variable to hold the full path
				
				//Place the passed directory at the front of the path and add the name of the file to the end
				temp_full_path=strcpy(temp_full_path,sub_dir); 
				temp_full_path=strcat(temp_full_path,temp_sub);
			
				//try to open the file path we just created
				DIR * subsubdp = opendir(temp_full_path);

				//if not null, we've found a subdirectory, otherwise it's a file
				if(subsubdp!=NULL){
					closedir(subsubdp); //Close stream, it'll be reopened in the recusive call. 
					find(temp_full_path,criteria, field, delete);//call the recursive function call.
				}else{
					
					if(strcmp(field, "name") == 0){ // if name is given as the criteria
						if(strcmp(sub_dirp ->d_name, criteria) == 0){ //If and only if a match has been found
							get_output(temp_full_path, sub_dirp, delete);
						}

					}else if(strcmp(field, "inode") == 0){ //if inode is given as the criteria
						sprintf(inode, "%d", sub_dirp->d_ino);
						if(strcmp(inode, criteria) == 0){
							get_output(temp_full_path, sub_dirp, delete);
						}

					}else if(strcmp(field, "mmin") == 0){ //if mmin is given as the criteria
						time_handler(temp_full_path, sub_dirp, delete, crit, criteria);

					}else{ //If no criteria is given, print every path or delete everything.
						get_output(temp_full_path, sub_dirp, delete);
					}

				}
			}
		}//end of while loop
		closedir(sub_dp);//close the stream
	}else{ //If the directory doesn't exist
		printf("cannot open directory\n");
		exit(2);
	}
}				

//Code for obtaining inputs below was provided by Dr. Yong Chen of Texas Tech University

int 
main(int argc, char **argv){
	int w=0, n=0, m=0, i=0, a=0, d=0, delete = 0;
	char  *where, *name, *mmin, *inum, *action;
	while (1) {
		char c;
		//A colon indicates that it requires an argument afterwards, eg -w testdir
		c = getopt(argc, argv, "w:n:m:i:a:d");
							 							 		 			 
		if (c == -1) {
			/* We have finished processing all the arguments. */
			break;
		}
		switch (c) {
		case 'w':
			w = 1;
			where = optarg;
			printf("where: %s\n", optarg);
			break;
		case 'n':
			n = 1;
			name = optarg;
			printf("name: %s\n", optarg);
			break;
		case 'm':
			m = 1;
			mmin = optarg;
			printf("mmin: %s\n", optarg);
			break;
		case 'i':
			i = 1;
			inum = optarg;
			printf("inum: %s\n", optarg);
			break;
		case 'a':
			a = 1;
			action = optarg;
			printf("action: %s\n", optarg);
			break;
		case 'd':
			d = 1;
			delete = 1;
			break;
		case '?':
		default:
			printf("An invalid option detected.\n");
		}
	}


	if(w == 0){ //If no directory is specified, use the current working directory
	        char cwd[1024];
                where = getcwd(cwd, sizeof(cwd));
	}

	//Call find(where, criteria, field, delete); passing arguments based on input
	if(n != 0){
		find(where, name, "name", delete);	
	}else if(m != 0){
		find(where, mmin, "mmin", delete);
	}else if(i != 0){
		find(where, inum, "inode", delete);
	}else{
		find(where, name, "None", delete);
	}

	
	/*
	 * Now set the values of "argc" and "argv" to the values after the
	 * options have been processed above.
	 */
	argc -= optind;
	argv += optind;

	/*
	 * Now do something with the remaining command-line arguments, if
	 * necessary.
	 */
	if (argc > 0) {
		printf("There are %d command-line arguments left to process:\n", argc);
		for (i = 0; i < argc; i++) {
			printf("Argument %d: '%s'\n", i + 1, argv[i]);
		}
	}
	return 0;
}

