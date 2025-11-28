// Use pointer to struct to modify and display data using -> operator.
// Student pointer modifying values: John 106 91
// Modified Data: Name: John | Roll: 106 | Marks: 91
// Pointer to structure accesses and modifies data using the arrow (->) operator.

#include <stdio.h>

// Define the structure for Student
struct Student {
    char name[50];
    int roll;
    int marks;
};

int main() {
    // Declare a structure variable
    struct Student s;
    
    // Declare a pointer to the structure and assign it the address of 's'
    struct Student *ptr = &s;

    // Read input (Name, Roll, Marks) using the pointer
    // ptr->name is already a pointer to the array, so no '&' needed
    // &ptr->roll accesses the address of the 'roll' member via the pointer
    scanf("%s %d %d", ptr->name, &ptr->roll, &ptr->marks);

    // Print the details using the pointer and the arrow operator
    printf("Modified Data: Name: %s | Roll: %d | Marks: %d\n", ptr->name, ptr->roll, ptr->marks);

    return 0;
}