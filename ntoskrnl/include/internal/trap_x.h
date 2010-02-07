/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         BSD - See COPYING.ARM in the top level directory
 * FILE:            ntoskrnl/include/trap_x.h
 * PURPOSE:         Internal Inlined Functions for the Trap Handling Code
 * PROGRAMMERS:     ReactOS Portable Systems Group
	!!! this is horrible. Portable? LOL
	TODO:
	- use macros whenever possible instead of inline assembly
	- create needed asm macros in <arch>\<include>, not here
	- avoid unnecessary conditional jumps by generating the right prolog
		and epilogs for each trap type
	- fix dependency of stack frame optimization
 */

#ifndef _TRAP_X_
#define _TRAP_X_

#include <debug.h>
#define DBGTRAP DPRINT1
// #define DBGTRAPENTRY DPRINT1("\n"); DbgDumpCpu(7|DBG_DUMPCPU_TSS); DPRINT1("TrapFrame=%p:\n", TrapFrame); DbgDumpMem(TrapFrame, 0x80)
// #define DBGTRAPENTRY DPRINT1("\n");


// !!! temp for testing
extern KINTERRUPT *TrapStubInterrupt;

#if 0
VOID KiFastCallEntry(VOID);
VOID KiTrap08(VOID);
VOID KiTrap13(VOID);
#endif

VOID
// DECLSPEC_NORETURN
FASTCALL
KiServiceExit(IN PKTRAP_FRAME TrapFrame,
              IN NTSTATUS Status);

VOID
// DECLSPEC_NORETURN
FASTCALL
KiServiceExit2(IN PKTRAP_FRAME TrapFrame);

VOID
_NORETURN
_FASTCALL
KiSystemServiceHandler(IN PKTRAP_FRAME TrapFrame);

VOID
_NORETURN
_FASTCALL
KiFastCallEntryHandler(IN PKTRAP_FRAME TrapFrame);

//
// Debug Macros
//
VOID
FORCEINLINE
KiDumpTrapFrame(IN PKTRAP_FRAME TrapFrame)
{
    /* Dump the whole thing */
    DbgPrint("DbgEbp: %x\n", TrapFrame->DbgEbp);
    DbgPrint("DbgEip: %x\n", TrapFrame->DbgEip);
    DbgPrint("DbgArgMark: %x\n", TrapFrame->DbgArgMark);
    DbgPrint("DbgArgPointer: %x\n", TrapFrame->DbgArgPointer);
    DbgPrint("TempSegCs: %x\n", TrapFrame->TempSegCs);
    DbgPrint("TempEsp: %x\n", TrapFrame->TempEsp);
    DbgPrint("Dr0: %x\n", TrapFrame->Dr0);
    DbgPrint("Dr1: %x\n", TrapFrame->Dr1);
    DbgPrint("Dr2: %x\n", TrapFrame->Dr2);
    DbgPrint("Dr3: %x\n", TrapFrame->Dr3);
    DbgPrint("Dr6: %x\n", TrapFrame->Dr6);
    DbgPrint("Dr7: %x\n", TrapFrame->Dr7);
    DbgPrint("SegGs: %x\n", TrapFrame->SegGs);
    DbgPrint("SegEs: %x\n", TrapFrame->SegEs);
    DbgPrint("SegDs: %x\n", TrapFrame->SegDs);
    DbgPrint("Edx: %x\n", TrapFrame->Edx);
    DbgPrint("Ecx: %x\n", TrapFrame->Ecx);
    DbgPrint("Eax: %x\n", TrapFrame->Eax);
    DbgPrint("PreviousPreviousMode: %x\n", TrapFrame->PreviousPreviousMode);
    DbgPrint("ExceptionList: %x\n", TrapFrame->ExceptionList);
    DbgPrint("SegFs: %x\n", TrapFrame->SegFs);
    DbgPrint("Edi: %x\n", TrapFrame->Edi);
    DbgPrint("Esi: %x\n", TrapFrame->Esi);
    DbgPrint("Ebx: %x\n", TrapFrame->Ebx);
    DbgPrint("Ebp: %x\n", TrapFrame->Ebp);
    DbgPrint("ErrCode: %x\n", TrapFrame->ErrCode);
    DbgPrint("Eip: %x\n", TrapFrame->Eip);
    DbgPrint("SegCs: %x\n", TrapFrame->SegCs);
    DbgPrint("EFlags: %x\n", TrapFrame->EFlags);
    DbgPrint("HardwareEsp: %x\n", TrapFrame->HardwareEsp);
    DbgPrint("HardwareSegSs: %x\n", TrapFrame->HardwareSegSs);
    DbgPrint("V86Es: %x\n", TrapFrame->V86Es);
    DbgPrint("V86Ds: %x\n", TrapFrame->V86Ds);
    DbgPrint("V86Fs: %x\n", TrapFrame->V86Fs);
    DbgPrint("V86Gs: %x\n", TrapFrame->V86Gs);
}

#ifdef TRAP_DEBUG
VOID
FORCEINLINE
KiFillTrapFrameDebug(IN PKTRAP_FRAME TrapFrame)
{
    /* Set the debug information */
    TrapFrame->DbgArgPointer = TrapFrame->Edx;
    TrapFrame->DbgArgMark = 0xBADB0D00;
    TrapFrame->DbgEip = TrapFrame->Eip;
    TrapFrame->DbgEbp = TrapFrame->Ebp;   
}

#if 0
VOID
FORCEINLINE
KiExitTrapDebugChecks(IN PKTRAP_FRAME TrapFrame,
                      IN KTRAP_EXIT_SKIP_BITS SkipBits)
{
    /* Make sure interrupts are disabled */
    if (__readeflags() & EFLAGS_INTERRUPT_MASK)
    {
        DbgPrint("Exiting with interrupts enabled: %lx\n", __readeflags());
        while (TRUE);
    }
    
    /* Make sure this is a real trap frame */
    if (TrapFrame->DbgArgMark != 0xBADB0D00)
    {
        DbgPrint("Exiting with an invalid trap frame? (No MAGIC in trap frame)\n");
        KiDumpTrapFrame(TrapFrame);
        while (TRUE);
    }
    
    /* Make sure we're not in user-mode or something */
    if (Ke386GetFs() != KGDT_R0_PCR)
    {
        DbgPrint("Exiting with an invalid FS: %lx\n", Ke386GetFs());
        while (TRUE);   
    }
    
    /* Make sure we have a valid SEH chain */
    if (KeGetPcr()->Tib.ExceptionList == 0)
    {
        DbgPrint("Exiting with NULL exception chain: %p\n", KeGetPcr()->Tib.ExceptionList);
        while (TRUE);
    }
    
    /* Make sure we're restoring a valid SEH chain */
    if (TrapFrame->ExceptionList == 0)
    {
        DbgPrint("Entered a trap with a NULL exception chain: %p\n", TrapFrame->ExceptionList);
        while (TRUE);
    }
    
    /* If we're ignoring previous mode, make sure caller doesn't actually want it */
    if ((SkipBits.SkipPreviousMode) && (TrapFrame->PreviousPreviousMode != -1))
    {
        DbgPrint("Exiting a trap witout restoring previous mode, yet previous mode seems valid: %lx", TrapFrame->PreviousPreviousMode);
        while (TRUE);
    }
}
#endif


VOID
FORCEINLINE
KiExitSystemCallDebugChecks(IN ULONG SystemCall,
                            IN PKTRAP_FRAME TrapFrame)
{
    KIRQL OldIrql;
    
    /* Check if this was a user call */
    if (TrapFrame->SegCs & MODE_MASK)
    {
        /* Make sure we are not returning with elevated IRQL */
        OldIrql = KeGetCurrentIrql();
        if (OldIrql != PASSIVE_LEVEL)
        {
            /* Forcibly put us in a sane state */
            KeGetPcr()->Irql = PASSIVE_LEVEL;
            _disable();
            
            /* Fail */
            KeBugCheckEx(IRQL_GT_ZERO_AT_SYSTEM_SERVICE,
                         SystemCall,
                         OldIrql,
                         0,
                         0);
        }
        
        /* Make sure we're not attached and that APCs are not disabled */
        if ((KeGetCurrentThread()->ApcStateIndex != CurrentApcEnvironment) ||
            (KeGetCurrentThread()->CombinedApcDisable != 0))
        {
            /* Fail */
            KeBugCheckEx(APC_INDEX_MISMATCH,
                         SystemCall,
                         KeGetCurrentThread()->ApcStateIndex,
                         KeGetCurrentThread()->CombinedApcDisable,
                         0);
        }
    }
}
#else
#define KiExitTrapDebugChecks(x, y)
#define KiFillTrapFrameDebug(x)
#define KiExitSystemCallDebugChecks(x, y)
#endif

//
// Helper Code
//
BOOLEAN
FORCEINLINE
KiUserTrap(IN PKTRAP_FRAME TrapFrame)
{
    /* Anything else but Ring 0 is Ring 3 */
    return (TrapFrame->SegCs & MODE_MASK);
}

//
// Assembly exit stubs
//
VOID
FORCEINLINE
// DECLSPEC_NORETURN /* Do not mark this as DECLSPEC_NORETURN because possibly executing code follows it! */
KiSystemCallReturn(IN PKTRAP_FRAME TrapFrame)
{
	DPRINTT("\n");
	/* Restore nonvolatiles, EAX, and do a "jump" back to the kernel caller */
#if defined(_MSC_VER)
	_ASM_BEGIN
	mov esp, TrapFrame
#if 1
	mov ds, KTRAP_FRAME.SegDs[esp]
	mov es, KTRAP_FRAME.SegEs[esp]
	mov fs, KTRAP_FRAME.SegFs[esp]
	mov ebp, KTRAP_FRAME.Ebp[esp]
	mov ebx, KTRAP_FRAME.Ebx[esp]
	mov esi, KTRAP_FRAME.Esi[esp]
	mov edi, KTRAP_FRAME.Edi[esp]
	mov eax, KTRAP_FRAME.Eax[esp]
	mov ecx, KTRAP_FRAME.Ecx[esp]
	mov edx, KTRAP_FRAME.Eip[esp]
	add esp, KTRAP_FRAME_ESP
	jmp edx
#else
	mov ebx, [esp+KTRAP_FRAME_EBX]
	mov esi, [esp+KTRAP_FRAME_ESI]
	mov edi, [esp+KTRAP_FRAME_EDI]
	mov ebp, [esp+KTRAP_FRAME_EBP]
	mov eax, [esp+KTRAP_FRAME_EAX]
	mov edx, [esp+KTRAP_FRAME_EIP]
	add esp, KTRAP_FRAME_ESP
	jmp edx
#endif
	_ASM_END
#elif defined(__GNUC__)
	__asm__ __volatile__
    (
        "movl %0, %%esp\n"
        "movl %c[b](%%esp), %%ebx\n"
        "movl %c[s](%%esp), %%esi\n"
        "movl %c[i](%%esp), %%edi\n"
        "movl %c[p](%%esp), %%ebp\n"
        "movl %c[a](%%esp), %%eax\n"
        "movl %c[e](%%esp), %%edx\n"
        "addl $%c[v],%%esp\n" /* A WHOLE *KERNEL* frame since we're not IRET'ing */
        "jmp *%%edx\n"
        ".globl _KiSystemCallExit2\n_KiSystemCallExit2:\n"
        :
        : "r"(TrapFrame),
          [b] "i"(KTRAP_FRAME_EBX),
          [s] "i"(KTRAP_FRAME_ESI),
          [i] "i"(KTRAP_FRAME_EDI),
          [p] "i"(KTRAP_FRAME_EBP),
          [a] "i"(KTRAP_FRAME_EAX),
          [e] "i"(KTRAP_FRAME_EIP),
          [v] "i"(KTRAP_FRAME_ESP)
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    UNREACHABLE;
}

VOID
DECLSPEC_NORETURN
FORCEINLINE
KiSystemCallTrapReturn(IN PKTRAP_FRAME TrapFrame)
{
	DPRINTT("\n");
	/* Regular interrupt exit, but we only restore EAX as a volatile */
#if defined(_MSC_VER)
	_ASM_BEGIN
	mov esp, TrapFrame
#if 1
	mov ds, KTRAP_FRAME.SegDs[esp]
	mov es, KTRAP_FRAME.SegEs[esp]
	mov fs, KTRAP_FRAME.SegFs[esp]
	mov ebp, KTRAP_FRAME.Ebp[esp]
	mov ebx, KTRAP_FRAME.Ebx[esp]
	mov esi, KTRAP_FRAME.Esi[esp]
	mov edi, KTRAP_FRAME.Edi[esp]
	mov eax, KTRAP_FRAME.Eax[esp]
	mov ecx, KTRAP_FRAME.Ecx[esp]
	mov edx, KTRAP_FRAME.Edx[esp]
	iretd
#else
	mov ebx, [esp+KTRAP_FRAME_EBX]
	mov esi, [esp+KTRAP_FRAME_ESI]
	mov edi, [esp+KTRAP_FRAME_EDI]
	mov ebp, [esp+KTRAP_FRAME_EBP]
	mov eax, [esp+KTRAP_FRAME_EAX]
	add esp, KTRAP_FRAME_EIP
	iretd
#endif
	_ASM_END
#elif defined(__GNUC__)
    __asm__ __volatile__
    (
        ".globl _KiSystemCallExit\n_KiSystemCallExit:\n"
		"movl %0, %%esp\n"
        "movl %c[b](%%esp), %%ebx\n"
        "movl %c[s](%%esp), %%esi\n"
        "movl %c[i](%%esp), %%edi\n"
        "movl %c[p](%%esp), %%ebp\n"
        "movl %c[a](%%esp), %%eax\n"
        "addl $%c[e],%%esp\n"
        "iret\n"
        :
        : "r"(TrapFrame),
          [b] "i"(KTRAP_FRAME_EBX),
          [s] "i"(KTRAP_FRAME_ESI),
          [i] "i"(KTRAP_FRAME_EDI),
          [p] "i"(KTRAP_FRAME_EBP),
          [a] "i"(KTRAP_FRAME_EAX),         
          [e] "i"(KTRAP_FRAME_EIP)
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    UNREACHABLE;
}


VOID
FORCEINLINE
DECLSPEC_NORETURN
KiSystemCallSysExitReturn(IN PKTRAP_FRAME TrapFrame)
{
	DPRINTT("\n");
	/* Restore nonvolatiles, EAX, and do a SYSEXIT back to the user caller */
#if defined(_MSC_VER)
	_ASM_BEGIN
	mov esp, TrapFrame
#if 1
	mov ds, KTRAP_FRAME.SegDs[esp]
	mov es, KTRAP_FRAME.SegEs[esp]
	mov fs, KTRAP_FRAME.SegFs[esp]
	mov ebp, KTRAP_FRAME.Ebp[esp]
	mov ebx, KTRAP_FRAME.Ebx[esp]
	mov esi, KTRAP_FRAME.Esi[esp]
	mov edi, KTRAP_FRAME.Edi[esp]
	mov eax, KTRAP_FRAME.Eax[esp]
	mov ecx, KTRAP_FRAME.HardwareEsp[esp]
	mov edx, KTRAP_FRAME.Eip[esp]
	add esp, dword ptr offset KTRAP_FRAME.V86Es
	sti
	CpuSysExit
#else
	mov ebx, [esp+KTRAP_FRAME_EBX]
	mov esi, [esp+KTRAP_FRAME_ESI]
	mov edi, [esp+KTRAP_FRAME_EDI]
	mov ebp, [esp+KTRAP_FRAME_EBP]
	mov eax, [esp+KTRAP_FRAME_EAX]
	mov edx, [edx+KTRAP_FRAME_EIP]
	mov ecx, [esp+KTRAP_FRAME_ESP]
	add esp, KTRAP_FRAME_V86_ES
	sti
	CpuSysExit
#endif
	_ASM_END
#elif defined(__GNUC__)
	__asm__ __volatile__
    (
        "movl %0, %%esp\n"
        "movl %c[s](%%esp), %%esi\n"
        "movl %c[b](%%esp), %%ebx\n"
        "movl %c[i](%%esp), %%edi\n"
        "movl %c[p](%%esp), %%ebp\n"
        "movl %c[a](%%esp), %%eax\n"
        "movl %c[e](%%esp), %%edx\n" /* SYSEXIT says EIP in EDX */
        "movl %c[x](%%esp), %%ecx\n" /* SYSEXIT says ESP in ECX */
        "addl $%c[v],%%esp\n" /* A WHOLE *USER* frame since we're not IRET'ing */
        "sti\nsysexit\n"
        :
        : "r"(TrapFrame),
          [b] "i"(KTRAP_FRAME_EBX),
          [s] "i"(KTRAP_FRAME_ESI),
          [i] "i"(KTRAP_FRAME_EDI),
          [p] "i"(KTRAP_FRAME_EBP),
          [a] "i"(KTRAP_FRAME_EAX),
          [e] "i"(KTRAP_FRAME_EIP),
          [x] "i"(KTRAP_FRAME_ESP),
          [v] "i"(KTRAP_FRAME_V86_ES)
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    UNREACHABLE;
}

#if 0
VOID
FORCEINLINE
DECLSPEC_NORETURN
KiTrapReturn(IN PKTRAP_FRAME TrapFrame)
{
    /* Regular interrupt exit */
#if defined(_MSC_VER)
	_ASM_BEGIN
	mov esp, TrapFrame
	mov eax, [esp+KTRAP_FRAME_EAX]
	mov ebx, [esp+KTRAP_FRAME_EBX]
	mov ecx, [esp+KTRAP_FRAME_ECX]
	mov edx, [esp+KTRAP_FRAME_EDX]
	mov esi, [esp+KTRAP_FRAME_ESI]
	mov edi, [esp+KTRAP_FRAME_EDI]
	mov ebp, [esp+KTRAP_FRAME_EBP]
	add esp, KTRAP_FRAME_EIP
	iretd
	_ASM_END
#elif defined(__GNUC__)
	__asm__ __volatile__
    (
        "movl %0, %%esp\n"
        "movl %c[a](%%esp), %%eax\n"
        "movl %c[b](%%esp), %%ebx\n"
        "movl %c[c](%%esp), %%ecx\n"
        "movl %c[d](%%esp), %%edx\n"
        "movl %c[s](%%esp), %%esi\n"
        "movl %c[i](%%esp), %%edi\n"
        "movl %c[p](%%esp), %%ebp\n"
        "addl $%c[e],%%esp\n"
        "iret\n"
        :
        : "r"(TrapFrame),
          [a] "i"(KTRAP_FRAME_EAX),
          [b] "i"(KTRAP_FRAME_EBX),
          [c] "i"(KTRAP_FRAME_ECX),
          [d] "i"(KTRAP_FRAME_EDX),
          [s] "i"(KTRAP_FRAME_ESI),
          [i] "i"(KTRAP_FRAME_EDI),
          [p] "i"(KTRAP_FRAME_EBP),
          [e] "i"(KTRAP_FRAME_EIP)
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    UNREACHABLE;
}
#endif

VOID
FORCEINLINE
DECLSPEC_NORETURN
KiDirectTrapReturn(IN PKTRAP_FRAME TrapFrame)
{
	DPRINTT("\n");
#if defined(_MSC_VER)
	_ASM_BEGIN
		mov esp, TrapFrame
#if 1
	mov ds, KTRAP_FRAME.SegDs[esp]
	mov es, KTRAP_FRAME.SegEs[esp]
	mov fs, KTRAP_FRAME.SegFs[esp]
	mov ebp, KTRAP_FRAME.Ebp[esp]
	mov ebx, KTRAP_FRAME.Ebx[esp]
	mov esi, KTRAP_FRAME.Esi[esp]
	mov edi, KTRAP_FRAME.Edi[esp]
	mov eax, KTRAP_FRAME.Eax[esp]
	mov ecx, KTRAP_FRAME.Ecx[esp]
	mov edx, KTRAP_FRAME.Edx[esp]
	add esp, KTRAP_FRAME_EIP
	iretd
#else
	add esp, KTRAP_FRAME_EIP
	iretd
#endif
	_ASM_END
#elif defined(__GNUC__)
    /* Regular interrupt exit but we're not restoring any registers */
    __asm__ __volatile__
    (
        "movl %0, %%esp\n"
        "addl $%c[e],%%esp\n"
        "iret\n"
        :
        : "r"(TrapFrame),
          [e] "i"(KTRAP_FRAME_EIP)
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    UNREACHABLE;  
}

VOID
FORCEINLINE
DECLSPEC_NORETURN
KiCallReturn(IN PKTRAP_FRAME TrapFrame)
{
	DPRINTT("\n");
    /* Pops a trap frame out of the stack but returns with RET instead of IRET */
#if defined(_MSC_VER)
	_ASM_BEGIN
		mov esp, TrapFrame
		mov ebx, [esp+KTRAP_FRAME_EBX]
		mov esi, [esp+KTRAP_FRAME_ESI]
		mov edi, [esp+KTRAP_FRAME_EDI]
		mov ebp, [esp+KTRAP_FRAME_EBP]
		add esp, KTRAP_FRAME_EIP
		ret
	_ASM_END
#elif defined(__GNUC__)
    __asm__ __volatile__
    (
        "movl %0, %%esp\n"
        "movl %c[b](%%esp), %%ebx\n"
        "movl %c[s](%%esp), %%esi\n"
        "movl %c[i](%%esp), %%edi\n"
        "movl %c[p](%%esp), %%ebp\n"
        "addl $%c[e],%%esp\n"
        "ret\n"
        :
        : "r"(TrapFrame),
          [b] "i"(KTRAP_FRAME_EBX),
          [s] "i"(KTRAP_FRAME_ESI),
          [i] "i"(KTRAP_FRAME_EDI),
          [p] "i"(KTRAP_FRAME_EBP),
          [e] "i"(KTRAP_FRAME_EIP)
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    UNREACHABLE;
}

VOID
FORCEINLINE
DECLSPEC_NORETURN
KiEditedTrapReturn(IN PKTRAP_FRAME TrapFrame)
{
	DPRINTT("\n");
	/* Regular interrupt exit */
#if defined(_MSC_VER)
	_ASM_BEGIN
	mov esp, TrapFrame
	mov eax, [esp+KTRAP_FRAME_EAX]
	mov ebx, [esp+KTRAP_FRAME_EBX]
	mov ecx, [esp+KTRAP_FRAME_ECX]
	mov edx, [esp+KTRAP_FRAME_EDX]
	mov esi, [esp+KTRAP_FRAME_ESI]
	mov edi, [esp+KTRAP_FRAME_EDI]
	mov ebp, [esp+KTRAP_FRAME_EBP]
	add esp, KTRAP_FRAME_ERROR_CODE /* We *WANT* the error code since ESP is there! */
	iretd
	_ASM_END
#elif defined(__GNUC__)
    __asm__ __volatile__
    (
        "movl %0, %%esp\n"
        "movl %c[a](%%esp), %%eax\n"
        "movl %c[b](%%esp), %%ebx\n"
        "movl %c[c](%%esp), %%ecx\n"
        "movl %c[d](%%esp), %%edx\n"
        "movl %c[s](%%esp), %%esi\n"
        "movl %c[i](%%esp), %%edi\n"
        "movl %c[p](%%esp), %%ebp\n"
        "addl $%c[e],%%esp\n"
        "movl (%%esp), %%esp\n"
        "iret\n"
        :
        : "r"(TrapFrame),
          [a] "i"(KTRAP_FRAME_EAX),
          [b] "i"(KTRAP_FRAME_EBX),
          [c] "i"(KTRAP_FRAME_ECX),
          [d] "i"(KTRAP_FRAME_EDX),
          [s] "i"(KTRAP_FRAME_ESI),
          [i] "i"(KTRAP_FRAME_EDI),
          [p] "i"(KTRAP_FRAME_EBP),
          [e] "i"(KTRAP_FRAME_ERROR_CODE) /* We *WANT* the error code since ESP is there! */
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    UNREACHABLE;
}

//
// "BOP" code used by VDM and V8086 Mode
//
VOID
FORCEINLINE
KiIssueBop(VOID)
{
#if defined(_MSC_VER)
	_ASM_BEGIN
		_emit 0xc4
		_emit 0xc4
	_ASM_END
#elif defined(__GNUC__)
	/* Invalid instruction that an invalid opcode handler must trap and handle */
    asm volatile(".byte 0xC4\n.byte 0xC4\n");
#elif
#error unsupported compiler
#endif
}

// Returns whether or not this is a V86 trap by checking the EFLAGS field.
BOOLEAN
FORCEINLINE
KiIsV8086TrapSafe(IN PKTRAP_FRAME TrapFrame)
{
#if defined(_MSC_VER)
	return TrapFrame->EFlags & EFLAGS_V86_MASK ? TRUE : FALSE;
#elif defined(__GNUC__)
    BOOLEAN Result;
    
    /*
     * The check MUST be done this way, as we guarantee that no DS/ES/FS segment
     * is used (since it might be garbage).
     *
     * Instead, we use the SS segment which is guaranteed to be correct. Because
     * operate in 32-bit flat mode, this works just fine.
     */
//
// FIXME: GCC 4.5 Can Improve this with "goto labels"
//
	asm volatile
     (
        "testl $%c[f], %%ss:%1\n"
        "setnz %0\n"
        : "=a"(Result)
        : "m"(TrapFrame->EFlags),
          [f] "i"(EFLAGS_V86_MASK)
     );
    /* If V86 flag was set */ 
    return Result;
#elif
#error unsupported compiler
#endif
}


//
// Returns whether or not this is a user-mode trap by checking the SegCs field.
//
// FIXME: GCC 4.5 Can Improve this with "goto labels"
//
BOOLEAN
FORCEINLINE
KiIsUserTrapSafe(IN PKTRAP_FRAME TrapFrame)
{
#if defined(_MSC_VER)
	return TrapFrame->SegCs != KGDT_R0_CODE ? TRUE : FALSE;
#elif defined(__GNUC__)
    /*
     * The check MUST be done this way, as we guarantee that no DS/ES/FS segment
     * is used (since it might be garbage).
     *
     * Instead, we use the SS segment which is guaranteed to be correct. Because
     * operate in 32-bit flat mode, this works just fine.
     */
    BOOLEAN Result;

	asm volatile
     (
        "cmp $%c[f], %%ss:%1\n"
        "setnz %0\n"
        : "=a"(Result)
        : "m"(TrapFrame->SegCs),
          [f] "i"(KGDT_R0_CODE)
     );
    /* If V86 flag was set */ 
    return Result;
#elif
#error unsupported compiler
#endif
   
}

VOID
FORCEINLINE
KiUserSystemCall(IN PKTRAP_FRAME TrapFrame)
{
    /*
     * Kernel call or user call?
     *
     * This decision is made in inlined assembly because we need to patch
     * the relative offset of the user-mode jump to point to the SYSEXIT
     * routine if the CPU supports it. The only way to guarantee that a
     * relative jnz/jz instruction is generated is to force it with the
     * inline assembler.
     */
	// !!!
	DPRINT1("!!!FIX\n");
#if 0
	if (TrapFrame->SegCs & 1)
		KiSystemCallExit(TrapFrame);
#endif
}
  
#if 0
//
// Generic Exit Routine
//
VOID
FORCEINLINE
KiExitTrap(IN PKTRAP_FRAME TrapFrame,
           IN UCHAR Skip)
{
    // KTRAP_EXIT_SKIP_BITS SkipBits = { .Bits = Skip };
    KTRAP_EXIT_SKIP_BITS SkipBits;
    PULONG ReturnStack;

	DPRINTT("\n");

	SkipBits.Bits = Skip;

    /* Debugging checks */
    KiExitTrapDebugChecks(TrapFrame, SkipBits);
	DPRINTT("DebugChecks r\n");

    /* Restore the SEH handler chain */
    KeGetPcr()->Tib.ExceptionList = TrapFrame->ExceptionList;
    
    /* Check if the previous mode must be restored */
    if (__builtin_expect(!SkipBits.SkipPreviousMode, 0)) /* More INTS than SYSCALLs */
    {
        /* Restore it */
        KeGetCurrentThread()->PreviousMode = (CCHAR)TrapFrame->PreviousPreviousMode;
    }

    /* Check if there are active debug registers */
    if (__builtin_expect(TrapFrame->Dr7 & ~DR7_RESERVED_MASK, 0))
    {
        /* Not handled yet */
        DbgPrint("Need Hardware Breakpoint Support!\n");
        DbgBreakPoint();
        while (TRUE);
    }
    
    DPRINTT("100\n");

	/* Check if this was a V8086 trap */
    if (__builtin_expect(TrapFrame->EFlags & EFLAGS_V86_MASK, 0)) KiTrapReturn(TrapFrame);

    /* Check if the trap frame was edited */
    if (__builtin_expect(!(TrapFrame->SegCs & FRAME_EDITED), 0))
    {   
        /*
         * An edited trap frame happens when we need to modify CS and/or ESP but
         * don't actually have a ring transition. This happens when a kernelmode
         * caller wants to perform an NtContinue to another kernel address, such
         * as in the case of SEH (basically, a longjmp), or to a user address.
         *
         * Therefore, the CPU never saved CS/ESP on the stack because we did not
         * get a trap frame due to a ring transition (there was no interrupt).
         * Even if we didn't want to restore CS to a new value, a problem occurs
         * due to the fact a normal RET would not work if we restored ESP since
         * RET would then try to read the result off the stack.
         *
         * The NT kernel solves this by adding 12 bytes of stack to the exiting
         * trap frame, in which EFLAGS, CS, and EIP are stored, and then saving
         * the ESP that's being requested into the ErrorCode field. It will then
         * exit with an IRET. This fixes both issues, because it gives the stack
         * some space where to hold the return address and then end up with the
         * wanted stack, and it uses IRET which allows a new CS to be inputted.
         *
         */
        DPRINTT("edited\n");
        /* Set CS that is requested */
        TrapFrame->SegCs = TrapFrame->TempSegCs;
         
        /* First make space on requested stack */
        ReturnStack = (PULONG)(TrapFrame->TempEsp - 12);
        TrapFrame->ErrCode = (ULONG_PTR)ReturnStack;
         
        /* Now copy IRET frame */
        ReturnStack[0] = TrapFrame->Eip;
        ReturnStack[1] = TrapFrame->SegCs;
        ReturnStack[2] = TrapFrame->EFlags;
        
        /* Do special edited return */
        KiEditedTrapReturn(TrapFrame);
    }
    
    /* Check if this is a user trap */
    if (__builtin_expect(KiUserTrap(TrapFrame), 1)) /* Ring 3 is where we spend time */
    {
        DPRINTT("user\n");
		/* Check if segments should be restored */
        if (!SkipBits.SkipSegments)
        {
            /* Restore segments */
            Ke386SetGs(TrapFrame->SegGs);
            Ke386SetEs(TrapFrame->SegEs);
            Ke386SetDs(TrapFrame->SegDs);
            Ke386SetFs(TrapFrame->SegFs);
        }
        
        /* Always restore FS since it goes from KPCR to TEB */
        Ke386SetFs(TrapFrame->SegFs);
    }
    
    /* Check for system call -- a system call skips volatiles! */
    if (__builtin_expect(SkipBits.SkipVolatiles, 0)) /* More INTs than SYSCALLs */
    {
        DPRINTT("skip vol\n");
		/* User or kernel call? */
        KiUserSystemCall(TrapFrame);
        
        /* Restore EFLags */
        __writeeflags(TrapFrame->EFlags);
            
        /* Call is kernel, so do a jump back since this wasn't a real INT */
        KiSystemCallReturn(TrapFrame);

        /* If we got here, this is SYSEXIT: are we stepping code? */
        if (!(TrapFrame->EFlags & EFLAGS_TF))
        {
                 /* Restore user FS */
                Ke386SetFs(KGDT_R3_TEB | RPL_MASK);
                
                /* Remove interrupt flag */
                TrapFrame->EFlags &= ~EFLAGS_INTERRUPT_MASK;
                __writeeflags(TrapFrame->EFlags);
                
                /* Exit through SYSEXIT */
                KiSystemCallSysExitReturn(TrapFrame);
            }
            
        /* Exit through IRETD, either due to debugging or due to lack of SYSEXIT */
        KiSystemCallTrapReturn(TrapFrame);
    }
  
	/* Return from interrupt */
	DPRINTT("KiTrapReturn");
    KiTrapReturn(TrapFrame);
	DPRINTT("r\n");
}
#endif

//
// Virtual 8086 Mode Optimized Trap Exit
//
VOID
FORCEINLINE
KiExitV86Trap(IN PKTRAP_FRAME TrapFrame)
{
    PKTHREAD Thread;
    KIRQL OldIrql;
    
    /* Get the thread */
    Thread = KeGetCurrentThread();
    while (TRUE)
    {
        /* Turn off the alerted state for kernel mode */
        Thread->Alerted[KernelMode] = FALSE;

        /* Are there pending user APCs? */
        if (__builtin_expect(!Thread->ApcState.UserApcPending, 1)) break;

        /* Raise to APC level and enable interrupts */
        OldIrql = KfRaiseIrql(APC_LEVEL);
        _enable();

        /* Deliver APCs */
        KiDeliverApc(UserMode, NULL, TrapFrame);

        /* Restore IRQL and disable interrupts once again */
        KfLowerIrql(OldIrql);
        _disable();
        
        /* Return if this isn't V86 mode anymore */
        if (__builtin_expect(TrapFrame->EFlags & EFLAGS_V86_MASK, 0)) return;
    }
     
    /* If we got here, we're still in a valid V8086 context, so quit it */
    if (__builtin_expect(TrapFrame->Dr7 & ~DR7_RESERVED_MASK, 0))
    {
        /* Not handled yet */
        DbgPrint("Need Hardware Breakpoint Support!\n");
        while (TRUE);
    }
     
    /* Return from interrupt */
    KiTrapReturn(TrapFrame);
}

//
// Virtual 8086 Mode Optimized Trap Entry
//
VOID
FORCEINLINE
KiEnterV86Trap(IN PKTRAP_FRAME TrapFrame)
{
    /* Load correct registers */
    Ke386SetFs(KGDT_R0_PCR);
    Ke386SetDs(KGDT_R3_DATA | RPL_MASK);
    Ke386SetEs(KGDT_R3_DATA | RPL_MASK);

	/* Save exception list */
    TrapFrame->ExceptionList = KeGetPcr()->Tib.ExceptionList;

    /* Clear direction flag */
    Ke386ClearDirectionFlag();
    
    /* Save DR7 and check for debugging */
    TrapFrame->Dr7 = __readdr(7);
    if (__builtin_expect(TrapFrame->Dr7 & ~DR7_RESERVED_MASK, 0))
    {
        DbgPrint("Need Hardware Breakpoint Support!\n");
        while (TRUE);
    }
}

#if 0
//
// Interrupt Trap Entry
//
VOID
FORCEINLINE
KiEnterInterruptTrap(IN PKTRAP_FRAME TrapFrame)
{

	// ULONG Ds, Es;
    
    /* Check for V86 mode, otherwise check for ring 3 code */
    if (__builtin_expect(KiIsV8086TrapSafe(TrapFrame), 0))
    {
        /* Set correct segments */
        // Ke386SetDs(KGDT_R3_DATA | RPL_MASK);
        // Ke386SetEs(KGDT_R3_DATA | RPL_MASK);
        // Ke386SetFs(KGDT_R0_PCR);

        /* Restore V8086 segments into Protected Mode segments */
        TrapFrame->SegFs = TrapFrame->V86Fs;
        TrapFrame->SegGs = TrapFrame->V86Gs;
        TrapFrame->SegDs = TrapFrame->V86Ds;
        TrapFrame->SegEs = TrapFrame->V86Es;
    }
    else if (__builtin_expect(KiIsUserTrapSafe(TrapFrame), 1)) /* Ring 3 is more common */
    {
        /* Save GS */
        // TrapFrame->SegFs = Ke386GetFs();
        TrapFrame->SegGs = Ke386GetGs();
        
        /* Set correct FS */
        // Ke386SetFs(KGDT_R0_PCR);
    }       
    
    /* Save exception list and terminate it */
    TrapFrame->ExceptionList = KeGetPcr()->Tib.ExceptionList;
    KeGetPcr()->Tib.ExceptionList = EXCEPTION_CHAIN_END;

    /* Clear direction flag */
    Ke386ClearDirectionFlag();
    
    /* Flush DR7 and check for debugging */
    TrapFrame->Dr7 = 0;
    if (__builtin_expect(KeGetCurrentThread()->DispatcherHeader.DebugActive & 0xFF, 0))
    {
        DbgPrint("Need Hardware Breakpoint Support!\n");
        while (TRUE);
    }
    
    /* Set debug header */
    // KiFillTrapFrameDebug(TrapFrame);
}
#endif


#if 0
//
// Generic Trap Entry
//
VOID
FORCEINLINE
KiEnterTrap(IN PKTRAP_FRAME TrapFrame)
{
    USHORT Ds, Es;
    
    /*
     * We really have to get a good DS/ES first before touching any data.
     *
     * These two reads will either go in a register (with optimizations ON) or
     * a stack variable (which is on SS:ESP, guaranteed to be good/valid).
     *
     * Because the assembly is marked volatile, the order of instructions is
     * as-is, otherwise the optimizer could simply get rid of our DS/ES.
     *
     */
    // !!! 
	Ds = Ke386GetDs();
    Es = Ke386GetEs();
    Ke386SetDs(KGDT_R3_DATA | RPL_MASK);
    Ke386SetEs(KGDT_R3_DATA | RPL_MASK);
    TrapFrame->SegDs = Ds;
    TrapFrame->SegEs = Es;
        
    /* Now we can save the other segments and then switch to the correct FS */
    TrapFrame->SegFs = Ke386GetFs();
    TrapFrame->SegGs = Ke386GetGs();
    Ke386SetFs(KGDT_R0_PCR);

    /* Save exception list */
    TrapFrame->ExceptionList = KeGetPcr()->Tib.ExceptionList;
    
    /* Check for V86 mode */
    if (__builtin_expect(TrapFrame->EFlags & EFLAGS_V86_MASK, 0))
    {
        /* Restore V8086 segments into Protected Mode segments */
        TrapFrame->SegFs = TrapFrame->V86Fs;
        TrapFrame->SegGs = TrapFrame->V86Gs;
        TrapFrame->SegDs = TrapFrame->V86Ds;
        TrapFrame->SegEs = TrapFrame->V86Es;
    }

    /* Clear direction flag */
    Ke386ClearDirectionFlag();
    
    /* Flush DR7 and check for debugging */
    TrapFrame->Dr7 = 0;
    if (__builtin_expect(KeGetCurrentThread()->DispatcherHeader.DebugActive & 0xFF, 0))
    {
        DbgPrint("Need Hardware Breakpoint Support!\n");
        while (TRUE);
    }
    
    /* Set debug header */
    KiFillTrapFrameDebug(TrapFrame);
}
#endif

//
// Generates a Trap Prolog Stub for the given name
//
#define KI_PUSH_FAKE_ERROR_CODE 0x1
#define KI_UNUSED               0x2
#define KI_NONVOLATILES_ONLY    0x4
#define KI_FAST_SYSTEM_CALL     0x8
#define KI_SOFTWARE_TRAP        0x10
#define KI_HARDWARE_INT         0x20
#define KiTrap(x, y)            VOID x(VOID) { KiTrapStub(y, (PVOID)x##Handler); UNREACHABLE; }
#define KiTrampoline(x, y)      VOID x(VOID) { KiTrapStub(y, x##Handler); }

//
// Trap Prolog Stub
// !!! ultrahorrible
// 
VOID
FORCEINLINE
KiTrapStub(IN ULONG Flags,
           IN PVOID Handler)
{
    ULONG FrameSize;
    
    /* Is this a fast system call? They don't have a stack! */
    if (Flags & KI_FAST_SYSTEM_CALL)
	{
#if defined(_MSC_VER)
		_ASM_BEGIN
		mov esp, ss:[KIP0PCRADDRESS + offset KPCR.TSS]
		mov esp, KTSS.Esp0[esp]
		_ASM_END
#elif defined(__GNUC__)
		__asm__ __volatile__
		(
			"movl %%ss:%c[t], %%esp\n"
			"movl %c[e](%%esp), %%esp\n"
			:
			: [e] "i"(FIELD_OFFSET(KTSS, Esp0)),
			  [t] "i"(&PCR->TSS)
			: "%esp"
		);
#elif
#error unsupported compiler
#endif
	}
    
    /* Check what kind of trap frame this trap requires */
    if (Flags & KI_SOFTWARE_TRAP)
    {
        /* Software traps need a complete non-ring transition trap frame */
        FrameSize = FIELD_OFFSET(KTRAP_FRAME, HardwareEsp);
    }
    else if (Flags & KI_FAST_SYSTEM_CALL)
    {
        /* SYSENTER requires us to build a complete ring transition trap frame */
        FrameSize = FIELD_OFFSET(KTRAP_FRAME, V86Es);
        
        /* And it only preserves nonvolatile registers */
        Flags |= KI_NONVOLATILES_ONLY;
    }
    else if (Flags & KI_PUSH_FAKE_ERROR_CODE)
    {
        /* If the trap doesn't have an error code, we'll make space for it */
        FrameSize = FIELD_OFFSET(KTRAP_FRAME, Eip);
    }
    else
    {
        /* The trap already has an error code, so just make space for the rest */
        FrameSize = FIELD_OFFSET(KTRAP_FRAME, ErrCode);
    }
    
    /* Software traps need to get their EIP from the caller's frame */
    if (Flags & KI_SOFTWARE_TRAP)
	{
#if defined(_MSC_VER)
		_ASM pop eax
#elif defined(__GNUC)
		__asm__ __volatile__ ("popl %%eax\n":::"%esp");
#elif
#error unsupported compiler
#endif
	}
    /* Save nonvolatile registers */
#if defined(_MSC_VER)
	_ASM_BEGIN
		mov KTRAP_FRAME.Ebp[esp], ebp
		mov KTRAP_FRAME.Ebx[esp], ebx
		mov KTRAP_FRAME.Esi[esp], esi
		mov KTRAP_FRAME.Edi[esp], edi
	_ASM_END
#elif defined(__GNUC)
    __asm__ __volatile__
    (
        /* EBX, ESI, EDI and EBP are saved */
        "movl %%ebp, %c[p](%%esp)\n"
        "movl %%ebx, %c[b](%%esp)\n"
        "movl %%esi, %c[s](%%esp)\n"
        "movl %%edi, %c[i](%%esp)\n"
        :
        : [b] "i"(- FrameSize + FIELD_OFFSET(KTRAP_FRAME, Ebx)),
          [s] "i"(- FrameSize + FIELD_OFFSET(KTRAP_FRAME, Esi)),
          [i] "i"(- FrameSize + FIELD_OFFSET(KTRAP_FRAME, Edi)),
          [p] "i"(- FrameSize + FIELD_OFFSET(KTRAP_FRAME, Ebp))
        : "%esp"
    );
#elif
#error unsupported compiler
#endif
    
    /* Does the caller want nonvolatiles only? */
    if (!(Flags & KI_NONVOLATILES_ONLY))
	{
#if defined(_MSC_VER)
		_ASM_BEGIN
			mov KTRAP_FRAME.Eax[esp], eax
			mov KTRAP_FRAME.Ecx[esp], ecx
			mov KTRAP_FRAME.Edx[esp], edx
		_ASM_END
#elif defined(__GNUC)
		__asm__ __volatile__
		(
			/* Otherwise, save the volatiles as well */
			"movl %%eax, %c[a](%%esp)\n"
			"movl %%ecx, %c[c](%%esp)\n"
			"movl %%edx, %c[d](%%esp)\n"
			:
			: [a] "i"(- FrameSize + FIELD_OFFSET(KTRAP_FRAME, Eax)),
			  [c] "i"(- FrameSize + FIELD_OFFSET(KTRAP_FRAME, Ecx)),
			  [d] "i"(- FrameSize + FIELD_OFFSET(KTRAP_FRAME, Edx))
			: "%esp"
		);
#elif
#error unsupported compiler
#endif
	}
	/* Now set parameter 1 (ECX) to point to the frame */
    /* make space for this frame */
#if defined(_MSC_VER)
	_ASM_BEGIN
		sub esp, FrameSize
		mov ecx, esp
	_ASM_END
	// !!! this is horrible and won't work after stkf optimization
#elif defined(__GNUC)
	__asm__ __volatile__ ("movl %%esp, %%ecx\n":::"%esp");
    __asm__ __volatile__ ("subl $%c[e],%%esp\n":: [e] "i"(FrameSize) : "%esp");
    __asm__ __volatile__ ("subl $%c[e],%%ecx\n":: [e] "i"(FrameSize) : "%ecx");
#elif
#error unsupported compiler
#endif

    /* Now jump to the C handler */

	/*
     * For hardware interrupts, set parameter 2 (EDX) to hold KINTERRUPT.
     * This code will be dynamically patched when an interrupt is registered!
	 !!! ultrahorrible
     */
	if (Flags & KI_HARDWARE_INT)
	{
#if defined(_MSC_VER)
		// !!! temp hack
		_ASM_BEGIN
			mov edx, [TrapStubInterrupt]
			jmp [Handler]
		_ASM_END
#elif defined(__GNUC)
		__asm__ __volatile__
		(
			".globl _KiInterruptTemplate2ndDispatch\n_KiInterruptTemplate2ndDispatch:\n"
			"movl $0, %%edx\n"
			".globl _KiInterruptTemplateObject\n_KiInterruptTemplateObject:\n"
			::: "%edx"
			"jmp *%0\n"
			".globl _KiInterruptTemplateDispatch\n_KiInterruptTemplateDispatch:\n"
			:
			: "a"(Handler)

		);
#elif
#error unsupported compiler
#endif
	}
    
#if defined(_MSC_VER)
		_ASM jmp [Handler]
#elif defined(__GNUC)
	else __asm__ __volatile__ ("jmp %c[x]\n":: [x] "i"(Handler));
#elif
#error unsupported compiler
#endif
}

#endif
