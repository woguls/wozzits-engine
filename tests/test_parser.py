import unittest
from Wozzits.parser import parse_program
from Wozzits.grammar import Program, Interface

class TestParseProgram(unittest.TestCase):

    def test_empty_input(self):
        program = parse_program([])
        self.assertIsInstance(program, Program)
        self.assertEqual(len(program.interfaces), 0)

    def test_single_interface(self):
        lines = [
            "interface MyInterface",
            "inputs a b",
            "outputs c d"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.interfaces), 1)
        interface = program.interfaces[0]
        self.assertEqual(interface.name, "MyInterface")
        self.assertEqual(interface.inputs, ["a", "b"])
        self.assertEqual(interface.outputs, ["c", "d"])

    def test_multiple_interfaces(self):
        lines = [
            "interface Int1",
            "inputs x",
            "outputs y",
            "interface Int2",
            "inputs p q",
            "outputs r"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.interfaces), 2)
        self.assertEqual(program.interfaces[0].name, "Int1")
        self.assertEqual(program.interfaces[1].name, "Int2")

    def test_interface_with_no_inputs_or_outputs(self):
        lines = [
            "interface EmptyInterface",
            "inputs",
            "outputs"
        ]
        program = parse_program(lines)
        interface = program.interfaces[0]
        self.assertEqual(interface.name, "EmptyInterface")
        self.assertEqual(interface.inputs, [])
        self.assertEqual(interface.outputs, [])

    def test_interface_with_extra_whitespace(self):
        lines = [
            "  interface   MyInterface  ",
            "  inputs   a   b  ",
            "  outputs   c   d  "
        ]
        program = parse_program(lines)
        interface = program.interfaces[0]
        self.assertEqual(interface.name, "MyInterface")
        self.assertEqual(interface.inputs, ["a", "b"])
        self.assertEqual(interface.outputs, ["c", "d"])

    def test_non_interface_lines_ignored(self):
        lines = [
            "some random line",
            "interface Test",
            "inputs i",
            "outputs o",
            "another random line"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.interfaces), 1)
        self.assertEqual(program.interfaces[0].name, "Test")

    def test_incomplete_interface_block_raises_index_error(self):
        # Parser assumes next two lines exist after 'interface'
        lines = [
            "interface Incomplete"
        ]
        with self.assertRaises(IndexError):
            parse_program(lines)

    def test_program_has_other_fields_empty(self):
        lines = [
            "interface Test",
            "inputs a",
            "outputs b"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.transforms), 0)
        self.assertEqual(len(program.pipelines), 0)
        self.assertEqual(len(program.states), 0)
        self.assertEqual(len(program.resources), 0)
        self.assertEqual(len(program.events), 0)

if __name__ == '__main__':
    unittest.main()
