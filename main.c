/*
 * Author: Akshat Chhajer
 * Date: 19 August 2019
 * Purpose: A unix style shell written in c supporting basix commands
 */
    
#include "nash.h"

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
                int code = (*cmd_functions[hash(tokens[0])])(no_tokens, tokens); 

            }
        } 
    }
    return 0;
}


