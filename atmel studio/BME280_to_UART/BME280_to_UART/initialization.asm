;========================================================
;				 Модуль инициализации
;========================================================

init:

	;-------------------------- Инициализация стека
	
	ldi		r17, RAMEND			; Выбор адреса вершины стека 
	out		SPL, r17			; Запись его в регистр стека

	;--------------------------- Инициализация компаратора

	ldi 	r17, 0x80			; Выключение компаратора
	out		ACSR, r17

	;-------------------------- Инициализация Главного предделителя

	ldi		r17, 0x80		    ; 
	out		CLKPR, r17			; 
	ldi		r17, 0x03			; Записываем 3 в регистр r17
	out		CLKPR, r17			; Записываем это число в CLKPR, указывая, что мы делим частоту на 8.

	;-------------------------- Инициализация портов ВВ по умолчанию (на вход с подтягивающим резистором)

	ser		r17
	out		PORTA, r17
	out		PORTB, r17
	out		PORTD, r17

	out		DDRA, CONST_ZERO
	out		DDRB, CONST_ZERO
	out		DDRD, CONST_ZERO

	;-------------------------- Инициализация TWI

	sbi		twi_DDR, twi_SCL
	sbi		twi_DDR, twi_SDA

	ldi		r17, (0<<USISIE)|(0<<USIOIE)|(1<<USIWM1)|(0<<USIWM0)|(1<<USICS1)|(0<<USICS0)|(1<<USICLK)|(0<<USITC)
	out		USICR, r17

	ldi		r17, (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0<<USICNT0)
	out		USISR, r17

	sbi		twi_PORT, twi_SCL
	sbi		twi_PORT, twi_SDA

	;--------------------------- Разрешаем режим сна (idle)

	ldi		r17, 1 << SE
	out		MCUCR, r17
	
	;--------------------------- Инициализация USART

	ldi 	r17, low(bauddivider)
	out 	UBRRL, r17
	ldi 	r17, high(bauddivider)
	out 	UBRRH, r17
 
	out 	UCSRA, r17
 
	ldi 	r17, (1 << RXEN) | (1 << TXEN) | (1 << RXCIE) | (0 << TXCIE) | (0 << UDRIE) | (0 << UCSZ2); прерывания запрещены, передача разрешен.
	out 	UCSRB, r17
	
	;формат кадра - 8 бит, пишем в регистр ucsrc, за это отвечает бит селектор
	ldi 	r17, (0 << UMSEL0) | (0 << UMSEL1) | (1 << UCSZ0) | (1 << UCSZ1) | (0 << UPM1) | (0 << UPM0) | (0 << USBS) | (0 << UCPOL)
	out 	UCSRC, r17
		