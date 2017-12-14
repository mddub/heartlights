#include <Adafruit_NeoPixel.h>

/* Pulse Sensor Amped with NeoPixels

Pulse an arbitrary number of Adafruit NeoPixels based on a heartbeat sensor

The pulsesensor.com code needs to be in module interrupt.ino in the sketch directory
   http://pulsesensor.com/pages/pulse-sensor-amped-arduino-v1dot1
Code also uses the Adafruit NeoPixel library code discussed at
   https://learn.adafruit.com/adafruit-neopixel-uberguide

Version 1.0 by Mike Barela for Adafruit Industries, Fall 2015 
*/
#include <Adafruit_NeoPixel.h>    // Library containing 


volatile int rate[10];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find the inter beat interval
volatile int P =512;                      // used to find peak in pulse wave
volatile int T = 512;                     // used to find trough in pulse wave
volatile int thresh = 512;                // used to find instant moment of heart beat
volatile int amp = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = true;       // used to seed rate array so we startup with reasonable BPM

volatile int rate2[10];                    // used to hold last ten IBI values
volatile unsigned long sampleCounter2 = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime2 = 0;           // used to find the inter beat interval
volatile int P2 =512;                      // used to find peak in pulse wave
volatile int T2 = 512;                     // used to find trough in pulse wave
volatile int thresh2 = 512;                // used to find instant moment of heart beat
volatile int amp2 = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat2 = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat2 = true;       // used to seed rate array so we startup with reasonable BPM



// Behavior setting variables
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int pulsePin2 = 1;                 // Pulse Sensor purple wire connected to analog pin 1
int blinkPin = 13;                // Digital pin to blink led at each beat
int fadePin  = 5;                 // pin to do fancy neopixel effects at each beat
int intensity = 0;                // used to fade LED on with PWM on fadePin
int delta= 0;                     // used to fade LED on with PWM on fadePin
int intensity2 = 0;                // used to fade LED on with PWM on fadePin
int delta2= 0;                     // used to fade LED on with PWM on fadePin
int xOffset = 0;


// these variables are volatile because they are used during the interrupt service routine
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

volatile int BPM2;                   // used to hold the pulse rate
volatile int Signal2;                // holds the incoming raw data
volatile int IBI2 = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse2 = false;     // true when pulse wave is high, false when it's low
volatile boolean QS2 = false;        // becomes true when Arduoino finds a beat.


// Set up use of NeoPixels
const int NUMPIXELS = 210;           // Put the number of NeoPixels you are using here
const int BRIGHTNESS = 20;          // Set brightness of NeoPixels here
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, fadePin, NEO_GRB + NEO_KHZ800);



void interruptSetup(){     
  // Initializes Timer2 to throw an interrupt every 2mS.
  TCCR2A = 0x02;     // DISABLE PWM ON DIGITAL PINS 3 AND 11, AND GO INTO CTC MODE
  TCCR2B = 0x06;     // DON'T FORCE COMPARE, 256 PRESCALER 
  OCR2A = 0X7C;      // SET THE TOP OF THE COUNT TO 124 FOR 500Hz SAMPLE RATE
  TIMSK2 = 0x02;     // ENABLE INTERRUPT ON MATCH BETWEEN TIMER2 AND OCR2A
  sei();             // MAKE SURE GLOBAL INTERRUPTS ARE ENABLED      
} 

// THIS IS THE TIMER 2 INTERRUPT SERVICE ROUTINE. 
// Timer 2 makes sure that we take a reading every 2 miliseconds
ISR(TIMER2_COMPA_vect){                         // triggered when Timer2 counts to 124
    cli();                                      // disable interrupts while we do this
    int Signal = analogRead(pulsePin);              // read the Pulse Sensor 
    sampleCounter += 2;                         // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

//  find the peak and trough of the pulse wave
    if(Signal < thresh && N > (IBI/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T){                        // T is the trough
            T = Signal;                         // keep track of lowest point in pulse wave 
         }
       }
      
    if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
        P = Signal;                             // P is the peak
       }                                        // keep track of highest point in pulse wave
    
  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
if (N > 250){                                   // avoid high frequency noise
  if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
    Pulse = true;                               // set the Pulse flag when we think there is a pulse
    digitalWrite(blinkPin,HIGH);                // turn on pin 13 LED
    IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
    lastBeatTime = sampleCounter;               // keep track of time for next pulse
         
         if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
             firstBeat = false;                 // clear firstBeat flag
             return;                            // IBI value is unreliable so discard it
            }   
         if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
            secondBeat = false;                 // clear secondBeat flag
               for(int i=0; i<=9; i++){         // seed the running total to get a realisitic BPM at startup
                    rate[i] = IBI;                      
                    }
            }
          
    // keep a running total of the last 10 IBI values
    word runningTotal = 0;                   // clear the runningTotal variable    

    for(int i=0; i<=8; i++){                // shift data in the rate array
          rate[i] = rate[i+1];              // and drop the oldest IBI value 
          runningTotal += rate[i];          // add up the 9 oldest IBI values
        }
        
    rate[9] = IBI;                          // add the latest IBI to the rate array
    runningTotal += rate[9];                // add the latest IBI to runningTotal
    runningTotal /= 10;                     // average the last 10 IBI values 
    BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
    QS = true;                              // set Quantified Self flag 
    // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }
  }

    if (Signal < thresh && Pulse == true){     // when the values are going down, the beat is over
      digitalWrite(blinkPin,LOW);            // turn off pin 13 LED
      Pulse = false;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
     }
  
  if (N > 2500){                             // if 2.5 seconds go by without a beat
      thresh = 512;                          // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
      firstBeat = true;                      // set these to avoid noise
      secondBeat = true;                     // when we get the heartbeat back
     }


    //////////////////////
    ////  BEAT 2    /////
    ///////////////////// hack day :{
    int Signal2 = analogRead(pulsePin2);              // read the Pulse Sensor 
    sampleCounter2 += 2;                         // keep track of the time in mS with this variable
    int N2 = sampleCounter2 - lastBeatTime2;       // monitor the time since the last beat to avoid noise

//  find the peak and trough of the pulse wave
    if(Signal2 < thresh2 && N2 > (IBI2/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal2 < T2){                        // T is the trough
            T2 = Signal2;                         // keep track of lowest point in pulse wave 
         }
       }
      
    if(Signal2 > thresh2 && Signal2 > P2){          // thresh condition helps avoid noise
        P2 = Signal2;                             // P is the peak
       }                                        // keep track of highest point in pulse wave
    
  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
  if (N2 > 250){                                   // avoid high frequency noise
    if ( (Signal2 > thresh2) && (Pulse2 == false) && (N2 > (IBI2/5)*3) ){        
      Pulse2 = true;                               // set the Pulse flag when we think there is a pulse
      digitalWrite(blinkPin,HIGH);                // turn on pin 13 LED
      IBI2 = sampleCounter2 - lastBeatTime2;         // measure time between beats in mS
      lastBeatTime2 = sampleCounter2;               // keep track of time for next pulse
          
          if(firstBeat2){                         // if it's the first time we found a beat, if firstBeat == TRUE
              firstBeat2 = false;                 // clear firstBeat flag
              return;                            // IBI value is unreliable so discard it
              }   
          if(secondBeat2){                        // if this is the second beat, if secondBeat == TRUE
              secondBeat2 = false;                 // clear secondBeat flag
                for(int i=0; i<=9; i++){         // seed the running total to get a realisitic BPM at startup
                      rate2[i] = IBI2;                      
                      }
              }
            
      // keep a running total of the last 10 IBI values
      word runningTotal2 = 0;                   // clear the runningTotal variable    

      for(int i=0; i<=8; i++){                // shift data in the rate array
            rate2[i] = rate2[i+1];              // and drop the oldest IBI value 
            runningTotal2 += rate2[i];          // add up the 9 oldest IBI values
          }
          
      rate2[9] = IBI2;                          // add the latest IBI to the rate array
      runningTotal2 += rate2[9];                // add the latest IBI to runningTotal
      runningTotal2 /= 10;                     // average the last 10 IBI values 
      BPM2 = 60000/runningTotal2;               // how many beats can fit into a minute? that's BPM!
      QS2 = true;                              // set Quantified Self flag 
      // QS FLAG IS NOT CLEARED INSIDE THIS ISR
      }                       
  }

  if (Signal2 < thresh2 && Pulse2 == true){     // when the values are going down, the beat is over
      digitalWrite(blinkPin,LOW);            // turn off pin 13 LED
      Pulse2 = false;                         // reset the Pulse flag so we can do it again
      amp2 = P2 - T2;                           // get amplitude of the pulse wave
      thresh2 = amp2/2 + T2;                    // set thresh at 50% of the amplitude
      P2 = thresh2;                            // reset these for next time
      T2 = thresh2;
     }
  
  if (N2 > 2500){                             // if 2.5 seconds go by without a beat
      thresh2 = 512;                          // set thresh default
      P2 = 512;                               // set P default
      T2 = 512;                               // set T default
      lastBeatTime2 = sampleCounter2;          // bring the lastBeatTime up to date        
      firstBeat2 = true;                      // set these to avoid noise
      secondBeat2 = true;                     // when we get the heartbeat back
     }
  
  sei();                                     // enable interrupts when youre done!
}// end isr




void setup(){
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  Serial.begin(115200);           // Serial output data for debugging or external use
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  for (int x=0; x < NUMPIXELS; x++) {  // Initialize all pixels to 'off'
     strip.setPixelColor(x, strip.Color(0, 0, 0));
  }
  strip.show();                     // Ensure the pixels are off 
  delay(1000);                      // Wait a second
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
}

const int DELAY_MS = 20;

void loop(){
//  sendDataSerial('S', Signal);      // send Processing the raw Pulse Sensor data
  if (QS == true && BPM < 120){                    // Quantified Self flag is true when arduino finds a heartbeat
     delta = 255 / (IBI / (DELAY_MS * 2));  // Set 'intensity' Variable to 255 to fade LED with pulse
     sendDataSerial('B',BPM);       // send heart rate with a 'B' prefix
     sendDataSerial('Q',IBI);       // send time between beats with a 'Q' prefix
     QS = false;                      // reset the Quantified Self flag for next time    
  }
  if (QS2 == true && BPM2 < 120){                    // Quantified Self flag is true when arduino finds a heartbeat
    delta2 = 255 / (IBI2 / (DELAY_MS * 2));  // Set 'intensity' Variable to 255 to fade LED with pulse
    sendDataSerial('b',BPM2);       // send heart rate with a 'B' prefix
    sendDataSerial('q',IBI2);       // send time between beats with a 'Q' prefix
    QS2 = false;                      // reset the Quantified Self flag for next time    
 }
  ledFadeToBeat();                    // Routine that fades color intensity to the beat
  delay(DELAY_MS);                          //  take a break
}

void ledFadeToBeat() {
    intensity += delta;
    if (intensity > 255) {
      delta = delta * -1;
    }
    intensity = constrain(intensity,0,255);   // Keep LED fade value from going into negative numbers

    intensity2 += delta2;
    if (intensity2 > 255) {
      delta2 = delta2 * -1;
    }
    intensity2 = constrain(intensity2,0,255);   // Keep LED fade value from going into negative numbers

    setStrip(intensity, intensity2);                     // Write the value to the NeoPixels 

//    sendDataSerial('R',intensity);
}

void sendDataSerial(char symbol, int data ) {
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
}

void setStrip(int r, int r2) {     // Set the strip to one color intensity (red)
   int g = 0;              // Green is set to zero (for non-red colors, change this)
   int b = 0;              // Blue is set to zero (for non-red colors, change this)
   int bpmDelta = abs(BPM - BPM2);
   if (bpmDelta <= 10) {
      xOffset += 1;
   }
   for (int x=xOffset; x < NUMPIXELS/2 + xOffset; x++) {
      strip.setPixelColor(x % NUMPIXELS, strip.Color(r, 0, r));
   }
   for (int x=NUMPIXELS/2 + xOffset; x < NUMPIXELS + xOffset; x++) {
    strip.setPixelColor(x % NUMPIXELS, strip.Color(r2/3, r2/2, r2/4));
   }

   strip.show();
}
