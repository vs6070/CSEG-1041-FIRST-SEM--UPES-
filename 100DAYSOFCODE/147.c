// Q147: Store employee data in a binary file using fwrite() and read using fread().

/*
Sample Test Cases:
Input 1:
Employee details entered and stored in file.
Output 1:
Displays employee data read from file.

*/
#include <stdio.h>
struct Employee
{
    char name[50];
    int id;
    float salary;
};

int main()
{
    struct Employee e, e_read;
    FILE *fp;

    printf("Enter Employee Name, ID, Salary:\n");
    scanf("%s %d %f", e.name, &e.id, &e.salary);

    fp = fopen("employee.dat", "wb");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    fwrite(&e, sizeof(struct Employee), 1, fp);
    fclose(fp);

    fp = fopen("employee.dat", "rb");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return 1;
    }
    fread(&e_read, sizeof(struct Employee), 1, fp);
    fclose(fp);

    printf("Name: %s | ID: %d | Salary: %.2f\n", e_read.name, e_read.id, e_read.salary);

    return 0;
}