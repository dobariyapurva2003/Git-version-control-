#include "ls.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <iomanip>
#include <sys/types.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>



// Helper function to get user name from UID
std::string get_user_name(uid_t uid) {
    struct passwd *pw = getpwuid(uid);
    return pw ? pw->pw_name : std::to_string(uid);
}

// Helper function to get group name from GID
std::string get_group_name(gid_t gid) {
    struct group *gr = getgrgid(gid);
    return gr ? gr->gr_name : std::to_string(gid);
}

// Helper function to format time
std::string format_time(time_t raw_time) {
    struct tm *timeinfo = localtime(&raw_time);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%b %d %H:%M", timeinfo);
    return std::string(buffer);
}

void print_file_info(const struct stat& st, std::string& filename) {
    std::cout << (st.st_mode & S_IFDIR ? "d" : "-");
    std::cout << (st.st_mode & S_IRUSR ? "r" : "-");
    std::cout << (st.st_mode & S_IWUSR ? "w" : "-");
    std::cout << (st.st_mode & S_IXUSR ? "x" : "-");
    std::cout << (st.st_mode & S_IRGRP ? "r" : "-");
    std::cout << (st.st_mode & S_IWGRP ? "w" : "-");
    std::cout << (st.st_mode & S_IXGRP ? "x" : "-");
    std::cout << (st.st_mode & S_IROTH ? "r" : "-");
    std::cout << (st.st_mode & S_IWOTH ? "w" : "-");
    std::cout << (st.st_mode & S_IXOTH ? "x" : "-");
    std::cout << " " << std::setw(4) << st.st_nlink;
    std::cout << " " << std::setw(8) << get_user_name(st.st_uid);
    std::cout << " " << std::setw(8) << get_user_name(st.st_gid);
    std::cout << " " << std::setw(6) << st.st_size;
    std::cout << " " << std::setw(12) << format_time(st.st_mtime);
    std::cout << " " << filename << std::endl;
}



void get_ls(std::string& path, bool all_files, bool long_listing) {
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        std::cerr << "ls: cannot open directory '" << path << "': No such file or directory\n";
        return;
    }

    struct dirent *entry;
    std::vector<std::string> filenames;
    while ((entry = readdir(dir)) != nullptr) {
        if (all_files || (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)) {
            filenames.push_back(entry->d_name);
        }
    }
    
    
    closedir(dir);

    std::sort(filenames.begin(), filenames.end());

    for (std::string& filename : filenames) {
        std::string full_path = path + "/" + filename;
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0) {
            if (long_listing) {
                print_file_info(st, filename);
            } else {
                std::cout << filename << std::endl;
            }
        } else {
            std::cerr << "ls: cannot access '" << full_path << "': No such file or directory\n";
        }
    }
}


void Ls_class::ls(std::vector<std::string>& str)
{
   // std::cout<<"directory needs to be changed is : "<<str<<std::endl;
   // std::string d="helllo from cd";
    //std::string res=str;
    bool all_files=false;
    bool long_list = false;
    if(str.size()==1)
    {
        std::string temp =".";
        get_ls(temp, all_files, long_list);
    }
    else {
         if(strcmp(const_cast<char*>(str[1].c_str()), "-a") == 0)
            {
              all_files = true;
            }
            else if(strcmp(const_cast<char*>(str[1].c_str()), "-l") == 0)
            {
              long_list = true;
            }
            else if( (strcmp(const_cast<char*>(str[1].c_str()) , "-al") == 0) || (strcmp(const_cast<char*>(str[1].c_str()), "-la") == 0) )
            {
              all_files = true;
              long_list = true;
            }
            if(str.size()==2)
            {
                std::string temp =".";
                get_ls(temp, all_files, long_list);
            }
            else if(str.size()>2)
            {
              for(int i=2;i<str.size();i++)
              {
                get_ls(str[i], all_files, long_list);
              }
            }
          }

    
    //return res;
}
