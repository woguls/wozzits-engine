from .grammar import Program, Interface

def parse_program(lines):

    program = Program()
    i = 0

    while i < len(lines):

        line = lines[i].strip()

        if line.startswith("interface"):
            name = line.split()[1]

            inputs = lines[i+1].split()[1:]
            outputs = lines[i+2].split()[1:]

            program.interfaces.append(
                Interface(name, inputs, outputs)
            )

            i += 3
            continue

        i += 1

    return program