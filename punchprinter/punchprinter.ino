//==============================================================
// Variables that might come in handy when testing
//==============================================================
// #define replaces keywords with their values during compilation

// Topping mode:
// 1: do pigments, when done add topping
// 2: do pigments, after TOPPING_DELAY (see below) also add topping
// 3: do pigments and topping at the same time
// 4: do only pigments, no topping
#define TOPPING_MODE 1
#define TOPPING_DELAY 1000.0 // only used by TOPPING_MODE 2

// Convert the low color values (range 0-100) to useable run times for the pumps
// milliseconds to run = color value * PIGMENT_PUMP_TIME_FACTOR
#define PIGMENT_PUMP_TIME_FACTOR 10.0 // Don't forget the decimal!

// How long the topping pumps should run in total (and so how much topping fluid
// to use. This time will be proportionally devided between the needed topping pumps.
#define TOPPING_PUMP_TOTAL_TIME 2000.0 // Don't forget the decimal!

// The print button gets blocked for time (in milliseconds) to prevent printing double.
#define BUTTON_BLOCK_PERIOD 1500.0

// The potentiometer values are checked multiple times and an average of the measured
// values is used to smooth the results. This sets the sample amount:
#define POT_SAMPLE_AMOUNT 10

//==============================================================
// Setup the Arduino pins to use
//==============================================================
#define NUMBER_OF_GLASSES 10

// Printer states:
// 0: just plugged in, waiting for all switches to go LOW
// 1: all switches are low, toggle 0 to 7 to run the pumps
// 2: normal print mode!
volatile int printer_state = 0;  // Volatile so it can be changed in interrupt

int relay_pins[8] = { 2, 3, 4, 5, 6, 7, 8, 9 }; // Pins for the pump relays
int switch_pins[10] = { 22, 24, 26, 28, 30, 32, 34, 36, 38, 40 }; // Switches
int pot_pins[4] = { 0, 1, 2, 3 }; // Potentiometers
int button_pin = 21; // Print button, 21 because it can use interrupts

int pot_readings[4][POT_SAMPLE_AMOUNT]; // the readings from the pots
int pot_index = 0; // the index of the current potreading
int pot_total[4] = {0, 0, 0, 0}; // the running total for the pots
int pot_average[4] = {0, 0, 0, 0}; // Holds the final averaged pot values, used for colors

boolean switch_states[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // Keep track of switch positions

long timers[8]; // Timers keep track of time pumps run

int cup_index = 0;  // The next cup to pour
boolean can_dispend = true; // Whether pressing the button will dispend anything
long button_block_timer = 0; // COunts back to zero when button is blocked after pressing

float generated_colors[10][4]; // Holds the generated colors in 2D matrix ([cup][color])

//==============================================================
// Setup
//==============================================================
void setup(){
  // Add some hardcoded colors for testing
  generated_colors[0][0] = 45.0;
  generated_colors[0][1] = 45.0;
  generated_colors[0][2] = 45.0;
  generated_colors[0][3] = 45.0;

  generated_colors[7][0] = 0.0;
  generated_colors[7][1] = 11.0;
  generated_colors[7][2] = 91.0;
  generated_colors[7][3] = 10.0;

  Serial.begin(115200);

  // Pump relays are active low so bring them HIGH to set to
  // their off position.
  for(int i=0;i<10;i++){
    pinMode(relay_pins[i], OUTPUT);
    digitalWrite(relay_pins[i], HIGH);
  }

  // Set all potentiometer readings to 0
  for(int i=0;i<4;i++){
    for(int k=0;k<10;k++){
      pot_readings[i][k] = 0;
    } 
  }

  // run the print_button_pressed when the print button is RELEASED
  attachInterrupt(0, print_button_pressed, FALLING); 
 }

//==============================================================
// Main loop
//==============================================================
void loop(){
    // Wait for all the switches to go OFF before going to the next phase
  if(printer_state == 0){
    int switches_on = 0;
    for(int i=0;i<8;i++){
      switches_on += digitalRead(switch_pins[i]);
    }
    if(switches_on == 0){
       printer_state = 1;
    }
    // Turn on pumps when switches are set to ON
  }else if(printer_state == 1){
    for(int i=0;i<8;i++){
      if(digitalRead(switch_pins[i]) == HIGH){
        digitalWrite(relay_pins[i], LOW);
      }else{
        digitalWrite(relay_pins[i], HIGH);      
      }
    }
  // Regular print mode
  }else if(printer_state == 2){

  }

  handle_input(); // Check switch / potmeter states
  handle_serial(); // Check if Serial commands came in
  check_timers();  // Check is running timers have expired
}

//==============================================================
// Check and handle changes in switch/pot positions
//==============================================================
void handle_input(){
  // Check the switch states
  for(int i=0;i<10;i++){
    switch_states[i] = digitalRead(switch_pins[i]);
  }

  // Check the potentiometers. An average will be calculated
  boolean pot_values_changed = false;
  for(int i=0;i<4;i++){
    pot_total[i] = pot_total[i] - pot_readings[i][pot_index];         
    pot_readings[i][pot_index] = analogRead(pot_pins[i]); 
    pot_total[i]= pot_total[i] + pot_readings[i][pot_index];       

    int old_average = pot_average[i];
    pot_average[i] = pot_total[i] / POT_SAMPLE_AMOUNT;         
    if(old_average != pot_average[i]){
      pot_values_changed = true;
    }
  }
  pot_index++;
  if (pot_index >= POT_SAMPLE_AMOUNT){
    pot_index = 0;
  }    
  // Only update the colors if pot values actually changed
  if(pot_values_changed){
    calculate_colors(pot_average);
  }
}

void handle_serial(){
  if(Serial.available() > 0){
    int input = Serial.read() - '0'; // Convert ASCI code to number
    // Do stuff when needed
  }
}

//==============================================================
// Executed when print button is pressed
//==============================================================
void print_button_pressed(){
  if(button_block_timer == 0){
    // Turn on / off pumps based on switches 1 - 8
    if(printer_state == 1){
      int switches_on = 0;
      for(int i=0;i<8;i++){
        switches_on = digitalRead(switch_pins[i]);
      }
      if(switches_on == 0){
        printer_state = 2;
      }
    }else if(printer_state == 2){
      dispend(cup_index);
      if(cup_index < 10){
        for(int i=cup_index+1;i<10;i++){
          if(digitalRead(switch_pins[i]) == HIGH){
            cup_index = i;
            break;
          }
        }
        // If this part gets reached, all selected cups are filles
        // Block the button for a while
        button_block_timer = millis() + (BUTTON_BLOCK_PERIOD * 2);
      }
    }
  }
}

//==============================================================
// Check if any of the timers have expired
//==============================================================
void check_timers (){

  // Pump timers
  for(uint8_t i=0;i<8;i++){
    if(timers[i] != 0){
      if(timers[i] < millis()){
        digitalWrite(relay_pins[i], HIGH);
        timers[i] = 0;
      }
    }
  }

  // Button block timer (blocks for a while after pressing)
  if(button_block_timer > 0){
    if(button_block_timer < millis()){
      button_block_timer = 0;
    } 
  }
}

//==============================================================
// Open the right tubes
//==============================================================
void dispend(const int index){
  /*
  float period = -1.0;
  float longest_period = 0.0; // 
  float total_period = 0.0; // used in calculating topping fluids
  
  if(color_C[index] != 0){
    run_pump(0, color_C[index] * PIGMENT_PUMP_TIME_FACTOR);
    period = color_C[index] * PIGMENT_PUMP_TIME_FACTOR;
    total_period += period;
    if(period > longest_period){
      longest_period = period; 
    }
  }
  if(color_M[index] != 0){
    run_pump(1, color_M[index] * PIGMENT_PUMP_TIME_FACTOR);
    period = color_M[index] * PIGMENT_PUMP_TIME_FACTOR;
    total_period += period;
    if(period > longest_period){
      longest_period = period; 
    }
  }
  if(color_Y[index] != 0){
    run_pump(2, color_Y[index] * PIGMENT_PUMP_TIME_FACTOR);
    period = color_Y[index] * PIGMENT_PUMP_TIME_FACTOR;
    total_period += period;
    if(period > longest_period){
      longest_period = period; 
    }
  }
  if(color_K[index] != 0){
    run_pump(3, color_K[index] * PIGMENT_PUMP_TIME_FACTOR);
    period = color_K[index] * PIGMENT_PUMP_TIME_FACTOR;
    total_period += period;
    if(period > longest_period){
      longest_period = period; 
    }
  }

    total_period += period;

  switch(TOPPING_MODE){
    case 1: // Start topping when pigments are done
      topping_fluid_timer = millis() + longest_period;
    break;
    case 2: // Start topping after TOPPING_DELAY milliseconds
      topping_fluid_timer = millis() + TOPPING_DELAY;
    break;
    case 3: // Start topping at the same time as pigments
      topping_fluid_timer = 1; // Very small amount so topping starts emidiately
    break;
  }
  // Set a timer for the topping fluid so it starts pouring as soon as the
  // last CMYK tube is closed
  
  // Block the button for two seconds to prevent double-pouring
  button_block_timer = millis() + BUTTON_BLOCK_TIME;
  */
}

//==============================================================
// Trigger a pin (run a pump)
//==============================================================
void run_pump(const int index, const int time_to_run){
  if(index < 8){ // make sure the index is valid
    if(timers[index] == 0){
      timers[index] = millis()+time_to_run;
      digitalWrite(relay_pins[index], LOW);
    }    
  }
}
