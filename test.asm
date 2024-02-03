format PE64 console
entry start

include 'WIN64A.inc'

section '.data' data readable writeable
version db '0.3.1'
section '.text' code readable executable


start:
push rbx
push rbp
mov rbp, rsp

sub rsp, 4
	mov dword[rsp], 97
mov [rbp - 4], 1
xor rax,rax
mov eax, 1
add eax, 24
add eax, 6
add eax, 7
mov [rbp - 4], eax
sub rsp, 4
	mov dword[rsp], 23

mov rsp, rbp
pop rbp;
pop rbx


invoke  exit, rax





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', msvcrt, 'MSVCRT.DLL'
import kernel,exit,'ExitProcess'
import msvcrt,printf, 'printf'