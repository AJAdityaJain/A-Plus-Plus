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
	mov dword[rsp], 11
sub rsp, 4
	mov dword[rsp], 22
sub rsp, 4
	mov dword[rsp], 44
xor rax,rax
mov eax, dword[rbp - 4]

add eax, dword[rbp - 8]

add eax, dword[rbp - 12]

sub rsp, 4
	mov dword[rsp], eax
xor rax,rax
mov eax, dword[rbp - 4]
sub rsp, 4
	mov dword[rsp], eax
xor rax,rax
mov eax, dword[rbp - 16]
mov [rbp - 20], eax

mov rsp, rbp
pop rbp;
pop rbx


invoke  exit, rax





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', msvcrt, 'MSVCRT.DLL'
import kernel,exit,'ExitProcess'
import msvcrt,printf, 'printf'