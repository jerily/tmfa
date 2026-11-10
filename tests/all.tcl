package require tcltest 2

if {[llength $argv] == 0} {
    package require tmfa 1.1
} else {
    load [file normalize [lindex $argv 0]] Tmfa
}
source [file join [file dirname [info script]] .. library manager.tcl]
::tcltest::configure -singleproc true -testdir [file dirname [info script]] {*}[lrange $argv 1 end]
exit [::tcltest::runAllTests]
