"""Host lifecycle tests. Supply Khronos header locations through the variables
GALAXY_VULKAN_INCLUDE, GALAXY_GL_INCLUDE and GALAXY_EGL_INCLUDE, or install the
Vulkan, GLES and EGL development headers in the system include path. No GPU is
used: the real renderer/UI are linked against the probes' fake platform APIs.
"""
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
TESTS = ROOT / 'tools/tests'


class MobilePerformanceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.compiler = shutil.which('g++')
        if not cls.compiler:
            raise unittest.SkipTest('g++ is required for the Linux host probes')
        cls.includes = [TESTS / 'android_stubs', ROOT / 'platform', ROOT / 'platform/src', ROOT / 'platform/include']
        for var in ('GALAXY_VULKAN_INCLUDE', 'GALAXY_GL_INCLUDE', 'GALAXY_EGL_INCLUDE'):
            if os.environ.get(var):
                cls.includes.append(Path(os.environ[var]))
        check = subprocess.run([cls.compiler, '-std=c++20', '-x', 'c++', '-fsyntax-only',
                                *['-I' + str(p) for p in cls.includes], '-'],
                               input='#include <vulkan/vulkan.h>\n#include <GLES3/gl32.h>\n#include <EGL/egl.h>\n',
                               text=True, capture_output=True)
        if check.returncode:
            raise unittest.SkipTest('Khronos headers unavailable; set GALAXY_{VULKAN,GL,EGL}_INCLUDE')

    def probe(self, source, probe, extra=()):
        with tempfile.TemporaryDirectory() as tmp:
            binary = Path(tmp) / 'probe'
            command = [self.compiler, '-std=c++20', '-O0', '-ffunction-sections', '-fdata-sections',
                       *['-I' + str(p) for p in self.includes], *extra,
                       *([str(ROOT / source)] if source else []), str(TESTS / probe), '-Wl,--gc-sections', '-pthread', '-o', str(binary)]
            built = subprocess.run(command, text=True, capture_output=True)
            self.assertEqual(built.returncode, 0, built.stderr)
            ran = subprocess.run([str(binary)], text=True, capture_output=True)
            self.assertEqual(ran.returncode, 0, ran.stdout + ran.stderr)
            self.assertIn('PASS:', ran.stdout)

    def test_vulkan_buffer_and_texture_lifecycles(self):
        self.probe('platform/src/gx/vk_renderer.cpp', 'vk_upload_probe.cpp')

    def test_resolution_saved_ids_and_render_height(self):
        self.probe(None, 'mobile_render_options_probe.cpp')

    def test_vulkan_draw_state_cache(self):
        self.probe(None, 'vk_draw_state_probe.cpp')

    def test_hardware_sampling_obeys_hud_mode(self):
        self.probe('platform/mobile/mobile_ui.cpp', 'ui_metrics_probe.cpp',
                   ('-D__ANDROID__', '-include', str(TESTS / 'android_stubs/host_compat.h'),
                    '-Wl,--wrap=fopen', '-Wl,--wrap=clock_gettime'))


if __name__ == '__main__':
    unittest.main()
