##
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


itcl_class SCIRun_Bundle_InsertNrrdsIntoBundle {
    inherit Module

    constructor {config} {
        set name InsertNrrdsIntoBundle
    }

    method ui {} {
        
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        # input nrrd names

        global $this-nrrd1-name
        global $this-nrrd2-name
        global $this-nrrd3-name
        global $this-nrrd4-name
        global $this-nrrd5-name
        global $this-nrrd6-name
        global $this-bundlename

        toplevel $w 

        wm minsize $w 100 150

        
        iwidgets::labeledframe $w.frame -labeltext "NRRD INPUTS"
        set childframe [$w.frame childsite]
        pack $w.frame -fill x
        frame $w.frame2
        pack $w.frame2 -fill x
        label $w.frame2.label -text "Name bundle object :"
        entry $w.frame2.entry -textvariable $this-bundlename
        pack $w.frame2.label -side left 
        pack $w.frame2.entry -side left -fill x

        iwidgets::tabnotebook $childframe.pw -height 100 -width 450 -tabpos n
        $childframe.pw add -label "Nrrd1"
        $childframe.pw add -label "Nrrd2" 
        $childframe.pw add -label "Nrrd3" 
        $childframe.pw add -label "Nrrd4"
        $childframe.pw add -label "Nrrd5" 
        $childframe.pw add -label "Nrrd6" 
        $childframe.pw select 0

        pack $childframe.pw -fill x -expand yes

        set nrrd1 [$childframe.pw childsite 0]
        set nrrd2 [$childframe.pw childsite 1]
        set nrrd3 [$childframe.pw childsite 2]
        set nrrd4 [$childframe.pw childsite 3]
        set nrrd5 [$childframe.pw childsite 4]
        set nrrd6 [$childframe.pw childsite 5]

        frame $nrrd1.name
        frame $nrrd1.options
        pack $nrrd1.name $nrrd1.options -side top -fill x -expand yes -padx 5p

        label $nrrd1.name.label -text "Name"
        entry $nrrd1.name.entry -textvariable $this-nrrd1-name
        pack $nrrd1.name.label -side left 
        pack $nrrd1.name.entry -side left -fill x -expand yes

        checkbutton $nrrd1.options.replace -variable $this-replace1 -text "Replace field, if it already exists"
        pack $nrrd1.options.replace -side left
        
        frame $nrrd2.name
        frame $nrrd2.options
        pack $nrrd2.name $nrrd2.options -side top -fill x -expand yes -padx 5p

        label $nrrd2.name.label -text "Name"
        entry $nrrd2.name.entry -textvariable $this-nrrd2-name
        pack $nrrd2.name.label -side left 
        pack $nrrd2.name.entry -side left -fill x -expand yes

        checkbutton $nrrd2.options.replace -variable $this-replace2 -text "Replace field, if it already exists"
        pack $nrrd2.options.replace -side left
        
        frame $nrrd3.name
        frame $nrrd3.options
        pack $nrrd3.name $nrrd3.options -side top -fill x -expand yes -padx 5p

        label $nrrd3.name.label -text "Name"
        entry $nrrd3.name.entry -textvariable $this-nrrd3-name
        pack $nrrd3.name.label -side left 
        pack $nrrd3.name.entry -side left -fill x -expand yes

        checkbutton $nrrd3.options.replace -variable $this-replace3 -text "Replace field, if it already exists"
        pack $nrrd3.options.replace -side left

        frame $nrrd4.name
        frame $nrrd4.options
        pack $nrrd4.name $nrrd4.options -side top -fill x -expand yes -padx 5p

        label $nrrd4.name.label -text "Name"
        entry $nrrd4.name.entry -textvariable $this-nrrd4-name
        pack $nrrd4.name.label -side left 
        pack $nrrd4.name.entry -side left -fill x -expand yes

        checkbutton $nrrd4.options.replace -variable $this-replace4 -text "Replace field, if it already exists"
        pack $nrrd4.options.replace -side left
        
        frame $nrrd5.name
        frame $nrrd5.options
        pack $nrrd5.name $nrrd5.options -side top -fill x -expand yes -padx 5p

        label $nrrd5.name.label -text "Name"
        entry $nrrd5.name.entry -textvariable $this-nrrd5-name
        pack $nrrd5.name.label -side left 
        pack $nrrd5.name.entry -side left -fill x -expand yes

        checkbutton $nrrd5.options.replace -variable $this-replace5 -text "Replace field, if it already exists"
        pack $nrrd5.options.replace -side left
        
        frame $nrrd6.name
        frame $nrrd6.options
        pack $nrrd6.name $nrrd6.options -side top -fill x -expand yes -padx 5p

        label $nrrd6.name.label -text "Name"
        entry $nrrd6.name.entry -textvariable $this-nrrd6-name
        pack $nrrd6.name.label -side left 
        pack $nrrd6.name.entry -side left -fill x -expand yes

        checkbutton $nrrd6.options.replace -variable $this-replace6 -text "Replace field, if it already exists"
        pack $nrrd6.options.replace -side left
        
        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}
