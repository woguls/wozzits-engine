# Wozzits Engine Rules

## 1. Architecture Layers

- api/      → public headers only (no implementation)
- core/     → low-level primitives (queues, utilities)
- src/      → engine systems (logging, threading, async, window)
- platform/ → OS-specific implementation

## 2. Include Rule

Public code MUST only include:
    <wozzits/...>

Never include:
    core/*
    src/*
    platform/*

## 3. Ownership Rule

Each system has exactly ONE owner:
- threading → src/threading
- logging   → src/logging
- window    → src/window

No duplication of implementations across layers.

## 4. Global State Rule

Global static variables are forbidden except:
- platform internal implementation
- explicitly marked internal namespaces

All engine state must live in EngineContext or subsystem state.

## 5. Threading Model

- MPSC queue: multi-producer, single-consumer
- SPSC queue: single-producer, single-consumer
- Logging uses dedicated worker thread
- Async system uses job queues (no direct thread spawning from API)

## 6. Naming

Namespace root: wz

Allowed:
- wz::core
- wz::logging
- wz::threading
- wz::platform::win32

Never:
- wz::src
- wz::api