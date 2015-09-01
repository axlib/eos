//
//  axOSTerminal.cpp
//  Minimal
//
//  Created by Alexandre Arsenault on 2015-08-30.
//  Copyright (c) 2015 axLib. All rights reserved.
//

#include "axOSTerminal.h"

#include <unistd.h>

//#define _XOPEN_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pty.h>
#include <utmp.h>
//#include <util/util.h>
//#include <libutil.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include <fcntl.h>

ax::os::Terminal::Terminal(ax::Event::Manager* manager):
ax::Event::Object(manager)
{
    
}

bool ax::os::Terminal::Execute(const std::string& program_folder_path,
                               const std::string& program_name,
                               const ax::StringVector& args)
{
    int fd_master = posix_openpt(O_RDWR | O_NOCTTY);
    
    int gd = grantpt(fd_master);
    
    if(fd_master == -1)
    {
        ax::Error("Can't open terminal.");
    }
    
    // Unlock PTY.
    if(unlockpt(fd_master) == -1)
    {
        ax::Error("Can't unlock pty.");
    }
    
    const char* pts = ptsname(fd_master);
    
    if(pts == NULL)
    {
        ax::Error("Can't get pty name.");

    }

//    int filedes[2]; // [0] Read, [1] Write.
//
//    if (pipe(filedes) == -1)
//    {
//        ax::Error("Can't creare pipe.");
//        return false;
//    }
    
    int fd_slave = open(pts, O_RDWR);
    
    if(fd_slave == -1)
    {
        ax::Error("Can't open slave pts.");
    }
    
    // Create fork to execute program.
    pid_t pid = fork();
    
    // Fork error.
    if (pid == -1)
    {
        ax::Error("Can't create fork process.");
        return false;
    }
    
    // Child process.
    else if (pid == 0)
    {
        int err = -1;
        
//        do
//        {
//            // Copy write file descriptor as standard output.
//            err = dup2(filedes[1], STDOUT_FILENO);
//            
//        } while(err == -1 && errno == EINTR);
        
        if(close(fd_master))
        {
            ax::Error("Can't close master pty.");
        }
        
        close(0); // Close standard input (current terminal)
        close(1); // Close standard output (current terminal)
        close(2); // Close standard error (current terminal)
        
        if(login_tty(fd_slave) == -1)
        {
            ax::Error("Can't login slave pts.");
        }
        
        dup2(fd_slave, STDIN_FILENO); // PTY becomes standard output (1)
        dup2(fd_slave, STDOUT_FILENO); // PTY becomes standard output (1)
        dup2(fd_slave, STDERR_FILENO); // PTY becomes standard error (2)

        close(fd_slave);

        // Add program name as first argument and copy all
        // std::string args in ax::StringVector to
        // std::vector<const char*> (needed to call execv).
        std::vector<const char*> c_args(args.size() + 2);
        
        // Add program path to arguments.
        c_args[0] = program_name.data();
        
        // Copy all arguments from args.
        std::transform(args.begin(), args.end(),
                       // Start adding after program path.
                       c_args.begin() + 1,
                       [](const std::string& arg)
                       {
                           return arg.c_str();
                       });
        
        // Add null character at the end off c_args.
        c_args.back() = (char*)0;
        
        std::string path = program_folder_path + program_name;
        
        // Call program.
        err = execv(path.c_str(), (char* const*) c_args.data());
        
        // This should never be called except in case of error.
        ax::Error("Can't execute program", program_name);
        
        // Not sure where goes the return value in child process.
        return false;
    }
    
       // Parent process.
    close(fd_slave);

//    write(fd_slave, "pwd\n", 4);

//    sleep(4);
//    write(fd_master, "pwd\n", 4);
//    ax::Print("TEXT");
//    write(fd_master, "pwd\n", 4);
    
    
    // Close write file descriptor.
//    close(filedes[1]);
    _fd_master = fd_master;
    
    // Return true if the thread object identifies an active thread
    // of execution, false otherwise.
    if(_read_child_thread.joinable() == false)
    {
        _read_child_thread = std::thread(&ReadFromChild,
                                         std::ref(*this),
                                         fd_master);
                                         //filedes[0]);
        _read_child_thread.detach();
    }

    
//    write(fd_master, "pwd\n", 4);
    // File descriptor filedes[0] will be close at the end of read.
    
    //wait(0);
    
    return true;
}

bool ax::os::Terminal::Write(const std::string& command)
{
     write(_fd_master, command.c_str(), command.size());
    return true;
}

void ax::os::Terminal::ReadFromChild(Terminal& terminal, const int& stdin_fd)
{
    char buffer[4096];
    
    terminal.PushEvent(READ_CHILD_BEGIN, new ax::Event::StringMsg("BEGIN"));
    
    // Read output from child program.
    while (1)
    {
        ssize_t count = read(stdin_fd, buffer, sizeof(buffer));
        
        // Read error.
        if(count == -1)
        {
            // Interrupted function call.
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                ax::Error("Can't read child process.");
                return;
            }
        }
        // End of reading file.
        else if(count == 0)
        {
            terminal.PushEvent(READ_CHILD_END, new ax::Event::StringMsg("END"));
            
            // Close read file descriptor.
            close(stdin_fd);
            return;
        }
        else
        {
            // Process output buffer.
            std::string out_str(buffer, count);
            
            terminal.PushEvent(READ_CHILD_OUTPUT,
                               new ax::Event::StringMsg(out_str));
        }
    }
}
