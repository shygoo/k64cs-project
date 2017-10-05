; Kirby 64: The Crystal Shards Analog Patch
; This patch adds basic analog stick support. A CRC update is required after use.
; shygoo 2017

.definelabel BTN_DPAD_RT 0x100
.definelabel BTN_DPAD_LT 0x200
.definelabel BTN_DPAD_DN 0x400
.definelabel BTN_DPAD_UP 0x800

; Y-axis deadzone to keep Kirby from ducking all the time
.definelabel AN_Y_DEADZONE 20

.orga 0x00030A1C
.headersize 0x7FFFF400

.org  0x8002FE1C

ctrl_hook:

	j     ctrl_hijack
	lbu   t6, 0x0000 (a1)
	ctrl_ra:

.org  0x80030388

ctrl_hijack:

	sb    t5, 0x0003 (a0) ; original y-axis write

ctrl_check_x:
	
	beq   t4, r0, ctrl_check_y ; if t4==0, no x
	nop
	bltzl t4, ctrl_check_y
	ori   t3, t3, BTN_DPAD_LT
	ori   t3, t3, BTN_DPAD_RT

ctrl_check_y:
    
	beq   t5, r0, ctrl_store ; if t5==0, no y
	nop
	slti  t9, t5, AN_Y_DEADZONE
	beql  t9, r0, ctrl_store
	ori   t3, t3, BTN_DPAD_UP
	slti  t9, t5, -AN_Y_DEADZONE
	bnel  t9, r0, ctrl_store
	ori   t3, t3, BTN_DPAD_DN
	
ctrl_store:
	
	sh    t3, 0x0000 (a0)
	j     ctrl_ra ; return
	nop