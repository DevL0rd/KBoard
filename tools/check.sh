#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
BUILD_DIR="${KBOARD_BUILD_DIR:-build}"
QMLLINT="${QMLLINT:-/usr/lib/qt6/bin/qmllint}"
SCRIPTS=(install.sh uninstall.sh packaging/lib.sh packaging/dependencies.sh packaging/system-update packaging/kboard-input-method packaging/kboard-update.zypp tools/check.sh tools/test-install.sh tests/install/*.sh)
PYTHON_SCRIPTS=(packaging/replace-keyboard-toggle)
QML_NOISE=(unqualified import unresolved-type missing-type missing-property incompatible-type unused-imports)

step() {
    printf '\n==> %s\n' "$1"
}

tracked() {
    git ls-files --cached --others --exclude-standard -- "$@"
}

step "file length (at most 400 lines per file)"
mapfile -t length_checked < <(tracked '*.cpp' '*.h' '*.py' '*.qml' '*.sh' '*.js'; printf '%s\n' "${SCRIPTS[@]}" "${PYTHON_SCRIPTS[@]}")
too_long=$(wc -l "${length_checked[@]}" | awk '$2 != "total" && $1 > 400 { print $1, $2 }' | sort -u)
if [[ -n $too_long ]]; then
    echo "$too_long"
    exit 1
fi

step "configure"
cmake -S . -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTING=ON -DKBOARD_WERROR=ON >/dev/null

step "build"
cmake --build "$BUILD_DIR"

step "unit tests"
ctest --test-dir "$BUILD_DIR" --output-on-failure --no-tests=error -L unit

step "install and uninstall (throwaway HOME)"
KBOARD_BUILD_DIR="$(realpath -m "${BUILD_DIR}-install")" tools/test-install.sh

mapfile -t cpp_files < <(tracked 'src/*.cpp' 'src/*.h' 'tests/*.cpp' 'tests/*.h')

step "clang-format"
clang-format --dry-run --Werror "${cpp_files[@]}"

step "clang build (compile database for clang-tidy)"
CLANG_BUILD_DIR="${BUILD_DIR}-clang"
cmake -S . -B "$CLANG_BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTING=ON >/dev/null
cmake --build "$CLANG_BUILD_DIR"

step "clang-tidy (complexity, bugprone, performance)"
mapfile -t tidy_files < <(printf '%s\n' "${cpp_files[@]}" | grep -E '^src/.*\.cpp$')
run-clang-tidy -quiet -p "$CLANG_BUILD_DIR" "${tidy_files[@]}"

step "unused functions (xunused)"
command -v xunused >/dev/null || { echo "xunused is not installed: https://github.com/mgehre/xunused"; exit 1; }
mapfile -t all_sources < <(tracked 'src/*.cpp' 'tests/*.cpp')
generated_root=$(realpath "$CLANG_BUILD_DIR")
unused=$(xunused -p "$CLANG_BUILD_DIR" --extra-arg=-resource-dir="$(clang -print-resource-dir)" "${all_sources[@]}" 2>&1 | grep -E "warning: Function|Failed to run" | grep -Fv "$generated_root/" || true)
if [[ -n $unused ]]; then
    echo "$unused"
    exit 1
fi

step "qmllint"
qml_noise=()
for category in "${QML_NOISE[@]}"; do
    qml_noise+=("--$category" disable)
done
mapfile -t qml_files < <(tracked '*.qml')
"$QMLLINT" -I "$BUILD_DIR/qml" --max-warnings 0 "${qml_noise[@]}" "${qml_files[@]}"

step "shell scripts (bash -n, shellcheck)"
for script in "${SCRIPTS[@]}"; do
    bash -n "$script"
done
for script in "${PYTHON_SCRIPTS[@]}"; do
    python3 -c "import ast, sys; ast.parse(open(sys.argv[1]).read(), sys.argv[1])" "$script"
done
shellcheck -x --source-path=SCRIPTDIR "${SCRIPTS[@]}"

step "typos"
typos

step "duplicate code (jscpd)"
npx --yes jscpd@5 src tests tools packaging

if [[ "${1:-}" == "--nested" ]]; then
    step "nested KWin tests"
    KBOARD_BUILD_DIR="$(realpath "$BUILD_DIR")" python3 tests/nested/run_all.py
    step "nested widget test"
    python3
fi

printf '\nAll checks passed.\n'
