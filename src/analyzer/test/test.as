;x86 asm code.
.686P
.XMM
.model flat
EXTRN __fltused:NEAR32
EXTRN _memset:NEAR32
_DATA  SEGMENT
EXTERN	_printf:NEAR32
_gs_0	DB	'%d',0aH,00H	;%d\n
_DATA  ENDS
PUBLIC	_fun
_TEXT	SEGMENT
_i_3 = 8 	;size: 4
_fun   PROC
push ebp
mov ebp,esp
sub esp,0
;param [i]
mov ebx,DWORD PTR _i_3[ebp]
push ebx
;param gs_0
lea ebx,_gs_0
push ebx
;t_0 = call printf,8
call _printf
add esp,8
;NOP
mov esp,ebp
pop ebp
ret 0
_fun   ENDP
_TEXT	ENDS
PUBLIC	_maim
_TEXT	SEGMENT
_i_7 = -4 	;size: 4
_f_8 = -8 	;size: 4
_maim   PROC
push ebp
mov ebp,esp
sub esp,8
;param 1
mov ebx,1
push ebx
;t_1 = call fun,4
call _fun
add esp,4
;param 2
mov ebx,2
push ebx
;t_2 = call fun,4
call _fun
add esp,4
;NOP
mov esp,ebp
pop ebp
ret 0
_maim   ENDP
_TEXT	ENDS
END
