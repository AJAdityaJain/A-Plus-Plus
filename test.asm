format PE64 console
entry start

include 'WIN64A.inc'

section '.text' code readable executable


start:
push rbp
mov rbp, rsp

mov dword[rsp-4], 0
sub rsp, 4
.LABBRNCH0:
xor rax,rax
mov eax, dword[rbp - 4]
cmp eax, 10
setl al
cmp al, 1
jnz .LABBRNCH1
xor rax,rax
mov eax, dword[rbp - 4]
add eax, 1
mov dword[rbp - 4], eax
add rsp,0
jmp .LABBRNCH0
.LABBRNCH1:
add rsp,4

mov rsp, rbp
pop rbp;


invoke  exit, rax

section '.data' data readable writeable
version db '0.3.1'





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', \msvcrt, 'MSVCRT.DLL'
import kernel,\exit,'ExitProcess'
import msvcrt,\printf, 'printf'