#include "nash.h"

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
    cmd_functions[hash("history")] = &history_nash;
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

    sprintf(p, "<\x1b[34m%s\x1b[0m@\x1b[34m%s\x1b[32m[%s]\x1b[0m>", u_name, host, path);

    strcpy(prompt, p);
    return prompt;
}


void local_history(char *cmd){

    char hist_path[BUF_PWD];
    strcpy(hist_path, home);
    strcat(hist_path, "/history.txt");

    char* l[BUF_COM];
    char c[BUF_COM];
    int n=0;
    FILE *f = fopen(hist_path, "r");


    fgets(c,BUF_COM, f);
    l[0] = strtok(c, ",");

    while(l[n] != NULL)
        l[++n] = strtok(NULL, ",");
    fclose(f);
    FILE *fd = fopen(hist_path, "w");

    int counter = 0;
    
    if(n>=20)
        counter++;

    char tp[BUF_COM];
    tp[0] ='\0';

    for(int i=counter;i<n;i++){
        strcat(tp, l[i]);    
        strcat(tp, ",");    
    }
    strcat(tp,cmd);
    fprintf(fd,"%s",tp);

    fclose(fd);
}

int get_commands(){

    char *pr = get_prompt();
    char *cmds = readline(pr);

    int n = 0;

    if(cmds == NULL){
        printf("\nExiting Shell, Goodbye!\n");
        exit(0);
    }
    add_history(cmds);
    local_history(cmds);

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
