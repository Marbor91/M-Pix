/*
Changelog:
1.05.20:
 - убраны лишние свои задержки по ненужности
 - добавлено обновление статичных цветов в ленте в ходе исполнения (защита от кривого отображения данных)
 - добавлено считывание файла конфигов. (BRIGHTNESS,)
 - добавлен буфер для параметров и переменная для параметров sd_param
 - изменена переменная sd_data на sd_prog
 - строковые константы оптимизированы с помощью print(F(""))
 - добавлено считывание отдельного файла с параметрами и его парсинг.
 - изменениа конструкция задержек нам милс
 - добавлен черный цвет
- плавное зажигание, погасание цветов
- статика, погасание и зажигание свернуто в функции
 
 Achtung - pixel.show blocks millis function!!!!!!!!!!
 
 
 7.05.21:
 - модернизирован парсер для построчного чтения с флешки блять!
 - добавлена функция чтения кол-ва диодов, яркости и типа ленты с файла параметров
 - функции проигрывания сгружены в settings.h
 - убраны лишние переменные и отладка режимов
 
 7.09.21:
 - Добавлено управление через АСК трансиверы, приемник установлен на вывод Д2. Либа - тиниРФ
 - Изменение работы ресивера. Запуск возможен с кнопки или по приходу радиопакета
 
 27.11.21:
 - Добавлена индикация зарядки акума. Замер и индикация при старте и при самой зарядке
 - Оптимизация функций. Сгрузил остатки шлака в ситинги
 Во время старта устройства от акума - индикация заряда один раз
 Во время зарядки - индикация постоянна
*/

#include <SPI.h>
#include <SD.h>
#include "Adafruit_NeoPixel.h"
#include "settings.h"									// О простите меня быдлокодера, но это был рабочий вариант куда  можно сгрузить всю лапшу из функций и переменных
#include "bits_macros.h"
#include "TinyRF_RX.h"							// ПРиемник вкллючен

#define RECEIVER_PIN		2					// пин для выхода от приемника

const char Param_file []=		"param.txt";		// параметры
const char Prog_file []=	 		"prog.txt";	// сама программа эффектов

//#define DEBUG_ENABLE								// тупо вывод отладки
#ifdef DEBUG_ENABLE
#define DEBUG(x) Serial.println(x)
#else
#define DEBUG(x)
#endif

#define VERSION  "MarPix V1.32"

//==== MILLISTIMER MACRO ====
#define EVERY_MS(x) \
  static uint32_t tmr;\
  bool flag = millis() - tmr >= (x);\
  if (flag) tmr = millis();\
  if (flag)
//===========================
#define LED_PIN 				8
#define VBAT_PIN				A7	// читаю напругу с акума
#define USBV_PIN				2		// power supply from USB

#define BUT_PIN 				7
#define CS_PIN 				4		// пин для флешки
#define BUT_DELAY 			50 	// величина задержки опроса кнопки

// ========ADC Settings abd Charging
#define ADC_RES_DEV 		2
#define SMPL_SIZE  			10
#define ADC_REF 			3300		// в миливольтах
#define ADC_RES				1024
#define V_MIN				3300
#define	 V_MAX				4200
#define V_IND_STEP			225
// пины с контроллера заряда
#define CHR_STDBY_PIN    A4		// если лог 0 то зарядка завершена
#define CHR_PROC_PIN 	A5 		// если лог 0 то зарядка в процессе, одновременно может быть только если зарядка выключена вообще

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_count, LED_PIN, NEO_GRB + NEO_KHZ800);

File SD_File;

#define NUM_SIZE 3					// определяет цифру задания времени (3 знака это число до 999)

const char *headers[]  = 
{
	"chern",
	"kras",
	"siny",
	"zel",
	"h_bel",
	"t_bel",
	"s_siny",
	"t_siny",
	"roz",
	"birus",
	"p_kras",
	"p_zel",
	"p_siny",
	"p_h_bel",
	"p_t_bel",
	"p_s_siny",
	"p_t_siny",
	"p_birus",
	"p_roz",
	"tsvet",
	"shum",
	"pixel",
	"strob",
	"pogas",
	"nebo",
	"meteor",
	"beg_ogn",
	"zmey",
	"Brightness",			// во время чтения параметр файла устанавливает яркость от 0 до 255
	"LEDS",				// во время чтения параметр файла устанавливает кол-во диодов
	"Type_strip"			// во время чтения параметр файла устанавливает тип ленты
	
};		// 0,1,2
				
byte comm_amount = sizeof(headers) / 2;								// вспомогательная переменная для пересчета имен

enum names 
// 0,1,2
{
	chern,
	kras,
	siny,
	zel,
	h_bel,
	t_bel,
	s_siny,
	t_siny,
	roz,
	birus,
	p_kras,
	p_zel,
	p_siny,
	p_h_bel,
	p_t_bel,
	p_s_siny,
	p_t_siny,
	p_birus,
	p_roz,
	tsvet,
	shum,
	pixel,
	strob,
	pogas,
	nebo,
	meteor,
	beg_ogn,
	zmey,
	Brightness,
	LEDS,
	Type_strip
	
} thisName;					// объект для перечета имен

uint16_t sort_type = 0;
uint8_t start_flag = 0;

int Parser(void);					// ф-ция парсинга.
void Play (void);					// проигрывание полученного эффекта
void SD_Error(void);				// ошибка чтения карты, мигаем зеленым
void Charge_state(void);

/*Нажатие кнопки приводит к установке флага и запуску проигрывания*/
ISR (PCINT2_vect)
{
		ClearBit(PCICR,PCIE2);
		start_flag = 1;
}

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(BUT_PIN, INPUT_PULLUP);
  
    analogReference(EXTERNAL);	// 3.3V from CH430 by AREF
  
	SetBit(PCICR,PCIE2);				// установить прерывания по кнопкам 
	SetBit(PCMSK2,PCINT23);			// маску на Д7
	sei();

  strip.begin();
  strip.clear(); // очистить
  strip.setBrightness(LED_Brightness);		// установим пооловину яркости по дефолту
  strip.show();
   // Open serial communications and wait for port to open:
  Serial.begin(115200);
  //Serial.setTimeout(100);   
  Serial.println(F(VERSION));
  Strip_clear();
   Charge_state();		// индикация текущего заряда 
   Strip_clear();
  
  /*================ RECIEVER =======================*/
  setupReceiver(RECEIVER_PIN);
  Serial.println(F("Receiver ready"));
  
/*    	  unsigned char number[10];
	  uint8_t receive_flag = 0;
	  Serial.print(F("Wait delay: "));
	  Serial.setTimeout(5000);
	  sort_type = (uint16_t)Serial.parseInt();
	Serial.print(F("delay: "));
	Serial.println(sort_type);  */
	
  /*================ END =======================*/
  Serial.print(F("Initializing SD card..."));
  	 // see if the card is present and can be initialized:
  while (!SD.begin(CS_PIN)) 
  {
    Serial.println(F("Card failed, or not present"));
	SD_Error();			// хуярим
  }  
  Serial.println(F("card initialized."));
  
// читаем данные параметров
   SD_File = SD.open(Param_file);
  if (SD_File)
	{
		Serial.println(F("Param file open ok"));
		Serial.print(F("Bytes:"));
		Serial.print(SD_File.available());
		while (SD_File.available())				// вообще крутим пока на флехе есть данные
		{
			Parser();					
			Play();			
		}
	SD_File.close(); 	
	}
  else {Serial.println(F("Param file open fail"));}
  
		Serial.print(F("LEDs_num="));
		Serial.println(LED_count);
		strip.updateLength(LED_count);		//обновляем кол-во диодов
		
		Serial.print(F("LED_Brightness="));
		Serial.println(LED_Brightness);
		strip.setBrightness(LED_Brightness);	//обновляем яркость
		
		Serial.print(F("Type_strip ="));			//обновляем тип ленты
		Serial.println(Type_of_strip);
		if		   (Type_of_strip==123) {strip.updateType(NEO_RGB);}
		else if (Type_of_strip==213) {strip.updateType(NEO_GRB);}
		else if (Type_of_strip==132) {strip.updateType(NEO_RBG);}
		else if (Type_of_strip==231) {strip.updateType(NEO_GBR);}
		else if (Type_of_strip==312) {strip.updateType(NEO_BRG);}
		else if (Type_of_strip==321) {strip.updateType(NEO_BGR);}
		else {strip.updateType(NEO_RGB); Serial.println(F("Set as Default RGB"));}
		Serial.println(F("1 - R, G - 2, B -3, 123  = RGB"));
		
// ниже это макрос с либы. чисто для понимания воткнул
//#define NEO_RGB  ((0<<6) | (0<<4) | (1<<2) | (2)) ///< Transmit as R,G,B
//#define NEO_RBG  ((0<<6) | (0<<4) | (2<<2) | (1)) ///< Transmit as R,B,G
//#define NEO_GRB  ((1<<6) | (1<<4) | (0<<2) | (2)) ///< Transmit as G,R,B
//#define NEO_GBR  ((2<<6) | (2<<4) | (0<<2) | (1)) ///< Transmit as G,B,R
//#define NEO_BRG  ((1<<6) | (1<<4) | (2<<2) | (0)) ///< Transmit as B,R,G
//#define NEO_BGR  ((2<<6) | (2<<4) | (1<<2) | (0)) ///< Transmit as B,G,R
     SD_File = SD.open(Prog_file);
  if (SD_File)
	{
		Serial.println(F("Prog file open ok"));
		Serial.print(F("Bytes:"));
		Serial.println(SD_File.available());
		SD_File.close(); 
	}
  else {Serial.println(F("Prog file open fail"));}
}
void loop()
{
	
while(digitalRead(USBV_PIN)&&start_flag==0)		// пока нет старта и есть питание с ЮСЬ крутим индикацию заряда
{
	Charge_state();
}
Strip_clear();

if(start_flag!=0)
	{
		SD_File = SD.open(Prog_file);
		while (SD_File.available())				// вообще крутим пока на флехе есть данные
		{
			Parser();					
			Play();			
		}
		start_flag=0;
		if(BitIsSet(PCIFR,PCIF2)) {SetBit(PCIFR,PCIF2);}		// проверяем флаг прерывания. если утановлен, то сбрасываем единицей
		SetBit(PCICR,PCIE2);													// разрешить снова прерывания по кнопке
	}

/*================ RECIEVER =======================*/
  const uint8_t bufSize = 15;
  byte buf[bufSize];
  uint8_t numLostMsgs = 0;
  uint8_t numRcvdBytes = 0;
  

  uint8_t err = getReceivedData(buf, bufSize, numRcvdBytes, numLostMsgs);
  
 if(err == TRF_ERR_NO_DATA){
    return;
  }

  if(err == TRF_ERR_BUFFER_OVERFLOW){
    //Serial.println("Buffer too small for received data!");
    return;
  }

/*   if(err == TRF_ERR_CORRUPTED){
    //Serial.println("Received corrupted data.");
    //return;
  } */
  
  //**************** ЕСли ловим хоотя бы поврежденный пакет с стартом, то начинаем проигрывание *********************//
  if(err == TRF_ERR_SUCCESS || err == TRF_ERR_CORRUPTED)
  {
/*    	for(uint8_t j =0; j<=bufSize; j++)
	{
		Serial.print((char)buf[j]);
	} 
	Serial.println ('!'); */ 
	//uint8_t i = 0;	 
	start_flag = 1;	
}
  /*================ END =======================*/
}

void Play ()
{
			for (byte i = 0; i < comm_amount; i++)  // пробегаемся по всем именам
			{   
				if (strcmp(sddata.cmd,headers[i])==0) 	// если строки равны, то
				{
					thisName = i;						// то присваиваем соответствующий номер					
					switch (thisName) 			// играем соответствующий эффект
					{
						case chern:
						{
							
							Static_color_burn(black_color);
						}
						break;
						case kras:
						{
							Static_color_burn(red_color);
						}
						break;
						case zel:
						{
							Static_color_burn(green_color);
						}
						break;
						case siny:
						{
							Static_color_burn(blue_color);
						}
						break;
						case h_bel:
						{
							Static_color_burn(coldwhite_color);
						}
						break;
						case t_bel:
						{
							Static_color_burn(warmwhite_color);
						}
						break;
						case s_siny:
						{
							Static_color_burn(lightblue_color);
						}
						break;
						case t_siny:
						{
							Static_color_burn(blue_color);
						}
						break;		
						case roz:
						{
							Static_color_burn(pink_color);
						}
						break;
						case birus:
						{
							Static_color_burn(turquoise_color);
						}
						break;
						case p_kras:
						{
							 FadeInOut(red_color);
						}
						break;
						case p_zel:
						{
							FadeInOut(green_color);
						}
						break;
						case p_siny:
						{
							FadeInOut(blue_color);
						}
						break;
						case p_birus:
						{
							FadeInOut(turquoise_color);
						}
						break;
						case p_roz:
						{
							FadeInOut(pink_color);
						}
						break;
						case p_h_bel:
						{
							FadeInOut(coldwhite_color);
						}
						break;
						case p_t_bel:
						{
							FadeInOut(warmwhite_color);
						}
						break;
						case p_s_siny:
						{
							FadeInOut(lightblue_color);
						}
						break;
						case p_t_siny:
						{
							FadeInOut(blue_color);
						}
						break;
						case nebo:
						{
							 Sparkle(coldwhite_color,30);
						}
						break;
						case meteor:
						{

							 meteorRain(coldwhite_color,15, 64, true, 20);
						}
						break;
						case zmey:
						{
							//DEBUG(F("strt"));
							//DEBUG(millis());
							 NewKITT(turquoise_color,8, 10, 50);
							 //DEBUG(F("fin"));
							//DEBUG(millis());
						}
						break;
						case beg_ogn:
						{
							//DEBUG(F("strt"));
							//DEBUG(millis());
							 RunningLights(lightblue_color,50);
							// DEBUG(F("fin"));
							//DEBUG(millis());
						}
						break;
						 case tsvet:
						{
							Tsvet();
						}
						break;
						case shum:
						{
							Noise();
						}
						break;
						case pixel:
						{
							Pixel();
						}
						break;
						case pogas:
						{
							Pogas();
						}
						break;
						case strob:
						{
							Strob();
						break;
						case Brightness:
						{
							LED_Brightness=sddata.time; // получаем яркость
							sddata.time = 0;						// обнуляем к хуям	
							//DEBUG (F("HUI!"));
						}
						break;
						case LEDS:
						{
							LED_count=sddata.time; // получаем кол-во диодов
							sddata.time = 0;		    // обнуляем к хуям		
						}
						break;
						case Type_strip:
						{
							Type_of_strip=sddata.time;			// получаем тип ленты через число.
							sddata.time = 0;		    // обнуляем к хуям	
						}
						break;
						default:
						{
							Serial.println(F("Unknown command!"));
						}
						break; 
					}
				}
				//else {DEBUG("Unknown command!");}
				}
			}
}

//  Выводит команду в виде символов	 	sddata.cmd [] 
//  Выводит число в виде инт		 	sddata.time[]
int Parser()
{
  volatile byte i = 0; 		//счет для масиивов 
  volatile char j = 0; 		// чтобы тупо сгружать лишние данные в пустоту
  volatile char number[NUM_SIZE];	// временно пихаем числа в виде символов
 
  		number[0] = '\0';			// очистка массива!
		number[1] = '\0';
		number[2] = '\0';
  
  i=0;
  memset(sddata.cmd, 0, sizeof(sddata.cmd));	// очистить массив
  sddata.time = 0;								// очистить переменную
	//Serial.println(F(("Com_read")));
  	while (SD_File.peek()!=',' 
		&& (SD_File.available()>0)
		&&  SD_File.peek()!='\0' ) 			// до того как запятую поймаем и вообще даннные есть, то
    {
    	sddata.cmd[i]=SD_File.read();		// пихаем посимвольно команду
      	i++;
		//Serial.write(sddata.cmd[i]);
		if (i>CMD_LENGTH)										// если команда боольше 15 символов, то что нахуй
		{
			Serial.println(F("Com_lenght_over"));
			return 0;
		}
    }
	//Serial.println(sddata.cmd);
	//Serial.println(i);

	j =SD_File.read(); 		// Запятую пропускаем

    //парсинг команды завершен, идем читать время
	//Serial.println (sddata.cmd);
    i=0;
    while ((SD_File.peek()!='\n'
			  &&SD_File.peek()!='\0'
			  &&SD_File.peek()!='\r')
			  &&(SD_File.available()>0)
			  )// пока не ловим конец строки или файла и данные есть, то
    {
		
    	number[i]=SD_File.read();					// пихаем посимвольно числа времени
      	i++;
		//Serial.write(number[i]);
		if (i>3)								// если команда боольше 20 символов, то что нахуй
		{
			Serial.println(F("Num_lenght_over"));
			return 0;
		}

    }
	i=0;
	j=SD_File.read(); 		// первый символ возврата каретки пропускаем.
	j=SD_File.read(); 		// второй символ возврата каретки пропускаем.



  	sddata.time = atoi(number);					// переводим в инт
	// DEBUG
/* 	Serial.print("NUM:");
	Serial.println(sddata.time,DEC);
	Serial.print(",");
	Serial.write(number[0]);
	Serial.write(number[1]);
	Serial.write(number[2]);
	Serial.write(number[3]); 
	
	//Serial.println(i);
	
	//Serial.println("Cmd_recieved");
	*/

	Serial.print(sddata.cmd);
	Serial.print(',');
	Serial.println(sddata.time);
	
	if (SD_File.available()==0) 
	{
		Serial.println(F("Data is over"));
		Strip_clear();
		return -1;
	}			// если данных больше нет, то выходим
	return 1;
  
}
/* 
void flush_controll_data(void)
{
	memset(sddata.cmd, 0, sizeof(sddata.cmd));	// очистить массив
	sddata.time=0;	// очистить массив
	memset(sddata.color, 0, sizeof(sddata.color));	// очистить массив
} */

// ошибка чтения карты, мигаем красным
void SD_Error()
{
	for (byte j =0;j<5;j++)
	{
		 for (byte i = 0; i < LED_CHRG_IND_COUNT; i++) {strip.setPixelColor(i, yellow_color);  }
		strip.show(); 
		delay(100);
		Strip_clear();
		delay(100);
	}
}
void Charge_state()
{
	/** индикация заряда
		- читаем ацп, усредняем по SMPL_SIZE кол-ву
		- вычисляем реальную напругу в мВ
		- сравниваем с опорными значениями, выводим цветами <0, 0-25,25-50,50-75,100%
		
		Цвета выводятся брутфорсом статикой на LED_CHRG_IND_COUNT кол-во диодов (дефолт 10шт)
	**/
	
				
				uint32_t buf = 0;
				uint16_t raw = 0; 
				//DEBUG("Raw:");
				
				// 10 выборок усредняем, макс число 1024
				for (uint8_t i = 0; i<=SMPL_SIZE-1; i++)
				{
					raw = analogRead(VBAT_PIN);
					//DEBUGL(raw);
					buf+=raw;
				}
				uint16_t val = buf/SMPL_SIZE;
				//DEBUG("Total:");
				//DEBUGL(val);
				
				// получаем напругу в миливольтах
				uint16_t volt_value = uint16_t(4.101*val);		// статический коэффициент при r1=r2 и опорнике в 3.3в
				//uint16_t volt_value = uint16_t((ADC_REF*val)>>10); // упорно криво считает результат. видимо не хватает модификаторов в дефайне
				//DEBUG(F("Total V:"));
				//DEBUGL(volt_value);
				
				// ветвление индикации
				if (volt_value<V_MIN)		// В говно усажен, мигаем
				{
					for (byte j =0;j<5;j++)
					{
						 for (byte i = 0; i < LED_CHRG_IND_COUNT; i++) {strip.setPixelColor(i, red_color);  }
						strip.show(); 
						delay(100);
						Strip_clear();
						delay(100);
					}
					// DEBUGL(F("<0%"));
				}
				
				else if (volt_value>=V_MIN && volt_value<=V_MIN+V_IND_STEP)//0-25%
				{
					 for (byte i = 0; i < LED_CHRG_IND_COUNT; i++)
					  {
						strip.setPixelColor(i, orange_color); 
					  }
					  strip.show();  // Передаем цвета ленте */
					  delay(100);
					//DEBUGL(F("0-25%"));

				}
				else if (volt_value>=V_MIN+V_IND_STEP && volt_value<=V_MIN+2*V_IND_STEP) //25-50%
				{
					for (byte i = 0; i < LED_CHRG_IND_COUNT; i++)
					  {
						strip.setPixelColor(i, yellow_color); 
					  }
					  strip.show();  // Передаем цвета ленте */
					  delay(100);
					//DEBUGL(F("25-50%"));

				}
				else if (volt_value>=V_MIN+2*V_IND_STEP && volt_value<=V_MIN+3*V_IND_STEP)//50-75%
				{
					for (byte i = 0; i < LED_CHRG_IND_COUNT; i++)
					  {
						strip.setPixelColor(i, green_color); 
					  }
					  strip.show();  // Передаем цвета ленте */
					  delay(100);
					//DEBUGL(F("50-75%"));

					//50-75%
				}
				else if (volt_value>=V_MIN+3*V_IND_STEP && volt_value<=V_MIN+4*V_IND_STEP)//75-99%
				{
					for (byte i = 0; i < LED_CHRG_IND_COUNT; i++)
					  {
						strip.setPixelColor(i, blue_color); 
					  }
					  strip.show();  // Передаем цвета ленте */
					  delay(100);
					//DEBUGL(F("75-99%"));

					//75-99%
				}
			
			else if (digitalRead(CHR_STDBY_PIN) ==0)	// 100%
			{
				for (byte i = 0; i < LED_CHRG_IND_COUNT; i++)
					  {
						strip.setPixelColor(i, turquoise_color); 
					  }
					  strip.show(); 
					  delay(100);
					  
					  Strip_clear();
					  delay(100);
				//DEBUGL(F("Full charge"));
			}
			

}
