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
            execute_program(commands[i]);
        } 
    }
    return 0;
}


