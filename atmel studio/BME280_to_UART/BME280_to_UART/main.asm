;
; attiny2313_tm1637.asm
;
; Created: 30.06.2020 14:09:01
; Author : verbkinm
;
; ��������:	
;	����������� ������� � ���� �� 4-� ���������� ������� � ����� TM1637, ������ ������� ds1302. 		
;	���������� ����� ��������, ���� ���������.

.include "tn2313adef.inc"
.include "config.inc"
.include "vars.asm"

;========================================================
;			 ������ ������������ ����
;========================================================

.cseg		 						; ����� �������� ������������ ����
	.org	0						; ��������� �������� ������ �� ����

start:	
	.include "interrupts_vector.asm"
	.include "interrupts.asm"
	.include "initialization.asm"

	ldi		YH,	high(USART_data)	; ���� ����� ���������� �������� ������ �� USART
	ldi		YL,	low(USART_data)

	;-------------------------- �������� ����������� ���� � �������� ����������
	sei
	main:	 
		sleep
		rjmp	main

.include "lib.asm"
.include "twi_mp.asm"
.include "uart.asm"
