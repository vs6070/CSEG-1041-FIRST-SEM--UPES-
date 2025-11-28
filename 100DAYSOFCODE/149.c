// Use malloc() to allocate structure memory dynamically and print details.
// Student allocated dynamically with details: Tina 105 88
// Name: Tina | Roll: 105 | Marks: 88
// Memory for structure is dynamically allocated on heap using malloc().

#include <stdio.h>
#include <stdlib.h>

// Define the structure for Student
struct Student {
    char name[50];
    int roll;
    int marks;
};

int main() {
    // Declare a pointer to struct Student
    struct Student *ptr;

    // Allocate memory dynamically using malloc
    ptr = (struct Student*) malloc(sizeof(struct Student));

    // Check if memory allocation was successful
    if (ptr == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // Read input (Name, Roll, Marks)
    // We access structure members using the arrow operator (->) for pointers
    scanf("%s %d %d", ptr->name, &ptr->roll, &ptr->marks);

    // Print the details in the specified format
    printf("Name: %s | Roll: %d | Marks: %d\n", ptr->name, ptr->roll, ptr->marks);

    // Free the allocated memory
    free(ptr);

    return 0;
}