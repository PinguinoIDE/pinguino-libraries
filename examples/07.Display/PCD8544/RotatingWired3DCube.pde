/**
    Author:       Régis Blanchot (Apr. 2014)
    Library :     Thomas Missonier (sourcezax@users.sourceforge.net).
    Tested on:    Pinguino 32MX250 and Pinguino 47J53
    Output:       Nokia 5110 84x48 LCD Display  with PCD8544 Controller
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

// Load one or more fonts and active them with PCD8544.setFont()
#include <fonts/font6x8.h>

// SPI Module
#define SPILCD       2 
#define NBPOINTS     8
#define SIZE         100.0
#define DISTANCE     256.0    // distance eye - screen

typedef struct { s16 x, y, z; } point3D;
typedef struct { u16 x, y;    } point2D;

point3D  Sommet[NBPOINTS];  // 3D cube's corners
point3D Point3D[NBPOINTS];  // 3D cube's corners after rotation
point2D Point2D[NBPOINTS];  // 2D cube's corners after projection

float matrice[3][3];        // 3*3 rotation matrix
u16  fps=0,maxfps=0;         // frame per second and stats
u16 Xa=0, Ya=0, Za=0;       // angles de rotation
u16 Xoff, Yoff, Zoff;       // position de l'observateur
u32 timeEnd;

void Rotation()
{
    u8 i;

    // 3*3 Rotation matrix

    matrice[0][0] = (cosr(Za) * cosr(Ya)); 
    matrice[1][0] = (sinr(Za) * cosr(Ya)); 
    matrice[2][0] = (-sinr(Ya)); 

    matrice[0][1] = (cosr(Za) * sinr(Ya) * sinr(Xa) - sinr(Za) * cosr(Xa)); 
    matrice[1][1] = (sinr(Za) * sinr(Ya) * sinr(Xa) + cosr(Xa) * cosr(Za)); 
    matrice[2][1] = (sinr(Xa) * cosr(Ya)); 

    matrice[0][2] = (cosr(Za) * sinr(Ya) * cosr(Xa) + sinr(Za) * sinr(Xa)); 
    matrice[1][2] = (sinr(Za) * sinr(Ya) * cosr(Xa) - cosr(Za) * sinr(Xa)); 
    matrice[2][2] = (cosr(Xa) * cosr(Ya));
    
    // Rotation

    for (i = 0; i < NBPOINTS; i++)
    {
        Point3D[i].x = matrice[0][0]*Sommet[i].x + matrice[1][0]*Sommet[i].y + matrice[2][0]*Sommet[i].z;
        Point3D[i].y = matrice[0][1]*Sommet[i].x + matrice[1][1]*Sommet[i].y + matrice[2][1]*Sommet[i].z;
        Point3D[i].z = matrice[0][2]*Sommet[i].x + matrice[1][2]*Sommet[i].y + matrice[2][2]*Sommet[i].z;
    }
}

void Projection()
{
    u8 i;

    for (i = 0; i < NBPOINTS; i++)
    {
        Point2D[i].x = ( Point3D[i].x * DISTANCE ) / ( Point3D[i].z + Zoff ) + Xoff;
        Point2D[i].y = ( Point3D[i].y * DISTANCE ) / ( Point3D[i].z + Zoff ) + Yoff;
    }
}

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

void drawCube()
{
    PCD8544.drawLine(SPILCD, Point2D[0].x, Point2D[0].y, Point2D[1].x, Point2D[1].y);
    PCD8544.drawLine(SPILCD, Point2D[1].x, Point2D[1].y, Point2D[2].x, Point2D[2].y);
    PCD8544.drawLine(SPILCD, Point2D[2].x, Point2D[2].y, Point2D[3].x, Point2D[3].y);
    PCD8544.drawLine(SPILCD, Point2D[3].x, Point2D[3].y, Point2D[0].x, Point2D[0].y);
    PCD8544.drawLine(SPILCD, Point2D[4].x, Point2D[4].y, Point2D[5].x, Point2D[5].y);
    PCD8544.drawLine(SPILCD, Point2D[5].x, Point2D[5].y, Point2D[6].x, Point2D[6].y);
    PCD8544.drawLine(SPILCD, Point2D[6].x, Point2D[6].y, Point2D[7].x, Point2D[7].y);
    PCD8544.drawLine(SPILCD, Point2D[7].x, Point2D[7].y, Point2D[4].x, Point2D[4].y);
    PCD8544.drawLine(SPILCD, Point2D[0].x, Point2D[0].y, Point2D[5].x, Point2D[5].y);
    PCD8544.drawLine(SPILCD, Point2D[1].x, Point2D[1].y, Point2D[4].x, Point2D[4].y);
    PCD8544.drawLine(SPILCD, Point2D[2].x, Point2D[2].y, Point2D[7].x, Point2D[7].y);
    PCD8544.drawLine(SPILCD, Point2D[3].x, Point2D[3].y, Point2D[6].x, Point2D[6].y);
}

void setup()
{
    /// Screen init

    // if SPILCD = SPISW (SPI Software)
    //PCD8544.init(SPILCD, 6, 7, 0, 1, 2); // DC, RST, SDO, SCK and CS pins
    //PCD8544.init(SPILCD, 0, 1); // DC and RST pin 47J53
    //PCD8544.init(SPILCD, 0, 2); // DC and RST pin 32MX2x0 SPI1
    PCD8544.init(SPILCD, 5, 6); // DC and RST pin 32MX2x0 SPI2
    PCD8544.setContrast(SPILCD, 40); // 0 to 127
    PCD8544.setFont(SPILCD, font6x8);
    PCD8544.clearScreen(SPILCD);

    // Cube's location
    Xoff = PCD8544[SPILCD].screen.width  / 2;
    Yoff = PCD8544[SPILCD].screen.height / 2 + 2;
    Zoff = 2000;

    // create 3D cube
    initCube();
}

void loop()
{
    fps=0;
    timeEnd = millis() + 1000; // 1000 ms = 1 sec

    while (millis() < timeEnd)
    {
        // calculations
        Rotation();
        Projection();
        
        // draw the cube
        PCD8544.clearScreen(SPILCD);
        drawCube();
        PCD8544.printNumber(SPILCD, maxfps, DEC);
        PCD8544.print(SPILCD, " fps");
        PCD8544.refresh(SPILCD);
        delay(50);
        
        // update angles
        Xa = (Xa + 1) % 360;
        Ya = (Ya + 3) % 360;
        Za = (Za + 1) % 360;
        
        // one frame done !
        fps++;
    }

    if (fps > maxfps)
        maxfps = fps;
}
