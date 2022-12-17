
/*
2022
code by:
nathan Koliha 
internalregister

released under mit 

this is the source code for the flatpack synthizer based on the ay-3-8910 (datasheet: https://github.com/nickbild/ay-3-8910/blob/main/docs/AY-3-8910-datasheet.pdf) t

his code supports:
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
const int RESET_PIN = 8;
const int BC1_PIN = A5;
const int BDIR_PIN = A4;
// const int bpm  =  60; //change for different bpm!
// const int blen  = bpm/60 * 1000; //lenght in ms of each beat
/* 
the code here is used as the basic systems for writing to registers and creating/ writing notes all are mostly self explanitory but
it is worth noting that the tp, period, fine, and coarse functions are all used to create the notes this is done through some basic algebra that can be found in the docs for the ay


*/

int tp(int freq){// turns freq into tone period 
return 2000000/(16*freq);


}
int env_period(double freq){ //envlope period
  return 2000000/(256*freq);

}
int coarse(int freq){ //calculates coarse tone value 
  return (int)tp(freq) / 256;
}

int fine(int freq){ // calculates  fine tone value 
return tp(freq) - (256 - coarse(freq));

}



int env_coarse(double freq){ //calculates coarse tone value 
  return (int)env_period(freq) / 256;
}

int env_fine(double freq){ // calculates  fine tone value 
return env_period(freq) - (256 -env_coarse(freq));

}
void set_mode_inactive()
{
  digitalWrite(BC1_PIN, LOW);
  digitalWrite(BDIR_PIN, LOW);
}

void set_mode_latch()
{
  digitalWrite(BC1_PIN, HIGH);
  digitalWrite(BDIR_PIN, HIGH);
}

void set_mode_write()
{
  digitalWrite(BC1_PIN, LOW);
  digitalWrite(BDIR_PIN, HIGH);
}

void write_register(char reg, char value)
{
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


void play_env(double freq){

 write_register(11, env_fine(freq));
write_register(12, env_coarse(freq));  


}



void play_a(int freq){ // plays a given note on channel a 

 write_register(0, fine(freq));
write_register(1, coarse(freq));  
}
void stop_a(){
  write_register(8, 0b00000000);
}
void play_b(int freq){ // plays a given note on channel b

 write_register(2, fine(freq));
write_register(3, coarse(freq));  
}
void stop_b(){
  write_register(9, 0b00000000);
}
void play_c(int freq){//plays given note on channel b 
  
 write_register(4, fine(freq));
write_register(5, coarse(freq)); 
}
void stop_c(){
  write_register(10, 0b00000000);
}

/*
the drum function is peculear it plays random noise based on a given freq on channel c only initally it was only meant for drum sounds however it gives interesting sounds 
when mixed with normal waveforms 
*/
void play_drum(int freq){ 

  write_register(7, 0b0100000);
    write_register(6, fine(freq));

 
  
}

void stop_drum(){
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
void wave_type(int type )// takes an int as a representation of different waves  below is chart for them controls the envlope!
{
 if(type == 0){
    write_register(8, 0b00001111);
    write_register(9, 0b00001111);
    write_register(10, 0b00001111);

  }
  else{
   write_register(8, 0b00011111);
    write_register(9, 0b00011111);
    write_register(10, 0b00011111);

  }
switch(type)
{
 
  case 1: //this is used to pick the wave based on the number sent 
write_register(13,0b00); //env control
  
  break;
  case 2:
write_register(13,0b01); //env control
break;  
 case 3:
write_register(13,0b1000); //env control
break;  
 case 4:
write_register(13,0b1010); //env control
break;  
 case 5:
write_register(13,0b1011); //env control
break;  
 case 6:
write_register(13,0b1100); //env control
break;  

 case 7:
write_register(13,0b1101); //env control
break;  
 case 8:
write_register(13,0b1110); //env control
break;  
  default:
  break;
}


}





/* 
this block of wonderous code is the adsr functions for controlling amplitude they take 2-3 params that are used to generate their amplitude change 
the level keyword sets the maximum level of the the function, the steps controls how long each step is normally the size of a step would be controlled but since integers cannot be used here
instead the length of each is used
and finally the level of the previous function is needed to set the the starting point this is not present on the attack because it is the start of the entire adsr 
the release function ends once the value of it reaches 0 or its min value
* the steps is in ms 
* all values are 4 bit ints and thus have a cap of 15

basic use is:
  adsr_mode();


  play_a(440);


  attack(14, 30);
  decay(7, 100, 14);
  sustain(9, 500, 7);
  release(0, 500, 9);



note that this will loop unless properly stopped


*/


void adsr_mode(){// this is not strictly nessecary but very useful bc it disabled envlope on all channels
    write_register(8, 0b00000000);
    write_register(9, 0b00000000);
    write_register(10, 0b00000000);





}

void attack(int level, int steps){ //attacks   level is how high to go max 15 steps is how many steps up each time
for (int i = 0; i<level; i++){
    write_register(8, 0b0000+i);
    write_register(9, 0b0000+i);
    write_register(10, 0b0000+i);
    delay(steps); // if set to 0 would go up instalty 

}
}



void decay(int level, int steps, int attack_level){ //decay  level is how high to go max 15 steps is how many steps up each time attack is the attack level
for (int i = attack_level; i>level; i--){
    write_register(8, 0b0000+i);
    write_register(9, 0b0000+i);
    write_register(10, 0b0000+i);
    delay(steps); // if set to 0 would go up instalty 

}
}


void sustain(int level, int steps, int decay_level){ //sustain level is how high to go max 15 steps  (note you CAN go down!) decay level is decays level 

 if(decay_level > level){
for (int i = decay_level; i>level; i--){
    write_register(8, 0b0000+i);
    write_register(9, 0b0000+i);
    write_register(10, 0b0000+i);
    delay(steps); // if set to 0 would go up instalty 

}}
else{for (int i = decay_level; i<level; i++){
    write_register(8, 0b0000+i);
    write_register(9, 0b0000+i);
    write_register(10, 0b0000+i);
    delay(steps); // if set to 0 would go up instalty 

}}
}


void release(int level, int steps, int sustain_level){
for (int i = sustain_level; i>level; i--){
  
    write_register(8, 0b0000+i);
    write_register(9, 0b0000+i);
    write_register(10, 0b0000+i);
    delay(steps); // if set to 0 would go up instalty 

}
}
void setup() {
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
  write_register(11, env_fine(10));
write_register(12,env_coarse(10));  

wave_type(6);


 
}

void loop() {
  // play_a(440);
  // play_env(10);
  // play_a(440);
  



play_a(440);


attack(15, 20);
decay(9, 50, 15);
sustain(12, 30, 9);
release(0, 200, 12);


}
