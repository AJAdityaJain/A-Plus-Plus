format PE64 console
entry start

include 'WIN64A.inc'

section '.text' code readable executable


start:
push rbp
mov rbp, rsp

xor rax,rax
mov eax, 45
xor eax, 42
xor eax, 23
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