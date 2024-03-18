;String length
;rcx - pointer to string
;rax - length of string
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
	mov rax, 0
ret
	strcmp0:
	mov rax, 1
ret

;Add char to end of string
;rcx - pointer to string
;rdx - character
addchar:
    mov eax, dword[rcx+8]
    add rax, 1
    cmp eax, dword[rcx+12]
    jl addcharskip
        push rcx
        push rdx
        sub rsp, 32
            mov r8, qword [rcx]
            mov r9d, dword[rcx+12]
            add r9, r9
            mov dword[rcx+12], r9d
            mov rdx, 0
            mov rcx, [hHeap]
            call [HeapReAlloc]
        add rsp, 32
        pop rdx
        pop rcx
    mov qword [rcx], rax
    addcharskip:
    mov rax, qword[rcx]
    add eax, dword[rcx+8]
    mov byte[rax], dl
    mov byte[rax+1], 0

    mov eax, dword[rcx+8]
    inc rax
    mov dword[rcx+8], eax
ret


;Add string to end of string
;rcx - pointer to string
;rdx - pointer to string 2
addstr:
    mov rax, 0
    mov rdx, qword[rdx]
    addstrwhile:
        cmp byte[rdx + rax],0
        je addstrend
            push rcx
            push rdx
            push rax
                mov dl, byte[rdx + rax]
                call addchar
            pop rax
            pop rdx
            pop rcx
            inc rax
        jmp addstrwhile
    addstrend:
ret

;Generate string with reverved space
;rcx - length of string
;rax - pointer to string
genstr:
    cmp rcx, 0
    je genstr0
    jmp genstr0end
    genstr0:
    add rcx, 16
    genstr0end:

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

    push rax
        lea rdx,  [delstr]
        mov rcx, rax
        call addgc
    pop rax
ret

;Generate string from label
;rcx - label
;rax - pointer to string
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

;Add Item
;rcx - item
;rdx - destructor
addgc:
    mov r8, [gc]
    mov rax, 0
    sub rax, 17
    addgcwhile:
        add rax, 17
        cmp byte[r8 + rax], 0
        jne addgcwhileend
            mov byte[r8 + rax], 1
            add rax, 1
            mov qword[r8 + rax], rcx
            add rax, 8
            mov qword[r8 + rax], rdx
jmp addgcend
        addgcwhileend:
            mov r9d, dword[gcsz]
            sub r9, 1
            imul r9, 17
            cmp rax, r9
            je addgcfail

        jmp addgcwhile
        addgcfail:
        ; jmp addgcfail
        push rcx
        push rdx
        sub rsp, 32
            mov r9d, dword [gcsz]
            add r9, r9
            mov dword[gcsz], r9d
            mov r8, [gc]
            mov rdx, 0
            mov rcx, [hHeap]
            call [HeapReAlloc]
            mov [gc], rax
        add rsp, 32
        pop rdx
        pop rcx
        call addgc        
    addgcend:
ret

;Mark a address
;rcx - address
markgc:
    push rax
    mov r8, [gc]

    mov rax, 0
    markgcwhile:
        cmp eax, dword [gcsz]
        je markgcwhileend
            mov rdx, rax
            imul rdx, 17
            add rdx, 1
            add rdx, r8

            cmp qword[rdx], rcx
            je markgcsub
            
        inc rax
    jmp markgcwhile
    markgcwhileend:
    pop rax
ret

;Sweep useless addresses
sweepgc:
    push rax
    mov rdx, [gc]
    mov rax, 0
    sweepgcwhile:
        cmp eax, dword [gcsz]
        je sweepgcwhileend
            mov rcx, rax
            imul rcx, 17
            add rcx, rdx
            cmp byte[rcx], 1
            je sweepclean
            cmp byte[rcx], 4
            je resetused
        inc rax 
        jmp sweepgcwhile

        resetused:
            mov byte[rcx], 1
        inc rax 
        jmp sweepgcwhile
        
        sweepclean:
            push rax
            push rcx
            push rdx
                mov rcx, qword[rcx+1]
                call delstr;;;;;;;;;;;;;;;;;;;;;;;
                ; call qword [rcx+9]
            pop rdx
            pop rcx
            pop rax


            mov byte[rcx], 0
            mov qword[rcx+1], 0
            mov qword[rcx+9], 0
        inc rax 
        jmp sweepgcwhile
    sweepgcwhileend:
    pop rax
ret

markgcsub:
    sub rdx, 1
    mov byte[rdx], 4
    jmp markgcwhileend

;Create GC
gengc:
    sub rsp, 32
        mov r8, gcsz
        imul r8, 17
        mov rdx, 0
        mov rcx, [hHeap]
        call [HeapAlloc]
    add rsp, 32
    mov r8d, gcsz
    imul r8, 17
    gengcwhile:
            sub r8, 8
            mov qword[rax + r8], 0
            sub r8, 8
            mov qword[rax + r8], 0
            sub r8, 1
            mov byte[rax + r8], 0
        cmp r8,0
        je gengcwhileend
        jmp gengcwhile
    gengcwhileend:
    mov [gc], rax
ret


prntn:
    mov rax, 0
    ssc:
    cmp rax, rcx
    jg ssend
            push rcx
            push rax
            push rdx
        sub rsp, 32
            mov rcx, intfmt
            mov al, byte[rdx + rax]
            mov rdx, 0
            mov dl, al
            call [printf]
        add rsp, 32
            pop rdx
            pop rax
            pop rcx
    inc rax
    jmp ssc
    ssend:
ret
