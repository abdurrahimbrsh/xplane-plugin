#!/bin/bash

XPLANE="/home/surban/data/Software/X-Plane 11"

set -e
rm -rf "$XPLANE/Resources/plugins/RealSimGear-GNSx30"
cp -av plugin/RealSimGear-GNSx30 "$XPLANE/Resources/plugins/"
