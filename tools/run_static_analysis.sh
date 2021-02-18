#!/usr/bin/env bash

set -Eeuo pipefail

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

get_dirname() {
  python3 -c "import os.path; print(os.path.dirname('$1'))"
}

setup_colors

proj_relpath=$(relpath $project_dir)
src_relpath=$(relpath $project_dir/src)

cd $project_dir
if [ -f "${proj_relpath}/compile_commands.json" ]; then
  database="${proj_relpath}/compile_commands.json"
  database_dir="${project_dir}"
elif [ -f "${proj_relpath}/build/compile_commands.json" ]; then
  database="${proj_relpath}/build/compile_commands.json"
  database_dir="${project_dir}/build"
fi

hpp_sources=$(find src include -type f -name "*.hpp")
cpp_sources=$(find src include -type f -name "*.cc")
sources="${hpp_sources} ${cpp_sources}"

msg "${GREEN}Using compile database:${NOFORMAT} ${database}"
msg "${GREEN}===== Running clang-tidy             =====${NOFORMAT}"
clang-tidy --quiet -p $database_dir --use-color $sources

msg "${GREEN}===== Running include-what-you-use   =====${NOFORMAT}"
iwyu_tool -p $database_dir -o iwyu $cpp_sources -- \
  -Xiwyu --mapping_file=$project_dir/tools/argus.imp \
  -Xiwyu --transitive_includes_only \
  -Xiwyu --cxx17ns

msg "${GREEN}===== Running cppcheck (on project)  =====${NOFORMAT}"
cppcheck \
  --std=c++17 \
  --project=$database\
  $sources -i $project_dir/.cache

msg "${GREEN}===== Running cppcheck (on files)    =====${NOFORMAT}"
cppcheck \
  --std=c++17 \
  $sources -i $project_dir/.cache
