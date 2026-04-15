import unittest
from Wozzits.parser import parse_program
from Wozzits.grammar import Program, Interface, Transform, Pipeline, State, Resource, Event

class TestParseProgram(unittest.TestCase):

    def test_empty_input(self):
        program = parse_program([])
        self.assertIsInstance(program, Program)
        self.assertEqual(len(program.interfaces), 0)
        self.assertEqual(len(program.transforms), 0)
        self.assertEqual(len(program.pipelines), 0)
        self.assertEqual(len(program.states), 0)
        self.assertEqual(len(program.resources), 0)
        self.assertEqual(len(program.events), 0)

    # Interface tests
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

    def test_interface_mixed_with_other_blocks(self):
        lines = [
            "interface Test",
            "inputs i",
            "outputs o",
            "transform T1",
            "input x",
            "output y"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.interfaces), 1)
        self.assertEqual(len(program.transforms), 1)
        self.assertEqual(program.interfaces[0].name, "Test")
        self.assertEqual(program.transforms[0].name, "T1")

    # Transform tests
    def test_single_transform(self):
        lines = [
            "transform MyTransform",
            "input a b",
            "output c d"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.transforms), 1)
        transform = program.transforms[0]
        self.assertEqual(transform.name, "MyTransform")
        self.assertEqual(transform.inputs, ["a", "b"])
        self.assertEqual(transform.outputs, ["c", "d"])

    def test_transform_with_no_inputs_or_outputs(self):
        lines = [
            "transform EmptyTransform",
            "input",
            "output"
        ]
        program = parse_program(lines)
        transform = program.transforms[0]
        self.assertEqual(transform.name, "EmptyTransform")
        self.assertEqual(transform.inputs, [])
        self.assertEqual(transform.outputs, [])

    def test_transform_with_extra_whitespace(self):
        lines = [
            "  transform   MyTransform  ",
            "  input   a   b  ",
            "  output   c   d  "
        ]
        program = parse_program(lines)
        transform = program.transforms[0]
        self.assertEqual(transform.name, "MyTransform")
        self.assertEqual(transform.inputs, ["a", "b"])
        self.assertEqual(transform.outputs, ["c", "d"])

    # Pipeline tests
    def test_single_pipeline(self):
        lines = [
            "pipeline MyPipeline",
            "stage1 stage2 stage3"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.pipelines), 1)
        pipeline = program.pipelines[0]
        self.assertEqual(pipeline.name, "MyPipeline")
        self.assertEqual(pipeline.stages, ["stage1", "stage2", "stage3"])

    def test_pipeline_with_single_stage(self):
        lines = [
            "pipeline SingleStagePipeline",
            "stage1"
        ]
        program = parse_program(lines)
        pipeline = program.pipelines[0]
        self.assertEqual(pipeline.name, "SingleStagePipeline")
        self.assertEqual(pipeline.stages, ["stage1"])

    def test_pipeline_with_extra_whitespace(self):
        lines = [
            "  pipeline   MyPipeline  ",
            "  stage1   stage2   stage3  "
        ]
        program = parse_program(lines)
        pipeline = program.pipelines[0]
        self.assertEqual(pipeline.name, "MyPipeline")
        self.assertEqual(pipeline.stages, ["stage1", "stage2", "stage3"])

    # State tests
    def test_single_state(self):
        lines = [
            "state MyState",
            "field1 field2 field3"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.states), 1)
        state = program.states[0]
        self.assertEqual(state.name, "MyState")
        self.assertEqual(state.fields, ["field1", "field2", "field3"])

    def test_state_with_single_field(self):
        lines = [
            "state SingleFieldState",
            "field1"
        ]
        program = parse_program(lines)
        state = program.states[0]
        self.assertEqual(state.name, "SingleFieldState")
        self.assertEqual(state.fields, ["field1"])

    def test_state_with_extra_whitespace(self):
        lines = [
            "  state   MyState  ",
            "  field1   field2   field3  "
        ]
        program = parse_program(lines)
        state = program.states[0]
        self.assertEqual(state.name, "MyState")
        self.assertEqual(state.fields, ["field1", "field2", "field3"])

    # Resource tests
    def test_single_resource(self):
        lines = [
            "resource MyResource type1"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.resources), 1)
        resource = program.resources[0]
        self.assertEqual(resource.name, "MyResource")
        self.assertEqual(resource.type, "type1")

    def test_resource_with_extra_whitespace(self):
        lines = [
            "  resource   MyResource   type1  "
        ]
        program = parse_program(lines)
        resource = program.resources[0]
        self.assertEqual(resource.name, "MyResource")
        self.assertEqual(resource.type, "type1")

    # Event tests
    def test_single_event(self):
        lines = [
            "event MyEvent",
            "field1 field2 field3"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.events), 1)
        event = program.events[0]
        self.assertEqual(event.name, "MyEvent")
        self.assertEqual(event.fields, ["field1", "field2", "field3"])

    def test_event_with_single_field(self):
        lines = [
            "event SingleFieldEvent",
            "field1"
        ]
        program = parse_program(lines)
        event = program.events[0]
        self.assertEqual(event.name, "SingleFieldEvent")
        self.assertEqual(event.fields, ["field1"])

    def test_event_with_extra_whitespace(self):
        lines = [
            "  event   MyEvent  ",
            "  field1   field2   field3  "
        ]
        program = parse_program(lines)
        event = program.events[0]
        self.assertEqual(event.name, "MyEvent")
        self.assertEqual(event.fields, ["field1", "field2", "field3"])

    # Mixed block tests
    def test_all_block_types_together(self):
        lines = [
            "interface Int1",
            "inputs a",
            "outputs b",
            "transform T1",
            "input x",
            "output y",
            "pipeline P1",
            "stage1 stage2",
            "state S1",
            "field1 field2",
            "resource R1 type1",
            "event E1",
            "field3 field4"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.interfaces), 1)
        self.assertEqual(len(program.transforms), 1)
        self.assertEqual(len(program.pipelines), 1)
        self.assertEqual(len(program.states), 1)
        self.assertEqual(len(program.resources), 1)
        self.assertEqual(len(program.events), 1)

    def test_interleaved_blocks(self):
        lines = [
            "interface Int1",
            "inputs a",
            "outputs b",
            "transform T1",
            "input x",
            "output y",
            "interface Int2",
            "inputs c",
            "outputs d"
        ]
        program = parse_program(lines)
        self.assertEqual(len(program.interfaces), 2)
        self.assertEqual(len(program.transforms), 1)
        self.assertEqual(program.interfaces[0].name, "Int1")
        self.assertEqual(program.interfaces[1].name, "Int2")
        self.assertEqual(program.transforms[0].name, "T1")

    # Error handling tests
    def test_incomplete_interface_block_raises_index_error(self):
        lines = [
            "interface Incomplete"
        ]
        with self.assertRaises(IndexError):
            parse_program(lines)

    def test_incomplete_transform_block_raises_index_error(self):
        lines = [
            "transform Incomplete"
        ]
        with self.assertRaises(IndexError):
            parse_program(lines)

    def test_incomplete_pipeline_block_raises_index_error(self):
        lines = [
            "pipeline Incomplete"
        ]
        with self.assertRaises(IndexError):
            parse_program(lines)

    def test_incomplete_state_block_raises_index_error(self):
        lines = [
            "state Incomplete"
        ]
        with self.assertRaises(IndexError):
            parse_program(lines)

    def test_incomplete_event_block_raises_index_error(self):
        lines = [
            "event Incomplete"
        ]
        with self.assertRaises(IndexError):
            parse_program(lines)

    def test_resource_with_missing_type_raises_index_error(self):
        lines = [
            "resource Incomplete"
        ]
        with self.assertRaises(IndexError):
            parse_program(lines)

if __name__ == '__main__':
    unittest.main()
