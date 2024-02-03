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

mov rsp, rbp
pop rbp;
pop rbx


invoke  exit, rax





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', msvcrt, 'MSVCRT.DLL'
import kernel,exit,'ExitProcess'
import msvcrt,printf, 'printf'