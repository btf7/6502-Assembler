

.ORG $1234

 
;
;;
;INX
;INX;INX
 ;
 ;INX

; This file should be used to test if valid ASM files are assembled correctly
; It will not detect any bugs relating to invalid ASM being accepted when it shouldn't

; Test start location
INX

; Test if instructions are read correctly with spacing/comments
.ORG $8000
INX
INX 
INX  
INX;asdf
INX ;asdf
INX  ;asdf
 INX
 INX 
 INX  
 INX;asdf
 INX ;asdf
 INX  ;asdf
  INX
  INX 
  INX  
  INX;asdf
  INX ;asdf
  INX  ;asdf

; Test if addressing modes are detected correctly
.ORG $8020
INX
ASL A
LDA #$12
LDA $34
LDA $56,X
STX $78,Y
BEQ *+2+$21
LDA $6543
LDA $0987,X
LDA $dcba,Y
JMP ($1357)
LDA ($24,X)
LDA ($68),Y

; Test if arguments read correctly with spacing/comments
.ORG $8040
LDA 123
LDA  123
LDA 123 
LDA  123 
LDA 123  
LDA  123  
LDA 123;asdf
LDA  123;asdf
LDA 123 ;asdf
LDA  123 ;asdf
LDA 123  ;asdf
LDA  123  ;asdf
.ORG $8060
 LDA 123
 LDA  123
 LDA 123 
 LDA  123 
 LDA 123  
 LDA  123  
 LDA 123;asdf
 LDA  123;asdf
 LDA 123 ;asdf
 LDA  123 ;asdf
 LDA 123  ;asdf
 LDA  123  ;asdf
.ORG $8080
  LDA 123
  LDA  123
  LDA 123 
  LDA  123 
  LDA 123  
  LDA  123  
  LDA 123;asdf
  LDA  123;asdf
  LDA 123 ;asdf
  LDA  123 ;asdf
  LDA 123  ;asdf
  LDA  123  ;asdf

; Test if numbers are read correctly
.ORG $80a0
LDA 0
LDA 7
LDA 10
LDA 39
LDA 100
LDA 153
LDA 255
.ORG $80c0
LDA $00
LDA $05
LDA $40
LDA $45
LDA $aB
LDA $Cd
LDA $ff
LDA $FF
LDA %00000000
LDA %01010110
LDA %11111111
.ORG $80e0
LDA 256
LDA 300
LDA 567
LDA 1000
LDA 3692
LDA 10000
LDA 14703
LDA 65535
.ORG $8100
LDA $0000
LDA $5000
LDA $0600
LDA $0070
LDA $0008
LDA $5678
LDA $abCD
LDA $AbCd
LDA $ffff
LDA $FFFF
.ORG $8120
LDA %0000000000000000
LDA %0000000010101001
LDA %0101011000000000
LDA %0101011010101001
LDA %1111111111111111

; Test if numbers are read correctly in different addressing modes
.ORG $8140
LDA #0
LDA #1
LDA #10
LDA #12
LDA #100
LDA #123
LDA #$12
LDA #%00010010
.ORG $8160
LDA 0
LDA 1
LDA 10
LDA 12
LDA 100
LDA 123
LDA $12
LDA %00010010
.ORG $8180
LDA 0,X
LDA 1,X
LDA 10,X
LDA 12,X
LDA 100,X
LDA 123,X
LDA $12,X
LDA %00010010,X
.ORG $81a0
STX 0,Y
STX 1,Y
STX 10,Y
STX 12,Y
STX 100,Y
STX 123,Y
STX $12,Y
STX %00010010,Y
.ORG $81c0
BEQ *+2+0
BEQ *+2+1
BEQ *+2+10
BEQ *+2+12
BEQ *+2+100
BEQ *+2+123
BEQ *+2+$12
BEQ *+2+%00010010
.ORG $81e0
LDA (0,X)
LDA (1,X)
LDA (10,X)
LDA (12,X)
LDA (100,X)
LDA (123,X)
LDA ($12,X)
LDA (%00010010,X)
.ORG $8200
LDA (0),Y
LDA (1),Y
LDA (10),Y
LDA (12),Y
LDA (100),Y
LDA (123),Y
LDA ($12),Y
LDA (%00010010),Y
.ORG $8220
LDA 256
LDA 300
LDA 1000
LDA 1234
LDA 10000
LDA 12345
LDA $1234
LDA %0001001000110100
.ORG $8240
LDA 256,X
LDA 300,X
LDA 1000,X
LDA 1234,X
LDA 10000,X
LDA 12345,X
LDA $1234,X
LDA %0001001000110100,X
.ORG $8260
LDA 256,Y
LDA 300,Y
LDA 1000,Y
LDA 1234,Y
LDA 10000,Y
LDA 12345,Y
LDA $1234,Y
LDA %0001001000110100,Y
.ORG $8280
JMP (256)
JMP (300)
JMP (1000)
JMP (1234)
JMP (10000)
JMP (12345)
JMP ($1234)
JMP (%0001001000110100)

; Test if addition and subtraction work
.ORG $82a0
LDA 100
LDA 100+20
LDA 100+20+3
LDA 100-20
LDA 100-20-3
LDA 100+20-3
LDA 100-20+3
.ORG $82c0
LDA (100,X)
LDA (100+20,X)
LDA (100+20+3,X)
LDA (100-20,X)
LDA (100-20-3,X)
LDA (100+20-3,X)
LDA (100-20+3,X)

; Test if * works
.ORG $82e0
LDA *
LDA *-3
LDA 10+*
LDA *+0-9
LDA 0+*-12
LDA 0-15+*

; Test .BYTE
.ORG $8300
.BYTE 0
.BYTE 1
.BYTE 10
.BYTE 12
.BYTE 100
.BYTE 123
.BYTE 255
.BYTE $00
.BYTE $12
.BYTE $ff
.BYTE %00000000
.BYTE %01010110
.BYTE %11111111
.ORG $8320
.BYTE 0 1 10 12 100 123 255 $00 $12 $ff %00000000 %01010110 %11111111
.ORG $8340
.BYTE  0  1  10  12  100  123  255  $00  $12  $ff  %00000000  %01010110  %11111111 
.ORG $8360
.BYTE 17+23
.BYTE 17+$17
.BYTE 17+%00010111
.BYTE $11+23
.BYTE $11+$17
.BYTE $11+%00010111
.BYTE %00010001+23
.BYTE %00010001+$17
.BYTE %00010001+%00010111
.BYTE 23-17
.BYTE $17-17
.BYTE %00010111-17
.BYTE 23-$11
.BYTE $17-$11
.BYTE %00010111-$11
.BYTE 23-%00010001
.BYTE $17-%00010001
.BYTE %00010111-%00010001
.BYTE 100
.BYTE 100+20
.BYTE 100+20+3
.BYTE 100-20
.BYTE 100-20-3
.BYTE 100+20-3
.BYTE 100-20+3
.ORG $8380
.BYTE 17+23 17+$17 17+%00010111 $11+23 $11+$17 $11+%00010111 %00010001+23 %00010001+$17 %00010001+%00010111
.BYTE 23-17 $17-17 %00010111-17 23-$11 $17-$11 %00010111-$11 23-%00010001 $17-%00010001 %00010111-%00010001
.BYTE 100 100+20 100+20+3 100-20 100-20-3 100+20-3 100-20+3

; Test .WORD
.ORG $83a0
.WORD 0
.WORD 1
.WORD 10
.WORD 12
.WORD 100
.WORD 123
.WORD 1000
.WORD 1234
.WORD 10000
.WORD 12345
.WORD 65535
.ORG $83c0
.WORD 0 1 10 12 100 123 1000 1234 10000 12345 65535
.ORG $83e0
.WORD  0  1  10  12  100  123  1000  1234  10000  12345  65535 
.ORG $8400
.WORD $00
.WORD $12
.WORD $ff
.WORD $0000
.WORD $1200
.WORD $0034
.WORD $1234
.WORD $ffff
.ORG $8420
.WORD $00 $12 $ff $0000 $1200 $0034 $1234 $ffff
.ORG $8440
.WORD %00000000
.WORD %00010010
.WORD %11111111
.WORD %0000000000000000
.WORD %0001001000000000
.WORD %0000000000110100
.WORD %0001001000110100
.WORD %1111111111111111
.ORG $8460
.WORD %00000000 %00010010 %11111111 %0000000000000000 %0001001000000000 %0000000000110100 %0001001000110100 %1111111111111111
.ORG $8480
.WORD 1
.WORD 1+20
.WORD 1+20+300
.WORD 1+20+300+4000
.WORD 1+20+300+4000+50000
.WORD 50000
.WORD 50000-4000
.WORD 50000-4000-300
.WORD 50000-4000-300-20
.WORD 50000-4000-300-20-1
.WORD 50000+4000-300+20-1
.ORG $84a0
.WORD 1 1+20 1+20+300 1+20+300+4000 1+20+300+4000+50000 50000 50000-4000 50000-4000-300 50000-4000-300-20 50000-4000-300-20-1 50000+4000-300+20-1

; Test if it picks the correct size (1 or 2 bytes) given different inputs
.ORG $84c0
LDA $ff
LDA $ff+1
LDA $00ff+1
LDA 1+$00ff
LDA 1+2+3+4+$0005

; Test constant definitions
.DEF BYTE ba 0
.DEF BYTE bb 1
.DEF BYTE bc 10
.DEF BYTE bd 12
.DEF BYTE be 100
.DEF BYTE bf 123
.DEF BYTE bg 255
.DEF BYTE bh $00
.DEF BYTE bi $12
.DEF BYTE bj $ff
.DEF BYTE bk %00000000
.DEF BYTE bl %01010110
.DEF BYTE bm %11111111
.DEF BYTE bn 100
.DEF BYTE bo 100+20
.DEF BYTE bp 100+20+3
.DEF BYTE bq 100-20
.DEF BYTE br 100-20-3
.DEF BYTE bs 100+20-3
.DEF BYTE bt 100-20+3
.ORG $84e0
.BYTE ba bb bc bd be bf bg bh bi bj bk bl bm bn bo bp bq br bs bt
.DEF WORD wa 0
.DEF WORD wb 1
.DEF WORD wc 10
.DEF WORD wd 12
.DEF WORD we 100
.DEF WORD wf 123
.DEF WORD wg 1000
.DEF WORD wh 1234
.DEF WORD wi 10000
.DEF WORD wj 12345
.DEF WORD wk 65535
.ORG $8500
.WORD wa wb wc wd we wf wg wh wi wj wk
.DEF WORD wl $00
.DEF WORD wm $12
.DEF WORD wn $ff
.DEF WORD wo $0000
.DEF WORD wp $1200
.DEF WORD wq $0034
.DEF WORD wr $1234
.DEF WORD ws $ffff
.ORG $8520
.WORD wl wm wn wo wp wq wr ws
.DEF WORD wt %00000000
.DEF WORD wu %00010010
.DEF WORD wv %11111111
.DEF WORD ww %0000000000000000
.DEF WORD wx %0001001000000000
.DEF WORD wy %0000000000110100
.DEF WORD wz %0001001000110100
.DEF WORD waa %1111111111111111
.ORG $8540
.WORD wt wu wv ww wx wy wz waa
.DEF WORD wab 1
.DEF WORD wac 1+20
.DEF WORD wad 1+20+300
.DEF WORD wae 1+20+300+4000
.DEF WORD waf 1+20+300+4000+50000
.DEF WORD wag 50000
.DEF WORD wah 50000-4000
.DEF WORD wai 50000-4000-300
.DEF WORD waj 50000-4000-300-20
.DEF WORD wak 50000-4000-300-20-1
.DEF WORD wal 50000+4000-300+20-1
.ORG $8560
.WORD wab wac wad wae waf wag wah wai waj wak wal

; Test forward referenced constants
.ORG $8580
.BYTE bu bv bw
.ORG $85a0
.WORD wam wan wao

; Test constant definitions based on previous constants
; be = 100
.DEF BYTE bu be+20
.DEF BYTE bv 20+be-3
.DEF BYTE bw 120-3-be
.ORG $85c0
.BYTE bu bv bw
; wi = 10000
.DEF WORD wam wi+5
.DEF WORD wan 200+wi-5
.DEF WORD wao 10000+12345-wi
.ORG $85e0
.WORD wam wan wao

; Test labels
.ORG $8600
la:
.BYTE $ff
lb:
lc:
.BYTE $ff
ld:
.BYTE $ff
.WORD $0000 $0000
le:
lf:
.BYTE $ff
.ORG $8620
.WORD la lb lc ld le lf

; Test forward referenced labels
.ORG $8640
.WORD lg lh li
.ORG $8660
.BYTE $00
lg:
.BYTE $ff $00
lh:
.BYTE $ff
.WORD $0000 $0000 $0000
li:
.BYTE $ff

; Test LO and HI
; wr = $1234
.ORG $8680
LDA LO wr
LDA HI wr
LDA LO wr+HI wr
LDA LO wr+$32-$11
LDA $32+LO wr-$11
LDA $32-$11+LO wr
LDA HI wr+$32-$11
LDA $32+HI wr-$11
LDA $32-$11+HI wr
LDA $50+LO wr-HI wr
LDA LO wr+$50-HI wr
LDA LO wr-HI wr+$50
.ORG $86a0
.BYTE LO wr HI wr LO wr+HI wr LO wr-HI wr
.WORD li
.BYTE LO li HI li LO li+HI li

; Test all instructions with all addressing modes
.ORG $86a0
ADC #$23
ADC $23
ADC $23,X
ADC $1234
ADC $1234,X
ADC $1234,Y
ADC ($23,X)
ADC ($23),Y

.ORG $86c0
AND #$23
AND $23
AND $23,X
AND $1234
AND $1234,X
AND $1234,Y
AND ($23,X)
AND ($23),Y

.ORG $86e0
ASL A
ASL $23
ASL $23,X
ASL $1234
ASL $1234,X

.ORG $8700
BCC *+$25

.ORG $8720
BCS *+$25

.ORG $8740
BEQ *+$25

.ORG $8760
BIT $23
BIT $1234

.ORG $8780
BMI *+$25

.ORG $87a0
BNE *+$25

.ORG $87c0
BPL *+$25

.ORG $87e0
BRK

.ORG $8800
BVC *+$25

.ORG $8820
BVS *+$25

.ORG $8840
CLC

.ORG $8860
CLD

.ORG $8880
CLI

.ORG $88a0
CLV

.ORG $88c0
CMP #$23
CMP $23
CMP $23,X
CMP $1234
CMP $1234,X
CMP $1234,Y
CMP ($23,X)
CMP ($23),Y

.ORG $88e0
CPX #$23
CPX $23
CPX $1234

.ORG $8900
CPY #$23
CPY $23
CPY $1234

.ORG $8920
DEC $23
DEC $23,X
DEC $1234
DEC $1234,X

.ORG $8940
DEX

.ORG $8960
DEY

.ORG $8980
EOR #$23
EOR $23
EOR $23,X
EOR $1234
EOR $1234,X
EOR $1234,Y
EOR ($23,X)
EOR ($23),Y

.ORG $89a0
INC $23
INC $23,X
INC $1234
INC $1234,X

.ORG $89c0
INX

.ORG $89e0
INY

.ORG $8a00
JMP $1234
JMP ($1234)

.ORG $8a20
JSR $1234

.ORG $8a40
LDA #$23
LDA $23
LDA $23,X
LDA $1234
LDA $1234,X
LDA $1234,Y
LDA ($23,X)
LDA ($23),Y

.ORG $8a60
LDX #$23
LDX $23
LDX $23,Y
LDX $1234
LDX $1234,Y

.ORG $8a80
LDY #$23
LDY $23
LDY $23,X
LDY $1234
LDY $1234,X

.ORG $8aa0
LSR A
LSR $23
LSR $23,X
LSR $1234
LSR $1234,X

.ORG $8ac0
NOP

.ORG $8ae0
ORA #$23
ORA $23
ORA $23,X
ORA $1234
ORA $1234,X
ORA $1234,Y
ORA ($23,X)
ORA ($23),Y

.ORG $8b00
PHA

.ORG $8b20
PHP

.ORG $8b40
PLA

.ORG $8b60
PLP

.ORG $8b80
ROL A
ROL $23
ROL $23,X
ROL $1234
ROL $1234,X

.ORG $8ba0
ROR A
ROR $23
ROR $23,X
ROR $1234
ROR $1234,X

.ORG $8bc0
RTI

.ORG $8be0
RTS

.ORG $8c00
SBC #$23
SBC $23
SBC $23,X
SBC $1234
SBC $1234,X
SBC $1234,Y
SBC ($23,X)
SBC ($23),Y

.ORG $8c20
SEC

.ORG $8c40
SED

.ORG $8c60
SEI

.ORG $8c80
STA $23
STA $23,X
STA $1234
STA $1234,X
STA $1234,Y
STA ($23,X)
STA ($23),Y

.ORG $8ca0
STX $23
STX $23,Y
STX $1234

.ORG $8cc0
STY $23
STY $23,X
STY $1234

.ORG $8ce0
TAX

.ORG $8d00
TAY

.ORG $8d20
TSX

.ORG $8d40
TXA

.ORG $8d60
TXS

.ORG $8d80
TYA

; Test if instructions read constants and labels both forwards and backwards referenced
.ORG $8da0
lj:
.DEF BYTE bx 123
.DEF WORD wap $0102
LDA bx
BEQ *+bx
BNE *+wap-$0100
BCC lj
BCS lj+2
.ORG $8dc0
LDA by
BEQ *+by
BNE *+waq-$2000
BCC lk
BCS lk+2
lk:
.DEF BYTE by 21
.DEF WORD waq $2010

; Test if constants with overlapping names (eg. asd and asdf) get confused
.DEF BYTE bsmallbig $12
.DEF BYTE bsmallbigg $13
.DEF BYTE bbigsmalll $31
.DEF BYTE bbigsmall $21
.DEF WORD wsmallbig $1245
.DEF WORD wsmallbigg $1346
.DEF WORD wbigsmalll $6431
.DEF WORD wbigsmall $5421
.ORG $1245
lsmallbig:
.ORG $1346
lsmallbigg:
.ORG $6431
lbigsmalll:
.ORG $5421
lbigsmall:
.ORG $8de0
.BYTE bsmallbig bsmallbigg bbigsmalll bbigsmall
.ORG $8e00
.WORD wsmallbig wsmallbigg wbigsmalll wbigsmall
.ORG $8e20
.WORD lsmallbig lsmallbigg lbigsmalll lbigsmall

; Test if instructions are correctly read with mix of upper and lower case
.ORG $8e40
LDA $12
LDa $12
LdA $12
Lda $12
lDA $12
lDa $12
ldA $12
lda $12
.DEF BYTE baa $01
.DEf BYTe bab $02
.DeF BYtE bac $03
.Def BYte bad $04
.dEF ByTE bae $05
.dEf ByTe baf $06
.deF BytE bag $07
.def Byte bah $08
.DEF bYTE bai $09
.DEf bYTe baj $10
.DeF bYtE bak $11
.Def bYte bal $12
.dEF byTE bam $13
.dEf byTe ban $14
.deF bytE bao $15
.def byte bap $16
.DEF WORD wba $0117
.DEf WORd wbb $0218
.DeF WOrD wbc $0319
.Def WOrd wbd $0420
.dEF WoRD wbe $0521
.dEf WoRd wbf $0622
.deF WorD wbg $0723
.def Word wbh $0824
.DEF wORD wbi $0925
.DEf wORd wbj $1026
.DeF wOrD wbk $1127
.Def wOrd wbl $1228
.dEF woRD wbm $1329
.dEf woRd wbn $1430
.deF worD wbo $1531
.def word wbp $1632
.ORG $8e60
.BYTE baa
.ORg $8e70
.BYTe bab
.OrG $8e80
.BYtE bac
.Org $8e90
.BYte bad
.oRG $8ea0
.ByTE bae
.oRg $8eb0
.ByTe baf
.orG $8ec0
.BytE bag
.org $8ed0
.Byte bah
.ORG $8ee0
.bYTE bai
.ORg $8ef0
.bYTe baj
.OrG $8f00
.bYtE bak
.Org $8f10
.bYte bal
.oRG $8f20
.byTE bam
.oRg $8f30
.byTe ban
.orG $8f40
.bytE bao
.org $8f50
.byte bap
.ORG $8f60
.WORD wba
.ORg $8f70
.WORd wbb
.OrG $8f80
.WOrD wbc
.Org $8f90
.WOrd wbd
.oRG $8fa0
.WoRD wbe
.oRg $8fb0
.WoRd wbf
.orG $8fc0
.WorD wbg
.org $8fd0
.Word wbh
.ORG $8fe0
.wORD wbi
.ORg $8ff0
.wORd wbj
.OrG $9000
.wOrD wbk
.Org $9010
.wOrd wbl
.oRG $9020
.woRD wbm
.oRg $9030
.woRd wbn
.orG $9040
.worD wbo
.org $9050
.word wbp

; Test indirect vector falling on page boundary warning

.org $9060
jmp ($00)
jmp ($0f)
jmp ($f0)
jmp ($ff)
.org $9080
jmp ($0000)
jmp ($000f)
jmp ($00f0)
jmp ($00ff)
.org $90a0
jmp ($0f00)
jmp ($0f0f)
jmp ($0ff0)
jmp ($0fff)
.org $90c0
jmp ($f000)
jmp ($f00f)
jmp ($f0f0)
jmp ($f0ff)
.org $90e0
jmp ($ff00)
jmp ($ff0f)
jmp ($fff0)
jmp ($ffff)