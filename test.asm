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
}
section '.data' data readable writeable
ns db '%d',10





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', \msvcrt, 'MSVCRT.DLL'
import kernel,\exit,'ExitProcess'
import msvcrt,\printf, 'printf'