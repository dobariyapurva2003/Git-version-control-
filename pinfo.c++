#include "pinfo.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/resource.h>

std::string getProcessExecutablePath(pid_t pid) {
    char path[PATH_MAX];
    sprintf(path, "/proc/%d/exe", pid);

    char buffer[PATH_MAX];
    ssize_t length = readlink(path, buffer, PATH_MAX - 1);
    if (length == -1) {
        return "";
    }

    buffer[length] = '\0';
    return (std::string(buffer));
}



long getProcessMemoryUsage(pid_t pid) {
    
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss;
    } else {
        std::cerr << "Error getting process memory usage" << std::endl;
        return 0;
    }
}


std::string getProcessStatus(int status) {
    if (WIFEXITED(status)) {
        return "Z"; // Exited
    } else if (WIFSIGNALED(status)) {
        return "T"; // Terminated by signal
    } else if (WIFSTOPPED(status)) {
        return "T"; // Stopped
    } else if (WIFCONTINUED(status)) {
        return "R"; // Continued
    } else {
        return "S"; // Sleeping
    }
}

void get_pinfo(pid_t pid) {
    int status;
    if (waitpid(pid, &status, WNOHANG) == 0) {
        // Process is still running
        std::cout << "pid -- " << pid << std::endl;
        std::cout << "Process Status -- ";
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            std::cout << getProcessStatus(status);
        } else {
            std::cout << getProcessStatus(status) << "+"; // Foreground process
        }
        std::cout << std::endl;
        std::cout << "memory -- " << getProcessMemoryUsage(pid) << " {Virtual Memory}" << std::endl;
        std::cout << "Executable Path -- " << getProcessExecutablePath(pid) << std::endl;
    } else {
        std::cout << "Process not found: " << pid << std::endl;
    }
}


void Pinfo_class::pinfo(std::string& pr_id) {
    pid_t pid = stoi(pr_id);
    get_pinfo(pid);

}
