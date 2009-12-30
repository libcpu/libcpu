# Generates an opcode decoder table from the table in opcode_id.tab
#
# Each line of the .tab file defines one bitmask and the corresponding
# instruction. 0/1 means that the bit at the given position must have
# that value, '-' means that the bit can have any value (eg. it's a
# register number or immediate data).
#
# This script uses that definition to generate an opcode table, and
# the function that tells you what a given m68k instruction is. You
# still need more code to actually decode the operand data and stuff.

import sys
import os.path

def precision_from_opcode(opcode):
    value, mask = opcode[1]
    return (mask << 16) | value

def value_from_desc(desc):
    return int(desc.replace('-', '0'), 2)

def mask_from_desc(desc):
    out = 0
    for x in desc:
        out <<= 1
        out |= (0 if x == '-' else 1)
    return out

def gen_opcode(line):
    desc, name = line.strip().split()
    return name, (value_from_desc(desc), mask_from_desc(desc))

def make_op_enum(opcodes):
    out = (['enum OpcodeID {'] +
           sorted('  %s,' % x[0] for x in opcodes) +
           ['};'])
    return '\n'.join(out)

def make_decoder_table(opcodes):
    out = ['static struct decoder {',
           '  OpcodeID name;',
           '  uint16_t value;',
           '  uint16_t mask;',
           '} decoder_table[] = {']
    for name, val_n_mask in opcodes:
        val, mask = val_n_mask
        out.append('  { %s, 0x%x, 0x%x },' % (name, val, mask))
    out.append('};')
    return '\n'.join(out)

def make_include_file(opcodes):
    return '''
#ifndef OPCODE_ID_H_
#define OPCODE_ID_H_

#include <stdint.h>

%s

OpcodeID identify_opcode(uint16_t word);

#endif // OPCODE_ID_H_
'''.lstrip() % make_op_enum(opcodes)


def make_implementation_file(opcodes):
    return '''
#include "opcode_id.h"

namespace {
%s
}  // namespace

OpcodeID identify_opcode(uint16_t word) {
  struct decoder* p = decoder_table;
  for (struct decoder* p = decoder_table; p->name != UNKNOWN; p++) {
    if ((word & p->mask) == p->value)
      return p->name;
  }
  return UNKNOWN;
}
'''.lstrip() % make_decoder_table(opcodes)


def main(args):
    print args
    with open(args[1]) as f:
        d = [gen_opcode(l) for l in f if not l.startswith('#')]
        d = sorted(d, key=lambda x: precision_from_opcode(x), reverse=True)

    with open(args[2], 'w') as f:
        f.write(make_include_file(d))

    with open(args[3], 'w') as f:
        f.write(make_implementation_file(d))

if __name__ == '__main__':
    main(sys.argv)
