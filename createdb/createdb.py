#!/usr/bin/python3

with open('INFO.TXT', 'w') as info, open('STATUS.TXT','w') as status, open('DB.TXT', 'w') as db :
    info.truncate()
    status.truncate()
    db.truncate()
    for rcid in range(0, 1024):
        db.write("000 000 000 000 000 000\n") # "000 59F 000 59F 07F 000
        info.write("000 000 000 000 000\n") # "000 59F 000 59F 07F
        status.write("000\n")
