/*
 * Author: Akshat Chhajer
 * Date: 19 August 2019
 * Purpose: A unix style shell written in c supporting basix commands
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <pwd.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <time.h>
#include <grp.h>
#include <sys/select.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>

#define BUF_PWD 1024
#define BUF_USR 1024
#define BUF_HST 1024
#define BUF_HOM 1024
#define BUF_PMT 1024
#define BUF_NCM 128
#define BUF_COM 1024
#define BUF_TOK 1024
#define HASH_MAX 8192

char pwd[BUF_PWD];
char u_name[BUF_USR];
char host[BUF_PWD];
char home[BUF_HOM];
char prompt[BUF_PMT];
char *commands[BUF_COM];
char *tokens[BUF_TOK];
int flag_hash[256];
int no_flags;
int no_tokens;

//nightswatch exit on pressing q

int (*cmd_functions[HASH_MAX]) (int, char **);


void torelative(char *path){
    if(strlen(path) >= strlen(home) && strncmp(path, home, strlen(home)) == 0)
        sprintf(path, "~%s", path + strlen(home));
}

int hash(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while (c = *str++){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        hash = hash % HASH_MAX;
    }
    return hash;
}

int exit_nash(int n, char **args){
    printf("Exiting Nash, Goodbye\n");
    exit(1);
    return 1;
}

int pwd_nash(int n, char **args){
    printf("%s\n", pwd);
    return 1;
}

int echo_nash(int n, char **args){
    for(int i=1; i<no_tokens; i++)
        printf("%s", tokens[i]);
    printf("\n");
    return 1;
}

int cd_nash(int n, char **args){

    char cdpath[BUF_PWD];

    for(int i=0;i<n;i++){
        //   printf("%s\n", args[i]);
    } 
    if(args[1][0] == '~'){
        strcpy(cdpath, home);
        strcat(cdpath, args[1]+1);
    }
    else{
        strcpy(cdpath, args[1]);
    }
    if(chdir(cdpath) < 0)
        perror("cd Error");

    return 1;
}

int clear_nash(int n, char **args){
    printf("\033[H\033[J");
    return 1;
}


int ls_nash(int n, char **args){

    struct dirent *dp;
    DIR *dir;
    char items[BUF_PWD][BUF_PWD];
    int no_items = 0;

    if(no_tokens > 1)
        cd_nash(n, args);

    dir = opendir(".");

    while((dp = readdir(dir)) != NULL){
        if((dp -> d_name[0] == '.' && flag_hash['a']) || dp -> d_name[0] != '.'){
            strcpy(items[no_items], dp -> d_name);
            no_items++;
        }
    }

    for(int i=0;i<no_items;i++){
        if(flag_hash['l']){

            struct stat fileStat;
            if(stat(items[i], &fileStat) < 0)
                continue;
            struct passwd *pws;
            pws = getpwuid(fileStat.st_uid);
            struct group *grp;
            grp = getgrgid(fileStat.st_gid);

            printf( (S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf( (fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf("%2ld", fileStat.st_nlink);
            printf("%9s", pws->pw_name);
            printf("%9s", grp->gr_name);
            printf("%6ld ", fileStat.st_size);
            printf("%.12s", ctime(&fileStat.st_mtime) + 4);
            printf(" %s", items[i]);
            printf("\n");

        }
        else
            printf("%s\n", items[i]);
    }
    chdir(pwd);
    return 1;
}

int pinfo_nash(int n, char **args){


    char pinfo_path[BUF_PWD];
    char p_path[BUF_PWD];

    if(n > 1)
        sprintf(p_path, "/proc/%s/", args[1]);  
    else
        strcpy(p_path, "/proc/self/");

    strcpy(pinfo_path, p_path);
    strcat(pinfo_path, "stat");

    FILE *stat = fopen(pinfo_path, "r");
    if(stat == NULL){
        perror("statfile Error:");
        return 0;
    }

    int pid, mem = 0;
    char status, expath[BUF_PWD], pname[BUF_PWD];

    fscanf(stat, "%d %s %c", &pid, pname, &status);
    fclose(stat);

    strcat(pinfo_path, "m");

    FILE *statm = fopen(pinfo_path, "r");
    if(statm == NULL){
        perror("statfile Error:");
        return 0;
    }
    fscanf(statm, "%d", &mem);
    fclose(statm);

    strcpy(pinfo_path, p_path);
    strcat(pinfo_path, "exe"); 

    readlink(pinfo_path, expath, sizeof(expath));
    torelative(expath);

    printf("pid -> %d\n", pid);
    printf("Status -> %c\n", status);
    printf("Memory -> %d\n", mem);
    printf("Executable Path -> %s\n", expath);

    return 1;
}

int nightswatch_nash(int n, char **args){
    
    int time = atoi(args[1]);
    fd_set input_set;
    struct timeval timeout;

    /* Empty the FD Set */
    FD_ZERO(&input_set);
    /* Listen to the input descriptor */
    FD_SET(STDIN_FILENO, &input_set);
    
    if (!strcmp(args[2],"dirty")){
		// dirty

		do {
			FILE *meminfo = fopen("/proc/meminfo", "r");       
			ssize_t reads;
			size_t len = 0;
			char * line = NULL;

			if (meminfo == NULL){
				perror("Error opening meminfo file: ");
				return 0;
			}

			int i = 0;

			while(i < 17 && (reads = getline(&line, &len, meminfo)) != -1) {
				i++;
			}
			printf("%s", line);

			fclose(meminfo);

			timeout.tv_sec = time;    // time seconds
			timeout.tv_usec = 0;    // 0 milliseconds
			select(1, &input_set, NULL, NULL, &timeout);
		}
		while(1);
		return 0;
	}
    
    return 1;
}

void child_exited(int n){

    int status;
    pid_t wpid = waitpid(-1, &status, WNOHANG);

    if(wpid > 0 && WIFEXITED(status)==0){
        printf("\nProcess with pid %d exited normally\n", wpid);
    }
    if(wpid > 0 && WIFSIGNALED(status)==0){
        printf("\nProcess with pid %d exited due to a user-defined signal\n", wpid);
    }
}

int execute_program(char* command){

    int n = 0, bg = 0;
    char *tok[BUF_TOK];

    tok[n] = strtok(command, " \t\n\r\a");

    while(tok[n] != NULL){
        tok[++n] = strtok(NULL, " \t\n\r\a");
    }
    pid_t pid = fork();

    if(strcmp(tok[n-1],"&")==0){
        n--;
        tok[n] = NULL;
        bg = 1;
    } 

    if(pid < 0){
        perror("Fork failed:");
        return 0;
    }
    else if(pid == 0){
        if(bg)
            setpgid(0, 0);

        int proc = execvp(tok[0], tok);
        if(proc == -1)
            perror("Error executing:");

        exit(EXIT_FAILURE);
    }
    else {
        int status;
        if(!bg){
            do{
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        else{
            signal(SIGCHLD, child_exited);
        }
    }
    return 1; 
}

void calculate_hash(){

    cmd_functions[hash("pwd")] = &pwd_nash;
    cmd_functions[hash("echo")] = &echo_nash;
    cmd_functions[hash("cd")] = &cd_nash;
    cmd_functions[hash("clear")] = &clear_nash;
    cmd_functions[hash("c")] = &clear_nash;
    cmd_functions[hash("quit")] = &exit_nash;
    cmd_functions[hash("exit")] = &exit_nash;
    cmd_functions[hash("ls")] = &ls_nash;
    cmd_functions[hash("pinfo")] = &pinfo_nash;
    cmd_functions[hash("nightswatch")] = &nightswatch_nash;
}


void update(){

    //Update username (u_name)
    struct passwd *p = getpwuid(getuid());
    char *name = p -> pw_name;
    strcpy(u_name, name);

    //Update hostname (host)
    gethostname(host, sizeof(host));

    //Update present working directory (pwd)
    getcwd(pwd, sizeof(pwd));

}

char* get_prompt(){

    char p[BUF_PMT];
    char path[BUF_PWD];

    update();

    //Check if ~ should be used
    strcpy(path, pwd);
    torelative(path);

    sprintf(p, "<%s@%s[%s]>", u_name, host, path);

    strcpy(prompt, p);
    return prompt;
}


int get_commands(){

    char *pr = get_prompt();
    char *cmds = readline(pr);

    int n = 0;

    if(cmds == NULL){
        printf("\nExiting Shell, Goodbye!\n");
        exit(0);
    }

    commands[0] = strtok(cmds,";");

    while(commands[n] != NULL)
        commands[++n] = strtok(NULL, ";");

    return n;
}

void tokenize(char *command){

    int n = 0;
    char *temp[BUF_TOK];

    char com[BUF_COM];
    strcpy(com,command);

    temp[0] = strtok(com, " \t\n\r\a");

    while(temp[n] != NULL)
        temp[++n] = strtok(NULL, " \t\n\r\a");

    no_tokens = 0;

    for(int i=0;i<256;i++)
        flag_hash[i] = 0;

    for(int i=0;i<n;i++){
        if(temp[i][0] == '-'){
            for(int j=1;j<strlen(temp[i]);j++)
                flag_hash[temp[i][j]] = 1;
        }
        else{
            tokens[no_tokens] = temp[i];
            no_tokens++;
        }
    }
}


int main(){

    getcwd(home, sizeof(home));
    calculate_hash();
    
    while(1){
        int no_commands = get_commands();
        for(int i=0 ;i<no_commands; i++){
            tokenize(commands[i]);
            if((*cmd_functions[hash(tokens[0])]) == NULL){
                execute_program(commands[i]);
            }
            else {
                //            for(int i=0;i<no_tokens;i++)
                //               printf("%s\n", tokens[i]);
                //          printf("%d\n", no_tokens);
                int code = (*cmd_functions[hash(tokens[0])])(no_tokens, tokens); 

            }
        } 
    }
    return 0;
}


