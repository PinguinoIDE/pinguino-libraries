/**
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53A & Pinguino 32MX250
        Output:	Oled display
        Wiring :
        
        if OLED_PMP6800
                OLED CS     connected to GND
                OLED RES    connected to any GPIO (D3)
                OLED D/C    connected to Pinguino PMA1 (D4)
                OLED W/R    connected to Pinguino PMRD/PMWR (D13)
                OLED E/RD   connected to GND
                OLED D[7:0] connected to Pinguino PMD[7:0] (D[31:24])
        if OLED_PORT6800
                OLED CS     connected to GND
                OLED RES    connected to any GPIO (D0)
                OLED D/C    connected to any GPIO (D1)
                OLED W/R    connected to any GPIO (D2)
                OLED E/RD   connected to GND
                OLED D[7:0] connected to Pinguino D[31:24]
        if OLED_PMP8080 
                OLED CS     connected to GND
                OLED RES    connected to any GPIO (D3)
                OLED D/C    connected to Pinguino PMA1 (D4)
                OLED W/R    connected to Pinguino PMWR (D14)
                OLED E/RD   connected to GND
                OLED D[7:0] connected to Pinguino PMD[7:0]
        if OLED_PORT8080 
                OLED CS     connected to GND
                OLED RES    connected to any GPIO (D0)
                OLED D/C    connected to any GPIO (D1)
                OLED W/R    connected to any GPIO (D2)
                OLED E/RD   connected to GND
                OLED D[7:0] connected to Pinguino D[31:24]
        if OLED_I2Cx
                OLED SDA    connected to SDAx pin
                OLED SCL    connected to SCLx pin
        if OLED_SPIx
                OLED SDI    connected to SDOx pin
                OLED SCK    connected to SCKx pin
                OLED D/C    connected to any GPIO
                OLED CS     connected to any GPIO (*optional on some displays)
		
**/

/**
    Load one or more fonts and active them with OLED.setFont()
**/

#include <fonts/fixednums5x7.h>

/*DISPLAY CONTROLLER*******************************************/
#define OLED_SH1106
//#define OLED_SSD1306
/*DISPLAY SIZE*************************************************/
//#define OLED_128X32
#define OLED_128X64
/**************************************************************/

#define NBVERTICES  8
#define NBFACES     6
#define NBEDGES     (NBVERTICES + NBFACES - 2)
#define DISTANCE    256.0       // Distance eye-screen
#define SIZE        100

typedef struct { float x, y, z; } point3D;
typedef struct { u16   x, y;    } point2D;

point3D Point3D[NBVERTICES];  // Vertices after rotation
point2D Point2D[NBVERTICES];  // Vertices after projection
  
//const u8 intf=OLED_I2C1;   // Interface
const u8 intf=OLED_SPI2;   // Interface
const u16 Zoff=1600;

const point3D Vertex[NBVERTICES] = {
    {-SIZE,-SIZE,-SIZE},
    { SIZE,-SIZE,-SIZE},
    { SIZE, SIZE,-SIZE},
    {-SIZE, SIZE,-SIZE},
    { SIZE,-SIZE, SIZE},
    {-SIZE,-SIZE, SIZE},
    {-SIZE, SIZE, SIZE},
    { SIZE, SIZE, SIZE},
};

const u8 Face[NBFACES][4] = {
    {0,5,6,3},
    {0,1,4,5},
    {6,5,4,7},
    {0,3,2,1},
    {2,3,6,7},
    {1,4,7,2}
};

float matrice[3][3];          // 3*3 rotation matrix
u16 gFps=0,gMaxFps=0;         // Frame per second
u16 Xa=0, Ya=0, Za=0;         // Rotation angles 

///
/// Rotate all the vertices
///

void Rotation()
{
    u8 i;

    // 3*3 Rotation Matrix Calculation

    matrice[0][0] = (cosr(Za) * cosr(Ya)); 
    matrice[1][0] = (sinr(Za) * cosr(Ya)); 
    matrice[2][0] = (-sinr(Ya)); 

    matrice[0][1] = (cosr(Za) * sinr(Ya) * sinr(Xa) - sinr(Za) * cosr(Xa)); 
    matrice[1][1] = (sinr(Za) * sinr(Ya) * sinr(Xa) + cosr(Xa) * cosr(Za)); 
    matrice[2][1] = (sinr(Xa) * cosr(Ya)); 

    matrice[0][2] = (cosr(Za) * sinr(Ya) * cosr(Xa) + sinr(Za) * sinr(Xa)); 
    matrice[1][2] = (sinr(Za) * sinr(Ya) * cosr(Xa) - cosr(Za) * sinr(Xa)); 
    matrice[2][2] = (cosr(Xa) * cosr(Ya)); 

    // Rotate each vertex of the object

    for (i = 0; i < NBVERTICES; i++)
    {
        Point3D[i].x = (matrice[0][0]*Vertex[i].x + matrice[1][0]*Vertex[i].y + matrice[2][0]*Vertex[i].z);
        Point3D[i].y = (matrice[0][1]*Vertex[i].x + matrice[1][1]*Vertex[i].y + matrice[2][1]*Vertex[i].z);
        Point3D[i].z = (matrice[0][2]*Vertex[i].x + matrice[1][2]*Vertex[i].y + matrice[2][2]*Vertex[i].z);
    }
}

///
/// Project all the vertices
///

void Projection()
{
    u8 i;

    for (i = 0; i < NBVERTICES; i++)
    {
        Point2D[i].x = (u16)((Point3D[i].x * DISTANCE) / (Point3D[i].z + Zoff)) + OLED.screen.width / 2;
        Point2D[i].y = (u16)((Point3D[i].y * DISTANCE) / (Point3D[i].z + Zoff)) + OLED.screen.height/ 2;
    }
}

///
/// Test if face ABC is visible
/// AB /\ BC = orthogonal vector to the face
/// K = vision vector
/// if K * (AB /\ BC) > 0 then ABC is visible
 
u8 isVisible(u8 A, u8 B, u8 C)
{
    float prod = ((Point3D[B].y - Point3D[A].y) * (Point3D[C].z - Point3D[B].z) - 
                  (Point3D[C].y - Point3D[B].y) * (Point3D[B].z - Point3D[A].z));
    
    return ((u32)prod > 0 ? 1:0);
}

///
/// Draw a polygon
///

void drawFace(u8 A, u8 B, u8 C, u8 D)
{
    OLED.drawLine(intf, Point2D[A].x, Point2D[A].y, Point2D[B].x, Point2D[B].y);
    OLED.drawLine(intf, Point2D[B].x, Point2D[B].y, Point2D[C].x, Point2D[C].y);
    OLED.drawLine(intf, Point2D[C].x, Point2D[C].y, Point2D[D].x, Point2D[D].y);
    OLED.drawLine(intf, Point2D[D].x, Point2D[D].y, Point2D[A].x, Point2D[A].y);
}

///
/// Draw the object
///

void drawObject()
{
    u8 i, A, B, C, D;
    
    for (i=0; i<NBFACES; i++)
    {
        A = Face[i][0];
        B = Face[i][1];
        C = Face[i][2];
        D = Face[i][3];
        //if (isVisible(A, B, C))
            drawFace(A, B, C, D);
    }
}

///
/// Init.
///

void setup()
{
    pinMode(USERLED, OUTPUT);

    // if 6800- or 8080-interface and PMP is used
    //OLED.init(intf, 1, PMA3); // RST on D1, DC on PMA3 (D2 on a 47J53A)
    
    // if i2c interface is used
    //OLED.init(intf, 0x78); // i2c address of the display
    
    // if 6800- or 8080-interface (but not PMP) is used
    //void OLED.init(u8 module, u8 rst, u16 dc, u8 d0, u8 d1, u8 d2, u8 d3, u8 d4, u8 d5, u8 d6, u8 d7)
    //OLED.init(intf, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7);

    // if SPI Hardware is used (you need to connect pins SDO, SCK and CS if needed)
    OLED.init(intf, 4, 5); // DC [, RST]

    // if SPI Software is used
    //OLED.init(intf, 0,1,2,3,5); // SDA, SCK, CS, DC, RST
        
    // set the fixednums7x15 font (no letters)
    OLED.setFont(intf, fixednums5x7);
}

///
/// Main loop
///

void loop()
{
    u16 frame=0;
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
        OLED.clearScreen(intf);
        OLED.printNumber(intf, gFps, DEC);
        drawObject();
        OLED.refresh(intf);

        // one frame done !
        frame++;
    }
    toggle(USERLED);
    gFps = frame;
}
