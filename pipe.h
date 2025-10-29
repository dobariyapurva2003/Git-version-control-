#include<iostream>
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

class Pipe_class {
public:
     // Method declaration
    std::vector<std::string> pipe(std::string& str, char delimiter);
    void pipe2(std::string& res);
};