;========================================================
;					TWI start
;========================================================
twi_start:
	push	r17

	sbi		twi_PORT, twi_SDA
	sbi		twi_PORT, twi_SCL
	cbi		twi_DDR, twi_SCL

	ldi		r17, (1<<USISIF)|(1<<USIOIF)|(1<<USIPF)|(1<<USIDC)|(0x0<<USICNT0)
	out		USISR, r17

	cbi		twi_PORT, twi_SDA
	rcall	MCU_wait_10mks

	sbi		twi_DDR, twi_SCL
	cbi		twi_PORT, twi_SCL
	sbi		twi_PORT, twi_SDA
	rcall	MCU_wait_10mks

	pop		r17

	ret

;========================================================
;					TWI stop
;========================================================
twi_stop:
 
	cbi		twi_PORT, twi_SCL
	rcall	MCU_wait_10mks
	cbi		twi_PORT, twi_SDA
	rcall	MCU_wait_10mks
	sbi		twi_PORT, twi_SCL
	rcall	MCU_wait_10mks
	sbi		twi_PORT, twi_SDA
	rcall	MCU_wait_10mks
	sbi		USISR, USIPF

	ret

;========================================================
; TWI master_transfer
; входящее r17 (USISR_8BIT или USISR_1BIT)
; исходящее BYTE
;========================================================
twi_master_transfer:

	out		USISR, r17
	ldi		r17, (0<<USISIE)|(0<<USIOIE)|(1<<USIWM1)|(0<<USIWM0)|(1<<USICS1)|(0<<USICS0)|(1<<USICLK)|(1<<USITC)

	loop_send:
		rcall	MCU_wait_10mks
		out		USICR, r17
		rcall	MCU_wait_10mks
		out		USICR, r17

		sbis	USISR, USIOIF
		rjmp	loop_send

	rcall	MCU_wait_10mks
	in		BYTE, USIDR
	ldi		r17, 0xff
	out		USIDR, r17
	sbi		twi_DDR, twi_SDA

	ret

;========================================================
; TWI send
; входящее BYTE
;========================================================
twi_send_byte:
	push	r17
	push	BYTE

	out		USIDR, BYTE
	ldi		r17,  USISR_8BIT
	rcall	twi_master_transfer

	cbi		twi_DDR, twi_SDA
	ldi		r17,  USISR_1BIT
	rcall	twi_master_transfer
	rcall	twi_set_status_bit

	pop		BYTE
	pop		r17
	
	ret

;========================================================
; TWI recive
; исходящее BYTE
;========================================================
twi_recive_byte:
	push	r17

	cbi		twi_DDR, twi_SDA

	ldi		r17,  USISR_8BIT
	rcall	twi_master_transfer
	push	BYTE

	sbrs	USER_REG_STATUS, twi_ACK_NACK
	rjmp	twi_recive_ack

	ldi		r17, 0xFF
	out		USIDR, r17
	rjmp	twi_recive_end

	twi_recive_ack:
		ldi		r17, 0x00
		out		USIDR, r17

	twi_recive_end:
		ldi		r17,  USISR_1BIT
		rcall	twi_master_transfer

		pop		BYTE
		pop		r17

	ret

;========================================================
; Считывание r17 регистров подряд начиная с регистра r18
; запись в RAM по занчению указателя X
;
; входящие:
; r17 - количесвто счтываемых регистров
; r18 - адрес первого считываемого регистра
; X - адрес, куда записываются принятые результаты 
;========================================================
twi_read_package:
	push	r17
	push	r18

	rcall	twi_start

	ldi		BYTE, BME280_addr_write
	rcall	twi_send_byte

	mov		BYTE, r18 
	rcall	twi_send_byte

	rcall	twi_start

	ldi		BYTE, BME280_addr_read
	rcall	twi_send_byte

	twi_read_package_recive:
		dec		r17
		breq	twi_read_package_recive_last_byte

		andi	USER_REG_STATUS, 0xFE	; сброс бита ACK_NACK
		rcall	twi_recive_byte
		st		X+, BYTE

		rjmp	twi_read_package_recive

		twi_read_package_recive_last_byte:
			ori		USER_REG_STATUS, 0x01	; установка бита ACK_NACK
			rcall	twi_recive_byte
			st		X+, BYTE

	rcall	twi_stop

	pop		r18
	pop		r17

	ret

;========================================================
; Установка бита ACK_NACK в регистре USER_REG_STATUS
; входящие:
; BYTE - ответ датчика BME280 (1 или 0)
;========================================================
twi_set_status_bit:
	cpi		BYTE, 0x00
	breq	twi_set_status_bit_ack

	ori		USER_REG_STATUS, 0x01	; установка бита ACK_NACK

	ret

	twi_set_status_bit_ack:
		andi	USER_REG_STATUS, 0xFE	; сброс бита ACK_NACK

	ret

;========================================================
; Запись одного байта
; входящие:
; r17 - адрес регистра
; BYTE - данные, которые запишутся в регистр
;========================================================
twi_write_one_byte:
	push	BYTE
	rcall	twi_start

	ldi		BYTE, BME280_addr_write
	rcall	twi_send_byte

	mov		BYTE, r17 
	rcall	twi_send_byte

	pop		BYTE
	rcall	twi_send_byte

	ret