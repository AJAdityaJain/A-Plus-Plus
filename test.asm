section .text
	extern GetStdHandle
	extern WriteFile
	extern ExitProcess
	global main
main:
push rbp
mov rbp, rsp
mov eax, 12
sub rsp, 4
mov dword[rsp], eax
mov eax, 0
sub rsp, 4
mov dword[rsp], eax
mov eax, dword[rbp - 4]
sub rsp, 4
mov dword[rsp], eax
mov eax, dword[rbp - 8]
mov [rbp - 12], eax
mov rsp, rbp
pop rbp;
mov rcx,2
call  ExitProcess
