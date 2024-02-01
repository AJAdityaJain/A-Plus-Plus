format PE64 console
entry start

include 'WIN64A.inc'

section '.data' data readable writeable
filler db 4
section '.text' code readable executable


start:
push rbp
mov rbp, rsp
sub rsp, 4
	mov dword[rsp], 0
mov eax, 1
add eax, 2
add eax, 3
mov ecx, 4
add ecx, 5
add ecx, 6
add eax, ecx
mov [rbp - 4], eax
mov rsp, rbp
pop rbp;


invoke  exit, 420





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', msvcrt, 'MSVCRT.DLL'
import kernel,exit,'ExitProcess'
import msvcrt,printf, 'printf'