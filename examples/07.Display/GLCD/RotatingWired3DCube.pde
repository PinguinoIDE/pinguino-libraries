/**
    Author:    Régis Blanchot (Nov. 2016)
    Tested on: Pinguino 47J53 & Pinguino 32MX250
    Output:    128x64 Graphic LCD with KS0108 controller
    Wiring :   (Panel E)
    
    1	Vss	Ground on breadboard
    2	Vdd	+5V on breadboard
    3 	Vout	Center pin of 10k pot
    4	RS	Pin 8
    5	RW	Pin 9
    6	E	Pin 10
    7	DB0	Pin 0
    8	DB1	Pin 1
    9	DB2	Pin 2
    10	DB3	Pin 3
    11	DB4	Pin 4
    12	DB5	Pin 5
    13	DB6	Pin 6
    14	DB7	Pin 7
    15	CS1	Pin 11
    16	CS2	Pin 12
    17	/RST	Pin 13
    18	Vee	An end of a 10k pot
    19	LED+	+5V on breadboard
    20	LED-	Ground on breadboard
**/

// Load one or more fonts and active them with GLCD.setFont()
#include <fonts/font5x7.h>

#define NBPOINTS     8
#define SIZE         100.0
#define DISTANCE     256.0

typedef struct { s16 x, y, z; } point3D;  //xy;
typedef struct { s16 x, y;    } point2D;

point3D Sommet[NBPOINTS];   // les sommets du cube
point3D Point3D[NBPOINTS];  // les sommets apres rotation
point2D Point2D[NBPOINTS];  // les sommets apres projection

float matrice[3][3];          // 3*3 rotation matrix

u16 Xa=0, Ya=0, Za=0;        // angles de rotation
u16 Xoff, Yoff, Zoff;        // position de l'observateur

///
/// Effectue la rotation des points Sommet -> Point3D
///

void Rotation()
{
    u8 i;
    //float a[3];

    // Calcul de la matrice de rotation 3*3

    matrice[0][0] = (cosr(Za) * cosr(Ya)); 
    matrice[1][0] = (sinr(Za) * cosr(Ya)); 
    matrice[2][0] = (-sinr(Ya)); 

    matrice[0][1] = (cosr(Za) * sinr(Ya) * sinr(Xa) - sinr(Za) * cosr(Xa)); 
    matrice[1][1] = (sinr(Za) * sinr(Ya) * sinr(Xa) + cosr(Xa) * cosr(Za)); 
    matrice[2][1] = (sinr(Xa) * cosr(Ya)); 

    matrice[0][2] = (cosr(Za) * sinr(Ya) * cosr(Xa) + sinr(Za) * sinr(Xa)); 
    matrice[1][2] = (sinr(Za) * sinr(Ya) * cosr(Xa) - cosr(Za) * sinr(Xa)); 
    matrice[2][2] = (cosr(Xa) * cosr(Ya)); 

    // Rotation des sommets de l'objet

    for (i = 0; i < NBPOINTS; i++)
    {
        Point3D[i].x = matrice[0][0]*Sommet[i].x + matrice[1][0]*Sommet[i].y + matrice[2][0]*Sommet[i].z;
        Point3D[i].y = matrice[0][1]*Sommet[i].x + matrice[1][1]*Sommet[i].y + matrice[2][1]*Sommet[i].z;
        Point3D[i].z = matrice[0][2]*Sommet[i].x + matrice[1][2]*Sommet[i].y + matrice[2][2]*Sommet[i].z;
    }
}

///
/// Projette en perspective les points apres rotation.
/// 256 = distance (oeil - écran)

void Projection(void)
{
    u8 i;

    for (i = 0; i < NBPOINTS; i++)
    {
        Point2D[i].x = ( Point3D[i].x * DISTANCE ) / ( Point3D[i].z + Zoff ) + Xoff;
        Point2D[i].y = ( Point3D[i].y * DISTANCE ) / ( Point3D[i].z + Zoff ) + Yoff;
    }
}

///
/// Initialise les coordonnees des sommets du cube
///

void initCube()
{
    Sommet[0].x = -SIZE;  Sommet[0].y = -SIZE;  Sommet[0].z = -SIZE;
    Sommet[1].x =  SIZE;  Sommet[1].y = -SIZE;  Sommet[1].z = -SIZE;
    Sommet[2].x =  SIZE;  Sommet[2].y =  SIZE;  Sommet[2].z = -SIZE;
    Sommet[3].x = -SIZE;  Sommet[3].y =  SIZE;  Sommet[3].z = -SIZE;
    
    Sommet[4].x =  SIZE;  Sommet[4].y = -SIZE;  Sommet[4].z =  SIZE;
    Sommet[5].x = -SIZE;  Sommet[5].y = -SIZE;  Sommet[5].z =  SIZE;
    Sommet[6].x = -SIZE;  Sommet[6].y =  SIZE;  Sommet[6].z =  SIZE;
    Sommet[7].x =  SIZE;  Sommet[7].y =  SIZE;  Sommet[7].z =  SIZE;
}

///
/// Trace un segment
///

void ligne(u8 a, u8 b)
{
    GLCD.drawLine( (u16)Point2D[a].x, (u16)Point2D[a].y, 
                   (u16)Point2D[b].x, (u16)Point2D[b].y);
}

///
/// Trace le cube
///

void drawCube()
{
    ligne(0,1); ligne(1,2); ligne(2,3); ligne(3,0);
    ligne(4,5); ligne(5,6); ligne(6,7); ligne(7,4);
    ligne(0,5); ligne(1,4); ligne(2,7); ligne(3,6);
}

///
/// Initialisation
///

void setup()
{
    // Initialize the LCD
    // rs (di), rw, en, cs1, cs2, rst, d0 to d7
    // Note taht some boards use pin 12 as USERLED
    // so if you use USERLED you cant use pin 12
    // to drive the GLCD
    GLCD.init(8,9,10,11,14,13,0,1,2,3,4,5,6,7);
    
    GLCD.setFont(font5x7);
    GLCD.clearScreen();
    
    // Cube's location
    Xoff = KS0108.screen.width  / 2;
    Yoff = KS0108.screen.height / 2;// + 9;
    Zoff = 1600;

    // create the 3D cube
    initCube();
    
    // calculations for the next time
    Rotation();
    Projection();
}

///
/// Boucle principale
///

void loop()
{
    // frame per second
    u8 fps=0, oldfps, maxfps;
    u32 timeEnd = millis() + 1000; // 1000 ms = 1 sec
    
    while (millis() < timeEnd)
    {
        // display
        GLCD.clearScreen();
        GLCD.printf("%u fps (max. %u)", oldfps, maxfps);
        drawCube();
        
        // one frame done !
        fps++;

        // update angles
        Xa = (Xa + 1) % 360;
        Ya = (Ya + 3) % 360;
        Za = (Za + 1) % 360;
        
        // calculations for the next time
        Rotation();
        Projection();
    }
    oldfps=fps;
    if (fps > maxfps)
        maxfps = fps;
}
