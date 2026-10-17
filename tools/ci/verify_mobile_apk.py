#!/usr/bin/env python3
"""Fail CI rather than upload an APK missing its native or installer runtime."""
import sys
import zipfile

REQUIRED = (
    'AndroidManifest.xml', 'classes.dex', 'assets/pythonhome.zip',
    'lib/arm64-v8a/libgame.so', 'lib/arm64-v8a/libmain.so',
    'lib/arm64-v8a/libreserve.so', 'lib/arm64-v8a/libinstaller.so',
    'lib/arm64-v8a/libpython3.10.so', 'lib/arm64-v8a/libnodtool.so',
    'lib/arm64-v8a/wrap.sh',
)


def verify(path):
    with zipfile.ZipFile(path) as apk:
        bad = apk.testzip()
        if bad:
            raise SystemExit('APK entry failed CRC check: ' + bad)
        for name in REQUIRED:
            try:
                entry = apk.getinfo(name)
            except KeyError:
                raise SystemExit('APK is missing required entry: ' + name) from None
            if entry.file_size == 0:
                raise SystemExit('APK contains empty required entry: ' + name)
        for entry in apk.infolist():
            if entry.filename.endswith('.so') and entry.compress_type != zipfile.ZIP_STORED:
                raise SystemExit('Native library is compressed: ' + entry.filename)
    print('APK runtime files verified:', path)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit('Usage: verify_mobile_apk.py PATH.apk')
    verify(sys.argv[1])
