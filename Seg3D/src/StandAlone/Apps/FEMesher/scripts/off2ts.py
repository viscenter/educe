import sys
import re

fnum = '([0-9\-\.eE\+]+)'
pnts_re = re.compile('\s*%s\s+%s\s+%s' % (fnum, fnum, fnum))
hdr_re = re.compile('\s*OFF\s+(\d+)\s+(\d+)\s+(\d+)')
tri_re = re.compile('\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+)')

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
    ntri = int(mo.group(2))
    
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
            pnts_list.append('%f %f %f 0.0 0.0 0.0 0.0\n' % (x,y,z))
        lncnt = lncnt + 1

    fp = open("%s.pts" % infile, 'w')
    fp.writelines(pnts_list)
    fp.close()

    pnts_list = ["%d\n" % int(mo.group(1))]
    tri_list = ["%d\n" % ntri]
    for ln in ln_list[npnts+1:] :
        mo = tri_re.match(ln)
        if mo == None :
            print "bad line at %d" % lncnt
        else :
            if int(mo.group(1)) != 3 :
                print "expected triangle at %d" % lncnt
            else :
                i0 = int(mo.group(2))
                i1 = int(mo.group(3))
                i2 = int(mo.group(4))   
                tri_list.append("%d %d %d\n" % (i0, i1, i2))
        lncnt = lncnt + 1

    fp = open("%s.tri" % infile, 'w')
    fp.writelines(tri_list)
    fp.close()
