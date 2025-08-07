#include <stdio.h>

int main()
{
    int a = 1, i;
    printf("La direccion de la variable a es %p\n\n", &a);
    int *p = &a;

    for (i = -3; i <= 3; i++)
    {
        printf("El valor de p+%d es: %p, que contiene %d\n", i, p + 1, *(p + i));
    }

    return 0;
}