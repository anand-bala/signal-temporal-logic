#!/usr/bin/env bash

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd -P)
project_dir=$(readlink -f $script_dir/..)

setup_colors() {
  if [[ -t 2 ]] && [[ -z "${NO_COLOR-}" ]] && [[ "${TERM-}" != "dumb" ]]; then
    NOFORMAT='\033[0m' RED='\033[0;31m' GREEN='\033[0;32m' ORANGE='\033[0;33m' BLUE='\033[0;34m' PURPLE='\033[0;35m' CYAN='\033[0;36m' YELLOW='\033[1;33m'
  else
    NOFORMAT='' RED='' GREEN='' ORANGE='' BLUE='' PURPLE='' CYAN='' YELLOW=''
  fi
}

msg() {
  echo >&2 -e "${1-}"
}

relpath(){
  python3 -c "import os.path; print(os.path.relpath('$1','${2:-$PWD}'))"
}

setup_colors

cd $project_dir
coverage_file="${project_dir}/coverage.info"
proj_relpath=$(relpath $project_dir)
src_relpath=$(relpath $project_dir/src)

msg "${GREEN}Capturing lcov data and saving to:${NOFORMAT} ${coverage_file}"
lcov --capture --directory . --output-file $coverage_file
msg "-- ${YELLOW}Extracting coverage information for directory:${NOFORMAT} ${src_relpath}"
lcov --extract $coverage_file "${project_dir}/src/*" --output-file $coverage_file
lcov --list $coverage_file

