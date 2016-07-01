/**
        Author: 	    Régis Blanchot (Apr. 2014)
        Library :     Thomas Missonier (sourcezax@users.sourceforge.net).
        Tested on:	Pinguino 32MX250 and Pinguino 47J53A
        Output:	    Nokia 5110 84x48 LCD Display  with PCD8544 Controller
    Wiring:

    1 NOKIA_RST    // any pin 
    2 NOKIA_SCE    // Pinguino CS or SS
    3 NOKIA_DC     // any pin
    4 NOKIA_SDIN   // Pinguino SDO
    5 NOKIA_SCLK   // Pinguino SCK 
    6 NOKIA_VCC    // 3.3V 
    7 NOKIA_LIGHT  // GND or 3.3V depends on models                                      
    8 NOKIA_GND    // GND 
**/

// SPI Module
#define SPILCD 1 

/**
    Load one or more fonts and active them with PCD8544.setFont()
**/

#include <fonts/font6x8.h>
//#include <fonts/font8x8.h>          // wrong direction
//#include <fonts/font10x14.h>        // ???
//#include <fonts/font12x8.h>         // wrong direction
//#include <fonts/font16x8.h>         // wrong direction
//#include <fonts/font16x16.h>        // ???

#define NBPOINTS 8

typedef struct { float x, y, z, xy; } point3D;
typedef struct { float x, y;        } point2D;

point3D  Sommet[NBPOINTS];  // 3D cube's corners
point3D Point3D[NBPOINTS];  // 3D cube's corners after rotation
point2D Point2D[NBPOINTS];  // 2D cube's corners after projection

float matrice[3][3];        // 3*3 rotation matrix

u16 fps=0, maxfps=0;        // frame per second and stats
u16 xa=0, ya=0, za=0;       // angles de rotation
u16 Xoff, Yoff, Zoff;       // position de l'observateur

void Rotation(u16 Xa, u16 Ya, u16 Za)
{
    u8 i;
    float a[3];

    // 3*3 Rotation matrix

    matrice[0][0] = cosr(Za) * cosr(Ya);
    matrice[1][0] = sinr(Za) * cosr(Ya);
    matrice[2][0] = - sinr(Ya);

    matrice[0][1] = cosr(Za) * sinr(Ya) * sinr(Xa) - sinr(Za) * cosr(Xa);
    matrice[1][1] = sinr(Za) * sinr(Ya) * sinr(Xa) + cosr(Xa) * cosr(Za);
    matrice[2][1] = sinr(Xa) * cosr(Ya);

    matrice[0][2] = cosr(Za) * sinr(Ya) * cosr(Xa) + sinr(Za) * sinr(Xa);
    matrice[1][2] = sinr(Za) * sinr(Ya) * cosr(Xa) - cosr(Za) * sinr(Xa);
    matrice[2][2] = cosr(Xa) * cosr(Ya);

    a[0] = - (matrice[0][0] * matrice[0][1]);
    a[1] = - (matrice[1][0] * matrice[1][1]);
    a[2] = - (matrice[2][0] * matrice[2][1]);

    // Rotation

    for (i = 0; i < NBPOINTS; i++)
    {
    /*
        Point3D[i].x =	((matrice[0][1] + Sommet[i].x) * (matrice[0][0] + Sommet[i].y)
            + a[0] + Sommet[i].xy
            + matrice[0][2] * Sommet[i].z);

        Point3D[i].y = ((matrice[1][1] + Sommet[i].x) * (matrice[1][0] + Sommet[i].y)
            + a[1] + Sommet[i].xy
            + matrice[1][2] * Sommet[i].z);

        Point3D[i].z =	((matrice[2][1] + Sommet[i].x) * (matrice[2][0] + Sommet[i].y)
            + a[2] + Sommet[i].xy
            + matrice[2][2] * Sommet[i].z);
    */
        Point3D[i].x =	matrice[0][0]*Sommet[i].x + matrice[1][0]*Sommet[i].y + matrice[2][0]*Sommet[i].z;
        Point3D[i].y =	matrice[0][1]*Sommet[i].x + matrice[1][1]*Sommet[i].y + matrice[2][1]*Sommet[i].z;
        Point3D[i].z =	matrice[0][2]*Sommet[i].x + matrice[1][2]*Sommet[i].y + matrice[2][2]*Sommet[i].z;
    }
}

void Projection(void)
{
    u8 i;

    for (i = 0; i < NBPOINTS; i++)
    {
        // 256 = distance eye - screen
        Point2D[i].x = ( Point3D[i].x * 256.0f ) / ( Point3D[i].z + Zoff ) + Xoff;
        Point2D[i].y = ( Point3D[i].y * 256.0f ) / ( Point3D[i].z + Zoff ) + Yoff;
    }
}

void initCube()
{
    u8 i;

    Sommet[0].x = -100.0f;  Sommet[0].y = -100.0f;  Sommet[0].z = -100.0f;
    Sommet[1].x =  100.0f;  Sommet[1].y = -100.0f;  Sommet[1].z = -100.0f;
    Sommet[2].x =  100.0f;  Sommet[2].y =  100.0f;  Sommet[2].z = -100.0f;
    Sommet[3].x = -100.0f;  Sommet[3].y =  100.0f;  Sommet[3].z = -100.0f;
    
    Sommet[4].x =  100.0f;  Sommet[4].y = -100.0f;  Sommet[4].z =  100.0f;
    Sommet[5].x = -100.0f;  Sommet[5].y = -100.0f;  Sommet[5].z =  100.0f;
    Sommet[6].x = -100.0f;  Sommet[6].y =  100.0f;  Sommet[6].z =  100.0f;
    Sommet[7].x =  100.0f;  Sommet[7].y =  100.0f;  Sommet[7].z =  100.0f;

    for (i = 0; i < NBPOINTS; i++)
        Sommet[i].xy = - Sommet[i].x * Sommet[i].y;
}

void drawCube()
{
    PCD8544.drawLine(SPILCD, Point2D[0].x, Point2D[0].y, Point2D[1].x, Point2D[1].y	);
    PCD8544.drawLine(SPILCD, Point2D[1].x, Point2D[1].y, Point2D[2].x, Point2D[2].y	);
    PCD8544.drawLine(SPILCD, Point2D[2].x, Point2D[2].y, Point2D[3].x, Point2D[3].y	);
    PCD8544.drawLine(SPILCD, Point2D[3].x, Point2D[3].y, Point2D[0].x, Point2D[0].y	);
    PCD8544.drawLine(SPILCD, Point2D[4].x, Point2D[4].y, Point2D[5].x, Point2D[5].y	);
    PCD8544.drawLine(SPILCD, Point2D[5].x, Point2D[5].y, Point2D[6].x, Point2D[6].y	);
    PCD8544.drawLine(SPILCD, Point2D[6].x, Point2D[6].y, Point2D[7].x, Point2D[7].y	);
    PCD8544.drawLine(SPILCD, Point2D[7].x, Point2D[7].y, Point2D[4].x, Point2D[4].y	);
    PCD8544.drawLine(SPILCD, Point2D[0].x, Point2D[0].y, Point2D[5].x, Point2D[5].y	);
    PCD8544.drawLine(SPILCD, Point2D[1].x, Point2D[1].y, Point2D[4].x, Point2D[4].y	);
    PCD8544.drawLine(SPILCD, Point2D[2].x, Point2D[2].y, Point2D[7].x, Point2D[7].y	);
    PCD8544.drawLine(SPILCD, Point2D[3].x, Point2D[3].y, Point2D[6].x, Point2D[6].y	);
}

void setup()
{
    pinMode(USERLED,     OUTPUT);

    /// Screen init

    // if SPILCD = SPISW (SPI Software)
    //PCD8544.init(SPILCD, 6, 7, 0, 1, 2); // DC, RST, SDO, SCK and CS pins
    PCD8544.init(SPILCD, 0, 2); // DC and RST pin

    PCD8544.setContrast(SPILCD, 40); // 0 to 127
    PCD8544.setFont(SPILCD, font6x8);
    PCD8544.clearScreen(SPILCD);
    
    // Cube's location
    Xoff = PCD8544[SPILCD].screen.width  / 2;
    Yoff = PCD8544[SPILCD].screen.height / 2;
    Zoff = 2048;

    // create 3D cube
    initCube();
}

void loop()
{
    u16 frame=0;
    u32 timeEnd = millis() + 1000; // 1000 ms = 1 sec
    
    while (millis() < timeEnd)
    {
        // calculations
        Rotation(xa, ya, za);
        Projection();
        
        // display
        PCD8544.clearScreen(SPILCD);
        
        //PCD8544.printf(SPILCD, "%ufps (max.%u)", fps, maxfps);
        // Or if your Pinguino runs out of memory :
        
        PCD8544.printNumber(SPILCD, fps, DEC);
        PCD8544.print(SPILCD, "fps (max.");
        PCD8544.printNumber(SPILCD, maxfps, DEC);
        PCD8544.print(SPILCD, ")");
        
        // draw the cube
        drawCube();
        PCD8544.refresh(SPILCD);

        // update angles
        xa = (xa + 1) % 360;
        ya = (ya + 3) % 360;
        za = (za + 1) % 360;
        
        // one frame done !
        frame++;
    }

    toggle(USERLED);

    fps = frame;
    if (fps > maxfps) maxfps = fps;
}
