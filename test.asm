format PE64 console
entry start

include 'WIN64A.inc'

section '.text' code readable executable


start:
push rbp
mov rbp, rsp

xor rax,rax
mov eax, 53
xor rcx,rcx
mov ecx, 2324
cdq
idiv ecx
mov eax, edx
xor rcx,rcx
mov ecx, 876
cdq
idiv ecx
mov eax, edx
mov dword[rsp-4], eax
sub rsp, 4

mov rsp, rbp
pop rbp;


invoke  exit, rax

section '.data' data readable writeable
version db '0.3.1'





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', \msvcrt, 'MSVCRT.DLL'
import kernel,\exit,'ExitProcess'
import msvcrt,\printf, 'printf'