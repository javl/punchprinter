void calculate_colors(const int pot_data[4]){
  //  Serial.println("Go create color");
  // new_colors[0] = left brightness
  // new_colors[1] = right brightness
  // new_colors[2] = right hue
  // new_colors[3] = left hue

  HSVtoRGBtoCMYK(0, map(pot_data[3], 0, 1023, 0, 360), 100, map(pot_data[0], 0, 1023, 5, 100));
  HSVtoRGBtoCMYK(10-1, map(pot_data[2], 0, 1023, 0, 360), 100, map(pot_data[1], 0, 1023, 5, 100));
  // Get change per glass for each color: (last color - first color) / (number of glasses - 1)
  float change_per_glass_C = (generated_colors[9][0] - generated_colors[0][0]) / 9;
  float change_per_glass_M = (generated_colors[9][1] - generated_colors[0][1]) / 9;
  float change_per_glass_Y = (generated_colors[9][2] - generated_colors[0][2]) / 9;
  float change_per_glass_K = (generated_colors[9][3] - generated_colors[0][3]) / 9;

  // Set all the seperate glass values (begincolor + n*change_per_glass
  for(int i=1; i < 10 -1; i++){
    generated_colors[i][0] = round(generated_colors[i-1][0] + change_per_glass_C);
    generated_colors[i][1] = round(generated_colors[i-1][1] + change_per_glass_M);
    generated_colors[i][2] = round(generated_colors[i-1][2] + change_per_glass_Y);
    generated_colors[i][3] = round(generated_colors[i-1][3] + change_per_glass_K);
  }
  //  Serial.println("Make color is done");
  /*  Serial.println("GENERATED COLORS");
   for(int k=0;k<4; k++){
   for(int i=0;i<10;i++){
   Serial.print(generated_colors[i][k]);
   Serial.print("\t");
   }
   Serial.println("");
   }
   */
}

void HSVtoRGBtoCMYK(int store_in_index, float h, float s, float v){
  h += 120.0;
  if(h > 360.0){
    h -= 360.0; 
  }
  h = 360.0 - h;

  //h = 200.0;
  //s = 100.0;
  //v = 50.0;

  h /= 360.0;
  s /= 100.0;
  v /= 100.0;

  float i, f, p, q, t;
  float r = 0.0;
  float g = 0.0;
  float b = 0.0;

  i = floor(h * 6.0);

  f = h * 6.0 - i;
  p = v * (1.0 - s);
  q = v * (1.0 - f * s);
  t = v * (1.0 - (1.0 - f) * s);

  switch (round(int(i) % 6)) {
  case 0: 
    r = v; 
    g = t; 
    b = p; 
    break;
  case 1: 
    r = q; 
    g = v; 
    b = p; 
    break;
  case 2: 
    r = p; 
    g = v; 
    b = t; 
    break;
  case 3: 
    r = p; 
    g = q; 
    b = v; 
    break;
  case 4: 
    r = t; 
    g = p; 
    b = v; 
    break;
  case 5: 
    r = v; 
    g = p; 
    b = q; 
    break;
  }

  r = max(0.0, min(255.0, float(round(r*255.0))));
  g = max(0.0, min(255.0, float(round(g*255.0))));
  b = max(0.0, min(255.0, float(round(b*255.0))));

  float computedC = 0.0;
  float computedM = 0.0;
  float computedY = 0.0;
  float computedK = 0.0;

  // BLACK
  if (r==0.0 && g==0.0 && b==0.0) {
    computedK = 1.0;
    //    float[] output = {
    //      0.0, 0.0, 0.0, 1.0
    //    };
    Serial.println("ALL BLACK");
    return;
  }

  computedC = 1.0 - (r/255.0);
  computedM = 1.0 - (g/255.0);
  computedY = 1.0 - (b/255.0);

  float minCMY = min(computedC, min(computedM, computedY));
  computedC = (computedC - minCMY) / (1.0 - minCMY) ;
  computedM = (computedM - minCMY) / (1.0 - minCMY) ;
  computedY = (computedY - minCMY) / (1.0 - minCMY) ;
  computedK = minCMY;

  //float[] output = {
  //  round(computedC*100), round(computedM*100), round(computedY*100), round(computedK*100)
  //  };
  //  Serial.println(computedC*100.0);
  //  Serial.println(computedM*100.0);
  //  Serial.println(computedY*100.0);
  //  Serial.println(computedK*100.0);

  generated_colors[store_in_index][0] = computedC*100;
  generated_colors[store_in_index][1] = computedM*100;
  generated_colors[store_in_index][2] = computedY*100;
  generated_colors[store_in_index][3] = computedK*100;

}

/*void HSVtoRGBtoCMYK(int store_in_index, float h, float s, float v) {
 // FIRST PART, HSV to RGB
 h /= 360.0;
 s /= 100.0;
 v /= 100.0;
 
 h = 90.0;
 s = 100.0;
 v = 100.0;
 
 Serial.println("HSV IN: ");
 Serial.print(h);
 Serial.print(", ");  
 Serial.print(s);
 Serial.print(", ");  
 Serial.print(v);
 Serial.println("");
 
 float i;
 float f, p, q, t;
 float r = 0;
 float g = 0;
 float b = 0;
 
 i = floor(h * 6.0);
 f = h * 6.0 - i;
 p = v * (1 - s);
 q = v * (1 - f * s);
 t = v * (1.0 - (1.0 - f) * s);
 switch (round(int(i) % 6)) {
 case 0: 
 r = v; 
 g = t; 
 b = p; 
 break;
 case 1: 
 r = q; 
 g = v; 
 b = p; 
 break;
 case 2: 
 r = p; 
 g = v; 
 b = t; 
 break;
 case 3: 
 r = p; 
 g = q; 
 b = v; 
 break;
 case 4: 
 r = t; 
 g = p; 
 b = v; 
 break;
 case 5: 
 r = v; 
 g = p; 
 b = q; 
 break;
 }
 Serial.print("R: ");
 Serial.println(r);
 Serial.print("G: ");
 Serial.println(r);
 Serial.print("B: ");
 Serial.println(r);
 r = max(0.0, floor(r*255.0));
 g = max(0.0, floor(r*255.0));
 b = max(0.0, floor(r*255.0));
 
 // SECOND PART, RGB to CMYK
 
 // BLACK
 if (r==0.0 && g==0.0 && b==0.0) {
 generated_colors[store_in_index][0] = 0.0;
 generated_colors[store_in_index][1] = 0.0;
 generated_colors[store_in_index][2] = 0.0;
 generated_colors[store_in_index][3] = 100.0;
 Serial.println("JUST BLACK");
 return;
 }
 
 float color_C = 1.0 - (r/255.0);
 float color_M = 1.0 - (g/255.0);
 float color_Y = 1.0 - (b/255.0);
 
 float minCMY = min(color_C, min(color_M, color_Y));
 color_C = 100.0 * ((color_C - minCMY) / (1.0 - minCMY));
 color_M = 100.0 * ((color_M - minCMY) / (1.0 - minCMY));
 color_Y = 100.0 * ((color_Y - minCMY) / (1.0 - minCMY));
 float color_K = 100.0 * minCMY;
 
 Serial.println("CMYK COLORS: ");
 Serial.println(color_C);
 Serial.println(color_M);
 Serial.println(color_Y);
 Serial.println(color_K);
 
 generated_colors[store_in_index][0] = max(0.0, floor(color_C));
 generated_colors[store_in_index][1] = max(0.0, floor(color_M));
 generated_colors[store_in_index][2] = max(0.0, floor(color_Y));
 generated_colors[store_in_index][3] = max(0.0, floor(color_K));
 
 float total_colors = color_C + color_M + color_Y + color_K;
 
 Serial.println("CMYK: ");
 Serial.println(generated_colors[store_in_index][0]);
 Serial.println(generated_colors[store_in_index][1]);
 Serial.println(generated_colors[store_in_index][2]);
 Serial.println(generated_colors[store_in_index][3]);
 // Prepare topping pump times, but set to a minus value so they won't run yet
 //pump_timers[4] = -1 * (color_C / (total_colors / 100.0));
 //pump_timers[5] = -1 * (color_M / (total_colors / 100.0));
 //pump_timers[6] = -1 * (color_Y / (total_colors / 100.0));
 //pump_timers[7] = -1 * (color_K / (total_colors / 100.0));
 //Serial.println("Set topping timers: ");
 //Serial.println(pump_timers[4]);
 //Serial.println(pump_timers[5]);
 //Serial.println(pump_timers[6]);
 //Serial.println(pump_timers[7]);
 }
 */








