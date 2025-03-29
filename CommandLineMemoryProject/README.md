Goal
This project is as a refresher for building software in ‘C’. In particular, the project involves  Linux system calls, buffering file I/O, memory management, and C-string parsing.

I write a program, memory, that provides a get/set memory abstraction for files in a Linux directory. My program should take a command from stdin and carries out the command in the current working directory. Below, are some examples of how memory works, outlines the commands formally, describes how I handle invalid commands, and provides notes about other functionality and limitations of memory.

Examples
1. Example 1. In this example, the user passes the string “get\nfoo.txt\n” to stdin. memory outputs the contents of foo.txt (“Hello from foo”) to stdout and then exits with a return code of 0.
2. Example 2. In this example, the user passes the string “set\nbaz.txt\n12\nHello World!” to stdin. memory should create a new file, named baz.txt, in the current directory, write “Hello World!” to baz.txt, write the message OK to stdout, and then exit with a return code of 0.
3. Example 3. In this example, the user passes the string “set\nfoo.txt\n12\nHello World!” to stdin. memory should change the contents of foo to “Hello World!”, write the message OK to stdout, and then exit with a return code of 0.
4. Example 4. In this example, a user passes the string “invalid\nfoo.txt” to stdin. memory should output the message “Invalid Command\n” to stderr, and exit with a return code of 1