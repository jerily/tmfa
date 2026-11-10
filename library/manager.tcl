# Framework-independent authenticator provisioning; no NaviServer/OpenACS required.
namespace eval ::tmfa {}

# Percent-encode a single URI component, preserving only RFC 3986 unreserved
# characters. Encode UTF-8 bytes, not Tcl character codes, so non-ASCII labels
# work across authenticator apps. Spaces become %20 rather than form-style '+'.
proc ::tmfa::_uri_component {text} {
    set result {}
    binary scan [encoding convertto utf-8 $text] cu* bytes
    foreach byte $bytes {
        if {($byte >= 65 && $byte <= 90) || ($byte >= 97 && $byte <= 122) ||
            ($byte >= 48 && $byte <= 57) || $byte in {45 46 95 126}} {
            append result [format %c $byte]
        } else {append result [format %%%02X $byte]}
    }
    return $result
}

# Build an otpauth:// provisioning URI for an existing Base32 secret. This does
# not generate or store secrets, advance HOTP counters, verify codes, or render
# a QR image. The caller owns those operations and must protect the returned URI:
# anyone who obtains it can reproduce the authenticator's codes.
#
# type: totp (time-based) or hotp (counter-based); account: the displayed account.
# Options: -issuer label, -digits 6|8, -algorithm SHA1|SHA256|SHA512,
#          -step seconds (TOTP only, default 30), -counter integer (HOTP required).
# Defaults match the common six-digit SHA1 TOTP configuration. An app may support
# fewer combinations than the URI format permits; configure generation to match.
proc ::tmfa::keyuri {secret type account args} {
    set config {-counter {} -step 30 -digits 6 -algorithm SHA1 -issuer {}}
    if {[llength $args] % 2} {error "options must be name/value pairs"}
    foreach {key value} $args {
        if {![dict exists $config $key]} {error "unknown option: $key"}
        dict set config $key $value
    }
    if {$type ni {totp hotp}} {error "type must be totp or hotp"}
    set issuer [dict get $config -issuer]
    # A colon separates issuer from account in the label; accepting one inside
    # either component would make that boundary ambiguous, even when encoded.
    foreach value [list $account $issuer] {
        if {[regexp {[:\x00-\x1f\x7f]} $value]} {error "account and issuer cannot contain colons or control characters"}
    }
    if {$account eq {}} {error "account must not be empty"}
    # Validate with the native Base32 decoder, then emit canonical uppercase
    # Base32 without padding, as expected by the Key URI format.
    set bytes [base32_decode $secret]
    if {[string length $bytes] == 0} {error "secret must not be empty"}
    set secret [string trimright [base32_encode $bytes] =]
    set algorithm [string toupper [dict get $config -algorithm]]
    set digits [dict get $config -digits]
    if {$algorithm ni {SHA1 SHA256 SHA512}} {error "unsupported algorithm"}
    if {$digits ni {6 8}} {error "provisioning digits must be 6 or 8"}
    set label [_uri_component $account]
    set query "secret=$secret&algorithm=$algorithm&digits=$digits"
    if {$issuer ne {}} {
        # Supply the same issuer in both places: older apps use the label prefix,
        # while newer apps can use the explicit issuer query parameter.
        set label "[_uri_component $issuer]:$label"
        append query "&issuer=[_uri_component $issuer]"
    }
    if {$type eq "totp"} {
        # Keep the provisioning period within the native get_totp limits.
        set step [dict get $config -step]
        if {![string is integer -strict $step] || $step < 1 || $step > 120} {error "step must be 1..120"}
        append query "&period=$step"
    } else {
        # HOTP needs an explicit initial counter; it has no time period.
        set counter [dict get $config -counter]
        if {![regexp {^[0-9]+$} $counter] || $counter > 18446744073709551615} {error "HOTP requires an unsigned 64-bit counter"}
        append query "&counter=$counter"
    }
    return "otpauth://$type/$label?$query"
}
