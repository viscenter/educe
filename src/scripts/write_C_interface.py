#  
#  For more information, please see: http://software.sci.utah.edu
#  
#  The MIT License
#  
#  Copyright (c) 2004 Scientific Computing and Imaging Institute,
#  University of Utah.
#  
#  License for the specific language governing rights and limitations under
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
#    File   : write_C_interface.py
#    Author : Martin Cole
#    Date   : Thu Oct 25 12:04:34 2007

from AlgoInfo import *
import os
import re

global header_lines
header_lines = []
global header_file
global write_cc
global write_h

def load_header() :
    global header_file
    global header_lines
    if len(header_lines) == 0 :
        hf = header_file
        if not os.path.exists(hf) :
            print "ERROR header file does not exist"
            print "\t\t%s" % hf
            exit(11)
        fd = open(hf)
        header_lines = fd.readlines()
        fd.close()

fixe = re.compile("(\d_d)|(color_map)")
def change_string_format(method_name):
    """changes a string from camal case to lower case with underscores"""
    
    fol_num = False;
    temp = ""
    for i in method_name:
	if (i.isupper() and not fol_num) :
            temp = temp + "_" + i.lower()
	else :
             temp = temp + i
    temp = temp.lstrip("_*")
    end = 0
    #allow colormap and 1d, 2d, 3d etc...
    while end < len(temp) :
        mo = fixe.search(temp)
        if mo != None :
            if mo.group(1) != None :
                #n_d
                temp = (temp[:mo.start(1)] + mo.group(1)[0] +
                        "d" + temp[mo.end(1):])
                end = mo.end(1)
            elif mo.group(2) != None :
                temp = temp[:mo.start(2)] + "colormap" + temp[mo.end(2):]
                end = mo.end(2)
        else :
            end = len(temp)    
    return temp

def write_param_file(algo_info, outpath) :
    algo_info.sort_params()
    
    global header_lines
    load_header()
    
    global write_h
    if write_h :
        flines = []
        flines.extend(header_lines)
        flines.append("#if !defined(%sAlg_h)\n" % algo_info.name_)
        flines.append("#define %sAlg_h\n\n" % algo_info.name_)
        flines.append("#include <Core/Algorithms/Interface/share.h>\n")
        flines.append("#include <Core/Util/ProgressReporter.h>\n")
        flines.append("#include <string>\n")
        flines.append("#include <vector>\n")
        flines.append("namespace SCIRun {\n\n")
        flines.append("using std::string;\n")
        flines.append("using std::vector;\n\n")
        flines.append("class SCISHARE %sAlg\n" % algo_info.name_)
        flines.append("{\n")
        flines.append("public:\n")

        #Initializers
        flines.append("  %sAlg() :\n" % algo_info.name_)
        for pname, ptype, dval in algo_info.params_ :
            if ptype == 'string' :
                flines.append('    %s_("%s"),\n' % (pname, dval))
            else :
                flines.append('    %s_(%s),\n' % (pname, dval))
        flines.append("    progress_(0)\n")
        flines.append("  {}\n\n")

        #destructor
        flines.append("  virtual ~%sAlg() \n" % algo_info.name_)
        flines.append("  {}\n\n")       
        
        #Accessors
        flines.append("  //! Accessors.\n")
        for pname, ptype, dval in algo_info.params_ :
            flines.append("  %s get_%s() const\n" % (ptype, pname))
            flines.append("  { return %s_; }\n" % pname)

        flines.append("  ProgressReporter* get_progress_reporter()\n")
        flines.append("  { return progress_; }\n")
        flines.append("\n")
        #Mutators
        flines.append("  //! Mutators.\n")
        for pname, ptype, dval in algo_info.params_ :
            flines.append("  void set_%s(%s val)\n" % (pname, ptype))
            flines.append("  { %s_ = val; }\n" % pname)

        flines.append("  void set_progress_reporter(ProgressReporter *p)\n")
        flines.append("  { progress_ = p; }\n\n")

        #Methods
        rval = None
        flines.append("  //! Algorithm Interface.\n")
        flines.append("  virtual\n")
        nout = len(algo_info.outputs_)        
        if (nout > 0 and (nout > 1 or
                          algo_info.outputs_[0][1] == "Geometry")) :
            rval = 'vector<size_t>'

        elif nout == 0 :
            rval = 'void'
        else :
            rval = 'size_t'

        flines.append("  %s execute(" % rval)
        i = 0
        for pname, ptype in algo_info.inputs_ :
            t = 'size_t'
            pn = change_string_format(ptype) + '_id%d' % i
            if ptype == 'String' :
                t = "string"
                pn = change_string_format(pname).replace(' ', '_')
                
            if i == 0 :
                flines.append("%s %s" % (t, pn))
            else :
                flines.append(", %s %s" % (t, pn))
            i = i + 1
        flines.append(");\n\n")

        #Declarations
        flines.append("private:\n")
        flines.append("  //! Data.\n")
        for pname, ptype, dval in algo_info.params_ :
            indent = 30 - len(ptype)
            ind = ""
            for i in range(0, indent) :
                ind = " %s" % ind
            flines.append("  %s%s%s_;\n" % (ptype, ind, pname))

        #flines.extend(p_dec)
        flines.append("  ProgressReporter             *progress_;\n")
        flines.append("};\n\n")

        flines.append("%sAlg* get_%s_alg();\n" % (algo_info.name_,
                                   change_string_format(algo_info.name_)))

        flines.append("} //end namespace SCIRun\n")
        flines.append("#endif // %sAlg_h\n\n" % algo_info.name_)


        dirnm = "%s%s%s" % (outpath, os.sep, algo_info.category_)
        if not os.path.exists(dirnm) :
            os.makedirs(dirnm)
        fd = open("%s/%sAlg.h" % (dirnm, algo_info.name_), "w")
        fd.writelines(flines)
        fd.close()

    global write_cc
    if write_cc :
        #write the .cc file with the function stub.
        flines = []
        flines.extend(header_lines)
        flines.append("#include <iostream>\n")
        flines.append("#include <Core/Events/DataManager.h>\n")
        flines.append(
            "#include <Core/Algorithms/Interface/%s/%sAlg.h>\n"%
            (algo_info.category_, algo_info.name_))
        flines.append("\nnamespace SCIRun {\n\n")
        flines.append("//! Algorithm Interface.\n")

        rval = None
        flines.append("  //! Algorithm Interface.\n")
        
        nout = len(algo_info.outputs_)        
        if (nout > 0 and (nout > 1 or
                          algo_info.outputs_[0][1] == "Geometry")) :
            rval = 'vector<size_t>'

        elif nout == 0 :
            rval = 'void'
        else :
            rval = 'size_t'

        flines.append("%s\n%sAlg::execute(" % (rval, algo_info.name_))

        i = 0
        for pname, ptype in algo_info.inputs_ :
            t = 'size_t'
            pn = change_string_format(ptype) + '_id%d' % i
            if ptype == 'String' :
                t = "string"
                pn = change_string_format(pname).replace(' ', '_')
            if ptype == 'String' :
                t = "string"
            if i == 0 :
                flines.append("%s %s" % (t, pn))
            else :
                flines.append(", %s %s" % (t, pn))
            i = i + 1
                
        flines.append(")\n")    
        flines.append("{\n")
        flines.append("  DataManager *dm = DataManager::get_dm();\n")
        flines.append(
            '  std::cerr << "WARNING: %sAlg::execute not implemented."'\
            ' << std::endl;\n' %
            algo_info.name_)
        if rval == 'void' :
            pass
        elif rval == 'size_t' :
            flines.append('  return 0;\n')
        else :
            flines.append('  return vector<size_t>(0);\n')
        flines.append("}\n\n")
        flines.append("} //end namespace SCIRun\n\n")

        dirnm = "%s%s%s" % (outpath, os.sep, algo_info.category_)
        if not os.path.exists(dirnm) :
            os.makedirs(dirnm)
        fd = open("%s%s%sAlg.cc" % (dirnm, os.sep, algo_info.name_), "w")
        fd.writelines(flines)
        fd.close()      





if __name__ == '__main__' : 
    global header_file
    global write_cc
    global write_h

    write_h = True
    write_cc = False

    if len(sys.argv) > 4 :
        write_h = False
        write_cc = True
        
    algo_file = sys.argv[1]
    output_path = sys.argv[2]
    header_file = "%s%sheader_comment.h" % (sys.argv[3], os.sep)
    
    ai = parse_algo_info(algo_file)
    write_param_file(ai, output_path) 
