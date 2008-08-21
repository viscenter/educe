
itcl_class SCIRun_String_CreateString {
    inherit Module

    constructor {config} {
        set name CreateString
    }

    method ui {} {

        global $this-inputstring
        set w .ui[modname]
        if {[winfo exists $w]} {
            return
        }

        sci_toplevel $w
        sci_frame $w.f     
        pack $w.f -expand yes -fill both

        option add *textBackground white	
        sci_scrolledtext $w.f.str -vscrollmode dynamic \
            -labeltext "String Contents" -height 100
        $w.f.str insert end [set $this-inputstring]
        pack $w.f.str -fill both -expand yes

        makeSciButtonPanel $w $w $this
        moveToCursor $w
    }

    method update_text {} {
        set w .ui[modname]
        if {[winfo exists $w]} {
            global $this-inputstring
            set $this-inputstring [$w.f.str get 1.0 end]
        }
        }
}


