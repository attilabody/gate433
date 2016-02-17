#!/usr/bin/python3


def reversecode( code ):
    reversed = 0
    for bit in range(0, 10):
        reversed <<= 1
        if code & 1:
            reversed |= 1
        code >>= 1
    return reversed

def convert( ifname, ofname ):
    with open(ifname, 'r') as infile, open(ofname, 'w') as outfile:
        for line in infile:
            sline = line.strip()
            try:
                code = int(sline)
                rc = reversecode(code)
                print(rc)
                outfile.write(str(rc))
                outfile.write('\n')
            except ValueError:
                print(line)
                outfile.write(line)

convert( 'owners_old.txt', 'owners.txt')
convert( 'tenants_old.txt', 'tenants.txt')