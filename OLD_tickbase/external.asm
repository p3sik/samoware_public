
.data

EXTERN ?rechargingShift@globals@@3_NA:BYTE
EXTERN ?ticksAllowed@globals@@3HA:DWORD
EXTERN ?sv_maxusrcmdprocessticks@globals@@3HA:DWORD
EXTERN ?CL_MoveOrig@cl_move@hooks@@3P6AXM_N@ZEA:PROTO

.code

setR14B PROC
	mov r14b, cl
	ret
setR14B ENDP

COMMENT @

if (ticksAllowed == sv_maxusrcmdprocessticks)
	rechargingShift = false;

if (rechargingShift) {
	ticksAllowed++;

	return;
}

CL_MoveOrig(float, bool);

@

CL_MoveHook PROC
	push rax

	; if ticksAllowed >= sv_maxusrcmdprocessticks then rechargingShift = false

	mov eax, ?ticksAllowed@globals@@3HA
	cmp eax, ?sv_maxusrcmdprocessticks@globals@@3HA
	jl skip
	mov ?rechargingShift@globals@@3_NA, 0
skip:
	; if rechargingShift then ticksAllowed++ return;

	mov al, ?rechargingShift@globals@@3_NA
	test al, al
	jz skip2
	inc ?ticksAllowed@globals@@3HA
	pop rax
	ret
skip2:
	pop rax

	jmp qword ptr [?CL_MoveOrig@cl_move@hooks@@3P6AXM_N@ZEA]
CL_MoveHook ENDP

END
