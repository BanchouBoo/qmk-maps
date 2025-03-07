#!/bin/sh

# https://github.com/qmk/qmk_firmware.git

die() {
    >&2 printf "[\033[0;31mERROR\033[0m] %s\n" "$*"
    exit 1
}

info() {
    printf "[ \033[00;34m...\033[0m ] %s\n" "$1"
}

GIT_DIR="${GIT_DIR:-${HOME}/git}"
map_dir=$(realpath "$0")
map_dir=${map_dir%/*}
repo_file="${map_dir}/repos.json"

list_keymaps() {
    printf '%s\n' "${map_dir}"/*/ | while read -r keymap; do
        keymap=${keymap%/}
        keymap=${keymap##*/}
        printf '%s\n' "$keymap"
    done
}

[ "$1" ] || { list_keymaps; exit 0; }
keymap=${map_dir}/$1

[ -d "${keymap}" ] || die "Keymap '${keymap}' not found!"

[ -f "${keymap}/vars" ] || die "Keymap missing vars file!"
. "${keymap}/vars"
[ "$KEYBOARD" ] || die "Keymap vars must include keyboard (KEYBOARD)!"
: ${REPO:=main}

repo_folder=$(jq -r ".${REPO}.folder" "$repo_file")
repo_folder=${GIT_DIR}/${repo_folder}
repo_url=$(jq -r ".${REPO}.url" "$repo_file")
repo_checkout=$(jq -r ".${REPO}.checkout" "$repo_file")
if [ "$repo_checkout" = "null" ]; then
    repo_checkout=""
fi

info "Pulling git repository"
if [ ! -d "$repo_folder" ]; then
    git clone --recurse-submodules "$repo_url" "$repo_folder"
fi

cd "$repo_folder"
git pull
if [ "$repo_checkout" ]; then
    git checkout "$repo_checkout"
fi
git submodule update --init --recursive

keymap_symlink="${repo_folder}/keyboards/${KEYBOARD}/keymaps/QMK-MAP-LINK"
# ensure there isn't a leftover symlink from previous runs, it shouldn't but just to be sure
[ -L "$keymap_symlink" ] && rm "$keymap_symlink"
ln -s "$keymap" "$keymap_symlink"
trap 'rm "$keymap_symlink"; trap - EXIT' EXIT INT HUP TERM

base="$KEYBOARD"
if [ "$REV" ]; then
    base="${base}/${REV}"
fi

export KEYMAP_DIR=$keymap
make "${base}:QMK-MAP-LINK:flash"
