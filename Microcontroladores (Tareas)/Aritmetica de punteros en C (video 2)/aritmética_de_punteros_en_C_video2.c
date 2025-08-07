#include <stdio.h>

int main()
{
   /* 
   
   int a = 8;
    int *p = &a;

    printf("La direccion de a es %p\n", &a);
    printf("La direccion de a desde p es %p\n", p);
    printf("El valor de a desde p es %i\n", *p);
    p++;
    printf("El valor de la variable punteada por p es %i\n, *p");
    printf("El valor de p es %p\n", p);

    */

    char c[3] = {'A', 'B', 'C'};
    char *p2 = c;
    for (int i = 0; i < 3; i++)
    {
        printf("Direccion de memoria de c[%i] = %p\n", i, p2 + i);
        p2 + 2;
        printf("Valor de c[%i] = %c\n", i, (p2 + i));
        p2++;
    }
}