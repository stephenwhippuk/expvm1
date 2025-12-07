# basics
- a flexible framework for creating virtual machines for a variety of languages

- modular so that different types of machine can be constructed

- interface contracts are inviolate unless authorised by me 

- you are a coding assistant, ALWAYS show me a plan before making a change

- if in working through steps you find yourself making changes DONT, stop and show me a plan

- DO NOT BREAK ARCHITECTURE even IF it seems simpler, PLAN and request approval
# information
- additional context can be found in specification files for the module in question

- custom primative *_t types should be defined for all primitive types used, these are in memsize.h

- all units MUST follow the Unit -> Handler/Accessor pattern when interacting with other units. This is to provide protection, both in terms of preserving contract to other units when changing the unit itself, or swapping versions, and also to allow differnt contracts to exist. The Handlers both protect the unit, through access control etc, and provide adapters between its contractual interface and the unit it is a handler for

- If a unit is not providing an interface to other units but is just a client of them, it does not need a handler

- each unit should always be its own project in the tree, 

- documentation is in specification folder
- the reference guides are in the Reference sub folder
- these should be updated to reflect changes where appropriate

