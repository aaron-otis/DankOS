# Generates assembly code for 256 interrupt handlers and a header file to
# hold the externs for each asm label so that c functions may access them.

def main():
    idt_size = 256 # Size of IDT table.

    # Files to write to.
    asm_file = "asm/isr_wrapper.asm"
    header_file = "drivers/interrupt_externs.h"
    code_file = header_file[:-1] + 'c'

    # Interrupt vectors with error codes.
    has_error = [0x8, 0xA, 0xB, 0xC, 0xD, 0xE, 0x11, 0x1E]
    f = open(asm_file, "w") # Assembly file.

    # Make all labels global.
    for i in range(256):
        f.write("global isr_wrapper{}\n".format(i))
    f.write("\n");
    f.write("extern IRQ_handler\n\nsection .text\n") # Common C handler.
    f.write("extern cur_proc\nextern next_proc\n\n") # Process structs.

    # Create common IRQ handler.
    f.write("common_irq_handler:\n") # Label.

    f.write("\tpush rsi\n\tpush rdx\n") # Push registers onto the stack.
    f.write("\tpush rcx\n\tpush r8\n\tpush r9\n")

    f.write("\tcall IRQ_handler\n") # Call IRQ handler function.

    # Set up check for context switching.
    f.write("\tmov rcx, [cur_proc]\n\tcmp rcx, [next_proc]\n")
    f.write("\tje no_swap\n\n")

    #
    # ASM for context swapping.
    #

    # Skip loading if current process is NULL.
    f.write("\n\tcmp rcx, 0\n\tje load_context\n\n")

    #
    # Save current context.
    #
    # Need to dereference the pointer to |cur_proc| and store the address
    # to the struct in a register.
    #

    f.write("save_context:\n")

    # Dereference |cur_proc| and store in rdi.
    f.write("\tmov rdi, [cur_proc]\n")

    # Save rax and rbx.
    f.write("\tmov [rdi], rax\n\tmov [rdi + 8], rbx\n")

    # Save rcx.
    f.write("\tmov rax, [rsp + 16]\n\tmov [rdi + 16], rax\n")

    # Save rdx.
    f.write("\tmov rax, [rsp + 24]\n\tmov [rdi + 24], rax\n")

    # Save rdi.
    f.write("\tmov rax, [rsp + 40]\n\tmov [rdi + 32], rax\n")

    # Save rsi.
    f.write("\tmov rax, [rsp + 32]\n\tmov [rdi + 40], rax\n")

    # Save r8.
    f.write("\tmov rax, [rsp + 8]\n\tmov [rdi + 48], rax\n")

    # Save r9.
    f.write("\tmov rax, [rsp]\n\tmov [rdi + 56], rax\n")

    # Save r10 and r11.
    f.write("\tmov [rdi + 64], r10\n\tmov [rdi + 72], r11\n")

    # Save r12 and r13.
    f.write("\tmov [rdi + 80], r12\n\tmov [rdi + 88], r13\n")

    # Save r14 and r15.
    f.write("\tmov [rdi + 96], r14\n\tmov [rdi + 104], r15\n")

    # Save rbp.
    f.write("\tmov [rdi + 112], rbp\n")

    # Save rsp.
    f.write("\tmov rax, [rsp + 72]\n\tmov [rdi + 120], rax\n")

    # Save rip.
    f.write("\tmov rax, [rsp + 48]\n\tmov [rdi + 128], rax\n")

    # Save rflags.
    f.write("\tmov rax, [rsp + 64]\n\tmov [rdi + 136], rax\n")

    # Save CS.
    f.write("\tmov rax, [rsp + 56]\n\tmov [rdi + 136], rax\n")

    # Save SS
    f.write("\tmov rax, [rsp + 80]\n\tmov [rdi + 152], rax\n")

    # Save DS and ES.
    f.write("\tmov [rdi + 160], ds\n\tmov [rdi + 168], es\n")

    # Save FS and GS.
    f.write("\tmov [rdi + 176], fs\n\tmov [rdi + 184], gs\n\n")

    #
    # Load next context.
    #

    f.write("load_context:\n")

    # Dereference |next_proc| and store in rdi.
    f.write("\tmov rdi, [next_proc]\n")

    # Load rax and rbx.
    f.write("\tmov rax, [rdi]\n\tmov rbx, [rdi + 8]\n")

    # Load rcx.
    f.write("\tmov rcx, [rdi + 16]\n\tmov [rsp + 16], rcx\n")

    # Load rdx.
    f.write("\tmov rcx, [rdi + 24]\n\tmov [rsp + 24], rcx\n")

    # Load rdi.
    f.write("\tmov rcx, [rdi + 32]\n\tmov [rsp + 40], rcx\n")

    # Load rsi.
    f.write("\tmov rcx, [rdi + 40]\n\tmov [rsp + 32], rcx\n")

    # Load r8.
    f.write("\tmov rcx, [rdi + 48]\n\tmov [rsp + 8], rcx\n")

    # Load r9.
    f.write("\tmov rcx, [rdi + 56]\n\tmov [rsp], rcx\n")

    # Load r10 and r11.
    f.write("\tmov r10, [rdi + 64]\n\tmov r11, [rdi + 72]\n")

    # Load r12 and r13.
    f.write("\tmov r12, [rdi + 80]\n\tmov r13, [rdi + 88]\n")

    # Load r14 and r15.
    f.write("\tmov r14, [rdi + 96]\n\tmov r15, [rdi + 104]\n")

    # Load rbp.
    f.write("\tmov rbp, [rdi + 112]\n")

    # Load rsp.
    f.write("\tmov rcx, [rdi + 120]\n\tmov [rsp + 72], rcx\n")

    # Load rip.
    f.write("\tmov rcx, [rdi + 128]\n\tmov [rsp + 48], rcx\n")

    # Load rflags.
    f.write("\tmov rcx, [rdi + 136]\n\tmov [rsp + 64], rcx\n")

    # Load CS.
    f.write("\tmov rcx, [rdi + 144]\n\tmov [rsp + 56], rcx\n")

    # Load SS.
    f.write("\tmov rcx, [rdi + 152]\n\tmov [rsp + 80], rcx\n")

    # Load DS and ES.
    f.write("\tmov ds, [rdi + 160]\n\tmov es, [rdi + 168]\n")

    # Load FS and GS.
    f.write("\tmov fs, [rdi + 176]\n\tmov gs, [rdi + 184]\n\n")

    # Set |cur_proc| equal to |next_proc|.
    f.write("\tmov rcx, [next_proc]\n\tmov [cur_proc], rcx\n\n")

    #
    # No context swap needed.
    #

    f.write("no_swap:\n")
    f.write("\tpop r9\n\tpop r8\n\tpop rcx\n") # Restore register values.
    f.write("\tpop rdx\n\tpop rsi\n\tpop rdi\n")

    f.write("\tiretq\n\n") # Return.

    # Create individual irq handlers.
    for i in range(idt_size):
        f.write("isr_wrapper{}:\n".format(i)) # Instruction label.

        f.write("\tpush rdi\n") # Save rdi register before clobbering it.

        if i in has_error: # Move error message to correct place.
            f.write("\tmov rdi, [rsp + 8]\n")
            f.write("\tpush rsi\n")
            f.write("\tmov rsi, [rsp + 8]\n")
            f.write("\tmov [rsp + 16], rsi\n")
            f.write("\tmov rsi, [rsp]\n")
            f.write("\tmov [rsp + 8], rsi\n")
            f.write("\tadd rsp, 8\n")
            f.write("\tmov rsi, rdi\n")
        
        # Put IRQ number into rdi as first parameter for function call.
        f.write("\tmov rdi, {}\n".format(i))

        # Put error code into rsi as second paramter, if an error code exists.
        if i in has_error:
            f.write("\tjmp common_irq_handler + 1\n")
        else:
            f.write("\tjmp common_irq_handler\n")

        f.write("\n")

    f.close()
    f = open(header_file, "w")

    # Create header file with externs of asm labels.
    header_mod = header_file[8:].replace(".", "_").upper()
    f.write("#ifndef _{}\n".format(header_mod))
    f.write("#define {}\n\n".format(header_mod))
    f.write('#include "interrupts.h"\n\n')

    for i in range(idt_size):
        f.write("extern void isr_wrapper{}();\n".format(i))

    f.write("\nvoid populate_IDT_table();\n\n")
    f.write("\n#endif")
    f.close()

    f = open(code_file, "w")
    # Write includes first.
    f.write('#include "{}"\n'.format(header_file[8:]))
    f.write('#include "../lib/stdint.h"\n')
    f.write('#include "../gdt.h"\n\n')

    # Write defines next.
    f.write("#define OFF_1_MASK 0xFFFF\n")
    f.write("#define OFF_2_MASK 0xFFFF\n")
    f.write("#define OFF_2_SHIFT 16\n")
    f.write("#define OFF_3_SHIFT 32\n")
    f.write("#define KERNEL_CS 0x08\n")
    f.write("#define OPTIONS 0x8700\n")
    f.write("#define INT_GATE 0xE\n\n")

    # Create function.
    f.write("void populate_IDT_table() {\n\tuint64_t addr;\n\n")
    segment = "(1<<43) | (1<<44) | (1<<47) | (1<<53)"

    for i in range(256):
        # Get address of assembly code labels and put them in the correct places.
        f.write("\t/* Setup IDT entry {}. */\n".format(i))
        f.write("\taddr = (uint64_t) isr_wrapper{};\n".format(i))
        f.write("\tIDT[{0}].offset1 = addr & OFF_1_MASK;\n".format(i))
        f.write("\tIDT[{0}].offset2 = (addr >> OFF_2_SHIFT) & OFF_2_MASK;\n".format(i))
        f.write("\tIDT[{0}].offset3 = addr >> OFF_3_SHIFT;\n".format(i))
        #f.write("\tIDT[{0}].offset1 = addr >> 0x30;\n".format(i))
        #f.write("\tIDT[{0}].offset2 = (addr >> 0x20) & 0xFFFF;\n".format(i))
        #f.write("\tIDT[{0}].offset3 = addr & 0xFFFFFFFF;\n".format(i))

        f.write("\tIDT[{0}].ist = 0;\n".format(i))
        f.write("\tIDT[{0}].type = INT_GATE;\n".format(i))
        f.write("\tIDT[{0}].dpl = 0;\n".format(i))
        f.write("\tIDT[{0}].present = 1;\n".format(i))
        f.write("\tIDT[{0}].selector = KERNEL_CS;\n".format(i));

    f.write("}\n\n")

if __name__ == '__main__':
    main()
