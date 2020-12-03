;
; attiny2313_tm1637.asm
;
; Created: 30.06.2020 14:09:01
; Author : verbkinm
;
; Описание:	
;	Отображение времени и даты на 4-х елементном дисплее с чипом TM1637, модуль времени ds1302. 		
;	Управление тремя кнопками, есть будильник.

.include "tn2313adef.inc"
.include "config.inc"
.include "vars.asm"

;========================================================
;			 Начало программного кода
;========================================================

.cseg		 						; Выбор сегмента программного кода
	.org	0						; Установка текущего адреса на ноль

start:	
	.include "interrupts_vector.asm"
	.include "interrupts.asm"
	.include "initialization.asm"

	ldi		YH,	high(USART_data)	; куда будем записывать принятые данные от USART
	ldi		YL,	low(USART_data)

	;-------------------------- Основной бесконечный цикл в ожидании прерывания
	sei
	main:	 
		sleep
		rjmp	main

.include "lib.asm"
.include "twi_mp.asm"
.include "uart.asm"
