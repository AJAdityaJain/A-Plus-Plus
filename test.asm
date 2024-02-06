format PE64 console
entry start

include 'WIN64A.inc'

section '.text' code readable executable


macro printnum[number]{
push rax
push rdx
push r8

push r10
push r11
invoke printf, ns,number
pop r11
pop r10
pop r8
pop rdx
pop rax
}
macro printstr[string]{
push rax
push rdx
push r8

push r10
push r11
invoke printf, string
pop r11
pop r10
pop r8
pop rdx
pop rax
}start:
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
printnum dword[rbp - 4]
add dword[rbp - 4], 1
add rsp,0
jmp .LABBRNCH0
.LABBRNCH1:
add rsp,4

mov rsp, rbp
pop rbp;


invoke  exit, rax

section '.data' data readable writeable
ns db '%d',10





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', \msvcrt, 'MSVCRT.DLL'
import kernel,\exit,'ExitProcess'
import msvcrt,\printf, 'printf'