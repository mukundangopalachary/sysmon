#!/bin/bash

COMMAND=$1

case "$COMMAND" in
    "check")
        exit 0
        ;;
    "metadata")
        cat "$(dirname "$0")/manifest.toml"
        ;;
    "panes")
        echo "Sensors"
        ;;
    "collect")
        echo "PANE:Sensors"
        echo "cpu.temperature=45:GAUGE:C"
        echo "ENDPANE"
        ;;
    "pane")
        if [ "$2" == "Sensors" ]; then
            echo "cpu.temperature=45:GAUGE:C"
        fi
        ;;
    *)
        echo "Unknown command: $COMMAND" >&2
        exit 1
        ;;
esac
