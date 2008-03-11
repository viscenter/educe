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


# WriteNrrd.tcl
# by Samsonov Alexei
# October 2000 

catch {rename Teem_DataIO_WriteNrrd ""}

itcl_class Teem_DataIO_WriteNrrd {
    inherit Module
    constructor {config} {
	set name WriteNrrd
	set_defaults
    }

    method set_defaults {} {
	global $this-filetype
	set $this-filetype Binary
	global $this-filename
	set $this-filename MyFile
	global $this-exporttype
	# set $this-split 0
    }
    
    method ui {} {
	set w .ui[modname]
	if {[winfo exists $w]} {
	    # Refresh UI
	    biopseFDialog_RefreshCmd $w
	    return
	}
	
	toplevel $w -class TkFDialog

	# place to put preferred data directory
	# it's used if $this-filename is empty
	set initdir [netedit getenv SCIRUN_DATA]

	#######################################################
	# to be modified for particular reader

	# extansion to append if no extension supplied by user
	set defext ".nrrd"
	
	# name to appear initially
	set defname "MyNrrd"
	set title "Save nrrd file"

	# file types to appers in filter box
	set types {
	    {{Nrrd}     {.nrrd}      }
	    {{Nrrd Header and Raw}     {.nhdr *.raw}      }
	    {{All Files}       {.*}   }
	}
	
	######################################################
	
	makeSaveFilebox \
		-parent $w \
		-filevar $this-filename \
	        -setcmd "wm withdraw $w" \
		-command "$this-c needexecute; wm withdraw $w" \
		-cancel "wm withdraw $w" \
		-title $title \
		-filetypes $types \
	        -initialfile $defname \
		-initialdir $initdir \
		-defaultextension $defext \
		-formatvar $this-filetype \
	        -selectedfiletype $this-exporttype
		#-splitvar $this-split
    }
}