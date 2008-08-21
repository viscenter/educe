itcl_class SCIRun_ChangeMesh_ScaleFieldMeshAndData {
    inherit Module
    constructor {config} {
        set name ScaleFieldMeshAndData
        set_defaults
    }

    method set_defaults {} {
        global $this-datascale
        global $this-geomscale
        global $this-usegeomcenter
    
        set $this-datascale 1
        set $this-geomscale 1
        set $this-usegeomcenter 1
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        sci_toplevel $w

        sci_labeledframe $w.frame -labeltext "SCALE FACTORS"
        set d [$w.frame childsite]
        pack $w.frame -fill both -expand yes
        
        sci_label $d.lab1 -text "Data scaling factor"
        sci_entry $d.e1 -textvariable $this-datascale       
        sci_label $d.lab2 -text "Geometry scaling factor"
        sci_entry $d.e2 -textvariable $this-geomscale       
        sci_checkbutton $d.center -variable $this-usegeomcenter -text "Use center of geometry for scaling"
        
        grid $d.lab1 -row 0 -column 0  -sticky news
        grid $d.e1 -row 0 -column 1  -sticky news
        grid $d.lab2 -row 1 -column 0  -sticky news
        grid $d.e2 -row 1 -column 1  -sticky news        
        grid $d.center -row 2 -column 0 -columnspan 2 -sticky w

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


