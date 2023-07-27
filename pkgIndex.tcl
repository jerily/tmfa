set dir [file dirname [info script]]

package ifneeded tmfa 0.1 [list load [file join $dir build libtmfa.so]]
