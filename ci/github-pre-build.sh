#!/bin/sh
#
# Install build dependencies for GitHub Actions
#

set -e -x

SUDO=sudo

case $(uname -s) in
    Linux)
        if [ -f /etc/apt/sources.list ]; then
            echo 'Installing Linux dependencies...'

            run_apt() {
                echo "-> Running apt-get $@"
                $SUDO apt-get -q -o=Dpkg::Use-Pty=0 "$@"
                rc=$?
                echo "-> Done with $rc"
                return $rc
            }

            run_apt update || echo 'Failed to update packages, but continuing.'

            # Install build dependencies
            $SUDO apt-get -qq install \
                build-essential \
                cmake \
                gettext \
                git \
                libgl1-mesa-dev \
                libgtk-3-dev \
                liblzma-dev \
                libwxgtk3.0-gtk3-dev || \
            $SUDO apt-get -qq install \
                build-essential \
                cmake \
                gettext \
                git \
                libgl1-mesa-dev \
                libgtk-3-dev \
                liblzma-dev \
                libwxgtk3.2-dev

            # Install cloudsmith CLI for uploads
            $SUDO apt-get -qq install python3-pip python3-setuptools
            python3 -m pip install --user --upgrade pip
            python3 -m pip install --user cloudsmith-cli
        fi
        ;;

    Darwin)
        echo 'Installing macOS dependencies...'
        here=$(cd "$(dirname "$0")"; pwd)

        # Install Homebrew packages
        for pkg in cmake gettext wget pipx ninja; do
            brew list --versions $pkg || brew install $pkg || :
            brew link --overwrite $pkg || :
        done

        # Install cloudsmith CLI
        pipx install cloudsmith-cli

        if [ ${USE_HOMEBREW:-1} -ne 0 ]; then
            # Use Homebrew wxWidgets
            brew install wxwidgets
        else
            # Download pre-built wxWidgets for OpenCPN
            wget -q https://dl.cloudsmith.io/public/nohal/opencpn-plugins/raw/files/macos_deps_universal.tar.xz \
                -O /tmp/macos_deps_universal.tar.xz
            sudo tar -C /usr/local -xJf /tmp/macos_deps_universal.tar.xz
        fi
        ;;
esac
