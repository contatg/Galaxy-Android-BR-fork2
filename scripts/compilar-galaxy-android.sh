#!/usr/bin/env bash
# Called by Compilar-Galaxy-Android.ps1; reuses existing WSL toolchains.
set -Eeuo pipefail
jobs="${1:-2}"
output_dir="${2:-}"
source_ref="${3:-otimizacoes-android}"
[[ "$jobs" =~ ^[0-9]+$ ]] && (( jobs >= 1 && jobs <= 16 )) || { echo 'Jobs deve ser entre 1 e 16.' >&2; exit 1; }
[[ "$(uname -m)" == x86_64 ]] || { echo 'O build exige WSL/Linux x86_64.' >&2; exit 1; }
for program in git cmake ninja python3 javac keytool; do
    command -v "$program" >/dev/null || { echo "Ferramenta nao encontrada no WSL: $program" >&2; exit 1; }
done
repo_url='https://github.com/contatg/Galaxy-Android-BR-fork2.git'
repo_dir="${GALAXY_BUILD_ROOT:-$HOME/Galaxy-Android-Otimizado}"
if [[ ! -d "$repo_dir/.git" ]]; then
    [[ ! -e "$repo_dir" ]] || { echo "A pasta $repo_dir ja existe e nao e um clone. Escolha GALAXY_BUILD_ROOT." >&2; exit 1; }
    git clone --depth 1 --branch otimizacoes-android "$repo_url" "$repo_dir"
fi
cd "$repo_dir"
[[ "$(git remote get-url origin)" == "$repo_url" ]] || { echo 'A pasta de build pertence a outro repositorio.' >&2; exit 1; }
[[ -z "$(git status --porcelain --untracked-files=no)" ]] || { echo 'Ha alteracoes locais nos fontes. Salve-as antes de atualizar este clone.' >&2; exit 1; }
git fetch --depth 1 -- origin "$source_ref"
git checkout --detach FETCH_HEAD
mkdir -p build-logs out
log_file="$PWD/build-logs/local-$(date +%Y%m%d-%H%M%S).log"
exec > >(tee "$log_file") 2>&1
trap 'result=$?; if (( result != 0 )); then echo "FALHA (codigo $result). Log: $log_file" >&2; fi; exit "$result"' EXIT

galaxy_sdk=''
for candidate in "${ANDROID_HOME:-}" "${ANDROID_SDK_ROOT:-}" "$HOME/Android/Sdk" "$HOME/Android/sdk" "$HOME/android-sdk" /opt/android-sdk /opt/android-sdk-linux /usr/local/lib/android/sdk; do
    if [[ -n "$candidate" && -d "$candidate/platforms" && -d "$candidate/build-tools" ]]; then
        galaxy_sdk="$candidate"; break
    fi
done
[[ -n "$galaxy_sdk" ]] || { echo 'SDK Linux nao encontrado. Defina ANDROID_HOME no WSL apontando para o SDK ja instalado.' >&2; exit 1; }
export ANDROID_HOME="$galaxy_sdk"
export ANDROID_SDK_ROOT="$galaxy_sdk"
galaxy_ndk=''
for candidate in "${ANDROID_NDK_HOME:-}" "${ANDROID_NDK_ROOT:-}" "${ANDROID_NDK:-}"; do
    if [[ -n "$candidate" && -x "$candidate/toolchains/llvm/prebuilt/linux-x86_64/bin/clang" ]]; then galaxy_ndk="$candidate"; break; fi
done
if [[ -z "$galaxy_ndk" && -d "$galaxy_sdk/ndk" ]]; then
    while IFS= read -r candidate; do
        if [[ -x "$candidate/toolchains/llvm/prebuilt/linux-x86_64/bin/clang" && -f "$candidate/sources/third_party/shaderc/Android.mk" ]]; then galaxy_ndk="$candidate"; break; fi
    done < <(find "$galaxy_sdk/ndk" -mindepth 1 -maxdepth 1 -type d | sort -Vr)
fi
[[ -n "$galaxy_ndk" ]] || { echo 'NDK Linux nao encontrado. Defina ANDROID_NDK_HOME no WSL apontando para seu NDK 27 ou mais recente.' >&2; exit 1; }
export ANDROID_NDK_HOME="$galaxy_ndk"
export ANDROID_NDK_ROOT="$galaxy_ndk"
echo "Fontes: $PWD"
echo "SDK: $ANDROID_HOME"
echo "NDK: $ANDROID_NDK_HOME"
echo "Log: $log_file"
cmake --version | head -1
ninja --version
javac -version

shaderc="$ANDROID_NDK_HOME/sources/third_party/shaderc"
if [[ ! -s "$shaderc/libs/c++_static/arm64-v8a/libshaderc.a" ]]; then
    [[ -d "$shaderc" ]] || { echo 'Este NDK nao inclui os fontes de shaderc; use o NDK 27 existente.' >&2; exit 1; }
    "$ANDROID_NDK_HOME/ndk-build" -C "$shaderc" NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk \
        APP_ABI=arm64-v8a APP_STL=c++_static APP_PLATFORM=android-30 libshaderc_combined -j"$jobs"
fi
cmake -S . -B build-mobile -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-30 \
    -DTARGET_MOBILE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-mobile --target game main reserve installer --parallel "$jobs"
# Reuse this clone's test key on subsequent builds, allowing local updates.
export GALAXY_ALLOW_DEBUGGABLE=1
export GALAXY_ALLOW_NEW_KEY=1
if [[ -z "${GALAXY_KEYSTORE:-}" && -f "$PWD/out/dev.keystore" ]]; then
    export GALAXY_KEYSTORE="$PWD/out/dev.keystore"
    export GALAXY_EXPECTED_SIGNER_SHA256=''
fi
python3 package_mobile_apk.py
python3 tools/ci/verify_mobile_apk.py out-mobile/GalaxyAndroid.apk
if [[ -n "$output_dir" ]]; then
    mkdir -p "$output_dir"
    cp out-mobile/GalaxyAndroid.apk "$output_dir/GalaxyAndroid-Otimizado.apk"
    echo "APK: $output_dir/GalaxyAndroid-Otimizado.apk"
else
    echo "APK: $PWD/out-mobile/GalaxyAndroid.apk"
fi
echo "Log: $log_file"
