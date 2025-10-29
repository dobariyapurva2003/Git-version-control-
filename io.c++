#include "io.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>


std::vector<std::string> tokeniz(std::string& input) {
    std::vector<std::string> tokens;
    char *token = strtok(const_cast<char*>(input.c_str()), " ");

    while (token != nullptr) {
        tokens.push_back(token);
        token = strtok(nullptr, " ");
    }

    return tokens;
}


void executeCmd(const char* command, const char* inputFile, const char* outputFile, const char* options[]) {

    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Fork failed" << std::endl;
        exit(1);
    }

    if (pid == 0) {
        // Child process

        // Open the input file and redirect standard input
        if (inputFile) {
            int inputFd = open(inputFile, O_RDONLY);
            if (inputFd < 0) {
                std::cerr << "Error opening input file: " << inputFile << std::endl;
                exit(1);
            }
            if (dup2(inputFd, STDIN_FILENO) < 0) {
                std::cerr << "Error redirecting standard input" << std::endl;
                close(inputFd);
                exit(1);
            }
            close(inputFd);
        }

        // Open the output file and redirect standard output
        if (outputFile) {
            int outputFd = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (outputFd < 0) {
                std::cerr << "Error opening output file: " << outputFile << std::endl;
                exit(1);
            }
            if (dup2(outputFd, STDOUT_FILENO) < 0) {
                std::cerr << "Error redirecting standard output" << std::endl;
                close(outputFd);
                exit(1);
            }
            close(outputFd);
        }

        // Execute the command
        execvp(command, const_cast<char* const*>(options));

        // If execvp fails
        std::cerr << "Execvp failed" << std::endl;
        exit(1);
    } else {
        // Parent process
        wait(nullptr);
    }
}


int Io_class::io(std::string& str) {
    std::string orig = str;
    std::vector<std::string> tokens = tokeniz(str);
    std::string ip_file,op_file;
    std::string content;
    bool echo=0;

    //this is for both input and output redirection
    if(str.find(">")!=std::string::npos && str.find("<")!=std::string::npos)
    {
        //to get input filename
        int pos1 = orig.find("<");
        std::string temp = orig.substr(pos1+2);
        pos1 = temp.find(">");
        temp = temp.substr(0,pos1-1);
        std::string ip_file = temp;

        //to get output filename
        int pos = orig.find(">");
        std::string temp1 = orig.substr(pos+2);
        std::string op_file = temp1;
        std::string op = tokens[0];
        
            if(op=="sort")
            {
                const char* args[] = {op.c_str() , nullptr};
                executeCmd(op.c_str(),ip_file.c_str(),op_file.c_str(),args);
            }
            else if(op=="grep")
            {
                std::string res;
                if(str.find("\"")!=std::string::npos)
                {
                    int pos = str.find("\"");
                    res = str.substr(pos+1);
                    pos = res.find("\"");
                    res = res.substr(0,pos);
                }
                else if(str.find("\'")!=std::string::npos)
                {
                    int pos = str.find("\'");
                    res = str.substr(pos+1);
                    pos = res.find("\'");
                    res = res.substr(0,pos);
                }
                const char* args[] = {op.c_str(),res.c_str(),nullptr};
                executeCmd(op.c_str(),ip_file.c_str(),op_file.c_str(),args);
            }
            else if(op=="awk")
            {
                std::string res = tokens[1];
                res+= " "+tokens[2];
                int pos = res.find("\'");
                std::string temp = res.substr(pos+1);
                pos = temp.find("\'");
                temp = temp.substr(0,pos);
                const char* args[] = {op.c_str(),temp.c_str(),ip_file.c_str(),nullptr};
                executeCmd(op.c_str(),ip_file.c_str(),op_file.c_str(),args);
            }
            else if(op=="head" || op=="tail")
            {
                //get head or tail flag exa: "-n 10"
                int pos = str.find("<");
                std::string temp = str.substr(8,pos-2);
                const char* args[] = {op.c_str(), "-n" ,temp.c_str(),ip_file.c_str(),nullptr};
                executeCmd(op.c_str(),ip_file.c_str(),op_file.c_str(),args);
            }
            else if(op=="wc")
            {
                int pos = str.find("<");
                //get wc flags
                std::string temp = str.substr(3,pos-4);
                std::cout<<temp<<std::endl;
                const char* args[] = {op.c_str(),temp.c_str(),ip_file.c_str(),nullptr};
                executeCmd(op.c_str(),ip_file.c_str(),op_file.c_str(),args);
            }        

        return 0;

    }

    //this is valid for cat and normal cmd not for echo
    if(tokens[0]!="echo")
    {
        if(str.find("<")!=std::string::npos && str.find(">")==std::string::npos && str.find(">>")==std::string::npos)
        {
        ip_file = tokens[2];
        op_file = tokens[0];
        }
        else if((str.find(">")!=std::string::npos || str.find(">>")==std::string::npos) && str.find("<")==std::string::npos)
        {
        ip_file = tokens[0];
        op_file = tokens[2];
        }
    }
    else if(tokens[0]=="echo")
    {
        echo=1;
        //this is for echo
        if(orig.find("<")!=std::string::npos && orig.find(">")==std::string::npos && orig.find(">>")==std::string::npos)
        {
        //simply print msg on terminal
        }
        //echo is not used for ">>"
        else if(orig.find(">")!=std::string::npos  && orig.find("<")==std::string::npos)
        {
            int pos = orig.find("\"");
            std::string temp = orig.substr(pos+1);
            int pos2 = temp.find("\"");
            std::string res = temp.substr(0,pos2);
            content = res;
            std::cout<<content<<std::endl;
            int pos1 = str.find(">");
            temp = str.substr(pos1+2);
            //std::cout<<temp<<std::endl;
            op_file = temp;
        }
    }


    //transfer data from ip_file to op_file
    if(!echo)
    {
        std::string sourceFile = ip_file;
        std::string destinationFile = op_file;

        FILE* file = fopen(ip_file.c_str(), "r");
        if (file == nullptr) {
            perror("Error opening file");
            return EXIT_FAILURE;
        }

        // Move the file pointer to the end to determine the file size
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET); // Move the file pointer back to the beginning

        // Allocate memory for the file content
        char* buffer = new char[fileSize + 1]; // +1 for null terminator

        // Read the file content into the buffer
        size_t bytesRead = fread(buffer, 1, fileSize, file);
        if (bytesRead != static_cast<size_t>(fileSize)) {
            perror("Error reading file");
            delete[] buffer;
            fclose(file);
            return EXIT_FAILURE;
        }

        // Null-terminate the buffer
        buffer[fileSize] = '\0';


        //we need to append if op=">>"
        if(str.find(">>")!=std::string::npos)
        {
                FILE* destFile = fopen(op_file.c_str(), "ab");
                if (destFile == nullptr) {
                    perror("Error opening destination file");
                    fclose(file);
                    return EXIT_FAILURE;
                }

                // Write the text to the file
                int retval = fwrite(buffer,1,fileSize,destFile);

                // Close the file
                fclose(destFile);
                fclose(file);
        }
        else if(orig.find(">")!=std::string::npos)
        {

            // Open the file for writing (this will clear its content)
            FILE* fileo = fopen(op_file.c_str(), "w");
            if (fileo == nullptr) {
                perror("Error opening file");
                return EXIT_FAILURE;
            }
            int retval = fwrite(buffer,1,fileSize,fileo);
            fclose(fileo);
            fclose(file);
        }
        //For echo this ("<") will simply prints on terminal and do nothings
        else if(orig.find("<")!=std::string::npos && op_file=="cat")
        {
            // Read and print file contents
            for(int i=0;i<fileSize;i++)
            {
                std::cout<<buffer[i];
            }

            // Close the file
            fclose(file);
        }


    }
    else if(echo)
    {
        if(str.find("<")!=std::string::npos)
        {
            int pos = orig.find("\"");
            std::string temp = orig.substr(pos+1);
            pos = temp.find("\"");
            temp = temp.substr(0,pos);
            std::cout<<temp<<std::endl;
        }
        else if(orig.find(">")!=std::string::npos)
        {
            
            // Write the string to the file
            int pos = orig.find("\"");
            std::string temp = orig.substr(pos+1);
            pos = temp.find("\"");
            temp = temp.substr(0,pos);
            std::cout<<temp<<std::endl;

            int fd = open(op_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH | S_IROTH);

            if (fd == -1) {
                perror("Error opening or creating file");
                return EXIT_FAILURE;
            }

            // Write the string to the file
            ssize_t bytesWritten = write(fd, temp.c_str(), strlen(temp.c_str()));
            if (bytesWritten == -1) {
                perror("Error writing to file");
                close(fd); // Ensure the file is closed before exiting
                return EXIT_FAILURE;
            }

            // Close the file descriptor
            if (close(fd) == -1) {
                perror("Error closing file");
                return EXIT_FAILURE;
            }
            
        }
    }
 

 
 return 1;

}
