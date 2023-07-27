set dir [file dirname [info script]]
lappend auto_path $dir
package require tmfa

# next two lines should succeed
puts [tmfa::get_totp abcdef 6 45 SHA256]
puts [tmfa::get_hotp abcdef 0 6 SHA1]

# next line should fail
puts [tmfa::get_totp asdf@#$ 8 30 SHA256]
