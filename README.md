# M-Pix LED controller
Led controller based on Atmega328p(arduino) for WS2812 leds
___
### Features:
* MicroSD support via txt file
* ~28 effects
* Simple radio controll via Tiny.h library. Master and slave angaged.

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
 
 # Features are not deployed