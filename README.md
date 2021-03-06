# Zdalnie sterowana łódź

Projekt składa się z dwóch urządzeń :
- nadajnika - kontrolera 
- odbiornika - zdalnie sterowanej łodzi

**Wykorzystane elementy w nadajniku**
- Nucleo F446RE jako element sterujący
- NRF24L01 jako element przesyłający sygnały do łódki (radiowo po SPI)
- LCD1602A jako element wyświetlający na żywo przesyłane informacje do łódki, czyli prędkość silników oraz kierunek płynięcia w skali 0 - 100 (I2C)
- 2x Joystick jako user input dla prędkości silników oraz kierunku płynięcia
- L4940V5 jako stabilizator LDO 5V dla zasilania wspomagany kondensatorami elektrolitycznymi 10uF low ESR na wejściu i wyjściu
- Akumulator Litowo - Polimerowy 250mAh o napięciu 7.4V

**Wykorzystane elementy w odbiorniku**
- Nucleo F446RE jako element sterujący
- NRF24L01 jako element odbierający sygnały z kontrolera (radiowo po SPI)
- TB6612FNG jako dwukanałowy sterownik do silników
- 2x mini silnik DC 3-9V z przekładnią 10:1 2000RPM
- L4940V5 jako stabilizator LDO 5V dla zasilania wspomagany kondensatorami elektrolitycznymi 10uF low ESR na wejściu i wyjściu
- Akumulator Litowo - Polimerowy 1600mAh o napięciu 7.4V

# Postępy pracy

**Na dzień 13.01.2021**
- Komunikacja między dwoma płytkami - zrobione
- Pobieranie danych z joysticków - zrobione
- Przesyłanie danych do wyświetlacza LCD - zrobione
- Układ zasilania dla obydwóch płytek - zrobione
- Sterowanie silnikami - zrobione
- Skonstruowanie kadłuba łódki - zrobione
- Wodowanie i testy - zrobione
- Nagranie filmiku - zrobione

**Link do filmiku w serwisie Youtube**
https://www.youtube.com/watch?v=K5Uee5F6XK4
