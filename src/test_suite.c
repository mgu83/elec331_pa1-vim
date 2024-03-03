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
 * @brief Create a large test file object
 *
 * @param filename
 * @param size
 */
void create_large_test_file(const char *filename, size_t size)
{
    FILE *file = fopen(filename, "w");
    if (!file)
    {
        perror("Failed to open file for writing");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < size; ++i)
    {
        fputc('A' + (i % 26), file); // Cycle through 'A' to 'Z'
    }

    fclose(file);
}

/**
 * Test for transferring and verifying a simple text file.
 */
void test_transfer_simple_file()
{
    printf("Starting simple file transfer test...\n");

    if (fork() == 0)
    {
        execl("./receiver", "receiver", "12345", "./testfiles/test1_dest.txt", "0");
        exit(EXIT_FAILURE);
    }

    sleep(1);

    execute_command("./sender 127.0.0.1 12345 ./testfiles/test1_start.txt 10");

    wait(NULL);

    if (compare_files("./testfiles/test1_dest.txt", "./testfiles/test1_start.txt"))
    {
        printf("Test 1 passed.\n");
    }
    else
    {
        printf("Test 1 failed.\n");
    }
}

/**
 * @brief Test for verifying single packet transfer
 *
 */
void test_single_packet_transfer()
{
    if (fork() == 0)
    {
        execl("./receiver", "receiver", "12345", "./testfiles/test2_dest.txt", "0");
        exit(EXIT_FAILURE); // execl only returns on error
    }

    sleep(1);

    execute_command("./sender 127.0.0.1 12345 ./testfiles/test2_start.txt 10");
    sleep(1);

    FILE *received_file = fopen("./testfile/test2_dest.txt", "r");
    if (compare_files("./testfiles/test2_dest.txt", "./testfile/test2_start.txt"))
    {
        printf("Test 2 passed.\n");
    }
    else
    {
        printf("Test 2 failed.\n");
    }
}

/**
 * @brief Test for verifying multiple packet transfer
 *
 */
void test_transfer_large_file()
{
    if (fork() == 0)
    {
        execl("./receiver", "receiver", "12345", "./testfiles/test3_dest.txt", "0");
        exit(EXIT_FAILURE); // execl only returns on error
    }

    sleep(1);

    execute_command("./sender 127.0.0.1 12345 ./testfiles/test3_start.txt 10");
    sleep(1);

    FILE *received_file = fopen("./testfile/test3_dest.txt", "r");
    if (compare_files("./testfiles/test3_dest.txt", "./testfiles/test3_start.txt"))
    {
        printf("Test 3 passed.\n");
    }
    else
    {
        printf("Test 3 failed.\n");
    }
}

int main()
{
    test_transfer_simple_file();
    test_single_packet_transfer();
    test_transfer_large_file();

    return 0;
}
