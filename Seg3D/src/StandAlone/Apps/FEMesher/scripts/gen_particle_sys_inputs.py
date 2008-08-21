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


#read the settings file for this system.
settings_file = "bm3d_settings.py"
settings = {}
if os.path.exists(settings_file) :
    execfile(settings_file, globals(), settings)
else :
    print "Could not find settings file."
    sys.exit(3)

#set the values from the settings.
mats = settings['mats']
mat_names = settings['mat_names']
mat_radii = settings['mat_radii']
base = settings['base']
vpbase = settings['vpbase']
v2n = settings['v2n']
jobs = settings['jobs']
inv_pad_idx = settings['inv_pad_idx']
resamp_scale = settings['resamp_scale']

import sr_py

#init the scirun python package.
env = []
for k in os.environ.keys() :
    estr = "%s=%s" % (k, os.environ[k])
    env.append(estr)
sr_py.init_sr_py(env)




maxval_re = re.compile("max: ([\d\.\d]+)")
axsizes_re = re.compile("sizes: (\d+) (\d+) (\d+)")
#space directions: (-0.671875,0,-0) (0,0.671875,0) (-0,0,5)
tri = "\(([\-\d\.]+),([\-\d\.]+),([\-\d\.]+)\)\s?"
spc_re = re.compile("space directions: %s%s%s" % (tri, tri, tri))

maxval = 0
axsizes = ()
spacing = None
print_only = False


def do_system(cmmd) :
    if print_only :
        print cmmd
    else :
        os.system(cmmd)



def inspect_nrrd(innrrd) :
    global axsizes
    global spacing
    global maxval
    cmmd = "unu minmax %s" % innrrd
    t = popen2.popen2(cmmd)
    for l in t[0].readlines() :
        mo = maxval_re.match(l)
        if mo != None :
            maxval = int(mo.group(1))

    cmmd = "unu head %s" % innrrd
    print cmmd
    t = popen2.popen2(cmmd)
    for l in t[0].readlines() :
        mo = axsizes_re.match(l)
        if mo != None :
            axsizes = (int(mo.group(1)), int(mo.group(2)), int(mo.group(3)))
        mo1 = spc_re.match(l)
        if mo1 != None :
            spacing = ((fabs(float(mo1.group(1))),
                        fabs(float(mo1.group(2))),
                        fabs(float(mo1.group(3)))),
                       (fabs(float(mo1.group(4))),
                        fabs(float(mo1.group(5))),
                        fabs(float(mo1.group(6)))),
                       (fabs(float(mo1.group(7))),
                        fabs(float(mo1.group(8))),
                        fabs(float(mo1.group(9)))))

    print "done inspect"


def pad_nrrd(innrrd, idx, name) :

    # zero out the first and last slice in z
    # grab a slice
    val = 0
    if idx == inv_pad_idx :
        val = 1
    oname = "%s.pad.nrrd" % name
    cmmd = "unu pad -min -4 -4 -4 -max M+4 M+4 M+4 "\
           "-b pad -v %d -i %s -o %s" % (val, innrrd, oname)

    print cmmd
    do_system(cmmd)
    return oname

def make_lut (idx, name) :
    global maxval
    
    if idx == -1 : return #done by make_others_lut
    
    hit_str = "1.0\n"
    miss_str = "0.0\n"
        
    f = open("%s.lut.raw" % name, "w")
    flist = []
    
    for i in range(0, maxval + 1) :
        if i == idx :
            flist.append(hit_str)
        else :
            flist.append(miss_str)
    f.writelines(flist)
    f.close()

    cmmd = "unu make -i %s.lut.raw -t float -s %d -e ascii "\
           "-o %s.lut.nrrd" % (name, maxval+1, name)
    print cmmd
    do_system(cmmd)

def make_others_lut (mats, name) :
    global maxval

    print name
    f = open("%s.lut.raw" % name, "w")
    flist = []

    print maxval
    for i in range(0, maxval + 1) :
        found = False
        for m in mats :
            if i == m :
                flist.append("0.0\n")
                found = True
                break
        if not found :
            flist.append("1.0\n")


    f.writelines(flist)
    f.close()

    cmmd = "unu make -i %s.lut.raw -t float -s %d -e ascii "\
           "-o %s.lut.nrrd" % (name, maxval+1, name)
    print cmmd
    do_system(cmmd)



def resample_nrrd(inrrd, name) :
    global spacing
    x = 1.0
    y = 1.0
    z = 1.0
    
    if resamp_scale != None :
        x = resamp_scale[0]
        y = resamp_scale[1]
        z = resamp_scale[2] 

    oname = "%s.resamp.nrrd" % name
    # create a uniformly sampled nrrd
    cmmd = "unu resample -s x%f x%f x%f -k cubic:1,0 "\
           "-i %s -o %s" % \
           (x, y, z, inrrd, oname)
    print cmmd
    do_system(cmmd)
    return oname

def make_vol(inrrd) :

    oname = "%svol" % inrrd[:-4]
    cmmd = base + "/nrrd2vol %s" % inrrd
    print cmmd
    do_system(cmmd) 
    return oname

def make_solo_material(idx, innrrd, name) :
    global maxval
    make_lut(idx, name)
    oname = "%s.solo.nrrd" % name
    cmmd = "unu lut -m %s.lut.nrrd -min 0 -max %d -t float -i %s"\
           " -o %s" % (name, maxval, innrrd, oname)
    print cmmd
    do_system(cmmd)
    return oname

def make_unified_other_material(mats, innrrd) :
    #make 0 material a non-zero material
    cmmd = "unu 2op if combined.nrrd 255 -o combined.nozeros.nrrd"


def make_trisurf(idx) :
    exe = "/scratch/mjc/branch/SCIRun/dbg/StandAlone/convert/MtoTriSurfField"
    cmmd = "%s mat%dmesh.m mat%d.ts.fld" % (exe, idx, idx)
    print cmmd
    do_system(cmmd)

def norm_values(idx) :
    cmmd = "unu 2op + 1 mat%d.resamp.tight.nrrd -o - | unu 2op x 0.5 - -o mat%d.resamp.tight.nrrd" % (idx, idx)
    print cmmd
    do_system(cmmd)

param_lines = []

def add_param_info(inrrd) :
    global spacing
    x = 1.0
    y = 1.0
    z = 1.0
##     if spacing != None :
##         print spacing
##         x = spacing[0][0]
##         y = spacing[1][1]
##         z = spacing[2][2]
    ivol = "%svol" % inrrd[:-4]
    param_lines.append('%f %f %f %s\n' % (x, y, z, ivol))

def parallel_cmmds(cmmds) :
    procs = []
    for c in cmmds :
        if print_only :
            print c
        else :
            procs.append(popen2.Popen4(c))
    return procs
        
def wait_on_procs(procs) :    

    # the output from processes need to be dumped or the
    # buffer fills up and it sleeps.
    for p in procs :
        print "starting thread to dump"
        print p
        start_new_thread(dump_log, (p, ))
        

    #once all tight procs are done, make the vols    
    done = print_only
    count = 0;
    while not done :
        time.sleep(1)
        ndone = 0
        #all tight procs need to be finished before moving on.
        np = 0
        for p in procs :
            if p.poll() != -1 : #-1 means process not finished.
                ndone = ndone + 1

        if ndone == len(procs) :
            done = True
        else :
            if count % 30 == 0 :
                print "%d processes are complete of %d" % (ndone, len(procs))
        count = count + 1

        
def make_MC_cmmd(idx, name) :
    cmmd = "%s/marchingcubes medial_axis_param_file.txt A 0.0 %d %s" % (base,
                                                                        idx,
                                                                        name)
    print cmmd
    return cmmd

# return the cmmd to run only.
def make_tight_cmmd(innrd, name, r) :
    oname = "%s.tight.nrrd" % name
    cmmd = vpbase + "/tighten/morphsmooth %s %s %s" % (r, innrd, oname)
    return oname, cmmd

# launch a process and return the Popen4 object to poll
def make_tight(innrd, name, r) :
    oname, cmmd = make_tight_cmmd(innrd, name, r)
    
    if print_only :
        print cmmd
    else :
        return oname, popen2.Popen4(cmmd)


def sanity_check_solos(mats) :
    cmmd = "unu 2op + %s.solo.nrrd %s.solo.nrrd -o -" % (mats[0], mats[1])
    for m in mats[2:] :
        cmmd = cmmd + "| unu 2op + - %s.solo.nrrd -o -" % m
        
    cmmd = cmmd + "| unu minmax -"
    do_system(cmmd)

def dump_log(p) :
    if print_only :
        return
    lst = []
    inputs = []

    for l in p.fromchild :
        lst.append(l)
    f = open("%d.log" % p.pid, "w")
    f.writelines(lst)
    f.close()

    
def make_medial_axis_points(name, lv, rf, iso, gma) :
    get_cmap = sr_py.get_create_standard_colormaps_alg()
    cmap = get_cmap.execute()
    surf, geom, mat = iso.execute(lv, cmap, 0)
    g, t, v = gma.execute(surf)

    
def timestamp(s) :
    cmmd = "date > %s.timestamp" % s
    do_system(cmmd)


def create_sizing_field_cmmd(ma_par, ma_base, indicator) :
    cmmd  = base + '/sizingfield %s %s %d' % (ma_par, ma_base, indicator)
    return cmmd

    
def create_sizing_field(ma_par, ma_base, indicator) :
    cmmd = create_sizing_field_cmmd(ma_par, ma_base, indicator)
    
    if print_only :
        print cmmd
    else :
        return popen2.Popen4(cmmd)
    
if __name__ == "__main__" :
    timestamp("start")
    procs = []
    nrrd = argv[1]
    inspect_nrrd(nrrd)

    #create the lut for remaining materials special
    count = 0
    others_name = ''
    for i in mats :
        others_name = mat_names[count]
        count = count + 1
        if i == -1 :
            break
    make_others_lut (mats, others_name)
    
    idx = 0
    for i in mats :

        n = mat_names[idx]
        r = mat_radii
        print "Working on material: %s" % n
        idx = idx + 1
        cur = make_solo_material(i, nrrd, n)
        print "cur is : %s" % cur
        cur = resample_nrrd(cur, n)
        cur = pad_nrrd(cur, i, n)
        print "cur is : %s" % cur
        cur, p = make_tight(cur, n, r)
        procs.append(p)
        add_param_info(cur)

    #write param file
    nmats = len(param_lines)
    print nmats
    lns = []
    lns.append("%d\n" % nmats)
    for l in param_lines :
        lns.append(l)
    f = open('medial_axis_param_file.txt', 'w')
    f.writelines(lns)
    f.close()

    timestamp("start-tight")
    wait_on_procs(procs)

    procs = []
    for n in mat_names :
        tn = "%s.tight.nrrd" % n 
        make_vol(tn)


    rn = sr_py.get_read_nrrd_alg()
    cnf = sr_py.get_convert_nrrd_to_field_alg()
    rf = sr_py.get_read_field_alg()
    iso = sr_py.get_extract_isosurface_alg()

    iso.set_p_np(jobs)
    iso.set_p_build_geom(0)
    iso.set_p_build_trisurf(1)
    gma = sr_py.get_gen_ray_cast_m_a_alg()
    gma.set_p_save_ptcl_file(1)
    gma.set_p_num_threads(jobs)
  
    timestamp("start-ma")
    idx = 0    
    for i in mats :
        n = mat_names[idx]
        gma.set_p_ptcl_filename('%s_ma.ptcl' % n)
        #convert the nrrd to a latvol.
        vol = rn.execute('%s.tight.nrrd' % n)
        lv = cnf.execute(vol[0])
        make_medial_axis_points(n, lv, rf, iso, gma)
        idx = idx + 1


    timestamp("start-sf")
    idx = 0    
    for i in mats :
        n = mat_names[idx]
        p = create_sizing_field('medial_axis_param_file.txt', n, idx)
        procs.append(p)
        idx = idx + 1
    
    wait_on_procs(procs)
    timestamp("done")

    sr_py.terminate()
