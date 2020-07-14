;cinthia, 2020.1
org 0x7c00
bits 16
mov ax,0 
mov ds, ax
cli

mov ax, 0x7e00  ;o que recebermos do teclado
call gets
call contrario 

gets:
	push ax
	mov di, ax
	mov bx, 0 ;contador

loop1:
	mov ah, 0x00 ;pega e tecla que vai para al
	int 0x16 
	cmp al, 13 ;se for enter
	je retorno ;vai para o retorno
	mov [ds:di], al 
	inc di 
	inc bx
	mov ah, 0x0e 
	int 0x10
	jmp loop1 

retorno:
	mov ah, 0x0e 
	int 0x10
	mov al, 10 
	int 0x10
	pop ax
	dec di
	ret

contrario:
	push ax 
	push di 
	mov ah, 0x0e

loop2:
	mov al, [ds:di]
	dec di 
	or bx, bx
	je fim 
	int 0x10
	dec bx
	jmp loop2

fim:
	pop di
	pop ax

    hlt
    times 510 - ($-$$) db 0
    dw 0xaa55