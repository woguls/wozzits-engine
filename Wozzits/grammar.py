from dataclasses import dataclass, field

@dataclass
class Interface:
    name: str
    inputs: list[str]
    outputs: list[str]

@dataclass
class Transform:
    name: str
    inputs: list[str]
    outputs: list[str]

@dataclass
class Pipeline:
    name: str
    stages: list[str]

@dataclass
class State:
    name: str
    fields: list[str]

@dataclass
class Resource:
    name: str
    type: str

@dataclass
class Event:
    name: str
    fields: list[str]

@dataclass
class Program:
    interfaces: list[Interface] = field(default_factory=list)
    transforms: list[Transform] = field(default_factory=list)
    pipelines: list[Pipeline] = field(default_factory=list)
    states: list[State] = field(default_factory=list)
    resources: list[Resource] = field(default_factory=list)
    events: list[Event] = field(default_factory=list)