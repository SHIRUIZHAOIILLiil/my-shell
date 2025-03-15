#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

/* Read a line of characters from stdin. */
int getcmd(char *buf, int nbuf)
{
    printf(">>> ");
    // ##### Place your code here
    char *b = buf;
    while (read(0, b, 1))
    {
        if (*b == '\n')
        {
            *b = '\0';
            break;
        }
        else
            b++;
    }

    return 0;
}

/*
  A recursive function which parses the command
  at *buf and executes it.
*/
__attribute__((noreturn))

void
run_command(char *buf, int nbuf, int *pcp)
{

    /* Useful data structures and flags. */
    char *arguments[10];
    int numargs = 0;
    /* Word start/end */
    int ws = -1;
    int we = 0;
    // Record the location of the redirector
    int redirection_left = 0;
    int redirection_right = 0;
    // Record the name of the file to be redirected.
    char *file_name_l = 0;
    char *file_name_r = 0;
    // Detects if the entered command contains a pipe character.
    int pipe_cmd = 0;
    
    int sequence_cmd = 0;

    int i = 0;
    // Pipe counter to keep track of how many pipeliners there are,
    // and also to keep track of how many commands need to be executed.
    int pipe_counter = 0;
    /* Parse the command character by character. */
    for (; i < nbuf; i++)
    {
        /* Parse the current character and set-up various flags:
           sequence_cmd, redirection, pipe_cmd and similar. */

        /* ##### Place your code here. */
        if (buf[i] == ' '|| buf[i] == '|' || buf[i] == ';' 
            || buf[i] == '>' || buf[i] == '<' || buf[i] == '\0')
        {
            if (ws != -1)   
            {
                we = i;
                int word_length = we - ws + 1;
                // Record how long each word is and have it saved to arguments.
                arguments[numargs] = malloc(word_length + 1);
                char temp = buf[ws + word_length - 1];
                // Copies a word into arguments and 
                // does not change the format of the originally entered command in buf.
                buf[ws + word_length - 1] = '\0';
                strcpy(arguments[numargs], &buf[ws]);
                buf[ws + word_length - 1] = temp;
                numargs++;
                ws = -1;
            }
            // Store encountered symbols into arguments as well.
            if(buf[i] == '|' || buf[i] == '>' || buf[i] == '<')
            {
                arguments[numargs] = malloc(2);
                arguments[numargs][0] = buf[i];
                arguments[numargs][1] = '\0';
                numargs++;
            }
            // Record the address each symbol appears.
            if (buf[i] == '<')
            {
                redirection_left = i;
            }

            if (buf[i] == '>')
            {
                redirection_right = i;
            }

            if (buf[i] == '|')
            {
                if (pipe_cmd == 0)
                {
                    pipe_cmd = i;
                }
                pipe_counter++;
            }
            // Record ';' where it appears and assign the buf as two commands.
            if (buf[i] == ';')
            {
                sequence_cmd = i;
                // sequence_nargs = numargs;
                buf[i] = '\0';
                break;
            }
            
            
        }else{
            if (ws == -1) {
                ws = i;
            }
        }

        if (!(redirection_left || redirection_right))
        {
            /* No redirection, continue parsing command. */
            continue;
            // Place your code here.
        }
        else
        {
            /* Redirection command. Capture the file names. */
        }
    }
    // A final truncation indicating the command to be executed.
    arguments[numargs] = 0;
    

    /*
      Sequence command. Continue this command in a new process.
      Wait for it to complete and execute the command following ';'.
    */
    if (sequence_cmd)
    {
        //sequence_cmd = 0;
        // Execute the procedure following the semicolon in the parent program,
        // calling the function for overloading.
        if (fork() != 0) {
          wait(0);
          char *new_path = &buf[sequence_cmd + 1];
          run_command(new_path, strlen(new_path) + 1, pcp);
        }  
    //       // ##### Place your code here.
    }

    /*
      If this is a redirection command,
      tie the specified files to std in/out.
    */
    if (redirection_left)
    {
        // Gain the filename
        for (int j = redirection_left + 1; j < nbuf; j++)
        {
            if (buf[j] != ' ' && buf[j] != '\0')
            {
                file_name_l = &buf[j];
                break;
            }
        }

        for (int k = 0; k < numargs; k++)
        {
            if (*arguments[k] == '<' || *arguments[k] == '\0')
            {
                int count = numargs;
                for (int j = k; j < count; j++)
                {
                    arguments[j] = 0;
                }
                break;
            }
        }
        // Closes standard input, opens the specified file,
        // and points the file descriptor to the file.
        close(0);
        if (open(file_name_l, O_RDONLY) < 0)
        {
            fprintf(2, "cannot open  %s\n", file_name_l);
            exit(1);
        }
        // ##### Place your code here.
    }

    if (redirection_right)
    {
        // ##### Place your code here.
        buf[redirection_right] = 0;
        for (int j = redirection_right + 1; j < nbuf; j++)
        {
            if (buf[j] != ' ' && buf[j] != '\0')
            {
                file_name_r = &buf[j];
                break;
            }
        }
        for (int k = 0; k < numargs; k++)
        {
            if (*arguments[k] == '>' || *arguments[k] == '\0')
            {
                int count = numargs;

                for (int j = k; j < count; j++)
                {
                    arguments[j] = 0;
                }
                break;
            }
        }
        // Closes standard input, opens the specified file,
        // and points the file descriptor to the file.
        // If this file doesn't exist, then create it, and if it does, overwrite what was there before.
        close(1);
        if (open(file_name_r, O_WRONLY | O_CREATE | O_TRUNC) < 0)
        {
            fprintf(2, "cannot open  %s\n", file_name_r);
            exit(1);
        }
    }

    /* Parsing done. Execute the command. */

    /*
      If this command is a CD command, write the arguments to the pcp pipe
      and exit with '2' to tell the parent process about this.
    */
   // 
    if (strcmp(arguments[0], "cd") == 0)
    {
        // ##### Place your code here.
        // If the input command is detected as cd,
        // the folder to be accessed is passed to the parent via a pipe.
        write(pcp[1], arguments[1], strlen(arguments[1]) + 1);
        // Passes a program state to the parent program.
        exit(2);
    }
    else
    {
        /*
          Pipe command: fork twice. Execute the left hand side directly.
          Call run_command recursion for the right side of the pipe.
        */
        if (pipe_cmd)
        {
            // Create a corresponding number of pipes
            // based on the previously detected pipe numbers.
            int m_pipe[pipe_counter][2];
            // Assign words to each instruction based on previously detected pipes.
            char *cmd[10][10];
            int row = 0, column = 0;
            
            for (int j = 0; j < numargs; j++)
            {
               if (strcmp(arguments[j], "|") == 0)
               {
                row++;
                column = 0;
               }else{
                cmd[row][column] = arguments[j];
                column++;
               }
               
            }
            
            for (int j = 0; j < pipe_counter; j++)
            {
                if (pipe(m_pipe[j]) < 0)
                {
                    printf("Pipe creation failed\n");
                    exit(1);
                }
            }
            // 
            for (int j = 0; j <= pipe_counter; j++)
            {
                if (fork() == 0)
                {
                    // If it is not the first command,
                    // then direct the file descriptor to the read end of the previous pipe.
                    if (j > 0) {
                        close(0); 
                        dup(m_pipe[j - 1][0]);
                    }

                   // If it is not the last command,
                   // then direct the file descriptor to the write end of the previous pipe.

                    if (j < pipe_counter) {
                        close(1); 
                        dup(m_pipe[j][1]);
                        
                    }
                    // Close unneeded ports
                    for (int k = 0; k < pipe_counter; k++)
                    {
                        close(m_pipe[k][0]);
                        close(m_pipe[k][1]);
                    }
                    // Execute the current command
                    exec(cmd[j][0], cmd[j]);
                    printf("exec %s failed\n", arguments[0]);
                    exit(1);
                }
                
            }
            for (int k = 0; k < pipe_counter; k++)
                {
                    close(m_pipe[k][0]);
                    close(m_pipe[k][1]);
                }
                // Waiting for the end of the programme
                for (int j = 0; j <= pipe_counter; j++)
                {
                    wait(0);
                }

        }
        // If the command does not have a pipe,
        // it is executed directly.
        else
        {
            // ##### Place your code here.
            exec(arguments[0], arguments);
        }
    }

    exit(0);
}

int main(void)
{
    // Used to receive commands entered by the user.
    static char buf[100];
    // Pipe, if the user executes a cd command,
    // it needs to switch folders in the main programme in time.
    int pcp[2];
    pipe(pcp);
    /* Read and run input commands. */
    while (getcmd(buf, sizeof(buf)) >= 0)
    {
        // If 'exit' is received, exit the current shell.
        if (strcmp(buf, "exit") == 0)
            break;
        // Execute commands received from the user
        if (fork() == 0)
        {
            run_command(buf, 100, pcp);
        }
        else
        {
            // The parent program waits for the result of the child program.
            int child_status;
            wait(&child_status);
            
            if ((child_status) == 2)
            {
                char *open_path = malloc(sizeof(buf));
                read(pcp[0], open_path, sizeof(open_path));
                if (chdir(open_path) != 0)
                {
                    printf("cannot cd %s\n", open_path);
                }
                free(open_path);
            }
            memset(buf, 0, sizeof(buf));
        }
        /*
          Check if run_command found this is
          a CD command and run it if required.
        */
        //  ##### Place your code here
    }

    exit(0);
}
