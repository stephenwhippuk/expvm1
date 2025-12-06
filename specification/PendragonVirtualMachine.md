# Pendragon Virtual Machine: Technical Overview
## Purpose
Pendragon is a sandbox experimental virtual machine designed to serve as a foundational platform for the Arthurian family of virtual machines. Its primary goal is to provide a flexible, extensible environment for exploring virtual machine design, language implementation, and systems programming.

## Arthurian VM Family
Pendragon is the first member of the Arthurian VM family. This family is intended to provide a range of virtual machines with increasing capability and word size, serving as targets for programming language development and research. Future family members will offer higher bit-width architectures and more advanced features.

## Design Goals
- Language Target: The VM is intended as a backend for new and experimental programming languages, making it easy to target and test language features.
- Sandboxed Experimentation: Provides a safe, isolated environment for experimenting with VM internals, instruction sets, and runtime behaviors.
- Extensibility: Designed to be easily extended with new instructions, devices, and architectural features.

## Architecture
- 16-bit Core: Pendragon is a 16-bit architecture, with all registers and primary data paths operating on 16-bit words. Future family members will expand to 32-bit, 64-bit, and beyond.
- Register-Based: The VM uses a register-based design, featuring a small set of general-purpose registers for computation and data movement.
- Assembler and Tooling: Pendragon provides its own assembler language and assembler utility, enabling direct programming of the VM and rapid prototyping of new instructions.
Development Tooling: Includes tools for assembling, debugging, and analyzing programs targeting the VM.

## Input/Output and Device Support
Simple I/O: The initial implementation provides basic input/output capabilities for program interaction.
Virtual Devices: While Pendragon starts with simple I/O, the architecture is designed to support more robust virtual device models in future family members, enabling richer system emulation and device experimentation.

## Summary
Pendragon is a flexible, extensible, and well-documented virtual machine platform, ideal for language implementers, systems researchers, and anyone interested in virtual machine architecture. It is the foundation for a family of VMs that will grow in capability and complexity, supporting a wide range of experimental and production use cases.