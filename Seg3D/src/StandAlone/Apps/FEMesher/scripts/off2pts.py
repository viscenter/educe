import sys
import re

fnum = '([0-9\-\.eE\+]+)'
pnts_re = re.compile('\s*%s\s+%s\s+%s' % (fnum, fnum, fnum))
hdr_re = re.compile('\s*OFF\s+(\d+)\s+(\d+)\s+(\d+)')


if __name__ == '__main__' :
    infile = sys.argv[1]
    fp = open(infile)
    ln_list = fp.readlines()
    fp.close()
    mo = hdr_re.match(ln_list[0])
    if mo == None :
        print "Bad header expected OFF file"
        exit(2)
    npnts = int(mo.group(1))
    pnts_list = ["%d\n" % int(mo.group(1))]
    lncnt = 1
    for ln in ln_list[1:npnts+1] :
        mo = pnts_re.match(ln)
        if mo == None :
            print "bad line at %d" % lncnt
        else :
            x = float(mo.group(1))
            y = float(mo.group(2))
            z = float(mo.group(3))
            #print "point: (%f, %f, %f)" % (x,y,z)
            pnts_list.append('%f %f %f\n' % (x,y,z))
        lncnt = lncnt + 1

    fp = open(sys.argv[2], 'w')
    fp.writelines(pnts_list)
    fp.close()
