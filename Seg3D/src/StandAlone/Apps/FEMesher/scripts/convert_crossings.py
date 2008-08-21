import os
import sys


def convert_crossings() :
    
    for m in mat_names :
        cmmd = "%s %s_crossing.vol" % (v2n, m)
        os.system(cmmd)
        cmmd = "mv out.nrrd %s_crossing.nrrd" % m
        os.system(cmmd)

if __name__ == '__main__' :
    global v2n
    global mat_names
    
    settings_file = "bm3d_settings.py"
    settings = {}
    if os.path.exists(settings_file) :
        execfile(settings_file, globals(), settings)
    else :
        print "Could not find settings file."
        sys.exit(3)

    mat_names = settings['mat_names']
    v2n = settings['v2n']
    
    convert_crossings()
