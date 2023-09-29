set dir [file dirname [info script]]
lappend auto_path $dir
package require tmfa

set base32_encoded_data [tmfa::base32_encode something]
puts base32_encoded_data=$base32_encoded_data
puts decoded_data=[tmfa::base32_decode $base32_encoded_data]

# next two lines should succeed
puts [tmfa::get_totp abcdef 6 45 SHA256]
puts [tmfa::get_hotp abcdef 0 6 SHA1]

# next line should fail
if { [catch {
puts [tmfa::get_totp asdf@#$ 8 30 SHA256]
} errmsg] } {
    puts "Error (expected): $errmsg"
}