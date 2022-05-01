

/*
WARNING - 2812 -800KHZ
				2813 - 400/800KHZ
*/

extern Adafruit_NeoPixel strip;

unsigned long time_finish;	
volatile int 	LED_Brightness =  80;		// яркость от 0 до 255
volatile int		 LED_count =	72;			// кол-во диодов. максимальное кстати, тут хз
volatile int Type_of_strip = 213;				// 1 -R, 2 - G, 3 - B. Тип ленты. Подбирается сука эмперически.

#define CMD_LENGTH			15				// длина команды максимальное

struct data {	char cmd[CMD_LENGTH]={};  		// держим в структуре команду в символах и время в инте ОСНОВНАЯ ПЕРЕМЕННАЯ
					unsigned int time=0;
				} sddata;		


#define dimm_DURATION 	1000
#define STEP_DELAY 		50 

//#define DEBUG_ENABLE								// тупо вывод отладки
#ifdef DEBUG_ENABLE
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

//#define DEBUG  0		// вывод отладки включить или нет.

unsigned long lightblue_color = 0x00BCFF;
//unsigned long deepblue_color = 	0x0000FF;
unsigned long turquoise_color = 0x00FFF7;
unsigned long pink_color = 		0xFF00DE;
unsigned long coldwhite_color = 0xFFFAFA;

unsigned long red_color = 			0xFF0000;
unsigned long green_color =		0x00FF00;
unsigned long blue_color = 		0x0000FF;
unsigned long black_color = 		0x000000;
unsigned long violet_color = 		0xEE82EE;
unsigned long warmwhite_color = 		0xfff178;

void Strip_clear(void);			// очистка ленты и проецирование. Можно добавить вывод отладки
void Pause (unsigned long fin_time, unsigned int pause); // пауза в исполнении эффектов
void Static_color_burn(unsigned long color);
void Fall_color_burn(unsigned long color);
void Rise_color_burn(unsigned long color);

void NewKITT(unsigned long color, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
// used by NewKITT
void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);

// used by NewKITT
void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay);
void RunningLights(unsigned long color, int WaveDelay);
void meteorRain(unsigned long color, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay);
void fadeToBlack(int ledNo, byte fadeValue);
void setAll(byte red, byte green, byte blue);
void setPixel(int Pixel, byte red, byte green, byte blue);
void showStrip();
void Sparkle(unsigned long color, int SpeedDelay);
void FadeInOut(unsigned long color);



void Static_color_burn(unsigned long color)
{

	time_finish  =millis() + (unsigned long)(sddata.time)*1000;
/* 	DEBUG("SD_TIME");
	DEBUG(sddata.time);
	DEBUG("CUR");
	DEBUG(millis());
	DEBUG("FIN:");
	DEBUG(time_finish); */
	while (time_finish>millis())		// крутим пока не истечет время эффекта
	{
		 for (byte i = 0; i < LED_count; i++)// Включаем все светодиоды
		  {
			strip.setPixelColor(i, color); //Холодно белый
		  }
		  strip.show();  // Передаем цвета ленте */
		  delay(2*STEP_DELAY);
	}
/* 	DEBUG("OUT:");
	DEBUG(millis()); */
	Strip_clear();
}
void Sparkle(unsigned long color, int SpeedDelay) 
{
	time_finish  =millis() + (unsigned long)(sddata.time)*1000;
	while (time_finish>millis())		// крутим пока не истечет время эффекта
	{
		  int Pixel1 = random(LED_count);
		  int Pixel2 = random(LED_count);
		  strip.setPixelColor(Pixel1, color); //Холодно белый
		  strip.setPixelColor(Pixel2, color); //Холодно белый
		  showStrip();
		  delay(SpeedDelay);
		  setPixel(Pixel1,0,0,0);
		  setPixel(Pixel2,0,0,0);
	}			 
}
void meteorRain(unsigned long color, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) 
{  
    uint8_t red, green, blue;
    red = (color & 0x00ff0000UL) >> 16;
    green = (color & 0x0000ff00UL) >> 8;
    blue = (color & 0x000000ffUL);
   strip.clear(); // очистить

 time_finish  =millis() + (unsigned long)(sddata.time)*1000;
	while (time_finish>millis())		// крутим пока не истечет время эффекта
	{
	   for(int i = 0; i < LED_count+LED_count; i++) {
	   
	   
		// fade brightness all LEDs one step
		for(int j=0; j<LED_count; j++) 
		{
			if( (!meteorRandomDecay) || (random(10)>5) ) { fadeToBlack(j, meteorTrailDecay );}
		}
		if(time_finish<millis()) {goto exit;}
	   
		// draw meteor
		for(int j = 0; j < meteorSize; j++) 
		{
			if( ( i-j <LED_count) && (i-j>=0) ) {strip.setPixelColor(i-j, red, green, blue);}
		}
	    if(time_finish<millis()) {goto exit;}
		
		 strip.show(); // Ждем 500 мс.
		Pause(time_finish,SpeedDelay);
	  }
	}
	exit:
	Strip_clear();
	strip.show();
}
void NewKITT(unsigned long color, int EyeSize, int SpeedDelay, int ReturnDelay){
      uint8_t red, green, blue;
    red = (color & 0x00ff0000UL) >> 16;
    green = (color & 0x0000ff00UL) >> 8;
    blue = (color & 0x000000ffUL);
   strip.clear(); // очистить
  
  time_finish  =millis() + (unsigned long)(sddata.time)*1000;
	while (time_finish>millis())		// крутим пока не истечет время эффекта
	{  
	  //RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	  //LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	  OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	  CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	  //LeftToRight(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	  //RightToLeft(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	  //OutsideToCenter(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	  //CenterToOutside(red, green, blue, EyeSize, SpeedDelay, ReturnDelay);
	}
	strip.clear(); // очистить
	strip.show();
}

// used by NewKITT
void CenterToOutside(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i =((LED_count-EyeSize)/2); i>=0; i--) {
    setAll(0,0,0);
    
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue); 
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    
    setPixel(LED_count-i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(LED_count-i-j, red, green, blue); 
    }
    setPixel(LED_count-i-EyeSize-1, red/10, green/10, blue/10);
    
    showStrip();
    Pause(time_finish,SpeedDelay);
  }
  Pause(time_finish,ReturnDelay);
}

// used by NewKITT
void OutsideToCenter(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i<=((LED_count-EyeSize)/2); i++) {
    setAll(0,0,0);
    
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue); 
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    
    setPixel(LED_count-i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(LED_count-i-j, red, green, blue); 
    }
    setPixel(LED_count-i-EyeSize-1, red/10, green/10, blue/10);
    
    showStrip();
    Pause(time_finish,SpeedDelay);
  }
  Pause(time_finish,ReturnDelay);
}

// used by NewKITT
void LeftToRight(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = 0; i < LED_count-EyeSize-2; i++) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue); 
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}

// used by NewKITT
void RightToLeft(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay) {
  for(int i = LED_count-EyeSize-2; i > 0; i--) {
    setAll(0,0,0);
    setPixel(i, red/10, green/10, blue/10);
    for(int j = 1; j <= EyeSize; j++) {
      setPixel(i+j, red, green, blue); 
    }
    setPixel(i+EyeSize+1, red/10, green/10, blue/10);
    showStrip();
    delay(SpeedDelay);
  }
  delay(ReturnDelay);
}
void RunningLights(unsigned long color, int WaveDelay) {

 uint8_t red, green, blue;
    red = (color & 0x00ff0000UL) >> 16;
    green = (color & 0x0000ff00UL) >> 8;
    blue = (color & 0x000000ffUL);
   strip.clear(); // очистить
  int Position=0;
  time_finish  =millis() + (unsigned long)(sddata.time)*1000;
  //DEBUG(sddata.time);
	while (time_finish>millis())		// крутим пока не истечет время эффекта
	{
					for(int i=0; i<LED_count*2; i++)
				   {
					  Position++; // = 0; //Position + Rate;
					  for(int i=0; i<LED_count; i++) {
						// sine wave, 3 offset waves make a rainbow!
						//float level = sin(i+Position) * 127 + 128;
						//setPixel(i,level,0,0);
						//float level = sin(i+Position) * 127 + 128;
						setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
								   ((sin(i+Position) * 127 + 128)/255)*green,
								   ((sin(i+Position) * 127 + 128)/255)*blue);
					  }
					  showStrip();
					  Pause(time_finish,WaveDelay);
					  if (time_finish<millis()) {goto exit;}
				  }
	}
	exit:
	Strip_clear();
	showStrip();
}
void fadeToBlack(int ledNo, byte fadeValue) 
{
    uint32_t oldColor;
    uint8_t r, g, b;
    //int value;
   
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   
    strip.setPixelColor(ledNo, r,g,b);
}
void FadeInOut(unsigned long color)
{
	byte  step = dimm_DURATION / LED_Brightness;
	//byte step = 15;
	time_finish  =millis() + (unsigned long)(sddata.time)*1000;
	DEBUG(F("BRG:"));
	DEBUG(LED_Brightness);
	while (time_finish>millis())		// крутим пока не истечет время эффекта
	{		
		 for(byte k = 0; k < LED_Brightness; k=k+1) 
		 { 
				strip.setBrightness(k);
				for (byte i = 0; i < LED_count; i++){strip.setPixelColor(i, color);}
				strip.show();
				Pause(time_finish,step);
		  }
		  for(byte k = LED_Brightness; k > 0; k=k-1) 
		  { 
				strip.setBrightness(k);
				//DEBUG(k);
				for (byte i = 0; i < LED_count; i++){strip.setPixelColor(i, color);}
				strip.show();
				Pause(time_finish,step);
		  } 
		/* 
			uint32_t oldColor;
			uint8_t r, g, b;
			byte fadeValue = 10;
			
			oldColor = color;
			r = (oldColor & 0x00ff0000UL) >> 16;
			g = (oldColor & 0x0000ff00UL) >> 8;
			b = (oldColor & 0x000000ffUL);
		while (r||g||b)										// пока до низа не дошли по яркости по всем =0
		{
			for (byte i = 0; i < LED_count; i++)			// в каждый диод пишем
			{
				oldColor = strip.getPixelColor(i);
				r = (oldColor & 0x00ff0000UL) >> 16;
				g = (oldColor & 0x0000ff00UL) >> 8;
				b = (oldColor & 0x000000ffUL);

				r=(r<=10)? 0 : (int) r-fadeValue;
				g=(g<=10)? 0 : (int) g-fadeValue;
				b=(b<=10)? 0 : (int) b-fadeValue;
			   
				strip.setPixelColor(i, r,g,b);			// меньше яркость пишем
				
			}
			strip.show();
			Pause(time_finish,step);							// пауза
		}
		while (r==250||g==250||b==250)										// пока до низа не дошли по яркости по всем =0
		{
			for (byte i = 0; i < LED_count; i++)			// в каждый диод пишем
			{
				oldColor = strip.getPixelColor(i);
				r = (oldColor & 0x00ff0000UL) >> 16;
				g = (oldColor & 0x0000ff00UL) >> 8;
				b = (oldColor & 0x000000ffUL);

				r=(r>=250)? 250 : (int) r+fadeValue;
				g=(g>=250)? 250 : (int) g+fadeValue;
				b=(b>=250)? 250 : (int) b+fadeValue;
			   
				strip.setPixelColor(i, r,g,b);			// меньше яркость пишем
				
			}
			strip.show();
			Pause(time_finish,step);							// пауза */
	}			
	Strip_clear();
 	strip.setBrightness(LED_Brightness);
	DEBUG(F("BRG:"));
	DEBUG(LED_Brightness);
}
// Apply LED color changes
void showStrip() 
{
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}

void Pause (unsigned long fin_time, unsigned int pause)
// - fin_time- время конца эффекта вообще
// - duration - время проигрывания эффекта вообще 
// - длительность паузы
{
	//DEBUG(F("p_fin:"));
	//DEBUG(fin_time);
	volatile unsigned long new_start_time = millis(); 
		//DEBUG(F("strt:"));
		//DEBUG(millis());
	// крутим паузу до тех пор пока
	if (fin_time>millis()) 
	{
		while(new_start_time+pause>millis())
		   {delay(1);
			//DEBUG(F("Millis:"));
			//DEBUG(millis());
			//DEBUG(F("Fin:"));
			//DEBUG(new_start_time);
			}		//// не завершилось общее время эффекта и время паузы
	}
	//DEBUG(F("finished:"));
	//DEBUG(millis());
}
// Set all LEDs to a given color and apply it (visible)
void setAll(byte red, byte green, byte blue) 
{
  for(int i = 0; i < LED_count; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}

void Strip_clear(void)
{
  strip.clear(); // очистить
  strip.show();
  //DEBUG(F("Com. finish:"));
  //DEBUG(headers[thisName]);
 
}
// Set a LED color (not yet visible)
void setPixel(int Pixel, byte red, byte green, byte blue) 
{
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}