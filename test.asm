format PE64 console
entry start

include 'WIN64A.inc'

section '.text' code readable executable


  macro cspush{
      push rax  
      push rcx  
      push rdx  
      push r8  
      push r9  
      push r10  
      push r11  
}  
  macro cspop{
      pop r11  
      pop r10  
      pop r9  
      pop r8  
      pop rdx  
      pop rcx  
      pop rax  
}  
    
  macro printint[int]{
      cspush  
      invoke printf, intfmt,int  
      cspop  
}  
    
  macro printstr[string]{
      cspush  
      invoke printf, string  
      cspop  
}  
    
  macro printdouble[xmm]{
      cspush  
        
      sub rsp, 8  
      movsd QWORD[rsp], xmm0  
        
      movsd xmm0, xmm  
      movq rdx, xmm0          
      invoke printf, doublefmt  
    
      movsd xmm0, QWORD[rsp]  
      add rsp, 8  
    
      cspop  
}  
  macro printfloat[dwrd]{
		sub rsp, 8
		movsd QWORD[rsp], xmm0
		cvtss2sd xmm0, dwrd
		printdouble xmm0
		movsd xmm0, QWORD[rsp]
		add rsp, 8
}start:
push rbp
mov rbp, rsp

mov dword [rsp-4], 123
sub rsp, 4
movss xmm0, DWORD [LABDAT1]
movss dword [rsp-4], xmm0
sub rsp, 4
movsd xmm0, QWORD [LABDAT3]
movsd qword [rsp-8], xmm0
sub rsp, 8
mov eax, dword [rbp - 4]
cvtsi2sd xmm1, eax
movsd xmm0, xmm1
movss xmm1, dword [rbp - 8]
cvtss2sd xmm1, xmm1
addsd xmm0, xmm1
addsd xmm0, qword [rbp - 16]
movsd qword [rsp-8], xmm0
sub rsp, 8
printdouble qword [rbp - 24]
add rsp,24

mov rsp, rbp
pop rbp;


invoke  exit, rax
section '.data' data readable writeable
intfmt db '%d',10
section '.data' data readable writeable
doublefmt db '%f',10
section '.data' data readable writeable
LABDAT0 dd 4.560000
section '.data' data readable writeable
LABDAT1 dd 4.560000
section '.data' data readable writeable
LABDAT2 dq 14.560000
section '.data' data readable writeable
LABDAT3 dq 14.560000





section '.idata' import data readable writeable
library kernel, 'KERNEL32.DLL', \msvcrt, 'MSVCRT.DLL'
import kernel,\exit,'ExitProcess'
import msvcrt,\printf, 'printf'