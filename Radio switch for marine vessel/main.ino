#include <avr/io.h>
#include <avr/interrupt.h>

// Pin definitions
const int prio0 = 7;
const int prio1 = 6;
const int prio2 = 5;
const int prio3 = 4;
const int prio4 = 3;
const int prioLck = 19;

const int detectCh1 = 14;
const int detectCh2 = 15;
const int detectCh3 = 16;
const int detectCh4 = 17;

const int S0 = 1;
const int S1 = 0;
const int nEN = 2;

uint16_t count = 0;
int priority_reading;
int priority_new;
int output_queue;
int active_output;
int input_reading[5] = {
  0, 0, 0, 0, 0
};


void setup() {
  pinMode(prio0, INPUT);  
  pinMode(prio1, INPUT);  
  pinMode(prio2, INPUT);
  pinMode(prio3, INPUT);
  pinMode(prio4, INPUT);
  pinMode(prioLck, INPUT);
  pinMode(detectCh1, INPUT);   
  pinMode(detectCh2, INPUT);
  pinMode(detectCh3, INPUT);
  pinMode(detectCh4, INPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(nEN, OUTPUT);
  
  // TIMER 'TCCR2' SETUP
  cli();
  TCCR2 = 0;                        // reset timer control register to 0
  TCNT2 = 0;                        // reset timer counter value to 0
  TCCR2 |= (1<<WGM21);              // set TCCR2 to CTC mode (compare to value set in OCR2 and reset on compare match)
  OCR2 = 0x7C;                      // value for 1ms with 64 prescaler @ 8 MHz clock (124)
  TIMSK |= (1<<OCIE2);              // enable interrupt on compare match in TCCR2
  sei();                            // enable interrupts
  TCCR2 |= (1<<CS21) | (1<<CS20);   // set prescaler to 64  
}

ISR(TIMER2_COMP_vect) {    // Increment counter value on interrupt.
  count++;
}

void loop() {

  // Read priority switch and comparator output states
  priority_reading = read_priority();
  read_inputs();

  // If priority lock is active, force priority_reading value to output
  if(digitalRead(prioLck) == 1) {
    set_output(priority_reading);

  // If priority lock is off, normal operation
  } else {
    
    // If priority channel is detected as active, set it to output instantly
    if (input_reading[priority_reading] == 1) {
      set_output(priority_reading);

    // If non-priority channel is detected as active and the output is off, set detected channel to output
    // If another channel is already set to ouput, set detected channel to queue.
    } else {                          
      if (input_reading[0] == 1) {  
        if (active_output == 4) {
          set_output(0);
        }
        output_queue = 0;
      } else if (input_reading[1] == 1) {
        if (active_output == 4) {
          set_output(1);
        }
        output_queue = 1;
      } else if (input_reading[2] == 1) {
        if (active_output == 4) {
          set_output(2);
        }
        output_queue = 2;
      } else if (input_reading[3] == 1) {
        if (active_output == 4) {
          set_output(3);
        }
        output_queue = 3;

      // If no channel is detected active, queue setting channel off
      } else {         
        output_queue = 4;
      }
    }
    
    // Set queued channel to ouput, when integer value is reached
    if (count > 250){
      set_output(output_queue);
      count = 0;
    }
  }
}

int read_priority() {
  static int priority_prev = 0;

  // read the priority switch inputs, and return the value (4 == no priority)
  if (digitalRead(prio0)) { priority_new = 4; }
  else if (digitalRead(prio1)) { priority_new = 0; }
  else if (digitalRead(prio2)) { priority_new = 1; }
  else if (digitalRead(prio3)) { priority_new = 2; }
  else if (digitalRead(prio4)) { priority_new = 3; }

  // If priority switch state has changed, 
  // set previous to new value and return the new value.
  // Delay to eliminate popping (bubblegum solution).
  if (priority_new != priority_prev) {
    priority_prev = priority_new;
    delay(1);
    return priority_new;
  }

  // If no change return the previous value.
  return priority_prev;
}

void read_inputs() {
  // Update input_reading[] array with values from activity comparator outputs
  input_reading[0] = digitalRead(detectCh1);
  input_reading[1] = digitalRead(detectCh2);
  input_reading[2] = digitalRead(detectCh3);
  input_reading[3] = digitalRead(detectCh4);

  // If the active output channel is still getting input signal reset the counter
  if (input_reading[active_output] == 1) {
    count = 0;
  }
  return;
}

void debounce() {
  // Jos tarvii tai jaksaa hifistellä, niin tämä prioriteetti kytkimelle

  return; 
}


void set_output(int outputCh) {
  static int output_prev = 5;

  // if no change in queued output, skip some code. Go fast.
  if (outputCh != output_prev) {
    
    // Output off, set 'not enable' high
    if (outputCh == 4) {  
      digitalWrite(nEN, 1);
      digitalWrite(S1, 0);
      digitalWrite(S0, 0);
      active_output = 4;    
  
    // Channel 1 input to output
    } else if (outputCh == 0) {
      digitalWrite(nEN, 0);
      digitalWrite(S1, 0);
      digitalWrite(S0, 0);
      active_output = 0;
  
    // Channel 2 input to output
    } else if (outputCh == 1) {
      digitalWrite(nEN, 0);
      digitalWrite(S1, 1);
      digitalWrite(S0, 0);
      active_output = 1;
  
    // Channel 3 input to output
    } else if (outputCh == 2) {
      digitalWrite(nEN, 0);
      digitalWrite(S1, 1);
      digitalWrite(S0, 1);
      active_output = 2;
  
    // Channel 4 input to output
    } else if (outputCh == 3) {
      digitalWrite(nEN, 0);
      digitalWrite(S1, 0);
      digitalWrite(S0, 1);
      active_output = 3;
    }
  }
  return;    
}
  
