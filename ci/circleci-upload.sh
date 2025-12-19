#!/usr/bin/env bash
#
# Upload Android artifacts to Cloudsmith
# Only runs on tagged releases
#

set -xe

# Check if this is a tag build
if [ -z "$CIRCLE_TAG" ]; then
    echo "Not a tagged release, skipping upload"
    exit 0
fi

# Check for Cloudsmith API key
if [ -z "$CLOUDSMITH_API_KEY" ]; then
    echo "CLOUDSMITH_API_KEY not set, skipping upload"
    exit 0
fi

cd build

# Find and upload tarball
TARBALL=$(ls *.tar.gz 2>/dev/null | head -1)
if [ -z "$TARBALL" ]; then
    echo "No tarball found, skipping upload"
    exit 0
fi

echo "Uploading $TARBALL to Cloudsmith..."

# Upload using cloudsmith CLI
# Adjust the repository path as needed for your Cloudsmith account
cloudsmith push raw \
    --republish \
    marineyachtradar/mayara-server-pi \
    "$TARBALL"

echo "Upload complete"
