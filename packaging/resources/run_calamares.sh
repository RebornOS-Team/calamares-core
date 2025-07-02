#! /usr/bin/env sh

SCRIPT_DIRECTORY="$(dirname -- "$(readlink -f -- "$0")")"

MODE="$(echo "$1" | tr "[:upper:]" "[:lower:]")"
if [ "$MODE" != "offline" ] && [ "$MODE" != "online" ]; then
    echo ""
    echo "ERROR: Please specify \"online\" or \"offline\" as the first argument to this script, to indicate the mode to run Calamares in."
    exit 1
fi
shift 1

UNMOUNT_FILESYSTEMS_COMMAND="umount -rf /run/media/rebornos/*"

UPDATE_INSTALL_MODE_COMMAND="cp -f \
    "$SCRIPT_DIRECTORY"/settings_"$MODE".conf \
    "$SCRIPT_DIRECTORY"/settings.conf"

LAUNCH_INSTALLER_COMMAND="env \
    DISPLAY="$DISPLAY" \
    WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
    XAUTHORITY="$XAUTHORITY" \
    XDG_RUNTIME_DIR="$XDG_RUNTIME_DIR" \
    XDG_SESSION_TYPE=$XDG_SESSION_TYPE \
    HOME="$HOME" \
    DEFAULT_USER="$USER" \
    PATH=$PATH \
    KDE_SESSION_VERSION=6 \
    KDE_FULL_SESSION=true \
    QT_QUICK_CONTROLS_STYLE="Fusion" \
    QT_QPA_PLATFORMTHEME="qt6ct" \
    calamares "$@""

{
    pkexec \
    bash -c \
        "$UNMOUNT_FILESYSTEMS_COMMAND; $UPDATE_INSTALL_MODE_COMMAND && $LAUNCH_INSTALLER_COMMAND" 
} > ~/install.log      