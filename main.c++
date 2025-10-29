#include "cd.h"
#include "ls.h"
#include "pwd.h"
#include "echo.h"
#include "pinfo.h"
#include "search.h"
#include "io.h"
#include "pipe.h"
#include "sysCmd.h"
#include<iostream>
#include <unistd.h>
#include <sys/wait.h>
#include<bits/stdc++.h>
#include <fstream>
#include<string>
#include<termio.h>
#include<signal.h>



using namespace std;


//For own created commands
set<string> command = {"cd","ls","pwd","echo", "pinfo", "search" , "history"};



//For tokenize user input
vector<string> tokenize(string input) {
    vector<string> tokens;
    char *token = strtok(const_cast<char*>(input.c_str()), " ");

    while (token != nullptr) {
        tokens.push_back(token);
        token = strtok(nullptr, " ");
    }
    return tokens;
}



//For detection of signals and uparrow key
char getch() {
    termios oldt, newt;

    //For getting original attributes of terminal
    termios term;
    tcgetattr(STDIN_FILENO, &term);
    char ch;

    oldt = term;
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); // Disable buffering and echo

    //to set terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

//For getting list of files
std::vector<std::string> getFileList(const std::string& path) {
    std::vector<std::string> fileList;
    DIR *dir;
    struct dirent *entry;

    if ((dir = opendir(path.c_str())) != nullptr) {
        while ((entry = readdir(dir)) != nullptr) {
            fileList.push_back(entry->d_name);
        }
        closedir(dir);
    } else {
        perror("opendir");
    }

    return fileList;
}



//For uparrow key
string autocomplete(const std::string& input) {
    std::vector<std::string> fileList = getFileList(".");
    std::vector<std::string> matches;
    string temp = input;
    reverse(temp.begin(),temp.end());
    int pos = temp.find(" ");
    temp = temp.substr(0,pos);
    reverse(temp.begin(),temp.end());
    //cout<<temp<<endl;

    //for returning whole cmd line
    string res = input;

    for (const auto& file : fileList) {
        if (file.find(temp) == 0) { // Check if file starts with the input
            matches.push_back(file);
        }
    }

    if (matches.size() == 1) {
        std::cout << "\r" << matches[0].substr(input.size()) << std::flush;
        res+=matches[0].substr(input.size());
    } else if (matches.size() > 1) {
        std::cout <<matches[0].substr(temp.size()) <<" ";
        res+=matches[0].substr(temp.size()) + " ";
        for (int i=1;i<matches.size()-1;i++) {
            std::cout << matches[i] << " ";
            res+=matches[i] + " ";
        }
        cout<<matches.back();
        res+=matches.back();
        //std::cout << "\n" << input << std::flush;
    }
    return res;
}


// Signal handler for SIGTSTP (CTRL-Z)
void handle_SIGTSTP(int signal) {
    if (fore) {
        // Send SIGTSTP to the foreground process to stop it
        kill(fg_id, SIGTSTP);
        std::cout << "Process " << fg_id << " stopped and moved to background." << std::endl;
        fore = false;
    }
}

// Signal handler for SIGINT (CTRL-C)
void handle_SIGINT(int signal) {
    if (fore) {
        // Send SIGINT to the foreground process to interrupt it
        kill(fg_id, SIGINT);
        std::cout << "Process " << fg_id << " interrupted." << std::endl;
        fore = false;
    }
}

// Signal handler for EOF (CTRL-D)  not working
void handle_EOF(int signal) {
    std::cout << "Logging out..." << std::endl;
    exit(0);

}



int main()
{

    string username = getlogin();
    char hostname[256];
    gethostname(hostname, 256);
    string current_dir = "~";
    char buff[1024];
    getcwd(buff,sizeof(buff));
    string root_sudo_dir = buff;

    //For "cd -" we have to store previous dir
    string prev_dir=""; 


    

    //For loading and storing history
    const std::string HISTORY_FILE = "history.txt";
    const size_t MAX_HISTORY_SIZE = 20;
    const size_t MAX_DISPLAY_HISTORY = 10;
    std::deque<std::string> history;
    std::ifstream infile(HISTORY_FILE);
    if(!infile)
    {
        cout<<"Error while opening file\n";
    }
    std::string line;

    while (std::getline(infile, line)) {
        if (!line.empty()) {
            history.push_back(line);
        }
    }
    infile.close();
   
   //for up arrow key
    int his_ind = history.size()-1;
    



        
    while (true) {
        cout << username << "@" << hostname << ":" << current_dir << "> ";
        string original = username+"@"+hostname+":"+current_dir+">"; 
        string input;
        signal(SIGTSTP, handle_SIGTSTP);  // Handle CTRL-Z
        signal(SIGINT, handle_SIGINT);    // Handle CTRL-C
        signal(SIGQUIT, handle_EOF);      // Handle EOF (CTRL-D) by exiting the shell

        

        //For autocomplete
        jump : 
        
        char ch = getch();

        if (ch == 27) { // Escape sequence (start of arrow keys or other special keys)
            char next = getch(); // '['
            char last = getch(); // actual key
            if (next == '[' && last == 'A') { // Up arrow key
                // Handle up arrow key functionality if needed
                if(!history.empty() && his_ind>=0)
                {
                    int pos = original.find(">");
                    input = history[his_ind];
                    
                    //Erase whole previous line and then print new one
                    cout<<"\r"<<std::flush;
                    cout<<"\033[K"<<std::flush;
                    cout<<"\r"<<original <<input<<std::flush;
                    his_ind--;
                }
                goto jump;
            }
        } else if (ch == '\t') { // TAB key
            
            string temp = autocomplete(input);
            //cout<<endl<<temp<<endl;
            input = temp;
            //cout<<input<<endl;
            //break;
            //getline(cin,input);
            goto jump;
        } else if (ch == '\n' && input.empty()) { // Enter key
            std::cout << "\n";
            //input.clear();
            continue;
        } else if(ch=='\n' && !input.empty())
        {
            cout<<"\n";
            goto start;
        }
        else if (ch == 127) { // Backspace key
            if (!input.empty()) {
                // Move the cursor back one position and erase the character
                std::cout << "\b \b" << std::flush;
                input.pop_back(); // Update the input string
                goto jump;
            }
            else if(input.empty())
            {
                cout<<"\r"<<std::flush;
                cout<<"\033[K"<<std::flush;
                cout<<"\r"<<original <<input<<std::flush;   
                goto jump;
            }
        } 
        else {
            input += ch;
            std::cout << ch;
            goto jump;
        }


        start :
        //cout<<input<<endl;

        vector<string> tokens = tokenize(input);

        if (tokens.empty()) {
            continue;
        }

        //For add cmd to history
         if(history.size()<MAX_HISTORY_SIZE)
         {
            history.push_back(input);
            his_ind = history.size()-1;
         }
        else
        {
            history.pop_front();
            history.push_back(input);
            his_ind = history.size()-1; 
        }

        //If user wants to exit then store history then exit from shell
        if(tokens[0]=="exit")
        {
            std::ofstream outfile(HISTORY_FILE, std::ios::trunc);
            if(!outfile.is_open())
            {
                cout<<"Error in writing\n";
            }
            for (const auto& cmd : history) {
                outfile << cmd << std::endl;
            }
            outfile.close();
            break;
        }


        

        //FORGROUND AND BACKGROUND SYSTEM CMDS
        if(!input.empty() && input.back()=='&')
        {
            input.pop_back();

            std::vector<std::string> args;
            std::istringstream stream(input);
            std::string token;
            while (stream >> token) {
                args.push_back(token);
            }            

            //This funstion is only for non-created commands
            if(command.find(args[0])==command.end())
            {
                //cout<<fg_id<<" "<<fore<<endl;
                SysCmd_class obj;
                obj.sys(args, true);
                //cout<<fg_id<<" "<<fore<<endl;
                continue;
            }
        }
        else if(!input.empty())
        {
            std::vector<std::string> args;
            std::istringstream stream(input);
            std::string token;
            while (stream >> token) {
                args.push_back(token);
            }  
            if(command.find(args[0])==command.end())
            {
                //cout<<fg_id<<" "<<fore<<endl;
                SysCmd_class obj;
                obj.sys(args, false);
                //cout<<fg_id<<" "<<fore<<endl;
                continue;
            }
        }
            


        //If we need I/O REDIRECTION PLUS PIPE
        if(input.find("|")!=string::npos && 
        (input.find(">")!=string::npos || input.find("<")!=string::npos || input.find(">>")!=string::npos))
        {
            Pipe_class obj;
            obj.pipe2(input);
            continue;
        }
        //Only for pipe
        else if(input.find("|")!=string::npos)
        {
            Pipe_class obj1;
            vector<string> res = obj1.pipe(input,'|');

            //For verify
            // for(int i=0;i<res.size();i++)
            // {
            //     obj1.pipe_ex(res[i]);
            // }

            continue;
        }

        char* argv[tokens.size()+1];
        for(int i=0;i<tokens.size();i++)
        {
            argv[i] = const_cast<char*>(tokens[i].c_str());
        }
        argv[tokens.size()]=nullptr;



        //If we need I/o redirecction only
        if((input.find(">")!=string::npos || input.find("<")!=string::npos || input.find(">>")!=string::npos) 
        && input.find("|")==string::npos)
        {
            Io_class obj1;
            int temp = obj1.io(input);
            continue;
        }

        //commands for given cd,ls,pwd,echo
        
        if(command.find(tokens[0]) != command.end())
        {
            if(tokens[0]=="cd")
            {
             Cd_class obj;
             //obj.demo();
             string st = obj.cd(tokens[1], root_sudo_dir , prev_dir);
             // if pwd contains psudo root shell dir
             std::size_t start_pos = st.find(root_sudo_dir);
             if(start_pos!=string::npos)
             {
                std::size_t end_pos = start_pos+root_sudo_dir.length();
                string res = "~"+ st.substr(end_pos);
                if(res=="")
                {
                    current_dir = "~";
                }
                else
                {
                    current_dir = res;
                }
             }
             else
             {
                if(st=="root")
                {
                    current_dir = root_sudo_dir;
                }
                else
                current_dir = st;
                //cout<<st<<endl;
             }   
            }
            else if(tokens[0]=="ls")
            {
                Ls_class object1;
                object1.ls(tokens);
            }
            else if(tokens[0]=="pwd")
            {
                Pwd_class obj;
                obj.pwd();
            }
            else if(tokens[0]=="echo")
            {
                Echo_class obj;
                obj.echo(input,tokens);
            }
            else if(tokens[0]=="pinfo")
            {
                Pinfo_class obj;

                //just for verify
            //    pid_t temp = fork();
            //    string pr = to_string(temp);
            //    obj.pinfo(pr);

                obj.pinfo(tokens[1]);
            }
            else if(tokens[0]=="search")
            {
                Search_class obj;
                bool res = obj.search(tokens[1]);
                if(res)
                cout<<"True"<<endl;
                else
                cout<<"False"<<endl;
            }
            else if(tokens[0]=="history")
            {
                int end;
                if(tokens.size()==1)
                {
                    end = MAX_DISPLAY_HISTORY;
                    
                }
                else
                {
                    end= stoi(tokens[1]);
                    std::reverse(history.begin(), history.end());
                    
                }
                int count = 0;
                for (auto it = history.begin(); it != history.end() && count < end; ++it, ++count) {
                        std::cout << *it << std::endl;
                }
                
            }
            
        }
        else
        {
            pid_t pid = fork();
            if(pid==0)
            {
                execvp(argv[0],argv);
                perror("Error : ");
            }
            else if(pid>0)
            {
                int status;
                waitpid(pid,&status,0);
            }
        }


    }
    return 0;

}
