# AOS Assignment-4

This assignment is basically demonstrate how git works or version control system. I have created single file "main.cpp" for compilation. I have included "makefile" to compile code. I have also included file "test.txt" and directory "test_tree" for verification.

We have to run commands as follows to implement this assignment : 

```
make
./mygit init
./mygit hash-oject [-w] <file name>
./mygit cat-file [flag : -p / -s / -t] <hash value of file>
./mygit write-tree
./mygit ls-tree [--name-only] <hash value of tree>
./mygit add .
./mygit add <specific filenames>
./mygit commit -m "Commit message"
./mygit commit
./mygit log
./mygit checkout <hash value of commit object>
```

If we want to run this code from another directory then follow this command :

```
/home/purva/purva/AOS/Assignment\ 4/2024202001_Assignment4/mygit.sh /home/purva/purva/AOS/Assignment\ 4/2024202001_Assignment4/mygit <commands>

```

here I have assumed that my "main.cpp" , "mygit.sh" , "mygit"(executable file) files are located at path : "/home/purva/purva/AOS/Assignment\ 4/2024202001_Assignment4" and inplace of command use : init , log , commit etc.

In short for different directory we have to run "mygit.sh" and as an argument we have to give path of executable file "mygit" and command which we want to run. Follow this formate :

```
 <path of mygit.sh> <path of mygit> <commands>
```
