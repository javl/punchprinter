//==============================================================
// Variables that might come in handy when testing
//==============================================================
// #define replaces keywords with their values during compilation

// Convert the low color values (range 0-100) to useable run times for the pumps
// milliseconds to run = color value * PIGMENT_PUMP_TIME_FACTOR
#define PIGMENT_PUMP_TIME_FACTOR 35.0 // Don't forget the decimal!

// How long the topping pumps should run in total (and so how much topping fluid
// to use. This time will be proportionally devided between the needed topping pumps.
#define TOPPING_PUMP_TOTAL_TIME 6000.0 // Don't forget the decimal!

// The print button gets blocked for time (in milliseconds) to prevent printing double.
#define BUTTON_BLOCK_PERIOD 5000.0

// The potentiometer values are checked multiple times and an average of the measured
// values is used to smooth the results. This sets the sample amount:
#define POT_SAMPLE_AMOUNT 10

// Topping mode:
// 1: do pigments, when done add topping
// 2: do pigments, after TOPPING_DELAY (see below) also add topping
// 3: do pigments and topping at the same time
// 4: do only pigments, no topping
#define TOPPING_MODE 1
#define TOPPING_DELAY 1000.0 // only used by TOPPING_MODE 2

//==============================================================
// Setup the Arduino pins to use
//==============================================================
//#define NUMBER_OF_GLASSES 10

// Printer states:
// 0: just plugged in, waiting for all switches to go LOW
// 1: all switches are low, toggle 0 to 7 to run the pumps
// 2: normal print mode!
int printer_state = 0; // 0 = waiting for testmode, 1 = testmode, 2 = printmode
// pump cmyk fill cmyk
//int relay_pins[8] = { 
//  2, 3, 4, 5, 6, 7, 8, 9 }; // Pins for the pump relays
int relay_pins[8] = { 
  5, 4, 3, 2, 9, 8, 7, 6 }; // Pins for the pump relays
int switch_pins[10] = { 
  22, 24, 26, 28, 30, 32, 34, 36, 38, 40 }; // Switches
int pot_pins[4] = { 
  0, 1, 2, 3 }; // Potentiometers
int button_pin = 18; // Print button, 21 because it can use interrupts
boolean print_button_is_available = true;

int pot_mins[4] = { 
  1023, 1023, 1023, 1023 }; // Will hold the minimum value of each pot
int pot_maxs[4] = { 
  0, 0, 0, 0 }; // will hold the maximum value of each pot
int pot_readings[4] = {
  0, 0, 0, 0}; // Holds the final averaged pot values, used for colors

boolean switch_states[] = { 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }; // Keep track of switch positions

long pump_timers[8]; // Timers keep track of time pumps run
long time_until_topping = 0; // How long to wait before adding topping
long check_input_timer = 0; // Used to generate the color every second

int cup_index = 0;  // The next cup to pour
boolean can_dispend = true; // Whether pressing the button will dispend anything
long button_block_timer = 0; // COunts back to zero when button is blocked after pressing

int generated_colors[10][4]; // Holds the generated colors in 2D matrix ([cup][color])

//==============================================================
// Setup
//==============================================================
void setup(){
  ////Serial.begin(115200); // Start a serial connection to PC

  // Pump relays are active low so bring them HIGH to set to
  // their off position.
  for(int i=0;i<10;i++){
    pinMode(switch_pins[i], INPUT);
  }
  //pinMode(button_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);


  for(int i=0;i<8;i++){
    pinMode(relay_pins[i], OUTPUT);
    digitalWrite(relay_pins[i], HIGH);
  }

}

//==============================================================
// Main loop
//==============================================================
void loop(){
  // At the start the printer is in printer_state 0: it won't do
  // anything until all of the switches are set to their OFF positions
  // When this happens, it changes to printer_state 1
  if(button_block_timer == 0){
    if(digitalRead(button_pin) == 0){
      // used to make sure a button needs to be depressed before recognizing it again
      if(print_button_is_available){ 
        print_button_is_available = false;
        print_button_pressed(); 
      }
      else{
        print_button_is_available = true; 
      }
    }
  }
  else{
    print_button_is_available = false; 
  }
  // 0: just plugged in, waiting for all switches to go LOW
  if(printer_state == 0){
    int switches_on = 0;
    for(int i=0;i<10;i++){
      if(digitalRead(switch_pins[i]) == 1){
        switches_on++;
      }
    }
    if(switches_on == 0){
      ////Serial.println("Switching to printer_state 1");
      printer_state = 1;
      // 1: all switches are low, toggle 0 to 7 to run the pumps
    }

    // Turn on pumps when switches are set to ON
  }
  // 1: all switches are low, toggle 0 to 7 to run the pumps
  else if(printer_state == 1){
    for(int i=0;i<8;i++){ // Check 8 pins (not 10) because there are 8 pumps
      if(digitalRead(switch_pins[i]) == HIGH){
        digitalWrite(relay_pins[i], LOW);
      }
      else{
        digitalWrite(relay_pins[i], HIGH);      
      }
    }
    // Regular print mode
  }
  // 2: normal print mode!
  else if(printer_state == 2){
    // Don't do anything. Input is checked every second using check_input_timer
  }
  ////handle_serial(); // Check if Serial commands came in
  check_timers();  // Check is running timers have expired
}

//==============================================================
// Check and handle changes in switch/pot positions
//==============================================================
void handle_input(){
  // Save the switch states
  for(int i=0;i<10;i++){
    switch_states[i] = digitalRead(switch_pins[i]);
  }
  int pot_average;
  for(int i=0;i<4;i++){
    pot_average = 0;
    for(int k=0;k<POT_SAMPLE_AMOUNT;k++){
      pot_average += analogRead(pot_pins[i]);
    } 
    pot_readings[i] = round(pot_average / POT_SAMPLE_AMOUNT);
    if(pot_readings[i] > pot_maxs[i]) pot_maxs[i] = pot_readings[i];
    if(pot_readings[i] < pot_mins[i]) pot_mins[i] = pot_readings[i];
    pot_readings[i] = map(pot_readings[i], pot_mins[i], pot_maxs[i], 0, 1023);
  }
   // De oude versie draaide de waarde van de pot sliders om
  //pot_readings[0] = max(0, 1023-analogRead(pot_pins[0])); // invert
  //pot_readings[1] = max(0, 1023-analogRead(pot_pins[1])); // invert
  //pot_readings[2] = analogRead(pot_pins[2]);
  //pot_readings[3] = analogRead(pot_pins[3]);

  calculate_colors(pot_readings);

}
/*
void handle_serial(){
 if(Serial.available() > 0){
 int serial_input = Serial.read() - '0'; // Convert ASCI code to number
 switch(serial_input){
 case 0:
 Serial.println("");
 Serial.println("Printer_state:");
 Serial.println(printer_state);
 Serial.println("");
 Serial.println("Printer button:");
 Serial.println(digitalRead(button_pin));
 Serial.println("Switches:");
 for(int i=0;i<10;i++){
 //        int switchState = digitalRead(switch_pins[i]);
 Serial.print(digitalRead(switch_pins[i]));
 Serial.print(" ");
 }
 Serial.println("");
 Serial.println("Averages Left (hue / darkness) ");
 Serial.println(pot_readings[3]);
 Serial.println(pot_readings[0]);
 Serial.println("Averages Right (hue / darkness) ");
 Serial.println(pot_readings[2]);
 Serial.println(pot_readings[1]);
 Serial.println("Color values: ");
 for(int i=0;i<NUMBER_OF_GLASSES;i++){
 for(int k=0;k<4;k++){
 Serial.print(generated_colors[i][k]);
 Serial.print("\t");
 }
 Serial.println("");
 }
 break;
 case 1:
 Serial.println("Switching to printer_state 1 (manually)");
 printer_state = 1;
 break;
 case 2:
 
 Serial.println("Switching to printer_state 2 (manually)");
 printer_state = 2;
 check_input_timer = millis() + 1000;
 break;
 case 9:
 print_button_pressed();
 break;
 }
 // Do stuff when needed
 }
 }
 */
//==============================================================
// Executed when print button is pressed
//==============================================================
void print_button_pressed(){
  if(button_block_timer == 0){ // First check if button is enabled
    // 1: all switches are low, toggle 0 to 7 to run the pumps
    if(printer_state == 1){
      // on the pumps. Press the print button to go to print mode
      int switches_on = 0;
      for(int i=0;i<10;i++){
        if(digitalRead(switch_pins[i]) == 1){
          switches_on++;
        }
      }
      if(switches_on == 0){
        ////Serial.println("Switching to printer_state 2");
        // 2: normal print mode!
        printer_state = 2;
        check_input_timer = millis() + 1000;      
      }
    }
    // 2: normal print mode!
    else if(printer_state == 2){
      int total_on = 0;
      for(int i=0; i<10;i++){
        if(digitalRead(switch_pins[i]) == 1){
          total_on++;
        }
      }
      if(total_on == 0){
        cup_index = 0; 
        ////Serial.println("RESET");
        return;
      }

      // Make sure it loops around to 0
      if(cup_index >= 10){
        cup_index = 0; 
      }
      ////Serial.println("GA PRINTEN");
      ////Serial.println(cup_index);
      for(int i=cup_index; i<10;i++){
        if(digitalRead(switch_pins[i]) == 1){
          ////Serial.print("WEL AAN EN PRINT: ");
          ////Serial.println(i);
          ////Serial.println("dispend: ");
          ////Serial.println(i);
          dispend(i);
          cup_index=i+1;
          if(cup_index < 10){
            boolean last_switch_reached = true;
            for(int k=cup_index; k<10;k++){
              if(digitalRead(switch_pins[k]) == 1){
                last_switch_reached = false;
                break;
              } 
            }
            if(last_switch_reached){
              ////Serial.println("TERUG NAAR NUL");             
              cup_index = 0; 
            }
          }
          break;
        }
        ////else{
        ////  Serial.print("NIET AAN: "); 
        ////  Serial.println(i); 
        ////}

      }      
    }
    // Block the button for a while
    button_block_timer = millis() + BUTTON_BLOCK_PERIOD;
  }
}

//==============================================================
// Check if any of the timers have expired
//==============================================================
void check_timers (){
  // Pump timers
  for(uint8_t i=0;i<8;i++){
    if(pump_timers[i] > 0){
      if(pump_timers[i] < millis()){
        digitalWrite(relay_pins[i], HIGH);
        pump_timers[i] = 0;
      }
    }
  }

  // Button block timer (blocks for a while after pressing)
  if(button_block_timer > 0){
    if(button_block_timer < millis()){
      button_block_timer = 0;
    }
  }

  if(time_until_topping > 0){
    if(time_until_topping < millis()){
      time_until_topping = 0;
      for(int i=4;i<8;i++){
        if(abs(pump_timers[i]) > 0){
          pump_timers[i] = millis() + ((float(abs(pump_timers[i]))/100.0) * TOPPING_PUMP_TOTAL_TIME);
          digitalWrite(relay_pins[i], LOW);
        }
      }
    }
  }
  // 2: normal print mode!
  if(printer_state == 2){
    if(check_input_timer < millis()){
      handle_input();
      check_input_timer = millis() + 1000;
    } 
  }

}

//==============================================================
// Open the right tubes
//==============================================================
void dispend(const int index){

  float longest_period = 0; // 
  float total_period = 0; // used in calculating topping fluids
  ////Serial.print("DISPENDING: ");
  ////Serial.println(index);
  for(int i=0;i<4;i++){
    if(generated_colors[index][i] != 0.0){
      int period = generated_colors[index][i] * PIGMENT_PUMP_TIME_FACTOR;
      run_pump(i, period);
      total_period += period;
      if(period > longest_period){
        longest_period = period;
      }
    }
    int total_color = 0;
    for(int i=0; i<4; i++){
      total_color += generated_colors[index][i];
    }
    //Prepare topping pump times, but set to a minus value so they won't run yet
    for(int i=0;i<4;i++){
      pump_timers[4+i] = -1 * (generated_colors[index][i] / (total_color / 100.0));
    }
  }

  switch(TOPPING_MODE){
  case 1: // Start topping when pigments are done
    time_until_topping = millis() + longest_period;
    break;
  case 2: // Start topping after TOPPING_DELAY milliseconds
    time_until_topping = millis() + TOPPING_DELAY;
    break;
  case 3: // Start topping at the same time as pigments
    time_until_topping = 1; // Very small amount so topping starts immediately
    break;
  }
  // Set a timer for the topping fluid so it starts pouring as soon as the
  // last CMYK tube is closed

  // Block the button for two seconds to prevent double-pouring
  button_block_timer = millis() + BUTTON_BLOCK_PERIOD;

}

//==============================================================
// Trigger a pin (run a pump)
//==============================================================
void run_pump(const int index, const int time_to_run){
  if(index < 8){ // make sure the index is valid
    if(pump_timers[index] == 0){
      pump_timers[index] = millis()+time_to_run;
      digitalWrite(relay_pins[index], LOW);
    }    
  }
}
























