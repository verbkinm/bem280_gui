;========================================================
;			������������ ������������ ��������
;========================================================

MCU_wait_10mks:						; 10 ��� + ����� �� ������� 
	push	r17

	ldi		r17, 10

	MCU_wait_loop:
		dec		r17
		brne	MCU_wait_loop

	pop		r17

	ret
