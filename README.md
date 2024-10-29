# Operating_Systems_Simple_Shell


 Custom Shell Implementation

 LAB 1

This assignment involves implementing a custom Command Line Interface (CLI) shell in C. The shell emulates fundamental functionalities of standard Unix shells, allowing users to execute commands, manage processes, handle input/output redirection, and utilize pipelines.

---

Prerequisites

Before proceeding, ensure you have the following:

- Access Credentials: Valid SSH credentials (`username` and `password`) for the server `courserv01.luddy.iupui.edu`.
- Development Environment: A Unix-like environment with GCC and Make installed.
- Project Files: Ensure all project files (`shell.c`, `Makefile`, etc.) are present in the designated directory on the server.

---

Features

Error Handling

Detects and reports invalid commands with appropriate error messages.
File Redirection

Input Redirection (<): Redirects the input of a command from a file.
Output Redirection (>): Redirects the output of a command to a file.
Multiple Pipes (|)

Supports chaining multiple commands using pipes to pass the output of one command as the input to the next.
Background Process Execution (&)

Executes commands in the background, allowing the shell to accept new commands without waiting for the background process to complete.
Maintains a list of background process PIDs, which can be viewed using the bgpids command.
Supports waiting for background processes to complete using the wait command.
Additional Functionalities

Built-in commands: pid, ppid, pidof, bgpids, wait, and exit

------------

Connecting to the Server

To begin, you'll need to connect to the provided server using SSH. Follow these steps:

1. Open Terminal: Launch your terminal application.

2. Connect via SSH:

    Use the following command to connect to the server. Replace `shiyadav` with your actual username if different.


    ssh shiyadav@courserv01.luddy.iupui.edu
    

3. Enter Password:

When prompted, enter your password. Note that the password input will be hidden for security reasons.

   

4. Successful Connection:

Upon successful authentication, you will be logged into the server and presented with a shell prompt similar to:


    shiyadav@courserv01:~$
   



Navigating to the Project Directory

Once connected to the server, navigate to the root directory of your project. Assuming your project is located in `Assignment/Assignment 1`, follow these steps:

1. Change Directory:

    Use the `cd` command to navigate to your project directory. If your directory name contains spaces, escape them using a backslash (`\`) or enclose the path in quotes.

    
    cd Assignment/Assignment\ 1
   

2. Verify Location:

    Confirm you're in the correct directory by listing its contents:


    ls


Compilation and Execution

To compile the custom shell, utilize the provided `Makefile`. This simplifies the build process and ensures all necessary flags and dependencies are correctly handled.

1. Run Make Command:

    
    make
    

    **Expected Output**:

    ```
    gcc -Wall -Wextra -std=c99 -c shell.c -o shell.o
    gcc -Wall -Wextra -std=c99 -o shell shell.o
    ```

    This compiles `shell.c` into an object file `shell.o` and then links it to create the executable `shell`.


2. Running the Shell:

 After successful compilation, launch the custom shell using the `make run` command.

    
    make run
  


    Expected Shell Prompt:

    
    Shiyadav@iu> 
    

---

Using the Custom Shell

Once the shell is running, you can utilize its various functionalities. Below is a guide to using the shell effectively.

 Executing Commands

You can execute standard Unix commands just as you would in a typical shell.

- Example:

    
    Shiyadav@iu> ls
    

    Output:

    ```
    finaltest.txt  ls_output.txt  os-lab-1-cli-1.pdf  result.txt  shell.c    sorted.txt  tester.txt   textfiles.txt
    input.txt      lsOutput.txt   output.txt          script.sh   shell.exe  temp.txt    test2.txt   unique_sorted.txt
    IsOutput.txt   Makefile       readonly.txt        shell       shell.o    testing.txt  unsorted.txt 
    ```


Exiting from Custom Shell 

use command 
exit


**Note**: do not use " " while providing string inputs .
example 1: grep pattern , instead of grep "pattern"
example 2: echo hello world , instead of echo "hello world"

Contents of the zip/tar file 

Assignment 1 ----copy of files on shiyadav@courserv01.luddy.iupui.edu
README.md
shell.c
MAKEFILE

-------------------------------------------------------------
Some Of the Sample Testcases and Commands for my shell testing based on the files in Assignment 1 folder



ls
command < input_file (example:  cat < input.txt)
ls > ls_output.txt
cat ls_output.txt
cat < input.txt | grep patterns | sort | uniq | head -n 5 | wc > result.txt &
sleep 10 &
date
ps e
pstree -p
pid
ppid
pidof shell
bgpids
wait
exit

