#!/bin/sh

# https://github.com/qmk/qmk_firmware.git

die() {
    >&2 printf "[\033[0;31mERROR\033[0m] %s\n" "$*"
    exit 1
}

info() {
    printf "[ \033[00;34m...\033[0m ] %s\n" "$1"
}

git_dir="${HOME}/opt/git"
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
[ "$REV" ] || die "Keymap vars must include revision (REV)!"
: ${REPO:=main}

repo_folder=$(jq -r ".${REPO}.folder" "$repo_file")
repo_folder=${git_dir}/${repo_folder}
repo_url=$(jq -r ".${REPO}.url" "$repo_file")

info "Pulling git repository"
if [ ! -d "$repo_folder" ]; then
    git clone --recurse-submodules "$repo_url" "$repo_folder"
else
    cd "$repo_folder"
    git pull
fi

cd "$repo_folder"

keymap_symlink="${repo_folder}/keyboards/${KEYBOARD}/keymaps/QMK-MAP-LINK"
# ensure there isn't a leftover symlink from previous runs, it shouldn't but just to be sure
[ -L "$keymap_symlink" ] && rm "$keymap_symlink"
ln -s "$keymap" "$keymap_symlink"
trap 'rm "$keymap_symlink"; trap - EXIT' EXIT INT HUP TERM

make "${KEYBOARD}/${REV}:QMK-MAP-LINK:flash"
