#!/usr/bin/python3
tenant_in_start = 7 * 60 + 30
tenant_out_start = 7 * 60 + 30
tenant_in_end = 17 * 60 + 30
tenant_out_end = 18 * 60 + 5

codecount = 0
active = {}
with open('owners.txt', 'r') as infile:
    for line in infile:
        sline = line.strip()
        if len(sline) > 0 and not sline.startswith('#'):
            # print( len( sline), ' ', sline)
            active[int(sline)] = 1
            codecount = codecount + 1

with open('tenants.txt', 'r') as infile:
    for line in infile:
        sline = line.strip()
        if len(sline) > 0 and not sline.startswith('#'):
            # print( len( sline), ' ', sline)
            active[int(sline)] = 2
            codecount = codecount + 1

with open('codes.cpp', 'w') as cppf, open('codes.h', 'w') as hf :
    cppf.write('#include <Arduino.h>\n\nuint8_t codes[')
    cppf.write(str(codecount))
    cppf.write('] = {')
    first = True
    cnt = 0
    for id in range(0, 1024):
        if id in active:
            if cnt >= 16:
                cppf.write('\n')
                cnt = 0
            if first == True:
                cppf.write('\n\t  ')
                first = False
            else:
                if cnt == 0:
                    cppf.write('\t, ')
                else:
                    cppf.write(', ')
            cppf.write( str(id) )
            cnt = cnt + 1
    cppf.write('\n};\n')

    hf.write('#include <Arduino.h>\n\nextern uint8_t codes[')
    hf.write(str(codecount))
    hf.write('];\n')
