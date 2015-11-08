#!/usr/bin/python3

with open('db.txt','r') as input, open('info.txt', 'w') as info:
    info.truncate()
    for inline in input:
        inline = inline.strip('\n')
        initems = inline.split(' ')
        flags = int(initems[-1])
        del initems[-1]
        outitems=[]
        for item in initems:
            outitems.append(item)
            outitems.append(' ')
        flags &= 0x7f
        outitems.append(format(flags, '03X'))
        outitems.append('\n')
        for item in outitems:
            info.write(item)


