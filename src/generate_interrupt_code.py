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
    f.write("extern IRQ_handler\n\nsection .text\n\n")

    # Create common IRQ handler.
    f.write("common_irq_handler:\n") # Label.

    f.write("\tpush rsi\n\tpush rdx\n") # Push registers onto the stack.
    f.write("\tpush rcx\n\tpush r8\n\tpush r9\n")

    f.write("\tcall IRQ_handler\n") # Call IRQ handler function.

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
