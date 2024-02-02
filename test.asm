format PE64 console
entry start

include 'WIN64A.inc'

section '.data' data readable writeable
filler db 4
section '.text' code readable executable


start:
push rbx
push rbp
mov rbp, rsp

sub rsp, 4
	mov dword[rsp], 0
xor rax,rax
xor rcx,rcx
mov cl, 1
xor cl, 1
mov al,cl
xor al, 1
mov [rbp - 4], al

mov rsp, rbp
pop rbp;
pop rbx


invoke  exit, rax





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', msvcrt, 'MSVCRT.DLL'
import kernel,exit,'ExitProcess'
import msvcrt,printf, 'printf'