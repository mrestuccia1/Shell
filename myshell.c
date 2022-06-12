#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
char error_message[30] = "An error has occurred\n";
void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}
char* cd_command(char* command) {
    int size_cmd = strlen(command);
    char* cd_cmd = (char*)malloc(sizeof(char) * size_cmd);
    if (cd_cmd == NULL) {
        write(STDOUT_FILENO, error_message, strlen(error_message));
    } else{
        for (int i = 3; i <= size_cmd - 1; i++) {
            cd_cmd[i - 3] = command[i];
        }
    }
    return (cd_cmd);
}

void removen(char* cmd) {
    int len = strlen(cmd);
    if (cmd[len - 1] == '\n') {
        cmd[len - 1] = '\0';
    }
}
void removeendingspaces(char* cmd) {
    int len = strlen(cmd);
    int count = 0;
    while (1) {
        if (cmd[len - 1 - count] == ' ' || cmd[len - 1 - count] == '\t') {
            count++;
        } else {
            break;
        }
    }
    cmd[len - count] = '\0';
}
int count_spaces(int start, char* cmd) {
    int num = 0;
    while (cmd[start] == ' ' || cmd[start] == '\t') {
        num++;
        start++;
    }
    return num;
}
char* rem(char* cmd) {
    int point = 0;
    int orig = 0;
    char* newcmd = (char*)malloc(sizeof(char*)*strlen(cmd));

    while (1) {
        
        while (cmd[point] != ' ' && cmd[point] != '\t') {
            newcmd[orig] = cmd[point];
            point++;
            orig++;
            if (point >= strlen(cmd)) {
                break;
            }
        }
        
        if (point < strlen(cmd) && point > 0) {
            newcmd[orig] = ' ';
            orig++;
        }
        int num = count_spaces(point, cmd);
        
        point+=num;

        if (point >= strlen(cmd)) {
            break;
        }
    }
    
    return newcmd;
}
int check_length(char* cmd) {
    int len = strlen(cmd) - 1;
    if (len > 512) {
        return 1;
    }
    return 0;
}
int count_args(char* cmd) {
    int on = 0;
    int count = 0;
    for (int i = 0; i < strlen(cmd);i ++) {
        if (cmd[i] != ' ' && cmd[i] != '\t' && on == 0) {
            count++;
            on = 1;
        } else if (cmd[i] == ' ' || cmd[i] == '\t') {
            on = 0;
        }
    }
    return count;
}
char* get_cmd(char* cmd) {
    char* ret = (char*)malloc(sizeof(char)*10);
    int point = 0;

    while (cmd[point] != ' ' && point < strlen(cmd)) {
        ret[point] = cmd[point];
        point++;
        if (point >= 10) {
            break;
        }
    }
    ret[point] = '\0';

    return ret;

}
void process (char* loc) {
    char* currcmd;
    
    while ((currcmd = strtok_r(loc, ";", &loc))) {
        
        removeendingspaces(currcmd);
        currcmd = rem(currcmd);

        char* cmd = get_cmd(currcmd);
        
        if (!currcmd) {
            exit(0);
        } else if (!strcmp(cmd, "exit")) {
            if (!strcmp(currcmd, "exit")) {
                exit(0);
            } else {
                write(STDOUT_FILENO, error_message, strlen(error_message));
                continue;
            }// Exit
            
        } else if (!strcmp(cmd, "pwd")) { // pwd
            if (!strcmp(currcmd, "pwd")) {
                char cwd[256];
                getcwd(cwd, sizeof(cwd));
                myPrint(cwd);
                myPrint("\n");
            } else {
                write(STDOUT_FILENO, error_message, strlen(error_message));
                continue;
            }

        } else if (!strcmp(cmd, "cd")) {
            char* cd_cmd = cd_command(currcmd);
            if (strcmp(currcmd, "cd") == 0) { // Home Direcotry
                char* home = getenv("HOME");
                chdir(home);
            } else if (strcmp(cd_cmd, "..") == 0) { // Go down
                chdir("..");
            } else {
                int success = chdir(cd_cmd); // Go up
                if (!!success) {
                    write(STDOUT_FILENO, error_message, strlen(error_message));
                }
            }
        } else {

            int tot = count_args(currcmd);
            
            char** cmd_arr = (char**)malloc(sizeof(char*)*(tot + 1));
            for (int i = 0; i < tot; i++) {
                cmd_arr[i] = (char*)malloc(sizeof(char) *512);
            }
            char* currcmd2;
            char* loc2 = currcmd;
            int counter = 0;
            
            while ((currcmd2 = strtok_r(loc2, " ", &loc2))) {

                strcpy(cmd_arr[counter], currcmd2);
                counter++;
            }

            cmd_arr[tot] = NULL;
            
            if (fork() == 0) {
                int status_code = execvp(cmd_arr[0], cmd_arr);
                if (status_code == -1) {
                    write(STDOUT_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
            }else{
                waitpid(-1, NULL, 0);
            }
            free(cmd);
        }
    }
}          
        
int main(int argc, char *argv[])
{
    char cmd_buff[50000];
    char *pinput;
    if (argc == 2) {
        char* file_name = argv[1];
        FILE* fp;
        fp = fopen(file_name, "r");
        if (!fp) {
            write(STDOUT_FILENO, error_message, strlen(error_message));
        }
        
        char* line = NULL;
        size_t len = 0;
        
        while (getline(&line, &len, fp) != -1) {
\
            int check_all_white = 0;
            for (int i = 0; i < strlen(line); i ++) {
                if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
                    check_all_white = 1;
                    break;
                } 
            }

            if (check_all_white == 1) {
                myPrint(line);
            }

            if (check_length(line)) {
                
                write(STDOUT_FILENO, error_message, strlen(error_message));
                continue;
            }
            removen(line);
            
            removeendingspaces(line);
            process(line);
            
        }
    } else if (argc > 2) {
        write(STDOUT_FILENO, error_message, strlen(error_message));
    } else {
        while (1) {
            myPrint("myshell> ");
            pinput = fgets(cmd_buff, 50000, stdin);
            if (check_length(pinput)) {
                write(STDOUT_FILENO, error_message, strlen(error_message));
                continue;
            }

            removen(pinput);
            removeendingspaces(pinput);
            process(pinput);
        }
    }
   
}