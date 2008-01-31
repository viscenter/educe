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


itcl_class SCIRun_Bundle_InsertBundlesIntoBundle {
    inherit Module

    constructor {config} {
        set name InsertBundlesIntoBundle
    }

    method ui {} {
        
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        # input bundle names

        global $this-bundle1-name
        global $this-bundle2-name
        global $this-bundle3-name
        global $this-bundle4-name
        global $this-bundle5-name
        global $this-bundle6-name
        global $this-bundlename

        toplevel $w 
        wm minsize $w 200 150
        
        iwidgets::labeledframe $w.frame -labeltext "BUNDLE INPUTS"
        set childframe [$w.frame childsite]
        pack $w.frame -fill x
        frame $w.frame2
        pack $w.frame2 -fill x
        label $w.frame2.label -text "Name bundle object :"
        entry $w.frame2.entry -textvariable $this-bundlename
        pack $w.frame2.label -side left 
        pack $w.frame2.entry -side left -fill x
        
        iwidgets::tabnotebook $childframe.pw -width 450 -tabpos n
        $childframe.pw add -label "Bundle1"
        $childframe.pw add -label "Bundle2" 
        $childframe.pw add -label "Bundle3" 
        $childframe.pw add -label "Bundle4"
        $childframe.pw add -label "Bundle5" 
        $childframe.pw add -label "Bundle6" 
        $childframe.pw select 0

        pack $childframe.pw -fill x -expand yes

        set bundle1 [$childframe.pw childsite 0]
        set bundle2 [$childframe.pw childsite 1]
        set bundle3 [$childframe.pw childsite 2]
        set bundle4 [$childframe.pw childsite 3]
        set bundle5 [$childframe.pw childsite 4]
        set bundle6 [$childframe.pw childsite 5]

        frame $bundle1.name
        frame $bundle1.options
        pack $bundle1.name $bundle1.options -side top -fill x -expand yes -padx 5p

        label $bundle1.name.label -text "Name"
        entry $bundle1.name.entry -textvariable $this-bundle1-name
        pack $bundle1.name.label -side left 
        pack $bundle1.name.entry -side left -fill x -expand yes
        
        checkbutton $bundle1.options.replace -variable $this-replace1 -text "Replace field, if it already exists"
        pack $bundle1.options.replace -side left
        
        frame $bundle2.name
        frame $bundle2.options
        pack $bundle2.name $bundle2.options -side top -fill x -expand yes -padx 5p

        label $bundle2.name.label -text "Name"
        entry $bundle2.name.entry -textvariable $this-bundle2-name
        pack $bundle2.name.label -side left 
        pack $bundle2.name.entry -side left -fill x -expand yes
        
        checkbutton $bundle2.options.replace -variable $this-replace2 -text "Replace field, if it already exists"
        pack $bundle2.options.replace -side left
        
        frame $bundle3.name
        frame $bundle3.options
        pack $bundle3.name $bundle3.options -side top -fill x -expand yes -padx 5p

        label $bundle3.name.label -text "Name"
        entry $bundle3.name.entry -textvariable $this-bundle3-name
        pack $bundle3.name.label -side left 
        pack $bundle3.name.entry -side left -fill x -expand yes

        checkbutton $bundle3.options.replace -variable $this-replace3 -text "Replace field, if it already exists"
        pack $bundle3.options.replace -side left

        frame $bundle4.name
        frame $bundle4.options
        pack $bundle4.name $bundle4.options -side top -fill x -expand yes -padx 5p

        label $bundle4.name.label -text "Name"
        entry $bundle4.name.entry -textvariable $this-bundle4-name
        pack $bundle4.name.label -side left 
        pack $bundle4.name.entry -side left -fill x -expand yes
        
        checkbutton $bundle4.options.replace -variable $this-replace4 -text "Replace field, if it already exists"
        pack $bundle4.options.replace -side left
        
        frame $bundle5.name
        frame $bundle5.options
        pack $bundle5.name $bundle5.options -side top -fill x -expand yes -padx 5p

        label $bundle5.name.label -text "Name"
        entry $bundle5.name.entry -textvariable $this-bundle5-name
        pack $bundle5.name.label -side left 
        pack $bundle5.name.entry -side left -fill x -expand yes

        checkbutton $bundle5.options.replace -variable $this-replace5 -text "Replace field, if it already exists"
        pack $bundle5.options.replace -side left
        
        frame $bundle6.name
        frame $bundle6.options
        pack $bundle6.name $bundle6.options -side top -fill x -expand yes -padx 5p

        label $bundle6.name.label -text "Name"
        entry $bundle6.name.entry -textvariable $this-bundle6-name
        pack $bundle6.name.label -side left 
        pack $bundle6.name.entry -side left -fill x -expand yes

        checkbutton $bundle6.options.replace -variable $this-replace6 -text "Replace field, if it already exists"
        pack $bundle6.options.replace -side left

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}
