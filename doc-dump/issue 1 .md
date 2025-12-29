# brief
we need facility to add pages into the data segment of the assembler that check against maximum sizes when assembling

# detail

this should allow the following format

```asm
DATA
PAGE myPage
    myVar: DW [1,2,3]
PAGE secondPage
    anotherVar: DW [3,4,5]
CODE
```

page is optional, if no page then page should just be set to 0 and left.

each page maps to a 16 bit number likewise each address withn a page is a 16 bit number

This will need optimising but for now the assembler should add in the page instruction before a memory location is read or written to when generating the binary.  