#define RED "\033[1;31m"
#define RESET "\033[0m"
#define GREEN "\e[0;32m"

#define PASSEDTEST \
        printf(GREEN "Test Passed\n\n" RESET);

#define FAILEDTEST \
        printf(RED "Test Failed\n\n" RESET);

#define LINE printf("---------------------------------------------------------------------------------------------\n")

