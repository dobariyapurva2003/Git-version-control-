#include "pwd.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>


void Pwd_class::pwd() {
    char buffer[1024];        
    if (getcwd(buffer, sizeof(buffer)) != NULL) 
    {
      std::cout<<buffer<<std::endl;
    }
    else
    {
      std::perror("getcwd");
    }
}