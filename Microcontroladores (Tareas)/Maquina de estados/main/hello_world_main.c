#include <stdio.h>
#include <stdlib.h>
 
#define ESTADO_INICIAL 0
#define ESTADO_CERRANDO 1
#define ESTADO_ABRIENDO 2
#define ESTADO_CERRADO 3
#define ESTADO_ABIERTO 4
#define ESTADO_ERR 5
#define ESTADO_STOP 6
 
int Func_ESTADO_INICIAL(void);
int Func_ESTADO_CERRANDO(void);
int Func_ESTADO_ABRIENDO(void);
int Func_ESTADO_CERRADO(void);
int Func_ESTADO_ABIERTO(void);
int Func_ESTADO_ERR(void);
int Func_ESTADO_STOP(void);
 
//variables globales de estado
int EstadoSiguiente = ESTADO_INICIAL;
int EstadoActual = ESTADO_INICIAL;
int EstadoAnterior = ESTADO_INICIAL;
 
struct IO
{
    unsigned int LSC:1;//entrada limitswitch de puerta cerrada
    unsigned int LSA:1;//entrada limitswitch de puerta abierta
    unsigned int BA:1;//Boton abrir
    unsigned int BC:1;//Boton cerrar
    unsigned int SE:1;//Entrada de Stop Emergency
    unsigned int MA:1;//Salida motor direccion abrir
    unsigned int MC:1;//Salida motor direccion cerrar
}io;
 
struct STATUS
{
    unsigned int cntTimerCA = 0;//Tiempo de cierre automatico en segundos
    unsigned int cntRunTimer = 0;//Tiempo de rodamiento en segundos
 
}status;
 
struct CONFIG
{
    unsigned int RunTimer = 180;
    unsigned int TimerCA  = 120;
}config;
 
int main()
{
 
 
    for(;;)
    {
        if(EstadoSiguiente == ESTADO_INICIAL)
        {
            EstadoSiguiente = Func_ESTADO_INICIAL();
        }
 
        if(EstadoSiguiente == ESTADO_ABIERTO)
        {
            EstadoSiguiente = Func_ESTADO_ABIERTO();
        }
 
        if(EstadoSiguiente == ESTADO_ABRIENDO)
        {
            EstadoSiguiente = Func_ESTADO_ABRIENDO();
        }
 
        if(EstadoSiguiente == ESTADO_CERRADO)
        {
            EstadoSiguiente = Func_ESTADO_CERRADO();
        }
 
        if(EstadoSiguiente == ESTADO_CERRANDO)
        {
            EstadoSiguiente = Func_ESTADO_CERRANDO();
        }
 
        if(EstadoSiguiente == ESTADO_ERR)
        {
            EstadoSiguiente = Func_ESTADO_ERR();
        }
 
        if(EstadoSiguiente == ESTADO_STOP)
        {
            EstadoSiguiente = Func_ESTADO_STOP();
        }
 
    }
    return 0;
}
int Func_ESTADO_INICIAL(void)
{
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_INICIAL;
 
    //verifica si existe un error
    if(io.LSC == true && io.LSA == true)
    {
        return ESTADO_ERR;
    }
    //puerta cerrada
    if(io.LSC == true && io.LSA == false)
    {
        return ESTADO_CERRADO;
    }
    //puerta abierta
    if(io.LSC == false && io.LSA == true)
    {
        return ESTADO_CERRANDO;
    }
 
    //puerta en estado desconocido
    if(io.LSC == false && io.LSA == false)
    {
        return ESTADO_STOP;
    }
}
int Func_ESTADO_CERRANDO(void)
{
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_CERRANDO;

      //funciones de estado estaticas (una sola vez)
    status.cntRunTimer = 0;//reinicio del timer
    io.MA = false;
    io.MC = true;
    io.BA = false;
    io.BC = false;
    //ciclo de estado
    for(;;)
    {
        //Verifica posicion de botones
        if(io.BA == true || io.BC == true)
        {
          return ESTADO_STOP;
        }

        //Verifica si la puerta llego a su posicion final
        if(io.LSA == false && io.LSC == true)
        {
            return ESTADO_CERRADO;
        }
 
        //verifica error de run timer (OVER TIME)
        if(status.cntRunTimer > config.RunTimer)
        {
            return ESTADO_ERR;
        }
    }
}
int Func_ESTADO_ABRIENDO(void)
{
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_ABRIENDO;
    //funciones de estado estaticas (una sola vez)
    //Activamos o desactivamos una vez estemos en estado

    status.cntRunTimer = 0;//reinicio del timer
    io.MA = true;
    io.MC = false;
    io.BA = false;
    io.BC = false;
     
    //ciclo de estado
    for(;;)
    {
        //Verifica si la puerta llego a su posicion final
        if(io.LSA == true && io.LSC == false)
        {
           return ESTADO_ABIERTO;
        }

        //Verifica posicion de los botones
        if(io.BA == true || io.BC == true)
        {
          return ESTADO_STOP;
        }

        //verifica error de run timer
        if(status.cntRunTimer > config.RunTimer)
        {
            return ESTADO_ERR;
        }
    }
}
int Func_ESTADO_CERRADO(void)
{
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_CERRADO;
    //funciones de estado estaticas (una sola vez)
    //Activamos o desactivamos una vez estemos en estado
    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    //ciclo de estado
    for(;;)
    {
        //Verfica posicion de los botones
        if(io.BA == true && io.BC == false)
        {
            return ESTADO_ABRIENDO;
        }
    }
}
int Func_ESTADO_ABIERTO(void)
{
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_ABIERTO;
     //funciones de estado estaticas (una sola vez)
    //Activamos o desactivamos una vez estemos en estado
    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    //ciclo de estado
    for(;;)
    {
        //Verfica posicion de los botones
        if(io.BA == false && io.BC == true)
        {
            return ESTADO_CERRANDO;
        }
    }
}
int Func_ESTADO_ERR(void)
{
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_ERR;
     //funciones de estado estaticas (una sola vez)
    //Activamos o desactivamos una vez estemos en estado
    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    //ciclo de estado
    for(;;)
    {
        
    }
}
int Func_ESTADO_STOP(void)
{
    EstadoAnterior = EstadoActual;
    EstadoActual = ESTADO_STOP;
     //funciones de estado estaticas (una sola vez)
    //Activamos o desactivamos una vez estemos en estado
    io.MA = false;
    io.MC = false;
    io.BA = false;
    io.BC = false;

    //ciclo de estado
    for(;;)
    {
        //Acciona boton para salir de estado
        if(io.BA == true && io.BC == false)
        {
            return ESTADO_ABRIENDO;
        }

        //Acciona boton para salir de estado
        if(io.BA == false && io.BC == true)
        {
            return ESTADO_CERRANDO;
        }
    }
}