import os
import sys


# -1 means all the rest of the materials combined
mats = (0, 4, 5, 7, 8)
mat_names = ('air', 'grey', 'fluid', 'lesion', 'white')
mat_radii = 0.8
inv_pad_idx = 0
resamp_scale = (1.0, 1.0, 1.0)

#paths
#base = '/home/sci/mjc/meshing/m-bin/MultiSurfaces'
#vpbase = '/home/sci/mjc/meshing/vp-bin/apps'
base = '/scratch/mjc/miriah/repo/opt/MultiSurfaces'
vpbase = '/scratch/mjc/miriah/repo/vpbin/apps'
v2n = '%s/distance/vol2nrrd' % vpbase

#srbase = '/home/sci/mjc/SCIRun/hex-opt'
srbase = '/scratch/mjc/cibc/SCIRun/opt'
v2n = '%s/distance/vol2nrrd' % vpbase
tg = '/scratch/mjc/tetgen1.4.2/tetgen'
m2ts = '%s/convert/MtoTriSurfField' % srbase
jobs = 8

#setup SCIRun/python environment
sys.path.append('%s/lib' % srbase)
os.environ['SCIRUN_ON_THE_FLY_LIBS_DIR'] = '%s/on-the-fly-libs' % srbase
