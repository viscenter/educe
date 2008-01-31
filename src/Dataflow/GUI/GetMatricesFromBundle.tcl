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


itcl_class SCIRun_Bundle_GetMatricesFromBundle {
    inherit Module

    constructor {config} {
        set name GetMatricesFromBundle

        initGlobal $this-matrix1-listbox ""
        initGlobal $this-matrix2-listbox ""
        initGlobal $this-matrix3-listbox ""
        initGlobal $this-matrix4-listbox ""
        initGlobal $this-matrix5-listbox ""
        initGlobal $this-matrix6-listbox ""
        initGlobal $this-matrix1-entry ""
        initGlobal $this-matrix2-entry ""
        initGlobal $this-matrix3-entry ""
        initGlobal $this-matrix4-entry ""
        initGlobal $this-matrix5-entry ""
        initGlobal $this-matrix6-entry ""
    }

    method ui {} {
        
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        # input matrix names

        global $this-matrix1-name
        global $this-matrix2-name
        global $this-matrix3-name
        global $this-matrix4-name
        global $this-matrix5-name
        global $this-matrix6-name
        global $this-matrix-selection
        global $this-matrix1-listbox
        global $this-matrix2-listbox
        global $this-matrix3-listbox
        global $this-matrix4-listbox
        global $this-matrix5-listbox
        global $this-matrix6-listbox
        global $this-matrix1-entry
        global $this-matrix2-entry
        global $this-matrix3-entry
        global $this-matrix4-entry
        global $this-matrix5-entry
        global $this-matrix6-entry
        global $this-transposenrrd1
        global $this-transposenrrd2
        global $this-transposenrrd3
        global $this-transposenrrd4
        global $this-transposenrrd5
        global $this-transposenrrd6
        
        toplevel $w 

        wm minsize $w 100 150
        
        iwidgets::labeledframe $w.frame -labeltext "MATRIX OUTPUTS"
        set childframe [$w.frame childsite]
        pack $w.frame -fill both -expand yes

        iwidgets::tabnotebook $childframe.pw -width 400 -tabpos n
        $childframe.pw add -label "Matrix1"
        $childframe.pw add -label "Matrix2" 
        $childframe.pw add -label "Matrix3" 
        $childframe.pw add -label "Matrix4"
        $childframe.pw add -label "Matrix5" 
        $childframe.pw add -label "Matrix6" 
        $childframe.pw select 0

        pack $childframe.pw -fill both -expand yes

        set matrix1 [$childframe.pw childsite 0]
        set matrix2 [$childframe.pw childsite 1]
        set matrix3 [$childframe.pw childsite 2]
        set matrix4 [$childframe.pw childsite 3]
        set matrix5 [$childframe.pw childsite 4]
        set matrix6 [$childframe.pw childsite 5]

        frame $matrix1.name
        frame $matrix1.transpose
        frame $matrix1.sel
        pack $matrix1.name -side top -fill x -padx 5p
        pack $matrix1.transpose -side top -fill x -padx 5p
        pack $matrix1.sel -side top -fill both -expand yes -padx 5p

        label $matrix1.name.label -text "Name"
        entry $matrix1.name.entry -textvariable $this-matrix1-name
        set $this-matrix1-entry $matrix1.name.entry
        pack $matrix1.name.label -side left 
        pack $matrix1.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $matrix1.sel.listbox  -selectioncommand [format "%s ChooseMatrix1" $this]
        set $this-matrix1-listbox $matrix1.sel.listbox
        $matrix1.sel.listbox component listbox configure -listvariable $this-matrix-selection -selectmode browse 
        pack $matrix1.sel.listbox -fill both -expand yes
        checkbutton $matrix1.transpose.cb -variable $this-transposenrrd1 -text "Assume nrrd data is transposed"
        pack $matrix1.transpose.cb -side left -fill x


        frame $matrix2.name
        frame $matrix2.sel
        frame $matrix2.transpose
        pack $matrix2.name -side top -fill x -padx 5p
        pack $matrix2.transpose -side top -fill x -padx 5p
        pack $matrix2.sel -side top -fill both -expand yes -padx 5p

        label $matrix2.name.label -text "Name"
        entry $matrix2.name.entry -textvariable $this-matrix2-name
        set $this-matrix2-entry $matrix2.name.entry
        pack $matrix2.name.label -side left 
        pack $matrix2.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $matrix2.sel.listbox  -selectioncommand [format "%s ChooseMatrix2" $this]
        set $this-matrix2-listbox $matrix2.sel.listbox
        $matrix2.sel.listbox component listbox configure -listvariable $this-matrix-selection -selectmode browse 
        pack $matrix2.sel.listbox -fill both -expand yes
        checkbutton $matrix2.transpose.cb -variable $this-transposenrrd1 -text "Assume nrrd data is transposed"
        pack $matrix2.transpose.cb -side left -fill x

        frame $matrix3.name
        frame $matrix3.transpose
        frame $matrix3.sel
        pack $matrix3.name -side top -fill x -padx 5p
        pack $matrix3.transpose -side top -fill x -padx 5p
        pack $matrix3.sel -side top -fill both -expand yes -padx 5p

        label $matrix3.name.label -text "Name"
        entry $matrix3.name.entry -textvariable $this-matrix3-name
        set $this-matrix3-entry $matrix3.name.entry
        pack $matrix3.name.label -side left 
        pack $matrix3.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $matrix3.sel.listbox  -selectioncommand [format "%s ChooseMatrix3" $this]
        set $this-matrix3-listbox $matrix3.sel.listbox
        $matrix3.sel.listbox component listbox configure -listvariable $this-matrix-selection -selectmode browse
        pack $matrix3.sel.listbox -fill both -expand yes
        checkbutton $matrix3.transpose.cb -variable $this-transposenrrd1 -text "Assume nrrd data is transposed"
        pack $matrix3.transpose.cb -side left -fill x

        frame $matrix4.name
        frame $matrix4.transpose
        frame $matrix4.sel
        pack $matrix4.name -side top -fill x -padx 5p
        pack $matrix4.transpose -side top -fill x -padx 5p
        pack $matrix4.sel -side top -fill both -expand yes -padx 5p

        label $matrix4.name.label -text "Name"
        entry $matrix4.name.entry -textvariable $this-matrix4-name
        set $this-matrix4-entry $matrix4.name.entry
        pack $matrix4.name.label -side left 
        pack $matrix4.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $matrix4.sel.listbox  -selectioncommand [format "%s ChooseMatrix4" $this]
        set $this-matrix4-listbox $matrix4.sel.listbox
        $matrix4.sel.listbox component listbox configure -listvariable $this-matrix-selection -selectmode browse 
        pack $matrix4.sel.listbox -fill both -expand yes
        checkbutton $matrix4.transpose.cb -variable $this-transposenrrd4 -text "Assume nrrd data is transposed"
        pack $matrix4.transpose.cb -side left -fill x

        frame $matrix5.name
        frame $matrix5.sel
        frame $matrix5.transpose
        pack $matrix5.name -side top -fill x -padx 5p
        pack $matrix5.transpose -side top -fill x -padx 5p
        pack $matrix5.sel -side top -fill both -expand yes -padx 5p

        label $matrix5.name.label -text "Name"
        entry $matrix5.name.entry -textvariable $this-matrix5-name
        set $this-matrix5-entry $matrix5.name.entry
        pack $matrix5.name.label -side left 
        pack $matrix5.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $matrix5.sel.listbox  -selectioncommand [format "%s ChooseMatrix5" $this]
        set $this-matrix5-listbox $matrix5.sel.listbox
        $matrix5.sel.listbox component listbox configure -listvariable $this-matrix-selection -selectmode browse 
        pack $matrix5.sel.listbox -fill both -expand yes
        checkbutton $matrix5.transpose.cb -variable $this-transposenrrd5 -text "Assume nrrd data is transposed"
        pack $matrix5.transpose.cb -side left -fill x

        frame $matrix6.name
        frame $matrix6.transpose
        frame $matrix6.sel
        pack $matrix6.name -side top -fill x -padx 5p
        pack $matrix6.transpose -side top -fill x -padx 5p
        pack $matrix6.sel -side top -fill both -expand yes -padx 5p

        label $matrix6.name.label -text "Name"
        entry $matrix6.name.entry -textvariable $this-matrix6-name
        set $this-matrix6-entry $matrix6.name.entry
        pack $matrix6.name.label -side left 
        pack $matrix6.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $matrix6.sel.listbox  -selectioncommand [format "%s ChooseMatrix6" $this]
        set $this-matrix6-listbox $matrix6.sel.listbox
        $matrix6.sel.listbox component listbox configure -listvariable $this-matrix-selection -selectmode browse
        pack $matrix6.sel.listbox -fill both -expand yes
        checkbutton $matrix6.transpose.cb -variable $this-transposenrrd1 -text "Assume nrrd data is transposed"
        pack $matrix6.transpose.cb -side left -fill x
        
        
        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
    
    
    method ChooseMatrix1 { } {
        global $this-matrix1-listbox
        global $this-matrix1-name
        global $this-matrix-selection
        
        set matrixnum [[set $this-matrix1-listbox] curselection]
        if [expr [string equal $matrixnum ""] == 0] {
            set $this-matrix1-name  [lindex [set $this-matrix-selection] $matrixnum] 
        }

    }

    method ChooseMatrix2 { } {
        global $this-matrix2-listbox
        global $this-matrix2-name
        global $this-matrix-selection
        
        set matrixnum [[set $this-matrix2-listbox] curselection]
        if [expr [string equal $matrixnum ""] == 0] {
            set $this-matrix2-name  [lindex [set $this-matrix-selection] $matrixnum] 
        }
    }

    method ChooseMatrix3 { } {
        global $this-matrix3-listbox
        global $this-matrix3-name
        global $this-matrix-selection
        
        set matrixnum [[set $this-matrix3-listbox] curselection]
        if [expr [string equal $matrixnum ""] == 0] {
            set $this-matrix3-name  [lindex [set $this-matrix-selection] $matrixnum] 
        }
    }

    method ChooseMatrix4 { } {
        global $this-matrix4-listbox
        global $this-matrix4-name
        global $this-matrix-selection
        
        set matrixnum [[set $this-matrix4-listbox] curselection]
        if [expr [string equal $matrixnum ""] == 0] {
            set $this-matrix4-name  [lindex [set $this-matrix-selection] $matrixnum] 
        }
    }

    method ChooseMatrix5 { } {
        global $this-matrix5-listbox
        global $this-matrix5-name
        global $this-matrix-selection
        
        set matrixnum [[set $this-matrix5-listbox] curselection]
        if [expr [string equal $matrixnum ""] == 0] {
            set $this-matrix5-name  [lindex [set $this-matrix-selection] $matrixnum] 
        }
    }

    method ChooseMatrix6 { } {
        global $this-matrix6-listbox
        global $this-matrix6-name
        global $this-matrix-selection
        
        set matrixnum [[set $this-matrix6-listbox] curselection]
        if [expr [string equal $matrixnum ""] == 0] {
            set $this-matrix6-name  [lindex [set $this-matrix-selection] $matrixnum] 
        }
    }
    
}
