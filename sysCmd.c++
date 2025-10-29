#include "sysCmd.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>


//Global variables declared in 'sysCmd.h' file

pid_t fg_id=-1;
bool fore = false;



void SysCmd_class::sys(std::vector<std::string>& args, bool flag) {
     pid_t pid = fork();

    if (pid == 0) {  // Child process
        // Convert vector of args to array of char*
        std::vector<char*> argv(args.size() + 1);
        for (size_t i = 0; i < args.size(); ++i) {
            argv[i] = const_cast<char*>(args[i].c_str());
        }
        argv[args.size()] = nullptr;
        //std::cout<<argv[0]<<"testing"<<std::endl;
        execvp(argv[0], argv.data());
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {  // Parent process
        if (flag) {
            std::cout << pid << std::endl;  // Print PID of the background process
        } else {
            fg_id = pid;
            fore = true;
            //std::cout<<"from syscmd file : pid = "<<fg_id<<std::endl;
            int status;
            waitpid(pid, &status, WUNTRACED); //Wait for forground process finished

            // Reset foreground process variables if stopped
            if (WIFSTOPPED(status)) {
                //std::cout << "Process " << pid << " stopped and moved to background." << std::endl;
            }
            fg_id=-1;
            fore = false;
        }
    } else {
        perror("fork failed");
    }
    return;
}
