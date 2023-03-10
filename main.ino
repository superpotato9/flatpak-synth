
/*
2022
code by:
nathan Koliha,
internalregister

released under mit 

this is the source code for the flatpack synthizer based on the ay-3-8910 (datasheet: https://github.com/nickbild/ay-3-8910/blob/main/docs/AY-3-8910-datasheet.pdf) 

this code supports:
* drum sounds
* adsr control
* envlope control
* control of all 3 channels 

limitations/ non supported features:
* because of Ay chip limitations the adsr can not be used with the envlope i may find hack around it but at this time that is not supported
* different envlopes on different channels simoultansy is also not supported since all channels share one
* good percussion this was not focus so it was not added 

how it works:

the code works through functions and sending instructions to the ay this is done because the ay does not require constant stream of notes and instead
will play whatever note it is last given
because of this 
notes are chosen then modifiers like envlope or adsr are added on top 
this allows control to be very simple since it it all function based 





*/


const int RESET_PIN = 8;// address bus etc stuff
const int BC1_PIN = A5;
const int BDIR_PIN = A4;

/* this code block handels all input systems 
there are 3 main input blocks:
   the first is the keyboard which is interfaced via the mcp23071
   the second is the mode button which sets the mode of the keyboard either settings or playing in playing it acts like normal keyboard in settings the keyboard mode can be 
   set according to the chart below keep in mind for settings mode once a setting is set it goes back to playing mode 
  function|c1,c2,c3,c4,c5,c6,c7,sqr,tri,stri,saw,isaw,up,dw,ssaw,rssaw,noise, adsr|
      key#|0, 1, 2, 3, 4, 5, 6,  7,  8,  9,  10,  11, 12,13, 14,  15,   16,    17,| 19-24 undefined
  on playing mode the keys correlate to notes with key 0 being  c and key 24 being c theirfore is spans two octaves with bass c at bottom high c at top and middle c in middle 
  the octave control can be set what ocative middle c is.
  the third system is the attack decay sustain and release knobs these are used to set what the keyboard adsr does

*/

// each key is +1 half step above the last and each from c0 to c8 are stored here as bools theirfor notes can be found by doing octave * 12 + steps above
//array of notes spanning from c0 to b8



double key_values[] = {  32.7, 34.65, 36.71, 38.89, 41.2, 43.65,
                        46.25, 49.0, 51.91, 55.0, 58.27, 61.74, 65.41, 69.3, 73.42, 77.78, 82.41, 87.31, 92.5, 98.0, 103.83, 110.0, 116.54, 123.47, 130.81, 138.59, 146.83,
                        155.56, 164.81, 174.61, 185.0, 196.0, 207.65, 220.0, 233.08, 246.94, 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.0, 415.3, 440.0, 466.16,
                        493.88, 523.25, 554.37, 587.33, 622.25, 659.25, 698.46, 739.99, 783.99, 830.61, 880.0, 932.33, 987.77, 1046.5, 1108.73, 1174.66, 1244.51, 1318.51, 1396.91,
                        1479.98, 1567.98, 1661.22, 1760.0, 1864.66, 1975.53, 2093.0, 2217.46, 2349.32, 2489.02, 2637.02, 2793.83, 2959.96, 3135.96, 3322.44, 3520.0, 3729.31, 3951.07,
                        4186.01, 4434.92, 4698.63, 4978.03, 5274.04, 5587.65, 5919.91, 6271.93, 6644.88, 7040.0, 7458.62, 7902.13 };









/*the code here is used as the basic systems for writing to registers and creating/ writing notes all are mostly self explanitory but
it is worth noting that the tp, period, fine, and coarse functions are all used to create the notes this is done through some basic algebra that can be found in the docs for the ay

*/












double tp(double freq) {  // turns freq into tone period
  return 2000000 / (16 * freq);
}
int env_period(double freq) {  //envlope period
  return (int)2000000 / (256 * freq);
}
int coarse(double freq) {  //calculates coarse tone value
  return (int)tp(freq) / 256;
}

int fine(double freq) {  // calculates  fine tone value
  return (int)tp(freq) - (256 - coarse(freq));
}



int env_coarse(double freq) {  //calculates coarse tone value
  return (int)env_period(freq) / 256;
}

int env_fine(double freq) {  // calculates  fine tone value
  return env_period(freq) - (256 - env_coarse(freq));
}
void set_mode_inactive() {
  digitalWrite(BC1_PIN, LOW);
  digitalWrite(BDIR_PIN, LOW);
}

void set_mode_latch() {
  digitalWrite(BC1_PIN, HIGH);
  digitalWrite(BDIR_PIN, HIGH);
}

void set_mode_write() {
  digitalWrite(BC1_PIN, LOW);
  digitalWrite(BDIR_PIN, HIGH);
}

void write_register(char reg, char value) {
  set_mode_latch();
  PORTD = reg;
  set_mode_inactive();
  set_mode_write();
  PORTD = value;
  set_mode_inactive();
}
/*
the functions notated by play-<a-c or drum> or the same thing but stop-<a-c or drum> are used to control the square wave freq on channel a b c respectvly  

the play_env functions sets thhe freq of env Note differnt from all other play functions play env takes double instead of int 

an example of normal play code on channel and a b playing 440 hz would be:
  play_a(440);
  play_b(440);
and for envlope and note on channel a:
  play_env(10);
  play_a(440);
  

*/


void play_env(double freq) {

  write_register(11, env_fine(freq));
  write_register(12, env_coarse(freq));
}



void play_a(double freq) {  // plays a given note on channel a

  write_register(0, fine(freq));
  write_register(1, coarse(freq));
}
void stop_a() {
  write_register(8, 0b00000000);
}
void play_b(int freq) {  // plays a given note on channel b

  write_register(2, fine(freq));
  write_register(3, coarse(freq));
}
void stop_b() {
  write_register(9, 0b00000000);
}
void play_c(int freq) {  //plays given note on channel b

  write_register(4, fine(freq));
  write_register(5, coarse(freq));
}
void stop_c() {
  write_register(10, 0b00000000);
}

/*
the drum function is peculear it plays random noise based on a given freq on channel c only initally it was only meant for drum sounds however it gives interesting sounds 
when mixed with normal waveforms 
*/
void play_drum(int freq) {

  write_register(7, 0b0100000);
  write_register(6, fine(freq));
}

void stop_drum() {
  write_register(7, 0b0110000);
}


/*
wave chart:
0 square wave
1 single decaying line \
2 single sawtooth  /|__
3 inverted sawtooth |\|\|\
4 triangle with soft ends \/\/\/
5 single inverted sawtooth |/
6 sawtooth /|/|
7 single upward line ___/
8 triangle \/\/\/\/


can be set via something: like wave_type(6);

*/
void wave_type(int type)  // takes an int as a representation of different waves  below is chart for them controls the envlope!
{
  if (type == 0) {
    write_register(8, 0b00001111);
    write_register(9, 0b00001111);
    write_register(10, 0b00001111);

  } else {
    write_register(8, 0b00011111);
    write_register(9, 0b00011111);
    write_register(10, 0b00011111);
  }
  switch (type) {

    case 1:                      //this is used to pick the wave based on the number sent
      write_register(13, 0b00);  //env control

      break;
    case 2:
      write_register(13, 0b01);  //env control
      break;
    case 3:
      write_register(13, 0b1000);  //env control
      break;
    case 4:
      write_register(13, 0b1010);  //env control
      break;
    case 5:
      write_register(13, 0b1011);  //env control
      break;
    case 6:
      write_register(13, 0b1100);  //env control
      break;

    case 7:
      write_register(13, 0b1101);  //env control
      break;
    case 8:
      write_register(13, 0b1110);  //env control
      break;
    default:
      break;
  }
}





/* 
this block of wonderous code is the adr functions for controlling amplitude 
the steps controls how long each step is normally the size of a step would be controlled but since integers cannot be used here
instead the length of each is used

and finally the level of the previous function is needed to set the the starting point this is not present on the attack because it is the start of the entire adr 
the release function ends once the value of it reaches 0 or its min value
* the steps is in ms 
* all values are 4 bit ints and thus have a cap of 15


basic use is:
  adsr_mode();


  play_a(440);


  attack(30);
  decay(14);

  release(0, 500, 9);



note that this will loop unless properly stopped

you might also note that there is not sustain function this is because decay takes care of it 
*/


void adsr_mode() {  // this is not strictly nessecary but very useful bc it disabled envlope on all channels
  write_register(8, 0b00000000);
  write_register(9, 0b00000000);
  write_register(10, 0b00000000);
}
int level = 15;           //the max ampltiude value

void setup() {
  pinMode(13, OUTPUT);//led pin
  digitalWrite(13, LOW);


  pinMode(14, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(16, INPUT_PULLUP);
  pinMode(17, INPUT_PULLUP);
  // Set up Timer 1 to output a 2 MHZ clock signal in Pin 9
  TCCR1A = bit(COM1A0);
  TCCR1B = bit(WGM12) | bit(CS10);
  OCR1A = 3;
  pinMode(9, OUTPUT);

  pinMode(RESET_PIN, OUTPUT);
  pinMode(BC1_PIN, OUTPUT);
  pinMode(BDIR_PIN, OUTPUT);


  // Set pins 0 to 7 to output
  DDRD = 0xFF;
  // Set pins 0 to 7 to output LOW
  PORTD = 0x00;

  set_mode_inactive();

  // Reset the AY-3-8910 digitalWrite(RESET_PIN, LOW);
  delay(1);
  digitalWrite(RESET_PIN, HIGH);

  // Enable

  write_register(7, 0b1111000);
  write_register(8, 0b00001111);
  // play_a(key_values[46]);
 
 adsr_mode();
 int cur_value = key_values[round(analogRead(A0)/17)] ;
 play_a(cur_value);

}
unsigned long time_now = 0;
int counter = 0;//counts up so the adsr can function


int attack_time = 70;
int sustain_level = 9;
int decay_time = 100;
int release_time = 100;
int i = 0;
int mod_a = 1; // if 0 then channel is off if 1 then channel is on 
int mod_b = 1;
int mod_c = 1;
int note_a = 0; //the key values to be played
int note_b = 0;
int note_c = 0;


char step = 'a'; // checks what step we are on between atk , dec , sus, rel

void loop() {
  
  
 

  // if(cur_play != pin_val){
  // play_a(map(pin_val, 0, 1023, 0, 99));
  // cur_play == pin_val;    
  // }

  // write_register(8, 0b0000 + 15);
  

  


  if(step ==  'n')
{
 write_register(8, 0b0000 +15);   //writes the ampltiudes to all 3 registers 
    write_register(9, 0b0000 + 15);
    write_register(10, 0b0000 + 15);  
    step = 'w';
}

  // attack(30);
  
  if(step == 'a'){ // attack mode 
    
  // for (int i = 0; i < 15; i++) { // i is being compared to level aka max ampltude
  if(millis() > time_now+attack_time ){ // sees if enough time has passed 
    time_now = millis();
    
   
    write_register(8, 0b0000 + i);   //writes the ampltiudes to all 3 registers 
    write_register(9, 0b0000 + i);
    write_register(10, 0b0000 + i);
    i++;
  }

if (i == 15){ // if i == max value then reset i  and move on to decay

      step = 'd';
      i = 15;   
    }   

  }
 
  if(step == 'd'){ //decay mode
 
  if(millis() > time_now+decay_time ){ // sees if enough time has passed 
    time_now = millis();
    
   
    write_register(8, 0b0000 + i);   //writes the ampltiudes to all 3 registers 
    write_register(9, 0b0000 + i);
    write_register(10, 0b0000 + i);
    i--;
  }

if (i == sustain_level){ // if i == min value then reset i  and move on to decay

      step = 's';
      i = sustain_level;
      
    }   
  }

  if(step == 's'){
//it just plays dont do anything   
    delay(300);
    step = 'r';

  }
 
  if(step == 'r'){

  if(millis() > time_now+release_time ){ // sees if enough time has passed 
    time_now = millis();
    
   
    write_register(8, 0b0000 + i);   //writes the ampltiudes to all 3 registers 
    write_register(9, 0b0000 + i);
    write_register(10, 0b0000 + i);
    i--;
      
  }

if (i == 0){ // if i == min value then reset i  and move on to decay

      step = 'a';
      i = 0;   
       int cur_value = key_values[round(analogRead(A0)/17)] ; //gets curvalue of the cv pin reads its value and divdes it by 17 (the space between each note) then rounds and calls that freq
 play_a(cur_value);
 
    }   
  }

  // adsr_mode();
  // attack(100);
  // decay(50, 12);

  // release(200, 12);
}
