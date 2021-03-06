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



catch {rename BioPSE_Forward_SimulateEITAnalytically ""}

itcl_class BioPSE_Forward_SimulateEITAnalytically {
    inherit Module
    constructor {config} {
	set name SimulateEITAnalytically
	set_defaults
    }
    method set_defaults {} {
	global $this-outerRadiusTCL
	global $this-innerRadiusTCL
	global $this-bodyGeomTCL
	set $this-outerRadiusTCL 0.150
	set $this-innerRadiusTCL 0.070
	set $this-bodyGeomTCL {"Concentric disks"}
    }
    method make_entry {w text v c} {
        frame $w
        label $w.l -text "$text"
        pack $w.l -side left
        entry $w.e -textvariable $v
        bind $w.e <Return> $c
        pack $w.e -side right
    }
    method ui {} {
	global $this-outerRadiusTCL
	global $this-innerRadiusTCL
	global $this-bodyGeomTCL

	set w .ui[modname]
	if {[winfo exists $w]} {
	    raise $w
	    return;
	}
	toplevel $w
	
	make_labeled_radio $w.bodyGeom "Body Geometry:" "" top 1 $this-bodyGeomTCL \
	    {{"Homogeneous disk"} {"Concentric disks"}}
	make_entry $w.outerRadius "Outer radius:" $this-outerRadiusTCL "$this-c needexecute"
	make_entry $w.innerRadius "Inner radius:" $this-innerRadiusTCL "$this-c needexecute"
	bind $w.outerRadius <Return> "$this-c needexecute"
	bind $w.innerRadius <Return> "$this-c needexecute"
	pack $w.bodyGeom $w.outerRadius $w.innerRadius -side top -fill x
    }
}
