
## AOS Assignment-2

Here for non-builtin commands, I have created different class (".h" files) for all ".c++" files except "main.c++". For executing these many files we need make file, to execute make file we have to type commands as follows :

```bash
$ make
$ ./a.out
```
To exit from shell we have to type following command :
```bash
exit
```

Here is the directory structure : 

- **2024202001_Assignment2/**: Contains all ".c++" and ".h" and make files.
  - **cd.c** : For "cd" command
  - **echo.c++** : For "echo" command.
  - **io.c++** : For handeling IO redirection.
  - **ls.c++** : For "ls" command.
  - **main.c++** : For operating main shell prompt.
  - **pinfo.c++** : For "pinfo" command.
  - **pipe.c++** : For handeling pipe inputs.
  - **pwd.c++** : For "pwd" command.
  - **search.c++** : For "search" command.
  - **sysCmd.c++** : For handeling foreground and background process.
  - **demo/**: For verification of output.
    - **abc/**: For verification of output.
  - **abc.txt** : For verification of output.
  - **abc2.txt** : For verification of output.
  - **abc3.txt** : For verification of output.
  - **abc4.txt** : For verification of output.



I have created 'main.c++' which operates shell prompt and created objects from class according to requirement. It also handels signals like CTRL+Z, CTRL+C, CTRL+D, up arrow key.

For "ls" command, I have created "ls.c++" which prints list of files according to user requirement. 

For "cd" command, I have created "cd.c++" which uses "chdir()" function to change directory and "getcwd()" for getting current directory.

For "echo" command, I have created "echo.c++" which uses "substr()" and "string.find(character)" functionality to print according to user input.

For IO redirection, I have created "io.c++" which uses "execvp()" for various commands except "echo" and "cat".

For "pinfo" command, I have created "pinfo.c++" which prints information regarding current process.

For pipe handeling, I have created "pipe.c++" which also handels ";" and uses pipe concept to pass output of one command to another command by using "execvp".

For "pwd" command, I have created "pwd.c++" which simply uses "getcwd()" and prints current directory.

For "search" command, I have created "search.c++" which "dirent" structure to findout given filename and returns output accordingly.

For foreground and background process handeling, I have created "sysCmd.c++" which uses "execvp()" and prints process id for background process.

For verification, I have created different files and directories to perform user operations.
 
For "history" command, I have created "history.txt" to store previous commands.