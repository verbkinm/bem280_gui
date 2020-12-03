;========================================================
;		Прерывание UART - приём завершен	
;========================================================

_UART_RX:
	push	r17
	push	r18
	push	BYTE

	in		BYTE, UDR	

	sbrs	USER_REG_STATUS, 1
	rjmp	_UART_RX_first_byte

	andi	USER_REG_STATUS, 0xFD	; сброс бита счетчика
	st		Y, BYTE

	ldi		YH,	high(USART_data)	; установка указателя в начало массива
	ldi		YL,	low(USART_data)

	ld		r17, Y					; сравниваем первый принятый байт с нулём
	cpi		r17, 0x00				; и если не ноль
	breq	_UART_RX_read_all_register_and_transmit

	ldd		BYTE, Y+1					; отправляем данные второго байта по адресу первого
	rcall	twi_write_one_byte
	
	_UART_RX_end:
		pop		BYTE
		pop		r18
		pop		r17

	reti

	;-------------------------- получение первого байта по USART
	_UART_RX_first_byte:
		st		Y+, BYTE
		ori		USER_REG_STATUS, 0x02	; установка бита счетчика

		rjmp	_UART_RX_end

	;-------------------------- считывание всех регистров из BME280 и отправка по USART
	_UART_RX_read_all_register_and_transmit:
		ldi		XH,	high(BME280_data)	; куда будем записывать принятые данные от BME280
		ldi		XL,	low(BME280_data)

		ldi		r17, 26					;(0x88 - 0xA1)
		ldi		r18, 0x88
		rcall	twi_read_package

		ldi		r17, 1					;(0xD0)
		ldi		r18, 0xD0
		rcall	twi_read_package

		ldi		r17, 17					;(0xE0 - 0xF0)
		ldi		r18, 0xE0
		rcall	twi_read_package

		ldi		r17, 4					;(0xF2 - 0xF5)
		ldi		r18, 0xF2
		rcall	twi_read_package

		ldi		r17, 8					;(0xF7 - 0xFE)
		ldi		r18, 0xF7
		rcall	twi_read_package

		;-------------------------- отправка по USART
		ldi		r17, 57					; счетчик
		ldi		XH,	high(BME280_data)	; от куда будем считывать данные для отправки по USART
		ldi		XL,	low(BME280_data)

		transmit_all_register_loop:
			dec		r17
			breq	transmit_all_register_end

			ld		BYTE, X+
			rcall	USART_Transmit

			rjmp	transmit_all_register_loop

			transmit_all_register_end:

				rjmp	_UART_RX_end
