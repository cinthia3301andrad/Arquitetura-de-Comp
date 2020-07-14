;cinthia, 2020.1
org 0x7c00
bits 16

mov ax, 0
mov ds, ax
cli

mov ax, 0xA000
mov es, ax 
mov ah, 0
mov al, 0x13
int 0x10
mov dx, 0 ; cor

loop:
	mov cx, 64000 ;tela
	inc dx ;cor
	mov ax, 0 
loop2:
	mov di, ax ; "onde" é para pintar
	mov [es:di], dx ; coloca a cor dx no segmento específico
    
    inc ax 
	inc dx 
    dec cx 	
	jz loop ;caso o contador chegue em 0
	jmp loop2 ;se não chegar em 0, ele repete o loop2

    hlt
    times 510 - ($-$$) db 0
    dw 0xaa55