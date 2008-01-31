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


itcl_class SCIRun_Bundle_GetColorMap2sFromBundle {
    inherit Module

    constructor {config} {
        set name GetColorMap2sFromBundle

        initGlobal $this-colormap21-listbox ""
        initGlobal $this-colormap22-listbox ""
        initGlobal $this-colormap23-listbox ""
        initGlobal $this-colormap24-listbox ""
        initGlobal $this-colormap25-listbox ""
        initGlobal $this-colormap26-listbox ""
        initGlobal $this-colormap21-entry ""
        initGlobal $this-colormap22-entry ""
        initGlobal $this-colormap23-entry ""
        initGlobal $this-colormap24-entry ""
        initGlobal $this-colormap25-entry ""
        initGlobal $this-colormap26-entry ""
    }

    method ui {} {
        
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        # input matrix names

        global $this-colormap21-name
        global $this-colormap22-name
        global $this-colormap23-name
        global $this-colormap24-name
        global $this-colormap25-name
        global $this-colormap26-name
        global $this-colormap2-selection
        global $this-colormap21-listbox
        global $this-colormap22-listbox
        global $this-colormap23-listbox
        global $this-colormap24-listbox
        global $this-colormap25-listbox
        global $this-colormap26-listbox
        global $this-colormap21-entry
        global $this-colormap22-entry
        global $this-colormap23-entry
        global $this-colormap24-entry
        global $this-colormap25-entry
        global $this-colormap26-entry

        toplevel $w 

        wm minsize $w 100 150

        
        iwidgets::labeledframe $w.frame -labeltext "BUNDLE FIELD OUTPUTS"
        set childframe [$w.frame childsite]
        pack $w.frame -fill both -expand yes

        iwidgets::tabnotebook $childframe.pw -width 400 -tabpos n
        $childframe.pw add -label "ColorMap21"
        $childframe.pw add -label "ColorMap22" 
        $childframe.pw add -label "ColorMap23" 
        $childframe.pw add -label "ColorMap24"
        $childframe.pw add -label "ColorMap25" 
        $childframe.pw add -label "ColorMap26" 
        $childframe.pw select 0

        pack $childframe.pw -fill both -expand yes

        set colormap21 [$childframe.pw childsite 0]
        set colormap22 [$childframe.pw childsite 1]
        set colormap23 [$childframe.pw childsite 2]
        set colormap24 [$childframe.pw childsite 3]
        set colormap25 [$childframe.pw childsite 4]
        set colormap26 [$childframe.pw childsite 5]

        frame $colormap21.name
        frame $colormap21.sel
        pack $colormap21.name -side top -fill x -expand no -padx 5p
        pack $colormap21.sel -side top -fill both -expand yes -padx 5p

        label $colormap21.name.label -text "Name"
        entry $colormap21.name.entry -textvariable $this-colormap21-name
        set $this-colormap21-entry $colormap21.name.entry
        pack $colormap21.name.label -side left 
        pack $colormap21.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $colormap21.sel.listbox  -selectioncommand [format "%s ChooseColorMap21" $this]
        set $this-colormap21-listbox $colormap21.sel.listbox
        $colormap21.sel.listbox component listbox configure -listvariable $this-colormap2-selection -selectmode browse
        pack $colormap21.sel.listbox -fill both -expand yes

        frame $colormap22.name
        frame $colormap22.sel
        pack $colormap22.name -side top -fill x -expand no -padx 5p
        pack $colormap22.sel -side top -fill both -expand yes -padx 5p

        label $colormap22.name.label -text "Name"
        entry $colormap22.name.entry -textvariable $this-colormap22-name
        set $this-colormap22-entry $colormap22.name.entry
        pack $colormap22.name.label -side left 
        pack $colormap22.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $colormap22.sel.listbox  -selectioncommand [format "%s ChooseColorMap22" $this]
        set $this-colormap22-listbox $colormap22.sel.listbox
        $colormap22.sel.listbox component listbox configure -listvariable $this-colormap2-selection -selectmode browse
        pack $colormap22.sel.listbox -fill both -expand yes
        
        frame $colormap23.name
        frame $colormap23.sel
        pack $colormap23.name -side top -fill x -expand no -padx 5p
        pack $colormap23.sel -side top -fill both -expand yes -padx 5p

        label $colormap23.name.label -text "Name"
        entry $colormap23.name.entry -textvariable $this-colormap23-name
        set $this-colormap23-entry $colormap23.name.entry
        pack $colormap23.name.label -side left 
        pack $colormap23.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $colormap23.sel.listbox  -selectioncommand [format "%s ChooseColorMap23" $this]
        set $this-colormap23-listbox $colormap23.sel.listbox
        $colormap23.sel.listbox component listbox configure -listvariable $this-colormap2-selection -selectmode browse
        pack $colormap23.sel.listbox -fill both -expand yes

        frame $colormap24.name
        frame $colormap24.sel
        pack $colormap24.name -side top -fill x -expand no -padx 5p
        pack $colormap24.sel -side top -fill both -expand yes -padx 5p

        label $colormap24.name.label -text "Name"
        entry $colormap24.name.entry -textvariable $this-colormap24-name
        set $this-colormap24-entry $colormap24.name.entry
        pack $colormap24.name.label -side left 
        pack $colormap24.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $colormap24.sel.listbox  -selectioncommand [format "%s ChooseColorMap24" $this]
        set $this-colormap24-listbox $colormap24.sel.listbox
        $colormap24.sel.listbox component listbox configure -listvariable $this-colormap2-selection -selectmode browse
        pack $colormap24.sel.listbox -fill both -expand yes

        frame $colormap25.name
        frame $colormap25.sel
        pack $colormap25.name -side top -fill x -expand no -padx 5p
        pack $colormap25.sel -side top -fill both -expand yes -padx 5p

        label $colormap25.name.label -text "Name"
        entry $colormap25.name.entry -textvariable $this-colormap25-name
        set $this-colormap25-entry $colormap25.name.entry
        pack $colormap25.name.label -side left 
        pack $colormap25.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $colormap25.sel.listbox  -selectioncommand [format "%s ChooseColorMap25" $this]
        set $this-colormap25-listbox $colormap25.sel.listbox
        $colormap25.sel.listbox component listbox configure -listvariable $this-colormap2-selection -selectmode browse
        pack $colormap25.sel.listbox -fill both -expand yes
        
        frame $colormap26.name
        frame $colormap26.sel
        pack $colormap26.name -side top -fill x -expand no -padx 5p
        pack $colormap26.sel -side top -fill both -expand yes -padx 5p

        label $colormap26.name.label -text "Name"
        entry $colormap26.name.entry -textvariable $this-colormap26-name
        set $this-colormap26-entry $colormap26.name.entry
        pack $colormap26.name.label -side left 
        pack $colormap26.name.entry -side left -fill x -expand yes
        
        iwidgets::scrolledlistbox $colormap26.sel.listbox  -selectioncommand [format "%s ChooseColorMap26" $this]
        set $this-colormap26-listbox $colormap26.sel.listbox
        $colormap26.sel.listbox component listbox configure -listvariable $this-colormap2-selection -selectmode browse
        pack $colormap26.sel.listbox -fill both -expand yes

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
    
    
    method ChooseColorMap21 { } {
        global $this-colormap21-listbox
        global $this-colormap21-name
        global $this-colormap2-selection
        
        set colormap2num [[set $this-colormap21-listbox] curselection]
        if [expr [string equal $colormap2num ""] == 0] {
            set $this-colormap21-name  [lindex [set $this-colormap2-selection] $colormap2num] 
        }
    }

    method ChooseColorMap22 { } {
        global $this-colormap22-listbox
        global $this-colormap22-name
        global $this-colormap2-selection
        
        set colormap2num [[set $this-colormap22-listbox] curselection]
        if [expr [string equal $colormap2num ""] == 0] {
            set $this-colormap22-name  [lindex [set $this-colormap2-selection] $colormap2num] 
        }
    }

    method ChooseColorMap23 { } {
        global $this-colormap23-listbox
        global $this-colormap23-name
        global $this-colormap2-selection
        
        set colormap2num [[set $this-colormap23-listbox] curselection]
        if [expr [string equal $colormap2num ""] == 0] {
            set $this-colormap23-name  [lindex [set $this-colormap2-selection] $colormap2num] 
        }
    }

    method ChooseColorMap24 { } {
        global $this-colormap24-listbox
        global $this-colormap24-name
        global $this-colormap2-selection
        
        set colormap2num [[set $this-colormap24-listbox] curselection]
        if [expr [string equal $colormap2num ""] == 0] {
            set $this-colormap24-name  [lindex [set $this-colormap2-selection] $colormap2num] 
        }
    }

    method ChooseColorMap25 { } {
        global $this-colormap25-listbox
        global $this-colormap25-name
        global $this-colormap2-selection
        
        set colormap2num [[set $this-colormap25-listbox] curselection]
        if [expr [string equal $colormap2num ""] == 0] {
            set $this-colormap25-name  [lindex [set $this-colormap2-selection] $colormap2num] 
        }
    }

    method ChooseColorMap26 { } {
        global $this-colormap26-listbox
        global $this-colormap26-name
        global $this-colormap2-selection
        
        set colormap2num [[set $this-colormap26-listbox] curselection]
        if [expr [string equal $colormap2num ""] == 0] {
            set $this-colormap26-name  [lindex [set $this-colormap2-selection] $colormap2num] 
        }
    }
    
}
