itcl_class SCIRun_FiniteElements_DefinePeriodicBoundaries {
    inherit Module
    constructor {config} {
        set name DefinePeriodicBoundaries
        set_defaults
    }


    method ui {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }
        
        sci_toplevel $w

        sci_labeledframe $w.f -labelpos nw \
                    -labeltext "Create Periodic Domain by linking opposing boundaries" 
        pack $w.f -fill x
        set f [$w.f childsite]

        sci_checkbutton $f.linkx -variable $this-linkx \
          -text "Link in X-direction"
        grid $f.linkx -column 0 -row 0 -sticky nws
        sci_checkbutton $f.linky -variable $this-linky \
          -text "Link in Y-direction"        
        grid $f.linky -column 0 -row 1 -sticky nws
        sci_checkbutton $f.linkz -variable $this-linkz \
          -text "Link in Z-direction"
        grid $f.linkz -column 0 -row 2 -sticky nws
        
        sci_entryfield $f.tol \
          -labeltext "Tolerance for matching node positions" \
          -textvariable $this-tol
        grid $f.tol -column 0 -row 3 -sticky news

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }
}


