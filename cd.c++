#include "cd.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>


void Cd_class::demo() {
    std::cout << "Hello from CD file!" << std::endl;
}

std::string Cd_class::cd(std::string& str , std::string& root , std::string& prev_dir)
{
   // std::cout<<"directory needs to be changed is : "<<str<<std::endl;
   // std::string d="helllo from cd";
    std::string res;


    char pr[1024];
    char temp[1024];

    if (str.empty()) {
        chdir(getenv("HOME"));
    } else if (str == ".") {
        // Do nothing, stay in current directory
    } else if (str == "..") {
        // Go to parent directory
        getcwd(pr,sizeof(pr));
        std::string check = pr;
        if(check==root && prev_dir=="")
        {
            prev_dir = pr;
            std::string r = "root";
            prev_dir = root;
            return r;
        }
        else
        {
            prev_dir = pr;
            chdir("..");
        }
    } else if (str == "-") {
        getcwd(temp,sizeof(pr));
        chdir(prev_dir.c_str());
        prev_dir = temp;
    } else if (str[0] == '~') {
        getcwd(pr,sizeof(pr));
        prev_dir = pr;
        chdir(root.c_str());
    }
    else {
        getcwd(pr,sizeof(pr));
        prev_dir = pr;
        //std::cout<<prev_dir<<std::endl;
        chdir(str.c_str());
    }

    char cwd[1024];
    std::string current_dir;
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
           current_dir = cwd;
            //cout<<current_dir<<endl;
    } else {
            perror("getcwd");
    }

    res = cwd;



    return res;
}