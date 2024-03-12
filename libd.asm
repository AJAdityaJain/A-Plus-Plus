;String length
;rcx - pointer to string
;rax - length of string
;rdx - temp
strlen:
    mov rcx, qword[rcx]
	mov rdx ,0
	strlenwhile:
        inc rcx
        inc rdx
        mov al, byte[rcx]
        test al, al
	jnz strlenwhile
	mov rax,rdx
ret

;String compare
;rcx - pointer to string 1
;rdx - pointer to string 2
;rax - 0 if strings are equal, 1 if not
;rdx - temp
strcmp:
    mov rcx, qword[rcx]
    mov rdx, qword[rdx]
	dec rcx
	dec rdx
	strcmpwhile:
		inc rcx
		inc rdx
		mov al, byte[rcx]
		mov ah, byte[rdx]
		cmp al, 0
		jne strcmpwhileend
			cmp ah, 0
			je strcmp0
		strcmpwhileend:
		cmp al, ah
	je strcmpwhile
	mov rax, 1
ret
	strcmp0:
	mov rax, 0
ret

;Add char to end of string
;rcx - pointer to string
;rdx - character
;rax,r9,r8 - temp
addchar:
    mov eax, dword[rcx+8]
    cmp eax, dword[rcx+12]
    jl addcharskip
        push rcx
        push rdx
        sub rsp, 32
            mov r8, rcx
            mov r9d, dword[rcx+12]
            add r9, r9
            mov dword[rcx+12], r9d
            mov rdx, 0
            mov rcx, [hHeap]
            call [HeapReAlloc]
        add rsp, 32
        pop rdx
        pop rcx
    mov rcx, rax
    addcharskip:
    mov rax, qword[rcx]
    add eax, dword[rcx+8]
    mov byte[rax], dl

    mov eax, dword[rcx+8]
    inc rax
    mov dword[rcx+8], eax
ret

;Generate string with reverved space
;rcx - length of string
;rax - pointer to string
;rdx, r8 - temp
genstr:
	push rcx
        sub rsp, 32
            mov r8, rcx
            mov rdx, 0
            mov rcx, [hHeap]
            call [HeapAlloc]
            mov rdx, rax
            push rdx
                sub rsp, 32
                    mov r8, 16
                    mov rdx, 0
                    mov rcx, [hHeap]
                    call [HeapAlloc]
                add rsp, 32
            pop rdx
    	add rsp, 32
	pop rcx
	mov qword [rax], rdx
	mov dword [rax+8], 0
    mov dword [rax+12], ecx
ret

;Generate string from label
;rcx - label
;rax - pointer to string
;rdx, r8, r9 - temp
genstrlab:
    mov rax, 0
    push rcx
    genstrlabwhile:
        mov dl, byte[rcx]
        test dl, dl
        jz genstrlabwhileend
    inc rcx
    inc rax
    jmp genstrlabwhile
    genstrlabwhileend:
        mov rcx, rax
        push rcx
            call genstr
        pop rcx
        mov dword [rax+8], ecx
    pop rcx
    push rcx
    push rax
        mov rax, qword [rax]
        genstrlabwhile2:
            mov dl, byte[rcx]
            mov byte [rax], dl
            cmp dl, 0
            je genstrlabwhileend2
        inc rcx
        inc rax
        jmp genstrlabwhile2
        genstrlabwhileend2:
    pop rax
    pop rcx
ret

;Delete string
;rcx - pointer to string
;rdx, r8 - temp
delstr:
    push rcx
	sub rsp, 32
			mov r8, qword[rcx]
			mov rdx, 0
			mov rcx, [hHeap]
			call [HeapFree]
	add rsp, 32
	pop rcx

	sub rsp, 32
			mov r8, rcx
			mov rdx, 0
			mov rcx, [hHeap]
			call [HeapFree]
	add rsp, 32
ret
