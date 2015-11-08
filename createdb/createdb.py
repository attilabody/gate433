#!/usr/bin/python3

with open('info.txt', 'w') as info, open('status.txt','w') as status:
    info.truncate()
    status.truncate()
    for rcid in range(0, 1024):
        info.write("000 000 000 000 000\n") # "000 59F 000 59F 07F
        status.write("000\n")
