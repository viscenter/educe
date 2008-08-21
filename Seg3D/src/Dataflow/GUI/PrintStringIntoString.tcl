itcl_class SCIRun_String_PrintStringIntoString {
    inherit Module

    constructor {config} {
        set name PrintStringIntoString
    }

    method ui {} {
    
        global $this-formatstring
        
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        sci_toplevel $w

        sci_frame $w.frame
        pack $w.frame -side top -fill x -expand yes -padx 5 -pady 5
        sci_frame $w.frame2
        pack $w.frame2 -side top -fill x -expand yes -padx 5 -pady 5
        
        sci_label $w.frame.label -text "Format string :"
        sci_entry $w.frame.string -textvariable $this-formatstring
        pack $w.frame.label -side left 
        pack $w.frame.string -side right -fill x -expand yes

        sci_label $w.frame2.label -text "Available format strings :"
        sci_label $w.frame2.string -text "%s %s %c %C"
        pack $w.frame2.label -side top -anchor w
        pack $w.frame2.string -side top -anchor w

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


