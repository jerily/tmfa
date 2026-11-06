#!/bin/sh
set -eu
: "${PROJECT_INSTALL_DIR:?Run this script through ttrek}"
for tcl_shell in "$PROJECT_INSTALL_DIR/bin/tclsh" "$PROJECT_INSTALL_DIR"/bin/tclsh9.* "$PROJECT_INSTALL_DIR/bin/tclsh8.6"; do
    if [ -x "$tcl_shell" ]; then
        exec "$tcl_shell" "$@"
    fi
done
echo "No Tcl shell found in $PROJECT_INSTALL_DIR/bin" >&2
exit 1
