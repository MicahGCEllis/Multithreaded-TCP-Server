.text
.global switch_context

switch_context:
    pushq %rbx
    pushq %rbp
    pushq %rdi
    pushq %rsi
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    subq $160, %rsp

    movups %xmm6, 0(%rsp)
    movups %xmm7, 16(%rsp)
    movups %xmm8, 32(%rsp)
    movups %xmm9, 48(%rsp)
    movups %xmm10, 64(%rsp)
    movups %xmm11, 80(%rsp)
    movups %xmm12, 96(%rsp)
    movups %xmm13, 112(%rsp)
    movups %xmm14, 128(%rsp)
    movups %xmm15, 144(%rsp)

    movq %rsp, (%rcx)
    movq (%rdx), %rsp

    movups 0(%rsp), %xmm6
    movups 16(%rsp), %xmm7
    movups 32(%rsp), %xmm8
    movups 48(%rsp), %xmm9
    movups 64(%rsp), %xmm10
    movups 80(%rsp), %xmm11
    movups 96(%rsp), %xmm12
    movups 112(%rsp), %xmm13
    movups 128(%rsp), %xmm14
    movups 144(%rsp), %xmm15

    addq $160, %rsp

    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %rsi
    popq %rdi
    popq %rbp
    popq %rbx

    ret

.data
.global REGISTER_PADDING
REGISTER_PADDING:
    .quad 224
