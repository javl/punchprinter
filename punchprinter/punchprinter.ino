//==============================================================
// Variables that might come in handy when testing
//==============================================================
// #define replaces keywords with values during compilation

// Topping mode:
// 1: do pigments, when done add topping
// 2: do pigments, after TOPPING_DELAY (see below) also add topping
// 3: do pigments and topping at the same time
// 4: do only pigments, no topping
#define TOPPING_MODE 1
#define TOPPING_DELAY 1000.0 // used by TOPPING_MODE 2

// Convert the low color values (range 0-100) to useable run times for the pumps
// milliseconds to run = color value * PIGMENT_PUMP_TIME_FACTOR
#define PIGMENT_PUMP_TIME_FACTOR 10.0 // Don't forget the decimal!

// How long the topping pumps should run in total (and so how much topping fluid
// to use. This time will be proportionally devided between the needed topping pumps.
#define TOPPING_PUMP_TOTAL_TIME 2000.0 // Don't forget the decimal!

// The print button gets blocked for time (in milliseconds) to prevent printing double.
#define BUTTON_BLOCK_TIME 1500.0

//==============================================================
// Setup the Arduino pins to use
//==============================================================
#define NUMBER_OF_GLASSES 10

// Multiplexer pins
#define INPUTPIN 5 // Input from multiplexer (Z)

#define SP0 2 // Select pin S0
#define SP1 3 // Select pin S1
#define SP2 4 // Select pin S2
#define SP3 5 // Select pin S3

boolean cup_enabled[] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // Keep track of enabled cups
int tubePins[] = { 6, 7, 8, 9, 10, 11, 12, 13 }; // Pins for the tube relays
long timers[8]; // Timers to set the amount of fluid
int current_cup_index = 0;  // The next cup to pour
boolean can_dispend = true; // Whether pressing the button will dispend anything
long button_blocked_timer = 0; // Unblock button at
float generated_colors[10][4]; // Holds the generated colors in 2D matrix ([cup][color])

// Add some hardcoded colors for testing
generated_colors[0][0] = 45.0;
generated_colors[0][1] = 45.0;
generated_colors[0][2] = 45.0;
generated_colors[0][3] = 45.0;

generated_colors[7][0] = 0.0;
generated_colors[7][1] = 11.0;
generated_colors[7][2] = 91.0;
generated_colors[7][3] = 10.0;

//==============================================================
// Setup
//==============================================================
void setup(){
  Serial.begin(115200);

  // Set modes for multiplexer input and select pins
  pinMode(INPUTPIN, INPUT);
  pinMode(SP0, OUTPUT);
  pinMode(SP1, OUTPUT);
  pinMode(SP2, OUTPUT);
  pinMode(SP3, OUTPUT);
  // Set the output pins to LOW. Might not be needed, but won't do
  // any harm either.
  digitalWrite(SP0, LOW);
  digitalWrite(SP1, LOW);
  digitalWrite(SP2, LOW);
  digitalWrite(SP3, LOW);

  // Pump relays are active low so bring them HIGH to set to
  // their off position.
  for(int i=0;i<8;i++){
    pinMode(tubePins[i], OUTPUT);
    digitalWrite(tubePins[i], HIGH);
  }

  // Run some sort of initialization function to fill the
  // tubes with pigments.
  initialize(); 
}

//==============================================================
// Main loop
//==============================================================
void loop(){
  handle_input();
  handle_serial();
  checkTimers();
//  calculate_color_rows();
}

void handle_input(){
    // Check current values of inputs (pots, sliders, switches)
  for(index=0;index<=14;index++){
    // Send 4 bits to select pins on multiplexer to select which
    // pin to read from. Convert the index value to these 4 bits:
    // 0 = 0000, 1 = 0001, 2 = 0010, etc...
    digitalWrite(SP0, bitRead(index, 0));
    digitalWrite(SP1, bitRead(index, 1));
    digitalWrite(SP2, bitRead(index, 2));
    digitalWrite(SP3, bitRead(index, 3));

    // Listen to input
    if (index < 10){ // switches
      cup_enabled[index] = digitalRead(INPUTPIN);
    }
    else{ // potmeters
      // handle potmeters
    }
  }
  // Update the color rows (do every loop so you can adjust values while printing)
}

void handle_serial(){
  if(Serial.available() > 0){
    char input = Serial.read();
    if(input == '0'){
      topping_fluid_number = 0; 
    }
    else if(input == '1'){
      topping_fluid_number = 1; 
    }
    else if(input == '2'){
      topping_fluid_number = 2; 
    }
    else if(input == '3'){
      topping_fluid_number = 3; 
    }else if(input == '4'){
  calculate_color_rows();
      
    }
    dispense_button_pressed();
  }
}

//==============================================================
// Executed when print button is pressed
//==============================================================
void dispense_button_pressed(){
  if(can_dispend && button_blocked_timer == 0){
    dispend(current_cup_index);
    if(current_cup_index < 7){
      for(int i=current_cup_index+1; i<8; i++){
        if(cup_enabled[current_cup_index]){
          current_cup_index = i;
          break;
        }
      }
    }
    else{
      can_dispend = false;
    }
  }
}

//==============================================================
// Check if any of the timers have expired
//==============================================================
void checkTimers (){

  // Pump timers
  for(uint8_t index=0;index<8;index++){
    if(timers[index] != 0){
      if(timers[index] < millis()){
        digitalWrite(tubePins[index], HIGH);
        timers[index] = 0;
      }
    }
  }

  // Button block timer (blocks for a while after pressing)
  if(button_blocked_timer > 0){
    if(button_blocked_timer < millis()){
      button_blocked_timer = 0;
    } 
  }

  // Check if need to add topping fluid
  if(topping_fluid_timer > 0){
    if(topping_fluid_timer < millis()){
      triggerTubePin(4+topping_fluid_number, topping_fluid_time);
      topping_fluid_timer = 0;
    } 
  }

}

//==============================================================
// Open the right tubes
//==============================================================
void dispend(const int index){
  float period = -1.0;
  float longest_period = 0.0; // 
  float total_period = 0.0; // used in calculating topping fluids

  if(color_C[index] != 0){
    triggerTubePin(0, color_C[index] * PIGMENT_PUMP_TIME_FACTOR);
    period = color_C[index] * PIGMENT_PUMP_TIME_FACTOR;
    total_period += period;
    if(period > longest_period){
      longest_period = period; 
    }
  }
  if(color_M[index] != 0){
    triggerTubePin(1, color_M[index] * PIGMENT_PUMP_TIME_FACTOR);
    period = color_M[index] * PIGMENT_PUMP_TIME_FACTOR;
    total_period += period;
    if(period > longest_period){
      longest_period = period; 
    }
  }
  if(color_Y[index] != 0){
    triggerTubePin(2, color_Y[index] * PIGMENT_PUMP_TIME_FACTOR);
    period = color_Y[index] * PIGMENT_PUMP_TIME_FACTOR;
    total_period += period;
    if(period > longest_period){
      longest_period = period; 
    }
  }
  if(color_K[index] != 0){
    triggerTubePin(3, color_K[index] * PIGMENT_PUMP_TIME_FACTOR);
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
  button_blocked_timer = millis() + BUTTON_BLOCK_TIME;
}

//==============================================================
// Trigger a pin (run a pump)
//==============================================================
void triggerTubePin(const int index, const int time){
  if(index < 8){ // 8 Tube pins 
    if(timers[index] == 0){
      timers[index] = millis()+time;
      digitalWrite(tubePins[index], LOW);
    }    
  }
}

//==============================================================
// Initialize the printer
// Runs all of the pumps for 5 seconds
//==============================================================
void initialize(){
  // Some initialization, like running the pumps long enough 
  // to fill the tubes
}