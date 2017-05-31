//John Goldsmith
//bass detection
//5_2017

#include <stdint.h>


int16_t sample[64] = {0};
uint16_t n = sizeof(sample)/2;

uint16_t smoother[40] = {0};

uint32_t global_temp;

uint16_t getCenter(){
  uint32_t sum = 0;
  uint32_t k = 2048/n;
  for (int i = 0; i < k; i++){
    uint32_t temp = 0;
    for (int j = 0; j < n; j++){
      sample[i] = analogRead(1);
    }
    for (int j = 0; j < n; j++){
      temp = temp + sample[i];
    }
    sum += temp/n;
  }
  sum = sum/k;
  //for (int i = 0; i < n; i++)Serial.println(sum);
  return sum;
}

uint16_t getSensitivity(){
  uint16_t sens = 0;
  uint16_t avg_amp = 0;
  uint16_t sample_count = 1024/n;
  for (int i = 0; i < sample_count; i++){
    for (int j=0;j<n;j++)sample[i]=analogRead(1);
    uint32_t sum = 0;
    smoother[19] = Sum();
    for(int i = 0; i < 20; i++){
      sum += i*smoother[i];
      smoother[i] = smoother[i+1];
    }
    sum = sum>>7;
    rectify();
    avg_amp = avg_amp + (sum / n);
  }
  avg_amp = avg_amp / sample_count;
  
  
}

uint16_t center = 332;

uint16_t R = 0;        
uint16_t G = 0;
uint16_t B = 0;          //color setting

static const uint8_t red = 9;
static const uint8_t green = 10;
static const uint8_t blue = 11;





void setup() {
  Serial.begin(230400);
  //setPwmFrequency(red, 1);
  //setPwmFrequency(green, 1);
  //setPwmFrequency(blue, 1);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
}

void mov_smooth(uint8_t smoothFactor){
  int16_t tempArray[smoothFactor];
  for (int i = 0; i < smoothFactor; i++){
    tempArray[i] = sample[i];
    sample[i] = 0;
  }
  for (int i = smoothFactor; i < (n-smoothFactor); i++){
    uint32_t temp = 0;
    for (int j = 0; j < smoothFactor; j++){
      temp += tempArray[j];
    }
    temp += sample[i];
    for (int j = i+1; j <= i+smoothFactor; j++){
      temp += sample[j];
    }
    for(int k = 0; k < smoothFactor; k++){
      tempArray[k] = tempArray[k+1];
    }
    tempArray[smoothFactor-1] = sample[i];
    
    temp = temp / (2*smoothFactor+1);
    sample[i] = temp;
  }
  for (int i = n - smoothFactor; i < n; i++){
    sample[i] = 0;
  }
}

void rectify(){
  for (int i = 0; i < n; i++){
    if (!sample[i]);
    else if(sample[i] > center) sample[i] = sample[i] - center;
    else sample[i] = center - sample[i];
  }
}

uint16_t Sum(){
  uint32_t sum = 0;
  for (int i = 0; i < n; i++){
    //Serial.println(sample[i]);
    sum = sum + sample[i];
  }
  return sum;
}



void DIALtoRGB(uint16_t dialpos){
  while (dialpos >= 360) dialpos-=360; 
  uint16_t temp = dialpos;
  uint8_t range = 0;
  
  if (dialpos >= 0 && dialpos <= 180){
    range = 1; 
    R = (255 - 1.416*temp);
    G = 1.416*temp;
  }
  else if (dialpos > 180 && dialpos <= 270){
    range = 2;
    temp -= 180;    //temp between 0 and 90
    G = (255 - 2.833*temp);
    B = 2.833*temp;
  }
  else if (dialpos > 270){
    range = 3;
    temp -= 270;  //temp between 0 and 90
    B = (255 - 2.833*temp);
    R = 2.833*temp;
  }
  return;
}

void setBrightness(uint8_t brightness){
  R = (R*(brightness))/255;
  G = (G*(brightness))/255;
  B = (B*(brightness))/255;
  if(R>255)R=255;
  if(G>255)G=255;
  if(B>255)B=255;
}

uint8_t soundAmp(uint8_t pin, uint8_t sensitivity){
    uint16_t scale = 16;
    
    for (int i=0;i<n;i++)sample[i]=analogRead(pin);
    //adjust sensitivity according to smoothing
    mov_smooth(6);
    rectify();
    uint32_t sum = 0;
    //average with previous values to get desired output smoothing
    ///////////////////////////////
    smoother[19] = Sum();
    for(int i = 0; i < 20; i++){
      sum += i*smoother[i];
      smoother[i] = smoother[i+1];
    }
    sum = sum>>7;
    /////////////////////////////////
    //newest value
    
    /*
    sum += prev_sum/2;    //a smoothing effect 
    sum = sum/2;          //still prone to jittery output graph
    prev_sum = sum;
    */
    sum = scale*sum/n; //scale for amplitude
    Serial.println(sum);
    if(sum < (sensitivity*scale)/4) return 0; //ignore remaining noise
    else sum = sum - (sensitivity*scale)/4;
    if (sum < 256) return sum;
    else return 255;
}

void ampReactive(uint16_t color, uint8_t max_brightness, uint16_t sensitivity){
  uint8_t audtemp = soundAmp(A1, sensitivity);
  DIALtoRGB(color);
  if (audtemp > max_brightness) setBrightness(max_brightness);
  else setBrightness(audtemp);
  analogWrite(red, R);
  analogWrite(blue, B);
  analogWrite(green, G);
}

void loop() {
  delay(50);
  center = getCenter();
  while(1)ampReactive(0, 50, 4);
}
