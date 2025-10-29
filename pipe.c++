#include "pipe.h"
#include <iostream>
#include<bits/stdc++.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>


//Fot IO redirection plus pipe, this will get arguments for IO redirection
void parseCommand(const std::string& command, std::vector<std::string>& args, std::string& inputFile, std::string& outputFile) {
    std::string trimmedCommand = command;
    // Remove spaces around '<' and '>'
    trimmedCommand.erase(trimmedCommand.find_last_not_of(" \t") + 1);
    trimmedCommand.erase(0, trimmedCommand.find_first_not_of(" \t"));

    size_t inputPos = trimmedCommand.find('<');
    if (inputPos != std::string::npos) {
        inputFile = trimmedCommand.substr(inputPos + 2);
        //std::cout<<inputFile<<std::endl;
        trimmedCommand = trimmedCommand.substr(0, inputPos);
    }

    size_t outputPos = trimmedCommand.find('>');
    if (outputPos != std::string::npos) {
        outputFile = trimmedCommand.substr(outputPos + 2);
        //std::cout<<outputFile<<std::endl;
        trimmedCommand = trimmedCommand.substr(0, outputPos);
    }

    std::istringstream iss(trimmedCommand);
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
}




//For only IO redrection this will trim spaces
std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t");
    auto end = str.find_last_not_of(" \t");
    
    if (start == std::string::npos || end == std::string::npos) {
        return "";
    }
    
    return str.substr(start, end - start + 1);
}


//For IO redirection only 
std::vector<std::string> Pipe_class::pipe(std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        result.push_back(trim(item)); // Trim and add to the result vector
    }



    int numCommands = result.size();
    int pipefds[2 * (numCommands - 1)];

    // Create pipes
    for (int i = 0; i < numCommands - 1; ++i) {
        if (::pipe(pipefds+i*2) == -1) {
            perror("pipe");
            exit(1);
        }
    }

    for (int i = 0; i < numCommands; ++i) {
        if (fork() == 0) {
            // Child process

            // Set up the input pipe for the first command
            if (i != 0) {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }

            // Set up the output pipe for all but the last command
            if (i != numCommands - 1) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }

            // Close all pipe file descriptors
            for (int j = 0; j < 2 * (numCommands - 1); ++j) {
                close(pipefds[j]);
            }

            // Tokenize the command string
            std::vector<char*> args;
            std::string command = result[i];
            std::stringstream ss(command);
            std::string arg;
            while (ss >> arg) {
                args.push_back(strdup(arg.c_str()));
            }
            args.push_back(nullptr); // Null-terminate the argument list

            // Execute the command
            execvp(args[0], args.data());

            // If execvp fails
            perror("execvp");
            exit(1);
        }
    }

    // Parent process closes all pipe file descriptors
    for (int i = 0; i < 2 * (numCommands - 1); ++i) {
        close(pipefds[i]);
    }

    // Wait for all child processes to finish
    for (int i = 0; i < numCommands; ++i) {
        wait(nullptr);
    }

    return result;
}


//For IO redirection plus pipe
void Pipe_class::pipe2(std::string& res) {
    std::vector<std::string> semi;
    std::string cmd;
    std::istringstream iss(res);
    


    //To get cmds for every ";"
    while (std::getline(iss, cmd, ';')) {
        semi.push_back(cmd);
    }

    for(int i=0;i<semi.size();i++)
    {
        std::istringstream iss(semi[i]);
        std::string command;
        std::vector<std::string> commands;

        //To get cmds for every pipeline
        while (std::getline(iss, command, '|')) {
            commands.push_back(command);
        }

        // Trim leading and trailing whitespace from each command
        for (auto& cmd : commands) {
            cmd.erase(0, cmd.find_first_not_of(" \t"));
            cmd.erase(cmd.find_last_not_of(" \t") + 1);
        }
        int numCommands = commands.size();
        int pipefds[2 * (numCommands - 1)];

        // Create pipes
        for (int i = 0; i < numCommands - 1; ++i) {
            if (::pipe(pipefds + i * 2) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < numCommands; ++i) {
            std::vector<std::string> args;
            std::string inputFile, outputFile;

            // Parse command to handle redirections
            parseCommand(commands[i], args, inputFile, outputFile);

            if (fork() == 0) {
                // Child process

                // Handle input redirection
                if (!inputFile.empty()) {
                    int fd = open(inputFile.c_str(), O_RDONLY);
                    if (fd == -1) {
                        perror("open input file");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(fd, STDIN_FILENO) == -1) {
                        perror("dup2 input");
                        exit(EXIT_FAILURE);
                    }
                    close(fd);
                }

                // Handle output redirection
                if (!outputFile.empty()) {
                    int fd = open(outputFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open output file");
                        exit(EXIT_FAILURE);
                    }
                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2 output");
                        exit(EXIT_FAILURE);
                    }
                    close(fd);
                }

                // Set up pipes
                if (i != 0) {
                    if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) == -1) {
                        perror("dup2 input pipe");
                        exit(EXIT_FAILURE);
                    }
                }
                if (i != numCommands - 1) {
                    if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) == -1) {
                        perror("dup2 output pipe");
                        exit(EXIT_FAILURE);
                    }
                }

                // Close all pipe file descriptors
                for (int j = 0; j < 2 * (numCommands - 1); ++j) {
                    close(pipefds[j]);
                }

                // Prepare arguments for execvp
                std::vector<char*> execArgs;
                for (const auto& arg : args) {
                    execArgs.push_back(strdup(arg.c_str()));
                }
                execArgs.push_back(nullptr);

                execvp(execArgs[0], execArgs.data());

                // If execvp fails
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }

        // Parent process closes all pipe file descriptors
        for (int i = 0; i < 2 * (numCommands - 1); ++i) {
            close(pipefds[i]);
        }

        // Wait for all child processes to finish
        for (int i = 0; i < numCommands; ++i) {
            wait(nullptr);
        }

    }
}
