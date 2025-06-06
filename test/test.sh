# compile: chmod +x test.sh
# run: ./test.sh xxxx.cpp

set -euo pipefail

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <source.cpp>"
  exit 1
fi

SRC="$1"
if [[ "${SRC##*.}" != "cpp" ]]; then
  echo "Error: must pass a .cpp file"
  exit 1
fi

BIN="/tmp/$(basename "${SRC%.cpp}")"

clang++ -std=c++20 -mavx2 -mfma -march=native -lpthread -ffast-math -O2 "$SRC" -o "$BIN"
exec "$BIN"
