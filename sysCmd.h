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

//For keeping track of foreground process
 extern bool fore;
 extern int fg_id;

class SysCmd_class {
public:
     // Method declaration
    void sys(std::vector<std::string>& res, bool flag);
};