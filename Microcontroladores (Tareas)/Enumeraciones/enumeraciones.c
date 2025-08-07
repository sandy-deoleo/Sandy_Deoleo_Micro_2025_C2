#include <stdio.h>
#include <stdlib.h>

//La enumeracion se inicializa en cero si no se le coloca vaor, y el siguiente le suma 1 a ese valor anterior
//Si se le coloca valor el siguiente le suma 1 a ese valor anterior y lo toma

enum dia_de_la_semana
{
    DOMINGO = 193,
    LUNES,
    MARTES,
    MIERCOLES = 0,
    JUEVES,
    VIERNES, 
    SABADO
};

int main (void)
{
    int x;
    x = MARTES;

    switch (x)
    {
    case DOMINGO:
    printf("DOMINGO\n");
        break;

     case LUNES:
     printf("LUNES\n");
        break;

    case MARTES:
    printf("MARTES\n");
        break;

    case MIERCOLES:
    printf("MIERCOLES\n");
        break;

    case JUEVES:
    printf("JUEVES\n");
        break;

    case VIERNES:
    printf("VIERNES\n");
        break;

    case SABADO:
    printf("SABADO\n");
        break;

    default:
    printf("ESTE DIA NO EXISTE(TA' INVENTANDO)");
        break;
    }

    printf("%d\n", MARTES);
}