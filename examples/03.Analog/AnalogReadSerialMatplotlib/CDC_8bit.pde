/*-----------------------------------------------------
Author: Yeison Cardona --<yeison.eng@gmail.com>
First release: 2012-05-03
Last release: 2014-10-13
Description: Pinguino control via CDC commands
-----------------------------------------------------*/

#include <stdlib.h>
#define TotalPines 28

char lectura[25], *bloque;
int par1, valor, cont, i;

char *strtok(char *str, char *control)
{
    //Written By -  Sandeep Dutta . sandeep.dutta@usa.net (1999)
    static  char * s;
    register char * s1;
    
    if (str)s = str; if (!s)return NULL;
    while (*s)
    {
        if (strchr(control,*s))
            s++;
        else
            break;
    }
    s1 = s;
    while (*s)
    {
        if (strchr(control,*s))
        {
            *s++ = '\0';
            return s1;
        }
        s++;
    }

    s = NULL;
    if (*s1)
        return s1;
    else
        return NULL;
}

void setup()
{
}

void leer_cadena()
{
    unsigned char valor_leido[25];
    unsigned char receivedbyte;
    int cont=1;
    while (cont)
    {
        receivedbyte = CDC.read(valor_leido);
        valor_leido[receivedbyte] = 0;
        if (receivedbyte>0)
            cont = 0;
    }
    strcpy(lectura, strlwr(valor_leido));
}

void loop()
{

    leer_cadena();
    cont=0;

    if (strncmp(lectura, "pinmode", 7)==0)
    {
        for (bloque = strtok(lectura, "("); bloque != NULL; bloque = strtok(NULL, ","))
        {
            switch (cont)
            {
                case 0:
                    break;
                case 1:
                    par1=atoi(bloque);
                    break;
                case 2:
                    if (strncmp(bloque, "input", 5)==0)
                        pinMode(par1, INPUT);
                    else if (strncmp(bloque, "output", 6)==0)
                        pinMode(par1, OUTPUT);
                    break;
            }
            cont++;
        }
    }

    else if (strncmp(lectura, "digitalwrite", 12)==0)
    {
        CDC.printf(lectura);
        for (bloque = strtok(lectura, "("); bloque != NULL; bloque = strtok(NULL, ",") )
        {
            switch (cont)
            {
                case 0:
                    CDC.printf(bloque);
                    break;
                case 1:
                    CDC.printf(bloque);
                    par1=atoi(bloque);
                    CDC.printf("%d", par1);
                    break;
                case 2:
                    if (strncmp(bloque, "high", 4)==0)
                    {
                        CDC.printf("%d", par1);
                        CDC.printf(bloque);
                        digitalWrite(par1, HIGH);
                    }
                    else if (strncmp(bloque, "low", 3)==0)
                    {                
                        CDC.printf("%d", par1);
                        CDC.printf(bloque);
                        digitalWrite(par1, LOW);
                    }
                    break;
            }
            cont++;
        }
    }

    else if (strncmp(lectura, "analogwrite", 11)==0)
    {
        for (bloque = strtok(lectura, "("); bloque != NULL; bloque = strtok(NULL, ",") )
        {
            switch (cont)
            {
                case 0:
                    break;
                case 1:
                    par1=atoi(bloque);
                    break;
                case 2:
                    bloque = strtok(bloque,")");
                    analogWrite(par1, atoi(bloque));
                    break;
            }
        cont++;
        }
    }

    else if (strncmp(lectura, "digitalread", 11)==0)
    {
        for (bloque = strtok(lectura, "("); bloque != NULL; bloque = strtok(NULL, ")") )
        {
            switch (cont)
            {
                case 0:
                    break;
                case 1:
                    CDC.printf("%d", digitalRead(atoi(bloque)));
                    break;
            }
            cont++;
        }
    }


    else if (strncmp(lectura, "analogread", 10)==0)
    {
        for (bloque = strtok(lectura, "("); bloque != NULL; bloque = strtok(NULL, ")") )
        {
            switch (cont)
            {
                case 0:
                    break;
                case 1:
                    CDC.printf("%d", analogRead(atoi(bloque)));
                    break;
            }
            cont++;
        }
    }

    else if (strncmp(lectura, "eepromread", 10)==0)
    {
        for (bloque = strtok(lectura, "("); bloque != NULL; bloque = strtok(NULL, ")") )
        {
            switch (cont)
            {
                case 0:
                    break;
                case 1:
                    CDC.printf("%d", EEPROM.read(atoi(bloque)));
                    break;
            }
            cont++;
        }
    }

    else if (strncmp(lectura, "eepromwrite", 11)==0)
    {
        for (bloque = strtok(lectura, "("); bloque != NULL; bloque = strtok(NULL, ",") )
        {
            switch (cont)
            {
                case 0:
                    break;
                case 1:
                    par1=atoi(bloque);
                    break;
                case 2:
                    bloque = strtok(bloque,")");
                    EEPROM.write(par1, atoi(bloque));
                    break;
            }
            cont++;
        }
    }

    else if (strcmp(lectura, "alloutput")==0)
    {
        for (i=0; i<=TotalPines; i++)
        {
            pinMode(i, OUTPUT);
            digitalWrite(i, LOW);
        }
    }

    else if (strcmp(lectura, "allinput")==0)
    {
        for (i=0; i<=TotalPines; i++)
            pinMode(i,INPUT);
    }

    else if (strcmp(lectura, "allhigh")==0)
    {
        for (i=0; i<=TotalPines; i++)
        {
            pinMode(i, OUTPUT);
            digitalWrite(i, HIGH);
        }
    }

    else if (strcmp(lectura, "alllow")==0)
    {
        for (i=0; i<=TotalPines; i++)
        {
            pinMode(i, OUTPUT);
            digitalWrite(i, LOW);
        }
    }

    else if (strcmp(lectura, "reset")==0)
        reset();

}
