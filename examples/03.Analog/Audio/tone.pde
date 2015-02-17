/*
    Melody
    Plays a melody on a piezo or 8-ohm speaker
    connected on a PWM pin :
    CCP1 or CCP2 for almost all 8-bit boards except for xxj53 boards
    xxj53 boards : CCP4, 5, 6, 7, 8, 9 or 10
*/

// notes in the melody:

int melody[] = {
    NOTE_C4,
    NOTE_G3,
    NOTE_G3,
    NOTE_A3,
    NOTE_G3,
    0,
    NOTE_B3,
    NOTE_C4    };

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4 };

void setup()
{
    pinMode(USERLED, OUTPUT);
    Audio.init(CDQUALITY);
}

void loop()
{

    int thisNote;	
    int pauseBetweenNotes;
    int noteDuration;
    
    // iterate over the notes of the melody:
    for (thisNote = 0; thisNote < 8; thisNote++)
    {
        // to calculate the note duration, take one second 
        // divided by the note type.
        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
        noteDuration = 1000 / noteDurations[thisNote];

        Audio.tone(CCP4, melody[thisNote], noteDuration);

        // to distinguish the notes, set a minimum time between them.
        // the note's duration + 30% seems to work well:
        pauseBetweenNotes = noteDuration +  noteDuration / 30;
        delay(pauseBetweenNotes);
    }
  
    // stop the tone playing:
    Audio.noTone(CCP4);
    toggle(USERLED);
    delay(1000);
}
