#ifndef ASM_NL
#define ASM_NL  ;
#endif

#ifndef __ALIGN
#define __ALIGN       .p2align 4,0x90
#define __ALIGN_STR   __stringify(__ALIGN)
#endif

#define ALIGN __ALIGN
#define ALIGN_STR __ALIGN_STR

#ifndef ENTRY
#define ENTRY(name) \
    .globl name ASM_NL \
    ALIGN ASM_NL \
    name:
#endif

#ifndef END
#define END(name) \
    .size name, .-name
#endif

#ifndef ENDPROC
#define ENDPROC(name) \
    .type name, @function ASM_NL \
    END(name)
#endif
