set dir [file dirname [info script]]
lappend auto_path $dir
package require tmfa

# next line should succeed
puts [tmfa::get_totp abcdef 6 45 SHA1]

# next line should fail
puts [tmfa::get_totp asdf@#$ 8 30 SHA256]
