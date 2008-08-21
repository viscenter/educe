itcl_class SCIRun_MiscField_GetMeshQualityField {
    inherit Module
    constructor {config} {
        set name GetMeshQualityField
        set_defaults
    }

    method set_defaults {} {
        global $this-method
        set $this-method "scaled_jacobian"  
    }

    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        sci_toplevel $w

        wm minsize $w 170 20
        sci_frame $w.f
        pack $w.f -padx 2 -pady 2 -side top -expand yes
        make_labeled_radio $w.f.r "Mesh Quality Metrics:" "" \
                top 1 $this-method \
                {{"Scaled jacobian" scaled_jacobian}\
                {"Jacobian" jacobian}\
                {"Volume" volume}}
                
        pack $w.f.r -side top -expand 1 -fill x
        pack $w.f -expand 1 -fill x

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


