#!/usr/bin/python3
OWNERS='owners.txt'
TENANTS='tenants.txt'

tenant_in_start = 7 * 60 + 30
tenant_out_start = 7 * 60 + 30
tenant_in_end = 17 * 60 + 30
tenant_out_end = 18 * 60 + 5

active = {}
with open(OWNERS, 'r') as infile:
    for line in infile:
        sline = line.strip()
        if len(sline) > 0 and not sline.startswith('#'):
            # print( len( sline), ' ', sline)
            active[int(sline)] = 1

with open(TENANTS, 'r') as infile:
    for line in infile:
        sline = line.strip()
        if len(sline) > 0 and not sline.startswith('#'):
            # print( len( sline), ' ', sline)
            active[int(sline)] = 2

with open('INFO.TXT', 'w') as info, open('STATUS.TXT', 'w') as status, open('DB.TXT', 'w') as dbfile :
    statusline = '000\n'
    for id in range(0, 1024):
        if id in active:
            if active[id] == 1:
                dbline = '000 FFF 000 FFF 07F 000\n'
                infoline = '000 FFF 000 FFF 07F\n'
            else:
                dbline = "%03X %03X %03X %03X 01F 000\n" % \
                         (tenant_in_start, tenant_in_end, tenant_out_start, tenant_out_end)
                infoline = "%03X %03X %03X %03X 01F\n" % \
                         (tenant_in_start, tenant_in_end, tenant_out_start, tenant_out_end)
        else:
            dbline = '000 000 000 000 000 000\n'
            infoline = '000 000 000 000 000\n'
        dbfile.write(dbline)
        info.write(infoline)
        status.write(statusline)
