.equ 	XTAL = 1000000
.equ 	baudrate = 4800
.equ 	bauddivider = XTAL/(16*baudrate)-1

.equ	BME280_addr_read = 0xED
.equ	BME280_addr_write = 0xEC
.equ	BME280_data_size = 56

.equ	twi_PORT	=	PORTB
.equ	twi_DDR		=	DDRB
.equ	twi_SCL		=	PB7
.equ	twi_SDA		=	PB5
.equ	twi_PIN		=	PINB

.equ	twi_ACK_NACK=	0
.equ	ACK			=	0
.equ	NACK		=	1

.equ	USISR_8BIT	=	0xF0
.equ	USISR_1BIT	=	0xFE
