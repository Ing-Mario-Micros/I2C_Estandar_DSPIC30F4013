/* 
 * File:   I2C.h
 * Author: mario
 *
 * Created on 24 de agosto de 2022, 10:29 PM
 */

#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif
#include<xc.h>
#define FCY 1842500
#include <libpic30.h>
/*--------------------------Funcion de Configuración I2C------------------*/
void Activar_I2C (void);
/*------------------------------ Funciones I2C Esclavo ---------------------*/
void Interrupcion_Recepcion_I2C(void);
void Recepcion_I2C(void);
/*------------------------------ Funciones I2C Maestro ---------------------*/
void Lectura_Dir (void);

void Start (void);
void Restart (void);
void Stop (void);


/**---- Variables de funcionamiento ----**/
char contador=0;
char recepcion =0;
/**Variables de comunicación Esclavo**/
unsigned char Tipo=0, Fase=0;
unsigned char Registro_Temp, Dato_Temp;
char Error ;
char Vector_Datos[25];

unsigned char Contador=0, Apuntador=0; //Variables del sistema para I2C Esclavo




/*--------------------------Funcion de Configuración I2C------------------*/
void Activar_I2C (void){
    /***CONFIGURACIÓN I2C***/
    /*La formula necesaria para deffinir el valor correspondiente al registro 
     * I2CBRG es: I2CBRG= ((FCY/FI2C)-(FCY/1111111))-1 con esto se logran 
     * definir la velocidad del Bus*/
    I2CBRG=44;//Configuración de velocidad a 100kHz
    I2CCON=0x9100; //Registro de configuración I2C
    /**** I2C Esclavo ****/
    /*-la direcion definida tiene un corrimiento de 1 hacia la izquierda donde
     * al definir una dirección como 0x08 la dirección con la cual se identifica
     * el DSPIC es 0x10 ya que tuvo un corrimiento de de uno cuando se cargo al
     * registro fisico del micro I2CRSA-*/
    I2CADD = 0x08; //Registro de dirección I2C, Direccion de identificación 0x10
    /**** Interrupción I2C ****/
    _SI2CIE = 1; //Activación de interrupción I2C modo esclavo
    _SI2CIF=0;   //Inicialización de bandera de interrupción en 0
}
/*------------------------------ Funciones I2C Esclavo ---------------------*/
void Interrupcion_Recepcion_I2C(void){
    char aux = 0;
    if(_R_W == 0){
        if(_D_A == 1){
            switch(Tipo){
                case 0:
                    if(I2CRCV>=0 && I2CRCV<=13){
                        Tipo=1;
                    }
                    else{
                        Tipo=2;
                    }
                    Registro_Temp=I2CRCV;
                    Fase=0;
                    break;
                case 2:
                    if(Fase==0){
                        Dato_Temp=I2CRCV;
                        Fase=1;
                    }
                    else{
                        if(Fase==2){
                            if(I2CRCV==2){
                                Vector_Datos[Registro_Temp]=Dato_Temp;
                                Fase=3;
                                Tipo=0;
                            }
                            else{
                                Error = Error & 0x08;
                            }
                        }
                    }
                    break;
            }
        }
        else{
            aux=I2CRCV;
        }
    }
    else{
        if(_SCLREL==0){
            if(Tipo==1){
                if(Fase==0){
                    I2CTRN=Vector_Datos[Registro_Temp];
                    while(_TBF==0);
                    _SCLREL=1;
                    Fase=1;
                }
                else{
                    if(Fase==1){
                        I2CTRN=Registro_Temp;
                        while(_TBF==0);
                        _SCLREL=1;
                        Fase=2;
                        Tipo=0;
                    }
                }
            }
            else{
                if(Tipo==2){
                    I2CTRN=Registro_Temp;
                    while(_TBF==0);
                    _SCLREL=1;
                    Fase=2;
                }
            }
        }
        aux = I2CRCV;
    }

    _SI2CIF=0;
}
/* Está es la función estandar de I2C que permite que cualquier dispositivo I2C 
 * pueda interactura correctamente con este dispositivo reconociendoce con el 
 * ladirección y un registro en el cual se guardan los datos llamado Vector de 
 * datos en el cual se guardan los valores enviados desde el controlador externo
 */
void Recepcion_I2C(void){
    char aux = 0;
    if(_R_W == 0){//Si el bit R_W esta en cero la dirección fue de escritura
        if(_D_A == 1){//Si el bit D_A es igual a 1 el valor resivido es un dato
            if(Contador==0){
                Apuntador=I2CRCV;//Guardado de la posición del vector en apuntador
                Contador=1; //Incremento para fase de guardado
            }
            else if(Contador==1){
                Vector_Datos[Apuntador]=I2CRCV;//Guardado de dato en vector de datos
                Contador=0; //reinicio de sistema de guardado
            }
        }
        else{
            /* En el caso que no sea un dato el valor resivido se guarda 
             * lo resivido en en Buffer de entrada I2CRCV en una variable
             * temporal para la evitar tener colición de datos
             */
            aux=I2CRCV;
        }
    }
    else{//la dirección fue de Lectura
        if(_SCLREL==0){
            I2CTRN=Vector_Datos[Apuntador];//Cargar dato al registro de envio
            while(_TBF==0);//Esperar a que el registro de envio esté lleno
            _SCLREL=1;     //Liberar Bus I2C
            Apuntador++;   //Aumentar el apuntador
            Contador=0;    //Reiniciar el sistema
        }
        aux=I2CRCV;
    }
    if(_P==1){
        Contador=0;    //Reiniciar el sistema
    }
    _SI2CIF=0; // Bandera de Interrupción en 0 Para que vuelva a ocurrir
}
/*------------------------------ Funciones I2C Maestro ---------------------*/
void Lectura_Dir (void){
    Start();
    /** Envio de Direccíon modo Escritura hacia el esclavo **/
    I2CTRN=0x0A;
    while(_TRSTAT == 1 && _TBF ==1);
    if(_ACKSTAT == 1){
        Error = Error|0x01;
    }
    /** Envio de Dato hacia el esclavo **/
    I2CTRN=contador;
    _MI2CIF = 0;
    while(_TRSTAT == 1 && _TBF == 1);
    __delay_us(1);
    if(_ACKSTAT == 1){
        Error = Error | 0x02;
    }
    /** Directiva de Reinicio **/
    Restart();
    /** Envio de Direccíon modo lectura hacia el esclavo **/   
    I2CTRN=0x0B;
    _MI2CIF = 0;
    while(_TRSTAT == 1 && _TBF == 1);
    __delay_us(1);
    if(_ACKSTAT == 1){
        Error = Error | 0x04;
    }
    /** Recepción de dato desde el esclavo **/
    _RCEN = 1;
    while(_RCEN == 1);
    _ACKDT = 1; //NACK
    _ACKEN = 1;
    _MI2CIF = 0;
    while(_ACKEN == 1);
    recepcion = I2CRCV;
    /** Directiva para terminar Comunicación **/
    Stop();
}
/*------------------------- Sistemas Propios I2C ----------------------------*/
void Start (void){
    if(_P ==1 || _RF3 == 1){
        _SEN = 1;
        while(_SEN==1);
    }
}
void Restart (void){
    _RSEN=1;
    while(_RSEN == 1);
}
void Stop (void){
    _PEN = 1;
    while(_PEN == 1);
}
#endif	/* I2C_H */

