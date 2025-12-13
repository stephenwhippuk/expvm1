#!/usr/bin/env python3
"""
Fix bracket syntax in assembly test files.

Converts:
  LD [label] → LDA label
  LD [label + offset] → LDA (label + offset)
  LDAB [label + reg] → LDAB (label + reg)
  LDAW [label] → LDAW label
  etc.
"""

import re
import sys
import os

def fix_bracket_syntax(content):
    """Fix bracket syntax in assembly code."""
    
    # Pattern 1: LD/LDAB/LDAW dest, [simple_label]
    # Replace with: LDA/LDAB/LDAW dest, simple_label
    content = re.sub(
        r'\b(LDA?W?)\s+([A-Z]{2}),\s*\[([a-zA-Z_][a-zA-Z0-9_]*)\]',
        r'LDA\2 \2, \3',
        content
    )
    
    # Pattern 2: LD/LDAB/LDAW dest, [expr with + or -]
    # Replace with: LDA/LDAB/LDAW dest, (expr)
    content = re.sub(
        r'\b(LDA?W?B?)\s+([A-Z]{2}),\s*\[([^\]]+[+\-][^\]]+)\]',
        r'LDA\1 \2, (\3)',
        content
    )
    
    # Pattern 3: LD/LDAB/LDAW [dest], source  (storing to memory)
    # Replace with: LDA/LDAB/LDAW dest, source
    content = re.sub(
        r'\b(LDA?W?B?)\s+\[([a-zA-Z_][a-zA-Z0-9_]*)\],\s*([A-Z]{2})',
        r'LDA\1 \2, \3',
        content
    )
    
    # Pattern 4: LD/LDAB/LDAW [expr], source  (storing to memory with expression)
    # Replace with: LDA/LDAB/LDAW (expr), source
    content = re.sub(
        r'\b(LDA?W?B?)\s+\[([^\]]+[+\-][^\]]+)\],\s*([A-Z]{2})',
        r'LDA\1 (\2), \3',
        content
    )
    
    return content

def main():
    if len(sys.argv) < 2:
        print("Usage: fix_brackets.py <file1.asm> [file2.asm ...]")
        sys.exit(1)
    
    for filepath in sys.argv[1:]:
        if not os.path.exists(filepath):
            print(f"Warning: {filepath} not found, skipping")
            continue
        
        with open(filepath, 'r') as f:
            content = f.read()
        
        original = content
        fixed = fix_bracket_syntax(content)
        
        if fixed != original:
            with open(filepath, 'w') as f:
                f.write(fixed)
            print(f"Fixed: {filepath}")
        else:
            print(f"No changes: {filepath}")

if __name__ == '__main__':
    main()
