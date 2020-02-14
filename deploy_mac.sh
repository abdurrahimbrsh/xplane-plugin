#!/usr/bin/env bash

XPLANE="/Volumes/Disk2/X-Plane 11"

set -e
rm -rf "$XPLANE/Resources/plugins/RealSimGear-GNSx30"
cp -av plugin/RealSimGear-GNSx30 "$XPLANE/Resources/plugins/"
