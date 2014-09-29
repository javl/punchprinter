void calculate_colors(const int pot_data[4]){

  // new_colors[0] = left brightness
  // new_colors[1] = right brightness
  // new_colors[2] = right hue
  // new_colors[3] = left hue

  HSVtoRGBtoCMYK(0, map(pot_data[3], 0, 1023, 0, 360), 100, map(pot_data[0], 0, 1023, 0, 100));
  HSVtoRGBtoCMYK(NUMBER_OF_GLASSES-1, map(pot_data[2], 0, 1023, 0, 360), 100, map(pot_data[1], 0, 1023, 0, 100));

  // Get change per glass for each color: (last color - first color) / (number of glasses - 1)
  float change_per_glass_C = (generated_colors[7][1] - generated_colors[0][1]) / (NUMBER_OF_GLASSES - 1);
  float change_per_glass_M = (generated_colors[7][2] - generated_colors[0][2]) / (NUMBER_OF_GLASSES - 1);
  float change_per_glass_Y = (generated_colors[7][3] - generated_colors[0][3]) / (NUMBER_OF_GLASSES - 1);
  float change_per_glass_K = (generated_colors[7][4] - generated_colors[0][4]) / (NUMBER_OF_GLASSES - 1);

  // Set all the seperate glass values (begincolor + n*change_per_glass
  for(int i=1; i < NUMBER_OF_GLASSES -1; i++){
    generated_colors[i][0] = round(generated_colors[i-1][0] + change_per_glass_C);
    generated_colors[i][1] = round(generated_colors[i-1][1] + change_per_glass_M);
    generated_colors[i][2] = round(generated_colors[i-1][2] + change_per_glass_Y);
    generated_colors[i][3] = round(generated_colors[i-1][3] + change_per_glass_K);
  }
}


void HSVtoRGBtoCMYK(int store_in_index, float h, float s, float v) {
  // FIRST PART, HSV to RGB
  h /= 360.0;
  s /= 100.0;
  v /= 100.0;

  int i;
  float f, p, q, t;
  float r = 0;
  float g = 0;
  float b = 0;

  i = floor(h * 6);
  f = h * 6 - i;
  p = v * (1 - s);
  q = v * (1 - f * s);
  t = v * (1 - (1 - f) * s);

  switch (round(i % 6)) {
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
  r = max(0, floor(r*255));
  g = max(0, floor(r*255));
  b = max(0, floor(r*255));

  // SECOND PART, RGB to CMYK

  // BLACK
  if (r==0 && g==0 && b==0) {
    generated_colors[store_in_index][0] = 0;
    generated_colors[store_in_index][1] = 0;
    generated_colors[store_in_index][2] = 0;
    generated_colors[store_in_index][3] = 100;
    return;
  }

  float color_C = 1 - (r/255);
  float color_M = 1 - (g/255);
  float color_Y = 1 - (b/255);
 
  float minCMY = min(color_C, min(color_M, color_Y));
  color_C = (color_C - minCMY) / (1 - minCMY) ;
  color_M = (color_M - minCMY) / (1 - minCMY) ;
  color_Y = (color_Y - minCMY) / (1 - minCMY) ;
  float color_K = minCMY;

  generated_colors[store_in_index][0] = max(0, floor(color_C * 100));
  generated_colors[store_in_index][1] = max(0, floor(color_M * 100));
  generated_colors[store_in_index][2] = max(0, floor(color_Y * 100));
  generated_colors[store_in_index][3] = max(0, floor(color_K * 100));

}

