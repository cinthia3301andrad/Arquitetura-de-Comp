;cinthia, 2020.1
org 0x7c00
bits 16

mov ax, 0
mov ds, ax
cli

main:	
	mov ax, 0x7e00 ;gravar a string digitada
	call gets

	mov ax, [0x7e00] ; valor do endereço de memória 0x7e00, colocado em ax
	;ax fica com valor da tecla pressionada
	call pintaTela	

	jmp main

gets:
	push ax
	push di

	mov di, ax
	mov ah, 0x00 ;leia tecla pressionada
	int 0x16 ;interrupção de teclado
	mov [di], al ;em al fica o valor da tecla pressionada
	
	pop di
	pop ax
	ret

pintaTela:
	push bx ; contador
	push cx ;
	push dx ;cor digitada

	mov cx, 0xA000 ;posição da memória de vídeo
	mov es, cx ;segmento de onde começa a tela para pintar

	mov dx, ax; valor da tecla, valor da cor
	mov al, 0x13; modo gráfico (VGA, 320 x 200, 256 cores);
	int 0x10

	mov cx, 64000 ;printar tela vai se repetir desse tanto
	mov bx, 0; contador
.loop:
	mov di, bx
	mov [es: di], dx ;dx tem a "cor"
	
	inc bx
	dec cx
	jz .fim ;limpa a pilha para pegar outra cor
	jmp .loop

.fim:
	pop bx
	pop cx
	pop dx
	ret
    times 510 - ($-$$) db 0
    dw 0xaa55