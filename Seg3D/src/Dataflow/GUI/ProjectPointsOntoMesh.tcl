itcl_class SCIRun_ChangeMesh_ProjectPointsOntoMesh {
    inherit Module
    constructor {config} {
        set name ProjectPointsOntoMesh
        set_defaults
    }

    method set_defaults {} {
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        toplevel $w
        
        frame $w.method -relief groove -borderwidth 2
        pack  $w.method -side top -e y -f both -padx 5 -pady 5
        label $w.method.t1 -text "Projection Method"
        pack  $w.method.t1 

        radiobutton $w.method.nodes -text "Project points onto nodes of object field" \
            -variable $this-method -value "nodes"
        radiobutton $w.method.elements -text "Project points onto elements of object field" \
            -variable $this-method -value "elements"

        pack $w.method.nodes $w.method.elements -side top -anchor w        

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


