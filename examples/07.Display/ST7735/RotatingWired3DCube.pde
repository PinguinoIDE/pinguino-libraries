/**
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53 & Pinguino 32MX250
        Output:	262K-color graphic TFT-LCD with ST7735 controller

        2 modes available :
        - Hardware SPI
            . default mode
            . SPI operations are handled by the CPU
            . pins have to be the CPU SPI pins
            . PINGUINO 32 have up to 4 SPI module (SPI1 to SPI4)
            . PINGUINO 8  have only one SPI module (SPI1)
        - Software SPI
            . SPISW
            . SPI operations are handled by the SPI library
            . pins can be any digital pin
        
        Wiring :
        
        ST7735    PINGUINO               47J53    32MX250
        -------------------------------------------------
        LED       VSS (backlight on)
        SCK       SCK                    2        0
        SDA       SDO                    1        4
        A0 (DC)   user defined
        RESET     VSS
        CS        SS                     0        3
        GND       GND
        VSS       VSS (+5V or +3.3V)
**/

// Load one or more fonts and active them with ST7735.setFont()
#include <fonts/fixednums5x7.h>

#define SPIMODULE    SPI2
#define NBPOINTS     8
#define SIZE         100.0
#define DISTANCE     256.0

typedef struct { s16 x, y, z; } point3D;  //xy;
typedef struct { s16 x, y;    } point2D;

point3D Sommet[NBPOINTS];   // les sommets du cube
point3D Point3D[NBPOINTS];  // les sommets apres rotation
point2D Point2D[NBPOINTS];  // les sommets apres projection

float matrice[3][3];          // 3*3 rotation matrix

u8 maxfps=0;         // frame per second and stats
u16 Xa=0, Ya=0, Za=0;        // angles de rotation
u16 Xoff, Yoff, Zoff;       // position de l'observateur

///
/// Effectue la rotation des points Sommet -> Point3D
///

void Rotation()
{
    u8 i;

    // 3*3 Rotation Matrix calculation

    matrice[0][0] = (cosr(Za) * cosr(Ya)); 
    matrice[1][0] = (sinr(Za) * cosr(Ya)); 
    matrice[2][0] = (-sinr(Ya)); 

    matrice[0][1] = (cosr(Za) * sinr(Ya) * sinr(Xa) - sinr(Za) * cosr(Xa)); 
    matrice[1][1] = (sinr(Za) * sinr(Ya) * sinr(Xa) + cosr(Xa) * cosr(Za)); 
    matrice[2][1] = (sinr(Xa) * cosr(Ya)); 

    matrice[0][2] = (cosr(Za) * sinr(Ya) * cosr(Xa) + sinr(Za) * sinr(Xa)); 
    matrice[1][2] = (sinr(Za) * sinr(Ya) * cosr(Xa) - cosr(Za) * sinr(Xa)); 
    matrice[2][2] = (cosr(Xa) * cosr(Ya)); 

    // Rotation of each object's vertex

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
    u8 i;

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
/// Draw an edge
///

void ligne(u8 a, u8 b)
{
    ST7735.drawLine(SPIMODULE, (u16)Point2D[a].x, (u16)Point2D[a].y, 
                               (u16)Point2D[b].x, (u16)Point2D[b].y);
}

///
/// Draw the object
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
    // Optimization for Pinguino 32 only
    // 1/ Compile the program with MIPS32 option instead of MIPS16 (+30% faster)
    // 2/ Use of optimized Sine routine (cosr and sinr)
    // 3/ Overclock your processor if possible (Pinguino 32MX270 only)
    // 4/ Set Fspi to 30MHz :
    //    Fspi max = Fpb max / 2 with Fpb max = Fosc max
    //    Fspi max for ST7735 is 30 MHz so Fpb max = 60 MHz
    //    Adjust the following according to your processor
    //System.setCpuFrequency(60000000);
    //System.setPeripheralFrequency(60000000);
    
    ST7735.init(SPIMODULE, 7);// Hard. SPI : DC
    //ST7735.init(SPIMODULE, 7, 1, 2, 0); // Soft. SPI : DC, SDA, SCK, CS

    ST7735.setFont(SPIMODULE, fixednums5x7);
    ST7735.setBackgroundColor(SPIMODULE, ST7735_BLUE);
    ST7735.setColor(SPIMODULE, ST7735_WHITE);
    ST7735.setOrientation(SPIMODULE, 90);
    ST7735.clearScreen(SPIMODULE);
    
    // Position du cube
    Xoff = ST7735[SPIMODULE].screen.width  / 2;
    Yoff = ST7735[SPIMODULE].screen.height / 2;// + 9;
    Zoff = 1000;

    // create 3D cube
    initCube();
}

///
/// Boucle principale
///

void loop()
{
    u8 fps=0;
    u32 timeEnd = millis() + 1000; // 1000 ms = 1 sec
    
    while (millis() < timeEnd)
    {
        // update angles
        Xa = (Xa + 1) % 360;
        Ya = (Ya + 3) % 360;
        Za = (Za + 1) % 360;
        
        // calculations
        Rotation();
        Projection();
        
        // display
        //ST7735.clearScreen(SPIMODULE);
        ST7735.clearWindow(SPIMODULE,
            0,
            ST7735[SPIMODULE].font.height,
            ST7735[SPIMODULE].screen.width,
            ST7735[SPIMODULE].screen.height - ST7735[SPIMODULE].font.height);
        drawCube();

        // one frame done !
        fps++;
    }
    
    ST7735.printNumber(SPIMODULE, fps, DEC);
}
