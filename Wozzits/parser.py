from .grammar import Program, Interface, Transform, Pipeline, State, Resource, Event

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

        if line.startswith("transform"):
            name = line.split()[1]
            inputs = lines[i+1].split()[1:]
            outputs = lines[i+2].split()[1:]

            program.transforms.append(
                Transform(name, inputs, outputs)
            )

            i += 3
            continue

        if line.startswith("pipeline"):
            name = line.split()[1]
            stages = lines[i+1].split()

            program.pipelines.append(
                Pipeline(name, stages)
            )

            i += 2
            continue

        if line.startswith("state"):
            name = line.split()[1]
            fields = lines[i+1].split()

            program.states.append(
                State(name, fields)
            )

            i += 2
            continue

        if line.startswith("resource"):
            name = line.split()[1]
            type_ = line.split()[2]

            program.resources.append(
                Resource(name, type_)
            )

            i += 1
            continue

        if line.startswith("event"):
            name = line.split()[1]
            fields = lines[i+1].split()

            program.events.append(
                Event(name, fields)
            )

            i += 2
            continue

        i += 1

    return program
