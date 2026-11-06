package require tmfa 1.1

set secret [tmfa::base32_encode "12345678901234567890"]
puts "Base32: $secret"
puts "Decoded: [tmfa::base32_decode $secret]"
puts "HOTP: [tmfa::get_hotp $secret 0 6 SHA1]"
puts "TOTP: [tmfa::get_totp $secret 6 30 SHA1]"
puts "TOTP at 59 seconds: [tmfa::get_totp $secret 8 30 SHA1 59]"
