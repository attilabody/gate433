#!/usr/bin/python3
tenant_in_start = int(7.5*60)
tenant_out_start = int(7.5*60)
tenant_in_end = int(18*60)
tenant_out_end = int(18.5*60)

active = {}
with open('owners.txt', 'r') as infile:
    for line in infile:
        sline = line.strip()
        if( len(sline) > 0 and not sline.startswith('#') ):
#            print( len( sline), ' ', sline)
            active[int(sline)] = 1

with open('tenants.txt', 'r' ) as infile:
    for line in infile:
        sline = line.strip()
        if( len(sline) > 0 and not sline.startswith('#') ):
#            print( len( sline), ' ', sline)
            active[int(sline)] = 2

with open( 'DB.TXT', 'w' ) as dbfile:
    for id in range(0,1024):
        if id in active:
            if active[id] == 1:
                dbline = '000 FFF 000 FFF 07F 000\n'
            else:
                dbline = "%03X %03X %03X %03X 01F 000\n" % \
                         (tenant_in_start, tenant_in_end, tenant_out_start, tenant_out_end)
        else:
            dbline = '000 000 000 000 000 000\n'
        print( dbline, end='' )
        dbfile.write( dbline )

