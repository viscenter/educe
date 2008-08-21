#  
#  For more information, please see: http://software.sci.utah.edu
#  
#  The MIT License
#  
#  Copyright (c) 2004 Scientific Computing and Imaging Institute,
#  University of Utah.
#  
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#  
#    Author : Martin Cole

from sys import argv
import sys
import popen2
import re
import os
import time
import glob
from thread import start_new_thread
from math import fabs

print_only = False

def do_system(cmmd) :
    if print_only :
        print cmmd
    else :
        os.system(cmmd)


def sanity_check_solos(mats) :
    cmmd = "unu 2op + %s.solo.nrrd %s.solo.nrrd -o -" % (mats[0], mats[1])
    for m in mats[2:] :
        cmmd = cmmd + "| unu 2op + - %s.solo.nrrd -o -" % m
        
    cmmd = cmmd + "| unu minmax -"
    do_system(cmmd)
    
    
if __name__ == "__main__" :
    global mats
    global mat_names
    
    settings_file = "bm3d_settings.py"
    settings = {}
    if os.path.exists(settings_file) :
        execfile(settings_file, globals(), settings)
    else :
        print "Could not find settings file."
        sys.exit(3)
        
    mat_names = settings['mat_names']
    mats = settings['mats']
        
    procs = []
    njunctions = int(argv[1])

    for j in range(0, njunctions) :

        dnm = "junctions%d" % j
        if j == 0 :
            dnm = "junctions"

        cwd = os.getcwd()
        if not os.path.exists(dnm) :
            os.mkdir(dnm)
        else :
            continue
        idx = 0
        lns = ["%d\n" % len(mats)]
        for i in mats :
            n = mat_names[idx]
            print n
            lns.append("1.0 1.0 1.0 %s/%s\n" % (cwd, n))
            cmmd = "ln -s %s.tight.vol %s.vol" % (n, n)
            do_system(cmmd)
            idx = idx + 1

        f = open('%s/particle_params.txt' % dnm, 'w')
        f.writelines(lns)
        f.close()


        psys_txt = [
            "ENERGY                  radial\n",
            "NUMBER_OF_SURFACES      %d\n" % len(mats),
            "NUMBER_OF_INTERSECTIONS %d\n" % 1,
            "INTERSECTION_FILE       %s/%s/m1.txt\n" % (cwd, dnm),
            "BASE_FILE_NAME          %s/%s/particle_params.txt\n" % (cwd,dnm),
            "INIT_NUM_POINTS         5\n", #unused
            "MAX_SF                  %f\n" % 10.0,
            "SIZING_SCALE            4.0\n"
            ]

        f = open('%s/psystem_input.txt' % dnm, 'w')
        f.writelines(psys_txt)
        f.close()

