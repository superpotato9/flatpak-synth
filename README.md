# flatpack-synth
this is the source code and pcb files for the flatpack synthizer based on the ay-3-8910 (datasheet: https://github.com/nickbild/ay-3-8910/blob/main/docs/AY-3-8910-datasheet.pdf) 

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

uses interrupts to store midi on offs and plays them whenever the program gets around to it 
