;========================================================
;		���������� UART - ���� ��������	
;========================================================

_UART_RX:
	push	r17
	push	r18
	push	BYTE

	in		BYTE, UDR	

	sbrs	USER_REG_STATUS, 1
	rjmp	_UART_RX_first_byte

	andi	USER_REG_STATUS, 0xFD	; ����� ���� ��������
	st		Y, BYTE

	ldi		YH,	high(USART_data)	; ��������� ��������� � ������ �������
	ldi		YL,	low(USART_data)

	ld		r17, Y					; ���������� ������ �������� ���� � ����
	cpi		r17, 0x00				; � ���� �� ����
	breq	_UART_RX_read_all_register_and_transmit

	ldd		BYTE, Y+1					; ���������� ������ ������� ����� �� ������ �������
	rcall	twi_write_one_byte
	
	_UART_RX_end:
		pop		BYTE
		pop		r18
		pop		r17

	reti

	;-------------------------- ��������� ������� ����� �� USART
	_UART_RX_first_byte:
		st		Y+, BYTE
		ori		USER_REG_STATUS, 0x02	; ��������� ���� ��������

		rjmp	_UART_RX_end

	;-------------------------- ���������� ���� ��������� �� BME280 � �������� �� USART
	_UART_RX_read_all_register_and_transmit:
		ldi		XH,	high(BME280_data)	; ���� ����� ���������� �������� ������ �� BME280
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

		;-------------------------- �������� �� USART
		ldi		r17, 57					; �������
		ldi		XH,	high(BME280_data)	; �� ���� ����� ��������� ������ ��� �������� �� USART
		ldi		XL,	low(BME280_data)

		transmit_all_register_loop:
			dec		r17
			breq	transmit_all_register_end

			ld		BYTE, X+
			rcall	USART_Transmit

			rjmp	transmit_all_register_loop

			transmit_all_register_end:

				rjmp	_UART_RX_end
