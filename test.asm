format PE64 console
entry start

include 'WIN64A.inc'

section '.data' data readable writeable
filler db 4
section '.text' code readable executable


start:
push rbx
push rsi
push rdi
push r12
push r13
push r14
push r15
push rbp
mov rbp, rsp

mov eax,1
neg eax
sub rsp, 4
	mov dword[rsp], eax

mov rsp, rbp
pop rbp;
pop r15
pop r14
pop r13
pop r12
pop rdi
pop rsi
pop rbx


invoke  exit, 420





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', msvcrt, 'MSVCRT.DLL'
import kernel,exit,'ExitProcess'
import msvcrt,printf, 'printf'