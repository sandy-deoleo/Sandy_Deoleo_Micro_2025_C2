#include <stdio.h>
#include <stdlib.h>

void crear_un_arreglo_entero( int * puntAInt, int dim);
int * crear_un_arreglo_entero2(int * puntAInt, int dim);
int crear_un_arreglo_entero3(int ** puntAInt, int dim);
void mostrar (int *a, int val);
int main()
{
    int arreglo [10];
    int *punt;
    int val = crear_un_arreglo_entero3(&punt, 10);
    mostrar (punt, val);
    return 0;
}

//Primera funcion
void crear_un_arreglo_entero(int * puntAInt, int dim)
{
    puntAInt = (int *)malloc (dim * sizeof (int));
    int i;
    for (i = 0; i < 10; i++)
    {
        puntAInt[i] = i;
    }
    printf("\n");
    mostrar (puntAInt, 10);
}

//Segunda funcion
int * crear_un_arreglo_entero2(int * puntAInt, int dim)
{
    puntAInt = (int *)malloc (dim * sizeof (int));
    int i;
    for (i = 0; i < 10; i++)
    {
        puntAInt[i] = i;
    }
    printf("\n");
    return puntAInt;
}

//Tercera funcion
int crear_un_arreglo_entero3(int ** puntAInt, int dim)
{
    *puntAInt = (int *)malloc (dim * sizeof (int));
    int i;
    for (i = 0; i < 10; i++)
    {
        (*puntAInt) [i] = i;
    }
    printf("\n");
    return i;
}

void mostrar(int a[], int val)
{
    int i;
    for(i = 0; i < val; i++)
    {
        printf("| %d |", a[i]);
    }
}