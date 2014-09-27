void calculate_color_rows(){

  float total_change_C = (generated_colors[7][1] - generated_colors[0][1]) / (NUMBER_OF_GLASSES - 1);
  float total_change_M = (generated_colors[7][2] - generated_colors[0][2]) / (NUMBER_OF_GLASSES - 1);
  float total_change_Y = (generated_colors[7][3] - generated_colors[0][3]) / (NUMBER_OF_GLASSES - 1);
  float total_change_K = (generated_colors[7][4] - generated_colors[0][4]) / (NUMBER_OF_GLASSES - 1);

  for(int i=1; i < NUMBER_OF_GLASSES -1; i++){
    generated_colors[i][0] = round(generated_colors[i-1][0] + total_change_C);
    generated_colors[i][1] = round(generated_colors[i-1][1] + total_change_M);
    generated_colors[i][2] = round(generated_colors[i-1][2] + total_change_Y);
    generated_colors[i][3] = round(generated_colors[i-1][3] + total_change_K);
  }

Serial.print("C: ");
  for(int i=0;i<10;i++){
    Serial.print(generated_colors[i][1]);
    Serial.print(",\t");
  }
  Serial.println("");
  Serial.print("M: ");
  for(int i=0;i<10;i++){
    Serial.print(color_M[i]);
    Serial.print(",\t");
  }
  Serial.println("");
  Serial.print("Y: ");
  for(int i=0;i<10;i++){
    Serial.print(color_Y[i]);
    Serial.print(",\t");
  }
  Serial.println("");
  Serial.print("K: ");
  for(int i=0;i<10;i++){
    Serial.print(color_K[i]);
    Serial.print(",\t");
  }
  
}


void HSVtoRGBtoCMYK(const float h, const float s, const float v) {
  // FIRST PART, HSV to RGB
  h /= 360;
  s /= 100;
  v /= 100;

  //h /= 559;
  //s /= 559;
  //v /= 559;
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

  float color_C = 0;
  float color_M = 0;
  float color_Y = 0;
  float color_K = 0;  

  // BLACK
  if (r==0 && g==0 && b==0) {
    color_K = 1;
    //float[] output = {
    //  0, 0, 0, 1
    //};
    Serial.print("Just black");
    //return output;
  }

  color_C = 1 - (r/255);
  color_M = 1 - (g/255);
  color_Y = 1 - (b/255);

  float minCMY = min(color_C, min(color_M, color_Y));
  color_C = (color_C - minCMY) / (1 - minCMY) ;
  color_M = (color_M - minCMY) / (1 - minCMY) ;
  color_Y = (color_Y - minCMY) / (1 - minCMY) ;
  color_K = minCMY;
  
  color_C = max(0, floor(color_C * 100));
  color_M = max(0, floor(color_M * 100));
  color_Y = max(0, floor(color_Y * 100));
  color_K = max(0, floor(color_K * 100));
}
