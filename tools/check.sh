#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
BUILD_DIR="${KBOARD_BUILD_DIR:-build}"
QMLLINT="${QMLLINT:-/usr/lib/qt6/bin/qmllint}"
SCRIPTS=(install.sh uninstall.sh packaging/lib.sh packaging/build-box.sh packaging/check-build-dependencies packaging/dependencies.sh packaging/system-update packaging/login-update packaging/kboard-input-method packaging/kboard-update.zypp tools/check.sh tools/test-install.sh tests/install/*.sh tests/ci/*.sh)
PYTHON_SCRIPTS=(packaging/replace-keyboard-toggle)
QML_NOISE=(unqualified import unresolved-type missing-type missing-property incompatible-type unused-imports)
STAGE=all
NESTED=false
while (($#)); do
    case "$1" in
        --nested) NESTED=true ;;
        --lint-only) STAGE=lint ;;
        --unit-only) STAGE=unit ;;
        --install-only) STAGE=install ;;
        --tidy-only) STAGE=tidy ;;
        --xunused-only) STAGE=xunused ;;
        *)
            echo "usage: tools/check.sh [--nested] [--lint-only | --unit-only | --install-only | --tidy-only | --xunused-only]" >&2
            exit 2
            ;;
    esac
    shift
done

want() {
    [[ $STAGE == all || $STAGE == "$1" ]]
}

step() {
    printf '\n==> %s\n' "$1"
}

tracked() {
    git ls-files --cached --others --exclude-standard -- "$@"
}

mapfile -t cpp_files < <(tracked 'src/*.cpp' 'src/*.h' 'tests/*.cpp' 'tests/*.h')

if want lint; then
    step "file length (at most 400 lines per file)"
    mapfile -t length_checked < <(tracked '*.cpp' '*.h' '*.py' '*.qml' '*.sh' '*.js'; printf '%s\n' "${SCRIPTS[@]}" "${PYTHON_SCRIPTS[@]}")
    too_long=$(wc -l "${length_checked[@]}" | awk '$2 != "total" && $1 > 400 { print $1, $2 }' | sort -u)
    if [[ -n $too_long ]]; then
        echo "$too_long"
        exit 1
    fi
fi

if want unit; then
    step "configure"
    cmake -S . -B "$BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTING=ON -DKBOARD_WERROR=ON >/dev/null

    step "build"
    cmake --build "$BUILD_DIR"

    step "unit tests"
    ctest --test-dir "$BUILD_DIR" --output-on-failure --no-tests=error -L unit
fi

if want install; then
    step "install and uninstall (throwaway HOME)"
    KBOARD_BUILD_DIR="$(realpath -m "${BUILD_DIR}-install")" tools/test-install.sh
fi

if want lint; then
    step "clang-format"
    clang-format --dry-run --Werror "${cpp_files[@]}"
fi

CLANG_BUILD_DIR="${BUILD_DIR}-clang"
if want tidy || want xunused; then
    step "clang build (compile database for clang-tidy)"
    cmake -S . -B "$CLANG_BUILD_DIR" -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DBUILD_TESTING=ON >/dev/null
    cmake --build "$CLANG_BUILD_DIR"
fi

if want tidy; then
    step "clang-tidy (complexity, bugprone, performance)"
    mapfile -t tidy_files < <(printf '%s\n' "${cpp_files[@]}" | grep -E '^src/.*\.cpp$')
    run-clang-tidy -quiet -p "$CLANG_BUILD_DIR" "${tidy_files[@]}"
fi

if want xunused; then
    step "unused functions (xunused)"
    command -v xunused >/dev/null || { echo "xunused is not installed: https://github.com/mgehre/xunused"; exit 1; }
    mapfile -t all_sources < <(tracked 'src/*.cpp' 'tests/*.cpp')
    generated_root=$(realpath "$CLANG_BUILD_DIR")
    unused=$(xunused -p "$CLANG_BUILD_DIR" --extra-arg=-resource-dir="$(clang -print-resource-dir)" "${all_sources[@]}" 2>&1 | grep -E "warning: Function|Failed to run" | grep -Fv "$generated_root/" || true)
    if [[ -n $unused ]]; then
        echo "$unused"
        exit 1
    fi
fi

if want unit; then
    step "qmllint"
    qml_noise=()
    for category in "${QML_NOISE[@]}"; do
        qml_noise+=("--$category" disable)
    done
    mapfile -t qml_files < <(tracked '*.qml')
    "$QMLLINT" -I "$BUILD_DIR/qml" --max-warnings 0 "${qml_noise[@]}" "${qml_files[@]}"
fi

if want lint; then
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
fi

if $NESTED; then
    step "nested KWin tests"
    KBOARD_BUILD_DIR="$(realpath "$BUILD_DIR")" python3 tests/nested/run_all.py
    step "nested widget test"
    python3
fi

printf '\nAll checks passed.\n'
