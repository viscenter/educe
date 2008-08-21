import os
from sys import argv
import glob

print_only = False

def do_system(cmmd) :
    if print_only :
        print cmmd
    else :
        print cmmd
        os.system(cmmd)

settings_file = "bm3d_settings.py"
settings = {}
if os.path.exists(settings_file) :
    execfile(settings_file, globals(), settings)
else :
    print "Could not find settings file."
    sys.exit(3)

mats = settings['mats']
mat_names = settings['mat_names']
base = settings['base']
tg = settings['tg']
m2ts = settings['m2ts']

def make_nodes_from_dir(d) :
    if os.path.exists('%s/all.pts' % d) :
        cmmd = 'rm %s/all.pts' % d
        do_system(cmmd)
        
    cmmd = 'cat %s/*.pts > %s/all.pts' % (d,d)
    do_system(cmmd)

    cmmd = '%s/pts2node %s/all.pts %s/all.node' % (base, d, d)
    do_system(cmmd)


# must be called after combined is created.
def make_surfs(d) :

    i = 0
    for nm in mat_names :
        cmmd = "%s/removetets %s/all.1 "\
               "medial_axis_param_file.txt "\
               "%s/%s.m %d" % (base, d, d, nm, i)
        do_system(cmmd)

        cmmd = '%s %s/%s.m %s/%s.ts.fld' % (m2ts, d, nm, d, nm)
        do_system(cmmd)
        i = i + 1
    

def make_combined_surf(d) :
    cmmd = '%s %s/all.node' % (tg, d)
    do_system(cmmd)
    
    cmmd = "%s/removetetsall %s/all.1 "\
           "medial_axis_param_file.txt %s/surf.m 0" % (base, d, d)
    do_system(cmmd)

    cmmd = '%s %s/surf.m %s/surf.ts.fld' % (m2ts, d, d)
    do_system(cmmd)
    
if __name__ == '__main__' :
    dir_name = argv[1]
    make_nodes_from_dir(dir_name) 
    make_combined_surf(dir_name) 
    make_surfs(dir_name) 
