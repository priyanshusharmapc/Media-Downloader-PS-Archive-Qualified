"""Injected subprocess failure cases for real Archive CLI normalization paths.

The fake FFmpeg executable is test-only. Real FFprobe checks the staged bytes;
existing integration tests retain real FFmpeg coverage for successful media.
"""
import json
import os
from pathlib import Path
import shutil
import subprocess


class NormalizationCleanupCases:
    def normalization_case(self, kind, mode):
        self.scan()
        suffix = '.exe' if os.name == 'nt' else ''
        ffmpeg = self.package / 'bin' / ('ffmpeg' + suffix)
        # Obtain an unsupported-but-decodable audio container using real FFmpeg
        # before replacing the process boundary with the deterministic fixture.
        if kind == 'audio':
            incompatible = self.base / 'incompatible.m4a'
            result = subprocess.run([str(ffmpeg), '-nostdin', '-v', 'error', '-n',
                                     '-i', str(self.audio), '-c:a', 'alac', str(incompatible)],
                                    capture_output=True, text=True, env=self.env, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
        else:
            incompatible = self.incompatible
        self.plan[kind + '_file'] = str(incompatible)
        self.plan['normalization_mode'] = mode
        self.plan['normalized_video_file'] = str(self.video)
        self.plan['normalized_audio_file'] = str(self.audio)
        self.plan['normalization_title'] = 'Known historical title'
        self.write_plan()
        ffmpeg.unlink()
        shutil.copy2(self.package / 'bin' / ('yt-dlp' + suffix), ffmpeg)
        # This unrelated temporary file and every downloaded original are not
        # owned by normalization and must survive both failure and success.
        sentinel = self.root / 'Temp' / 'unrelated.tmp'
        sentinel.write_bytes(b'not normalization-owned')
        result = self.command('sync-item', 'https://www.youtube.com/watch?v=abc123DEF45',
                              expect=0 if mode == 'success' else 1)
        item = self.first()
        expected = 'complete' if mode == 'success' else 'failed'
        self.assertEqual(item[kind]['state'], expected, result.stdout + result.stderr)
        calls = [json.loads(line) for line in Path(self.plan['calls_path']).read_text().splitlines()]
        normalizations = [call for call in calls if '-n' in call and 'normalize-' in call[-1]]
        self.assertEqual(len(normalizations), 1, calls)
        self.assertFalse(Path(normalizations[0][-1]).exists(), 'owned staging media leaked')
        self.assertEqual(list((self.root / 'Temp').glob('normalize-*')), [], 'owned staging directory leaked')
        self.assertEqual(sentinel.read_bytes(), b'not normalization-owned')
        folder = self.root / ('Video' if kind == 'video' else 'Audio')
        originals = [p for p in folder.iterdir() if '[download-' in p.name]
        self.assertEqual(len(originals), 1)
        self.assertEqual(originals[0].read_bytes(), incompatible.read_bytes(), 'original was changed')
        if mode == 'collision':
            collisions = [p for p in folder.iterdir() if '[NORMALIZED]' in p.name]
            self.assertEqual(len(collisions), 1)
            self.assertEqual(collisions[0].read_bytes(), b'existing destination')
        elif mode == 'success':
            self.assertTrue((self.root / item[kind]['path']).is_file())
            self.assertIn('[NORMALIZED]', item[kind]['path'])

    def test_normalization_video_encode_failure_cleanup(self):
        self.normalization_case('video', 'encode-fail')

    def test_normalization_audio_encode_failure_cleanup(self):
        self.normalization_case('audio', 'encode-fail')

    def test_normalization_video_verification_failure_cleanup(self):
        self.normalization_case('video', 'invalid-stage')

    def test_normalization_audio_verification_failure_cleanup(self):
        self.normalization_case('audio', 'invalid-stage')

    def test_normalization_video_publication_failure_cleanup(self):
        self.normalization_case('video', 'collision')

    def test_normalization_audio_publication_failure_cleanup(self):
        self.normalization_case('audio', 'collision')

    def test_normalization_video_success_cleanup(self):
        self.normalization_case('video', 'success')

    def test_normalization_audio_success_cleanup(self):
        self.normalization_case('audio', 'success')
