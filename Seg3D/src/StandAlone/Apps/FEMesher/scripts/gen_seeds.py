import math, os
import time

#read the settings file for this system.
settings_file = "bm3d_settings.py"
settings = {}
if os.path.exists(settings_file) :
    execfile(settings_file, globals(), settings)
else :
    print "Could not find settings file."
    sys.exit(3)

mat_names = settings['mat_names']

import sr_py

env = []
for k in os.environ.keys() :
    estr = "%s=%s" % (k, os.environ[k])
    env.append(estr)
sr_py.init_sr_py(env)

if __name__ == '__main__' :
  rn = sr_py.get_read_nrrd_alg()
  sp = sr_py.get_seed_intersections_alg()

  sp.set_p_save_ptcl_files(1)
  sp.set_p_ptcl_path('./seeds/')

  nids = []
  for n in mat_names :
      nid = rn.execute("%s_crossing.nrrd" % n)
      nids.append(nid[0])

  print nids
  results = sp.execute(nids)
  print results
  sr_py.terminate()
