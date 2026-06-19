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
        echo "Overview"
        ;;
    "collect")
        echo "PANE:Overview"
        echo "gpu.load=15:GAUGE:%"
        echo "gpu.memory=2048:GAUGE:MB"
        echo "ENDPANE"
        ;;
    "pane")
        if [ "$2" == "Overview" ]; then
            echo "gpu.load=15:GAUGE:%"
            echo "gpu.memory=2048:GAUGE:MB"
        fi
        ;;
    *)
        echo "Unknown command: $COMMAND" >&2
        exit 1
        ;;
esac
