#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/**
 * Executes a command using the system shell.
 *
 * @param command The command to execute.
 * @return The exit status of the command.
 */
int execute_command(const char *command)
{
    int status = system(command);
    if (status == -1)
    {
        perror("Error executing command");
        exit(EXIT_FAILURE);
    }
    return WEXITSTATUS(status);
}

/**
 * Compares two files for equality.
 *
 * @param file1 Path to the first file.
 * @param file2 Path to the second file.
 * @return 1 if files are identical, 0 otherwise.
 */
int compare_files(const char *file1, const char *file2)
{
    char command[256];
    snprintf(command, sizeof(command), "diff -q %s %s > /dev/null", file1, file2);
    return execute_command(command) == 0;
}

/**
 * Test for transferring and verifying a simple text file.
 */
void test_transfer_simple_file()
{
    printf("Starting simple file transfer test...\n");

    // Assuming the receiver and sender are compiled as receiver and sender respectively.
    // Start receiver in the background
    if (fork() == 0)
    {
        execl("./receiver", "receiver", "12345", "received.txt", "0");
        exit(EXIT_FAILURE); // execl only returns on error
    }

    // Give the receiver a moment to start up
    sleep(1);

    // Start sender
    execute_command("./sender 127.0.0.1 12345 testfile.txt 10");

    // Wait for the receiver to finish
    wait(NULL);

    // Compare the original file with the received file
    if (compare_files("testfile.txt", "received.txt"))
    {
        printf("Simple file transfer test passed.\n");
    }
    else
    {
        printf("Simple file transfer test failed.\n");
    }
}

int main()
{
    test_transfer_simple_file();
    // Add more test calls here

    return 0;
}
