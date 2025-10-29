#include "echo.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string>


void Echo_class::echo(std::string& str, std::vector<std::string>& vec) {
            int pos;
            //std::cout<<str<<std::endl;
          if(str.find("\"")!=std::string::npos)
          {
            pos = str.find("\"");
            std::string temp =str.substr(pos+1);
            pos = temp.find("\"");
            std::cout<<temp.substr(0,pos)<<std::endl;
          }
          else if(str.find("\'")!=std::string::npos)
          {
            pos = str.find("\'");
            std::string temp =str.substr(pos+1);
            pos = temp.find("\'");
            std::cout<<temp.substr(0,pos)<<std::endl;
          }
          else
          {
            for(int i=1;i<vec.size();i++)
            {
              std::cout<<vec[i]<<" ";
            }
            std::cout<<std::endl;
          }
}
