#include <xc.inc>

; When assembly code is placed in a psect, it can be manipulated as a
; whole by the linker and placed in memory.  
;
; In this example, barfunc is the program section (psect) name, 'local' means
; that the section will not be combined with other sections even if they have
; the same name.  class=CODE means the barfunc must go in the CODE container.
; PIC18's should have a delta (addressible unit size) of 1 (default) since they
; are byte addressible.  PIC10/12/16's have a delta of 2 since they are word
; addressible.  PIC18's should have a reloc (alignment) flag of 2 for any
; psect which contains executable code.  PIC10/12/16's can use the default
; reloc value of 1.  Use one of the psects below for the device you use:

psect   initdisplayfunc,local,class=CODE,reloc=2 ; PIC18

global _initdisplay ; extern of bar function goes in the C source file
;E=RE0, W=RE1, RS=RE2 D4 to D7=RD0 to RD3

_initdisplay:	Call    delay5ms
                movlw   0x04                ;Repeat 8 bit init three times
                movwf   loopr
initdsp8:       movlw   0x03                ;Msb is E on set 4 bit mode,high nibble
		call	dispw1
		call	delay5ms
		decf	loopr
		bnz	initdsp8
init8busy:	call	rbusy
		btfsc	busy, 7
		bra	init8busy
		movlw	0x02
		call	dispw1		    ;After this instruction is 4 bit mode
		nop
init4busy:	call	rbusy
		btfsc	busy, 7		    ;Read the busy flag
		bra	init4busy
initdisp:	movlw   0x02                ;Msb is enable on lower nibble is control
		call	dispw1
		nop
		movlw	0x08
		call	dispw1
		nop
                call    dispset		    ;4 bit type can only set once
		nop
initdisp4:	call	displayoff
		nop
		call    dispclear
		nop
		call	dispentry
		nop
                call    displayon
		nop
                return

dispset:	call	rbusy		    ;Read instruction for busy flag
		incf	loopr		    ;Address 0x87
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	dispset		    ;Reread if busy flag set
		movlw	dispfunc	    ;4 bit mode, 1 line, 5x8 character
		call	dispctlwr	    ;Write instruction
		return

displayoff:	call	rbusy		    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	displayoff	    ;Reread if busy flag set
		movlw	0x08		    ;Display off instruction 0x08
		call	dispctlwr	    ;Write instruction
		return

displayon:	call	rbusy		    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	displayon	    ;Reread if busy flag set
		movlw	dispon		    ;Display on instruction 0x0C
		call	dispctlwr	    ;Write instruction
		return

dispentry:	call	rbusy		    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	dispentry	    ;Reread if busy flag set
		movlw	dispnormal	    ;Increment, right, noshift
		call	dispctlwr	    ;Write instruction
		return

dispclear:	call	rbusy		    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	dispclear	    ;Reread if busy flag set
		clrf	dspaddr		    ;digit address location to zero
		movlw	dispclr		    ;Display clear instruction
		call	dispctlwr	    ;Write instruction
		return

disphome:	call	rbusy		    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	disphome	    ;Reread if busy flag set
		movlw	dsphome		    ;Display home instruction
		call	dispctlwr	    ;Write instruction
		return

displine2:	call	rbusy		    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	displine2	    ;Reread if busy flag set
		movlw	dispaddr	    ;Display address upper 8 digits
		movwf	dspaddr		    ;Current digit address location
		call	dispctlwr	    ;Write instruction
		return

displine1:	call	dispctlrd	    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	displine1	    ;Reread if busy flag set
		movlw	0x80	    	    ;Display first digit address
		call	dispctlwr	    ;Write instruction
		return

dispstring:	tblrd*+
		movff	TABLAT, loopr	    ;Move string length to loopr
dispchar1:	incf	TBLPTRL
		tblrd*+
		movff	TABLAT, WREG
		call	charwrite
		decf	loopr
		bnz	dispchar1
		nop
		return

charwrite:	call	rbusy		    ;Write 1 WREG char to dspaddr digit
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	charwrite	    ;Reread if busy flag set
		movff	WREG, dispbyte
		call	dispdtrwr
		movlw	0x08
		cpfseq	dspaddr
		return
		call	displine2	    ;Skip to upper 8 digits		;
		return

dispinitial:	movlw	"A"
		movff	WREG, dispbyte	    ;Store in dispbyte
		call	disphome
dispiniti:	call	rbusy		    ;Read instruction for busy flag
		btfsc	busy, 7		    ;Bit 7 = busy flag
		bra	dispiniti	    ;Reread if busy flag set
		movff	dispbyte, WREG
		call	dispdtrwr
		incf	dispbyte
		movlw	"I"
		xorwf	dispbyte,0
		bnz	dispini2
		call	displine2	    ;Damned display writes 8 char only
dispini2:	movlw	"P"
		cpfsgt	dispbyte
		bra	dispiniti
		return

dispw1:		movwf	LATB		    ;Write nibble in WREG to display
                bsf	LATB, 4		    ;Enable on
		call	dlynop
		bcf	LATB, 4		    ;Enable off
		call	dlynop
		return

dispctlwr:                                  ;Write control, WREG contains the value to be written
		movff	WREG, WREG_TEMP	    ;save working register
		movlw	0x00		    ;Write instruction R/W 0 & RS=0 0x0
		bra	displaywr
dispdtrwr:	movff	WREG, WREG_TEMP	    ;save working register
		incf	dspaddr		    ;Next digit address
		movlw	0x04		    ;Write data R/W low & RS high 0x04
displaywr:	movwf	LATE		    ;Set RS high or low for write
		movff	WREG_TEMP, WREG	    ;Restore WREG
		swapf	WREG		    ;exchange nibbles
		bcf	WREG, 4		    ;set E bit low
		call	dispw1		    ;Write msn first
		movff	WREG_TEMP, WREG	    ;Restore WREG
		andlw	0x0f		    ;Clear upper 4 bits
		call	dispw1
		nop
		return

dispctlrd:	pushwr			    ;Read control, busy flag = bit 7 Return in WREG
		movlw	0x02		    ;Read instruction R/W high & RS low
		bra	displayrd
dispdtrrd:	movlw	0x06		    ;Read data R/W high & RS high
displayrd:	movwf	LATE		    ;Set up for read
		movlw	0xef		    ;Set port b lower nibble to read
		movwf	TRISB
		bsf	LATB, 4		    ;Set E high
		call	dlynop
		movff	PORTB, WREG	    ;Transfer hi nibble to WREG low nibble
		bcf	LATB, 4		    ;Set E low
		swapf	WREG		    ;Move low nibble to upper
		andlw	0xf0		    ;Set lower nibble to zero
		movff	WREG, busy	    ;Store in WREG_TEMP
		bsf	LATB, 4		    ;Set E high for lower nibble
		call	dlynop
		movff	PORTB, WREG	    ;Transfer low nibble to WREG low nibble
		bcf	LATB, 4		    ;Set E low
		andlw	0x0f		    ;Clear upper bits
		iorwf	busy		    ;Combine upper nibble into WREG_TEMP
		movlw	0xe0
		movwf	TRISB		    ;Revert portb to output
		clrf	LATB
		popwr			    ;Return result in busy
		return

rbusy:		pushwr
		movlw	0x02
		movwf	LATE		    ;Set read instruction
		movff	TRISB, WREG
		iorlw	0x0f		    ;Set B direction to input
		movff	WREG, TRISB
		bsf	LATB, 4		    ;set E on
		call	dlynop
		movff	PORTB, busy	    ;Bit 3 in busy is the busy flag
		bcf	LATB, 4
		call	dlynop
		swapf	busy,0		    ;Swap nibbles result in WREG
		andlw	0xf0
		movff	WREG, busy
		bsf	LATB, 4
		call	dlynop
		movwf	PORTB		    ;Lower nibble to busy
		bcf	LATB, 4
		call	dlynop
		andlw	0x0f
		iorwf	busy, 1
		clrf	TRISB		    ;Restore direction to write.
		popwr
		return

dlynop:		movff	WREG, loopwsv	    ;Set LCD delay here
		movlw	0x14
		movwf	loopnop
dlynop1:	decfsz	loopnop
		bra	dlynop1
		movff	loopwsv, WREG



