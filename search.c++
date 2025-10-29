#include "search.h"
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

bool get_search(const char* path, std::string& target) {
    DIR* dir = opendir(path);
    if (dir == NULL) {
        return false; // Path is not a directory
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string filename = entry->d_name;
        if (filename != "." && filename != "..") {
            std::string fullPath = path;
            fullPath += "/" + filename;
            if (filename == target) {
                closedir(dir);
                return true; // Target found
            } else {
                struct stat st;
                if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
                    if (get_search(fullPath.c_str(), target)) {
                        closedir(dir);
                        return true; // Target found in subdirectory
                    }
                }
            }
        }
    }

    closedir(dir);
    return false; // Target not found
}


bool Search_class::search(std::string& str) {
        char buff[1024];
        getcwd(buff,sizeof(buff));
        if(get_search(buff, str))
        return true;
        else
        return false;
}
