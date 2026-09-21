"""Offline application integration: real CLI, real FFmpeg, deterministic yt-dlp boundary."""
from __future__ import annotations
import argparse
import csv
import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import subprocess
import sys
import tempfile
import time
import unittest
import zipfile
from archive_normalization_cleanup import NormalizationCleanupCases

parser = argparse.ArgumentParser()
parser.add_argument('--cli', required=True, type=Path)
parser.add_argument('--fake-tool', required=True, type=Path)
opts, test_args = parser.parse_known_args()
CLI, FAKE = opts.cli.resolve(), opts.fake_tool.resolve()
FFMPEG = os.environ.get('ARCHIVE_TEST_FFMPEG') or shutil.which('ffmpeg')
FFPROBE = os.environ.get('ARCHIVE_TEST_FFPROBE') or shutil.which('ffprobe')
if not FFMPEG or not FFPROBE:
    raise SystemExit('Integration tests require real FFmpeg and FFprobe on PATH or ARCHIVE_TEST_FFMPEG/FFPROBE')
VIDEO_ID = 'abc123DEF45'
VIDEO_URL = 'https://www.youtube.com/watch?v=' + VIDEO_ID
SOURCE_URL = 'https://www.youtube.com/playlist?list=PLAUDIT'

def sha(path: Path) -> str:
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()

def run(command, *, env=None, timeout=45):
    return subprocess.run([str(x) for x in command], capture_output=True, text=True, env=env, timeout=timeout)

def cleanup_path(path: Path, attempts: int = 120) -> None:
    last = None
    for attempt in range(attempts):
        try:
            shutil.rmtree(path)
            return
        except (PermissionError, OSError) as exc:
            last = exc
            if attempt + 1 == attempts:
                raise
            time.sleep(0.25)
    if last:
        raise last

class ArchiveIntegration(NormalizationCleanupCases, unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fixture = Path(tempfile.mkdtemp(prefix='archive-media-fixtures-'))
        cls.video = cls.fixture / 'good.mp4'
        cls.audio = cls.fixture / 'good.m4a'
        cls.incompatible = cls.fixture / 'normalize-me.mkv'
        commands = [
            [FFMPEG, '-nostdin', '-v', 'error', '-y', '-f', 'lavfi', '-i', 'testsrc2=size=160x90:rate=8', '-f', 'lavfi', '-i', 'sine=sample_rate=48000', '-t', '1', '-c:v', 'libx264', '-threads', '1', '-pix_fmt', 'yuv420p', '-c:a', 'aac', '-movflags', '+faststart', cls.video],
            [FFMPEG, '-nostdin', '-v', 'error', '-y', '-i', cls.video, '-vn', '-c:a', 'copy', cls.audio],
            [FFMPEG, '-nostdin', '-v', 'error', '-y', '-i', cls.video, '-c:v', 'ffv1', '-threads', '1', '-c:a', 'pcm_s16le', cls.incompatible],
        ]
        for command in commands:
            result = run(command)
            if result.returncode:
                raise RuntimeError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cleanup_path(cls.fixture)

    def setUp(self):
        self.base = Path(tempfile.mkdtemp(prefix='archive integration '))
        self.addCleanup(lambda: cleanup_path(self.base))
        self.root = self.base / 'archive root with spaces'
        self.package = self.base / 'portable package'
        self.package.mkdir()
        self.cli = self.package / CLI.name
        shutil.copy2(CLI, self.cli)
        (self.package / 'bin').mkdir()
        shutil.copy2(FAKE, self.package / 'bin' / ('yt-dlp.exe' if os.name == 'nt' else 'yt-dlp'))
        # Normal Archive execution is sealed to package-owned tool executables.
        # Keep the real tool directories on PATH only to prove they are not used
        # as a silent fallback when a bundled executable is removed.
        shutil.copy2(FFMPEG, self.package / 'bin' / ('ffmpeg.exe' if os.name == 'nt' else 'ffmpeg'))
        shutil.copy2(FFPROBE, self.package / 'bin' / ('ffprobe.exe' if os.name == 'nt' else 'ffprobe'))
        self.env = dict(os.environ, ARCHIVE_TEST_PLAN=str(self.base / 'plan.json'))
        self.env['PATH'] = str(Path(FFMPEG).parent) + os.pathsep + str(Path(FFPROBE).parent) + os.pathsep + self.env.get('PATH', '')
        self.plan = {
            'video_id': VIDEO_ID, 'video_file': str(self.video), 'audio_file': str(self.audio),
            'calls_path': str(self.base / 'calls.jsonl'),
            'discovery': {'id': 'PLAUDIT', 'entries': [
                {'id': VIDEO_ID, 'title': 'Known historical title', 'playlist_index': 1, 'availability': 'public'},
                {'id': 'xyz987QWE65', 'title': 'Second historical title', 'playlist_index': 2, 'availability': 'public'},
            ]},
        }
        self.write_plan()

    def write_plan(self):
        Path(self.env['ARCHIVE_TEST_PLAN']).write_text(json.dumps(self.plan), encoding='utf-8')

    def command(self, command, *args, expect=0):
        result = run([self.cli, command, self.root, *args], env=self.env)
        self.assertEqual(result.returncode, expect, result.stdout + '\n' + result.stderr)
        return result

    def scan(self):
        return self.command('scan', SOURCE_URL, 'Test playlist')

    def canonical(self):
        return json.loads((self.root / 'State/ArchiveMode/items.json').read_text(encoding='utf-8'))

    def first(self):
        return next(x for x in self.canonical() if x['key'] == 'youtube:' + VIDEO_ID)

    def media_hashes(self):
        return {str(p.relative_to(self.root)): sha(p) for directory in ('Video', 'Audio') for p in (self.root / directory).rglob('*') if p.is_file()}

    def make_package(self, *, name='recovery-package', bad_audio=False, metadata_only=False, id_only=False):
        directory = self.root / 'State/ArchiveMode/Imports/Pending' / name
        (directory / 'files').mkdir(parents=True)
        manifest = {
            'schema_version': 1, 'package_id': name,
            'target': {'youtube_id': VIDEO_ID} if id_only else {'item_key': 'youtube:' + VIDEO_ID, 'youtube_id': VIDEO_ID},
            'provenance': {'method': 'old_local_backup', 'confidence': 'high'},
            'metadata': {'canonical_title': 'Recovered title', 'user_tags': ['recovered']},
        }
        if not metadata_only:
            shutil.copy2(self.incompatible, directory / 'files/video.mkv')
            if bad_audio:
                (directory / 'files/audio.m4a').write_bytes(b'not-media')
            else:
                shutil.copy2(self.audio, directory / 'files/audio.m4a')
            manifest['representations'] = {'video': {'file': 'files/video.mkv'}, 'audio': {'file': 'files/audio.m4a'}}
        (directory / 'manifest.json').write_text(json.dumps(manifest), encoding='utf-8')
        return directory

    def block_catalog(self):
        """Inject a generated-file failure without touching authoritative state."""
        catalog = self.root / 'Playlists/PLAUDIT/catalog.csv'
        catalog.unlink()
        catalog.mkdir()
        return catalog

    def durable_bytes(self):
        paths = [self.root / 'State/ArchiveMode/items.json',
                 self.root / 'State/ArchiveMode/sources.json',
                 self.root / 'Playlists/PLAUDIT/items.json',
                 self.root / 'Playlists/PLAUDIT/history.jsonl']
        paths += list((self.root / 'State/ArchiveMode/Imports/Accepted').rglob('*'))
        return {str(p.relative_to(self.root)): p.read_bytes() for p in paths if p.is_file()}

    def test_scan_projection_failure_reports_committed_and_repairs_only_reports(self):
        self.scan()
        catalog = self.block_catalog()
        self.plan['discovery']['entries'][0]['title'] = 'Committed changed title'
        self.write_plan()
        result = self.command('scan', SOURCE_URL, expect=4)
        self.assertIn('committed=true', result.stdout)
        self.assertIn('projections=dirty', result.stdout)
        self.assertIn('active=2', result.stdout)
        self.assertIn('State committed', result.stderr)
        self.assertEqual(self.first()['title'], 'Committed changed title')
        marker = self.root / 'State/ArchiveMode/projections-dirty.json'
        self.assertTrue(marker.exists())
        committed = self.durable_bytes()
        catalog.rmdir()
        self.command('rebuild-projections')
        self.assertFalse(marker.exists())
        self.assertIn('Committed changed title', catalog.read_text())
        self.assertEqual(self.durable_bytes(), committed)
        self.command('rebuild-projections')
        self.assertEqual(self.durable_bytes(), committed)

    def test_recovery_projection_failure_counts_accepted_and_never_repromotes(self):
        self.scan()
        package = self.make_package()
        manifest = (package / 'manifest.json').read_bytes()
        catalog = self.block_catalog()
        result = self.command('ingest-pending', expect=4)
        self.assertIn('accepted=1', result.stdout)
        self.assertIn('warnings=1', result.stdout)
        self.assertIn('State committed', result.stderr)
        self.assertNotIn('retryable failure', result.stderr)
        accepted = self.root / 'State/ArchiveMode/Imports/Accepted/recovery-package'
        self.assertFalse(package.exists())
        self.assertEqual((accepted / 'manifest.json').read_bytes(), manifest)
        self.assertEqual(self.first()['video']['state'], 'complete')
        committed = self.durable_bytes()
        media = self.media_hashes()
        catalog.rmdir()
        self.command('rebuild-projections')
        self.assertIn('accepted=0', self.command('ingest-pending').stdout)
        self.assertEqual(self.durable_bytes(), committed)
        self.assertEqual(self.media_hashes(), media)

    def test_playlist_binding_requires_active_occurrence(self):
        scan = self.scan()
        self.assertIn('source_key=PLAUDIT', scan.stdout)
        matching = self.command('playlist-binding', SOURCE_URL, VIDEO_URL)
        self.assertIn('member=true', matching.stdout)
        self.assertIn('source_key=PLAUDIT', matching.stdout)
        self.assertIn('item_key=youtube:' + VIDEO_ID, matching.stdout)
        self.assertIn('active_occurrences=1', matching.stdout)
        self.assertIn('entry_key=', matching.stdout)

        unrelated = 'https://www.youtube.com/watch?v=ZZZ999yyy88'
        rejected = self.command('playlist-binding', SOURCE_URL, unrelated, expect=1)
        self.assertIn('member=false', rejected.stdout)
        self.assertIn('not an active occurrence', rejected.stderr)

        # A removed historical occurrence is evidence, not active membership.
        self.plan['discovery']['entries'] = [
            {'id': 'xyz987QWE65', 'title': 'Second historical title', 'playlist_index': 1, 'availability': 'public'}
        ]
        self.write_plan()
        self.scan()
        removed = self.command('playlist-binding', SOURCE_URL, VIDEO_URL, expect=1)
        self.assertIn('member=false', removed.stdout)

        # Duplicate active occurrences still prove membership without guessing
        # which occurrence is the canonical target.
        entry = {'id': VIDEO_ID, 'title': 'Known historical title', 'playlist_index': 1, 'availability': 'public'}
        self.plan['discovery']['entries'] = [entry, dict(entry, playlist_index=2)]
        self.write_plan()
        self.scan()
        duplicate = self.command('playlist-binding', SOURCE_URL, VIDEO_URL)
        self.assertIn('member=true', duplicate.stdout)
        self.assertIn('active_occurrences=2', duplicate.stdout)

    def test_missing_sealed_ffprobe_refuses_path_substitute(self):
        self.scan()
        bundled = self.package / 'bin' / ('ffprobe.exe' if os.name == 'nt' else 'ffprobe')
        self.assertTrue(bundled.exists())
        bundled.unlink()
        # A valid FFprobe remains visible on PATH. Qualified/default execution
        # must still fail instead of crossing the package trust boundary.
        result = self.command('sync-item', VIDEO_URL, expect=1)
        self.assertIn('sync=FAIL', result.stderr)
        item = self.first()
        self.assertNotEqual(item['video']['state'], 'complete')
        self.assertNotEqual(item['audio']['state'], 'complete')

    def test_scan_sync_verify_and_idempotent_rerun(self):
        self.scan()
        self.command('sync-item', VIDEO_URL)
        self.command('verify-item', VIDEO_URL)
        before = self.media_hashes()
        canonical = self.canonical()
        calls = (self.base / 'calls.jsonl').read_bytes()
        self.command('sync-item', VIDEO_URL)
        self.assertEqual(before, self.media_hashes())
        self.assertEqual(canonical, self.canonical())
        self.assertEqual(calls, (self.base / 'calls.jsonl').read_bytes())
        for path in before:
            result = run([FFMPEG, '-nostdin', '-v', 'error', '-i', self.root / path, '-f', 'null', '-'])
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_truncated_faststart_media_is_rejected_and_repaired(self):
        self.scan()
        self.command('sync-item', VIDEO_URL)
        video = self.root / self.first()['video']['path']
        original = video.read_bytes()
        self.assertGreater(len(original), 4096)
        video.write_bytes(original[:max(4096, len(original) * 60 // 100)])
        damaged_hash = sha(video)
        probe = run([FFPROBE, '-v', 'error', '-show_streams', '-show_format', '-of', 'json', video])
        self.assertEqual(probe.returncode, 0, probe.stderr)
        self.command('verify-item', VIDEO_URL, expect=1)
        self.command('sync-item', VIDEO_URL)
        self.command('verify-item', VIDEO_URL)
        repaired = self.root / self.first()['video']['path']
        self.assertNotEqual(sha(repaired), damaged_hash)
        decoded = run([FFMPEG, '-nostdin', '-v', 'error', '-xerror', '-i', repaired, '-map', '0:v:0?', '-map', '0:a:0?', '-f', 'null', '-'])
        self.assertEqual(decoded.returncode, 0, decoded.stderr)

    def test_partial_scan_preserves_membership(self):
        self.scan()
        self.plan['discovery'] = {}
        self.write_plan()
        self.command('scan', SOURCE_URL, expect=3)
        playlist = json.loads((self.root / 'Playlists/PLAUDIT/items.json').read_text())
        self.assertEqual(len(playlist), 2)
        self.assertTrue(all(x['membership'] == 'active' for x in playlist))

    def test_normalization_preserves_original_bytes(self):
        self.plan['video_file'] = str(self.incompatible)
        self.write_plan()
        self.scan()
        self.command('sync-item', VIDEO_URL)
        originals = list((self.root / 'Video').glob('*.mkv'))
        self.assertEqual(len(originals), 1)
        self.assertEqual(sha(originals[0]), sha(self.incompatible))
        self.assertTrue(self.first()['video']['path'].endswith('.mp4'))
        self.command('verify-item', VIDEO_URL)

    def test_unavailable_media_is_durably_blocked_then_recovers(self):
        self.plan['discovery']['entries'][0]['availability'] = 'deleted'
        self.write_plan()
        self.scan()
        before_calls = (self.base / 'calls.jsonl').read_bytes() if (self.base / 'calls.jsonl').exists() else b''
        failed = self.command('sync-item', VIDEO_URL, expect=1)
        item = self.first()
        self.assertEqual(item['video']['state'], 'blocked_unavailable')
        self.assertEqual(item['audio']['state'], 'blocked_unavailable')
        self.assertIn('external recovery required', item['video']['error'])
        self.assertIn('external recovery required', item['audio']['error'])
        after_calls = (self.base / 'calls.jsonl').read_bytes() if (self.base / 'calls.jsonl').exists() else b''
        self.assertEqual(before_calls, after_calls, 'unavailable source invoked downloader')
        self.assertIn('source is unavailable', failed.stderr)

        # When discovery later proves availability again, the normal sync path
        # is reachable from blocked_unavailable and clears the blocking state.
        self.plan['discovery']['entries'][0]['availability'] = 'public'
        self.write_plan()
        self.scan()
        self.command('sync-item', VIDEO_URL)
        item = self.first()
        self.assertEqual(item['video']['state'], 'complete')
        self.assertEqual(item['audio']['state'], 'complete')

    def test_missing_complete_media_cannot_report_pass(self):
        self.scan()
        self.command('sync-item', VIDEO_URL)
        (self.root / self.first()['video']['path']).unlink()
        self.plan['download_exit'] = 7
        self.write_plan()
        self.command('sync-item', VIDEO_URL, expect=1)
        self.assertNotEqual(self.first()['video']['state'], 'complete')
        self.command('verify-item', VIDEO_URL, expect=1)
        self.plan['download_exit'] = 0
        self.write_plan()
        self.command('sync-item', VIDEO_URL)
        self.command('verify-item', VIDEO_URL)

    def test_real_import_and_immutable_accepted_evidence(self):
        self.scan()
        directory = self.make_package()
        self.command('validate', directory)
        self.command('ingest-pending')
        self.command('verify-item', VIDEO_URL)
        accepted = self.root / 'State/ArchiveMode/Imports/Accepted/recovery-package'
        receipt = json.loads((accepted / 'receipt.json').read_text())
        self.assertEqual(receipt['result'], 'accepted')
        self.assertEqual(len(receipt['promoted_paths']), 2)
        self.assertEqual(receipt['file_sha256']['manifest.json'], sha(accepted / 'manifest.json'))
        self.assertEqual(self.first()['video']['origin'], 'external_recovery')
        self.assertIn('recovered', self.first()['user_tags'])
        self.assertIn('external_recovery_accepted', (self.root / 'Playlists/PLAUDIT/history.jsonl').read_text())
        before = {str(p.relative_to(accepted)): sha(p) for p in accepted.rglob('*') if p.is_file()}
        self.make_package()
        self.command('ingest-pending', expect=1)
        self.assertEqual(before, {str(p.relative_to(accepted)): sha(p) for p in accepted.rglob('*') if p.is_file()})
        self.assertFalse((self.root / 'State/video-archive.txt').exists())
        self.assertFalse((self.root / 'State/audio-archive.txt').exists())

    def test_submitted_receipt_is_reserved_and_rejection_preserves_every_byte(self):
        self.scan()
        directory = self.make_package(name='reserved-receipt')
        submitted_receipt = b'operator supplied historical receipt\\n'
        (directory / 'receipt.json').write_bytes(submitted_receipt)
        before = {str(p.relative_to(directory)): p.read_bytes()
                  for p in directory.rglob('*') if p.is_file()}
        canonical_before = (self.root / 'State/ArchiveMode/items.json').read_bytes()

        result = self.command('ingest-pending', expect=1)
        self.assertIn('receipt.json is reserved', result.stderr)
        self.assertFalse(directory.exists())

        rejected_root = self.root / 'State/ArchiveMode/Imports/Rejected'
        moved = [p for p in rejected_root.iterdir()
                 if p.is_dir() and p.name.startswith('reserved-receipt-')]
        self.assertEqual(len(moved), 1)
        after = {str(p.relative_to(moved[0])): p.read_bytes()
                 for p in moved[0].rglob('*') if p.is_file()}
        self.assertEqual(after, before)
        self.assertEqual((moved[0] / 'receipt.json').read_bytes(), submitted_receipt)

        receipt_path = Path(str(moved[0]) + '.receipt.json')
        self.assertTrue(receipt_path.is_file())
        generated = json.loads(receipt_path.read_text())
        self.assertEqual(generated['result'], 'rejected')
        self.assertIn('receipt.json is reserved', generated['reason'])
        self.assertEqual((self.root / 'State/ArchiveMode/items.json').read_bytes(),
                         canonical_before)

    def test_failed_second_representation_is_atomic_and_retryable(self):
        self.scan()
        directory = self.make_package(bad_audio=True)
        before = self.canonical()
        self.command('ingest-pending', expect=1)
        self.assertEqual(before, self.canonical())
        self.assertEqual({}, self.media_hashes())
        self.assertTrue(directory.exists())
        shutil.copy2(self.audio, directory / 'files/audio.m4a')
        self.command('ingest-pending')
        self.command('verify-item', VIDEO_URL)

    def test_metadata_only_id_only_import(self):
        self.scan()
        directory = self.make_package(metadata_only=True, id_only=True)
        self.command('validate', directory)
        self.command('ingest-pending')
        self.assertEqual(self.first()['title'], 'Recovered title')
        self.assertEqual(self.first()['video']['state'], 'missing')
        self.assertEqual({}, self.media_hashes())

    def test_archive_skip_retry_argument_order(self):
        self.plan['archive_skip'] = True
        self.write_plan()
        self.scan()
        self.command('sync-item', VIDEO_URL)
        calls = [json.loads(x) for x in (self.base / 'calls.jsonl').read_text().splitlines()]
        retries = [x for x in calls if '--no-download-archive' in x]
        self.assertEqual(len(retries), 2)
        for args in retries:
            self.assertLess(args.index('--no-download-archive'), args.index('--'))
            self.assertNotIn('--download-archive', args)

    def test_invalid_cli_arguments_have_no_filesystem_side_effects(self):
        self.command('sync-item', 'https://notyoutube.com/watch?v=' + VIDEO_ID, expect=2)
        self.assertFalse(self.root.exists())
        self.command('scan', '--exec=bad', expect=2)
        self.assertFalse(self.root.exists())
        self.command('unknown-command', expect=2)
        self.assertFalse(self.root.exists())

    def test_concurrent_writer_and_old_live_lock_are_rejected(self):
        self.plan['delay_ms'] = 1500
        self.write_plan()
        first = subprocess.Popen([str(self.cli), 'scan', str(self.root), SOURCE_URL], env=self.env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.addCleanup(lambda: first.kill() if first.poll() is None else None)
        lock = self.root / 'State/ArchiveMode/sync.lock'
        deadline = time.monotonic() + 5
        while not (self.base / 'calls.jsonl').exists() and time.monotonic() < deadline:
            time.sleep(0.01)
        self.assertTrue(lock.exists())
        old = time.time() - 3600
        os.utime(lock, (old, old))
        result = self.command('ingest-pending', expect=1)
        self.assertIn('Another Archive operation', result.stderr)
        stdout, stderr = first.communicate(timeout=10)
        self.assertEqual(first.returncode, 0, stdout + stderr)

    def test_crashed_writer_lock_is_recovered(self):
        self.plan['delay_ms'] = 3000
        self.write_plan()
        kwargs = {'start_new_session': True} if os.name != 'nt' else {}
        first = subprocess.Popen([str(self.cli), 'scan', str(self.root), SOURCE_URL], env=self.env, stdout=subprocess.PIPE, stderr=subprocess.PIPE, **kwargs)
        self.addCleanup(lambda: first.kill() if first.poll() is None else None)
        deadline = time.monotonic() + 5
        while not (self.base / 'calls.jsonl').exists() and time.monotonic() < deadline:
            time.sleep(0.01)
        if os.name == 'nt':
            subprocess.run(['taskkill', '/PID', str(first.pid), '/T', '/F'], capture_output=True, timeout=10)
        else:
            os.killpg(first.pid, signal.SIGKILL)
        first.communicate(timeout=10)
        self.plan['delay_ms'] = 0
        self.write_plan()
        self.scan()
        self.assertEqual(len(self.canonical()), 2)

    def stage_transaction(self, *, conflict=False):
        self.scan()
        state = self.root / 'State/ArchiveMode'
        operations = []
        for filename in ('items.json', 'sources.json'):
            path = state / filename
            before = path.read_bytes()
            value = json.loads(before)
            value[0]['title'] = 'Journal replay title'
            after = json.dumps(value).encode()
            import base64
            operations.append({'path': 'State/ArchiveMode/' + filename, 'before_exists': True,
                               'before_sha256': hashlib.sha256(before).hexdigest(),
                               'after_base64': base64.b64encode(after).decode(),
                               'after_sha256': hashlib.sha256(after).hexdigest()})
            if filename == 'items.json':
                path.write_bytes(after)  # Simulate a crash after the first committed target.
            elif conflict:
                value[0]['title'] = 'External conflicting edit'
                path.write_text(json.dumps(value))
        journal = state / 'transaction.json'
        journal.write_text(json.dumps({'schema_version': 1, 'operations': operations}))
        return journal

    def test_mid_commit_restart_rolls_forward_idempotently(self):
        journal = self.stage_transaction()
        self.command('ingest-pending')
        self.assertFalse(journal.exists())
        self.assertEqual(self.canonical()[0]['title'], 'Journal replay title')
        source = json.loads((self.root / 'State/ArchiveMode/sources.json').read_text())
        self.assertEqual(source[0]['title'], 'Journal replay title')
        self.command('ingest-pending')

    def test_transaction_conflict_preserves_all_bytes(self):
        journal = self.stage_transaction(conflict=True)
        before = {p: p.read_bytes() for p in (journal, self.root / 'State/ArchiveMode/items.json', self.root / 'State/ArchiveMode/sources.json')}
        self.command('ingest-pending', expect=1)
        self.assertEqual(before, {p: p.read_bytes() for p in before})

    def test_corrupt_state_is_not_silently_replaced(self):
        self.scan()
        path = self.root / 'State/ArchiveMode/items.json'
        path.write_text('{}')
        self.command('sync-item', VIDEO_URL, expect=1)
        self.assertEqual(path.read_text(), '{}')

    def test_duplicate_playlist_occurrences_survive_removal_and_reappearance(self):
        entry = self.plan['discovery']['entries'][0]
        self.plan['discovery']['entries'] = [entry, dict(entry, playlist_index=2)]
        self.write_plan(); self.scan()
        path = self.root / 'Playlists/PLAUDIT/items.json'
        self.assertEqual(len(self.canonical()), 1)
        rows = json.loads(path.read_text())
        self.assertEqual(len({x['entry_key'] for x in rows}), 2)
        self.plan['discovery']['entries'] = [entry]
        self.write_plan(); self.scan()
        rows = json.loads(path.read_text())
        self.assertEqual(sum(x['membership'] == 'removed' for x in rows), 1)
        self.plan['discovery']['entries'] = [entry, dict(entry, playlist_index=2)]
        self.write_plan(); self.scan()
        rows = json.loads(path.read_text())
        self.assertTrue(all(x['membership'] == 'active' for x in rows))
        self.assertEqual(len(self.canonical()), 1)

    def test_playlist_occurrence_identity_corruption_fails_closed_and_legacy_migrates(self):
        entry = self.plan['discovery']['entries'][0]
        self.plan['discovery']['entries'] = [entry, dict(entry, playlist_index=2)]
        self.write_plan(); self.scan()
        path = self.root / 'Playlists/PLAUDIT/items.json'
        history = self.root / 'Playlists/PLAUDIT/history.jsonl'

        rows = json.loads(path.read_text())
        rows[1]['entry_key'] = rows[0]['entry_key']
        corrupt = json.dumps(rows)
        path.write_text(corrupt)
        result = self.command('scan', SOURCE_URL, expect=1)
        self.assertIn('duplicate occurrence identity', result.stderr)
        self.assertEqual(path.read_text(), corrupt)

        # A specifically supported legacy archive has no occurrence IDs at all.
        # It is migrated deterministically and journaled before reconciliation.
        rows = json.loads(corrupt)
        for row in rows:
            row.pop('entry_key', None)
        path.write_text(json.dumps(rows))
        self.scan()
        migrated = json.loads(path.read_text())
        self.assertEqual([row['entry_key'] for row in migrated],
                         ['youtube:' + VIDEO_ID + '#1', 'youtube:' + VIDEO_ID + '#2'])
        self.assertIn('playlist_entry_key_migrated', history.read_text())

        # Partial legacy/current mixtures are ambiguous and remain fail-closed.
        mixed = migrated
        mixed[0].pop('entry_key')
        mixed_bytes = json.dumps(mixed)
        path.write_text(mixed_bytes)
        self.command('scan', SOURCE_URL, expect=1)
        self.assertEqual(path.read_text(), mixed_bytes)

    def test_cross_file_graph_integrity_fails_closed_and_accepts_valid_relationships(self):
        self.scan()
        state = self.root / 'State/ArchiveMode'
        canonical_path = state / 'items.json'
        sources_path = state / 'sources.json'
        playlist_path = self.root / 'Playlists/PLAUDIT/items.json'

        canonical_bytes = canonical_path.read_bytes()
        canonical = json.loads(canonical_bytes)
        canonical_path.write_text(json.dumps(canonical[1:]))
        corrupt_bytes = canonical_path.read_bytes()
        result = self.command('ingest-pending', expect=1)
        self.assertIn('missing canonical item', result.stderr)
        self.assertEqual(canonical_path.read_bytes(), corrupt_bytes)

        # Restoring the authoritative canonical record restores graph validity.
        canonical_path.write_bytes(canonical_bytes)
        self.command('ingest-pending')

        # A registered source that has never been scanned is legitimate and
        # therefore does not require a playlist directory yet.
        sources = json.loads(sources_path.read_text())
        sources.append({'key': 'UNSCANNED', 'url': 'https://example.invalid/list',
                        'title': 'Not scanned yet'})
        sources_path.write_text(json.dumps(sources))
        self.command('ingest-pending')
        self.assertFalse((self.root / 'Playlists/UNSCANNED').exists())

        # The same canonical item may legitimately be referenced by more than
        # one registered playlist.
        sources.append({'key': 'SECOND', 'url': 'https://example.invalid/second',
                        'title': 'Second playlist'})
        sources_path.write_text(json.dumps(sources))
        second = self.root / 'Playlists/SECOND'
        second.mkdir()
        rows = json.loads(playlist_path.read_text())
        second_rows = [dict(rows[0], entry_key=rows[0]['item_key'] + '#second')]
        (second / 'items.json').write_text(json.dumps(second_rows))
        (second / 'playlist.json').write_text(json.dumps(sources[-1]))
        self.command('ingest-pending')

        # Managed playlist state with no registered source is corruption.
        orphan = self.root / 'Playlists/ORPHAN'
        orphan.mkdir()
        (orphan / 'items.json').write_text(json.dumps(second_rows))
        orphan_bytes = (orphan / 'items.json').read_bytes()
        result = self.command('ingest-pending', expect=1)
        self.assertIn('no registered source', result.stderr)
        self.assertEqual((orphan / 'items.json').read_bytes(), orphan_bytes)

    def test_resource_upgrade_preserves_prior_contract(self):
        self.scan()
        contract = self.root / 'ARCHIVE_AGENT.md'
        original = contract.read_bytes()
        contract.write_text('locally customized legacy contract')
        self.command('ingest-pending')
        self.assertEqual(contract.read_bytes(), original)
        backups = list(self.root.glob('ARCHIVE_AGENT.md.previous-*'))
        self.assertEqual(len(backups), 1)
        self.assertEqual(backups[0].read_text(), 'locally customized legacy contract')

    def test_csv_formula_and_playlist_line_injection_are_neutralized(self):
        self.plan['discovery']['entries'][0]['title'] = '=HYPERLINK("bad")\n#EXTINF:injected'
        self.write_plan(); self.scan()
        path = self.root / 'Playlists/PLAUDIT/catalog.csv'
        with path.open(newline='', encoding='utf-8') as stream:
            rows = list(csv.DictReader(stream))
        self.assertTrue(any(str(value).startswith("'=HYPERLINK") for value in rows[0].values()))
        self.command('sync-item', VIDEO_URL)
        playlist = (self.root / 'Playlists/PLAUDIT/video.m3u8').read_text()
        self.assertEqual(sum(line.startswith('#EXTINF:') for line in playlist.splitlines()), 1)

    def test_unbound_existing_media_is_not_adopted_when_source_unavailable(self):
        self.scan()
        shutil.copy2(self.video, self.root / 'Video' / ('backup [' + VIDEO_ID + '].mp4'))
        shutil.copy2(self.audio, self.root / 'Audio' / ('backup [' + VIDEO_ID + '].m4a'))
        self.plan['discovery']['entries'][0]['availability'] = 'private'
        self.write_plan(); self.scan()
        self.command('sync-item', VIDEO_URL, expect=1)
        item = self.first()
        self.assertNotEqual(item['video']['state'], 'complete')
        self.assertNotEqual(item['audio']['state'], 'complete')

    def test_bound_existing_media_is_adopted_when_source_unavailable(self):
        self.scan()
        self.command('sync-item', VIDEO_URL)
        item = self.first()
        self.assertEqual(item['video']['state'], 'complete')
        self.assertEqual(item['audio']['state'], 'complete')
        bindings = list((self.root / 'State/ArchiveMode/MediaBindings').glob('*.json'))
        self.assertGreaterEqual(len(bindings), 2)

        items_path = self.root / 'State/ArchiveMode/items.json'
        items = json.loads(items_path.read_text(encoding='utf-8'))
        target = next(x for x in items if x['key'] == 'youtube:' + VIDEO_ID)
        target['video']['state'] = 'missing'
        target['audio']['state'] = 'missing'
        items_path.write_text(json.dumps(items), encoding='utf-8')
        self.plan['discovery']['entries'][0]['availability'] = 'private'
        self.write_plan(); self.scan()
        self.command('sync-item', VIDEO_URL)
        adopted = self.first()
        self.assertEqual(adopted['video']['state'], 'complete')
        self.assertEqual(adopted['audio']['state'], 'complete')
        self.command('verify-item', VIDEO_URL)

    @unittest.skipUnless(os.name == 'nt', 'Windows PowerShell acceptance wrapper')
    def test_windows_harness_positive_and_tampered_package_negative(self):
        powershell = shutil.which('pwsh') or shutil.which('powershell')
        self.assertIsNotNone(powershell)
        script = self.package / 'archive-local-harness.ps1'
        shutil.copy2(Path(__file__).resolve().parents[1] / 'scripts/archive-local-harness.ps1', script)
        shutil.copy2(FAKE, self.package / 'bin/deno.exe')
        ffbin = self.package / '3rdParty/ffmpeg/bin'
        shutil.copytree(Path(FFMPEG).parent, ffbin)
        commit = 'a' * 40
        run_id = '123456789'
        repository = 'example/qualified-repo'
        release_tag = 'qualification-' + commit
        (self.package / 'build-identity.json').write_text(json.dumps({
            'repository': repository, 'commit': commit, 'run_id': run_id,
            'qualification': 'windows-ci-qualified-for-local-harness'
        }))
        (self.package / 'PORTABLE_MANIFEST.txt').write_text('deterministic fixture package')
        manifest = '\n'.join(sha(p) + '  ' + p.relative_to(self.package).as_posix() for p in self.package.rglob('*') if p.is_file()) + '\n'
        (self.package / 'SHA256SUMS.txt').write_text(manifest, encoding='ascii')

        artifact = self.package.parent / 'qualified-artifact.zip'
        with zipfile.ZipFile(artifact, 'w', zipfile.ZIP_DEFLATED) as archive:
            for path in self.package.rglob('*'):
                if path.is_file():
                    archive.write(path, (Path('Media-Downloader-PS') / path.relative_to(self.package)).as_posix())
        artifact_sha = sha(artifact)

        args = [
            powershell, '-NoProfile', '-NonInteractive', '-File', script,
            '-ArchiveRoot', self.root, '-PlaylistUrl', SOURCE_URL, '-VideoUrl', VIDEO_URL,
            '-ExpectedCommit', commit, '-ExpectedArtifactSha256', artifact_sha,
            '-ExpectedReleaseTag', release_tag, '-ExpectedRunId', run_id,
            '-ExpectedRepository', repository, '-ArtifactZipPath', artifact
        ]
        result = run(args, env=self.env, timeout=120)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        receipt = list(self.root.glob('local-harness-evidence-*.json'))
        self.assertEqual(len(receipt), 1)
        evidence = json.loads(receipt[0].read_text(encoding='utf-8-sig'))
        self.assertEqual(evidence['item_key'], 'youtube:' + VIDEO_ID)
        self.assertEqual(evidence['source_key'], 'PLAUDIT')
        self.assertEqual(evidence['active_occurrences'], 1)
        self.assertTrue(evidence['entry_key'])
        self.assertEqual(evidence['release_tag'], release_tag)
        self.assertEqual(evidence['artifact_sha256'], artifact_sha)
        self.assertEqual(evidence['repository'], repository)
        self.assertEqual(str(evidence['ci_run_id']), run_id)
        self.assertEqual(evidence['result'], 'PASS')

        # The same anchored package must not qualify a requested video that is
        # unrelated to the scanned playlist.
        unrelated_args = list(args)
        unrelated_args[unrelated_args.index('-VideoUrl') + 1] = 'https://www.youtube.com/watch?v=ZZZ999yyy88'
        unrelated = run(unrelated_args + ['-AllowExistingArchive'], env=self.env, timeout=120)
        self.assertNotEqual(unrelated.returncode, 0)
        self.assertIn('not an active occurrence', unrelated.stdout + unrelated.stderr)
        self.assertEqual(len(list(self.root.glob('local-harness-evidence-*.json'))), 1)

        # A wrong artifact is rejected even if the package claims the same
        # repository/run/commit identity.
        wrong_artifact = self.package.parent / 'wrong-artifact.zip'
        shutil.copy2(artifact, wrong_artifact)
        with wrong_artifact.open('ab') as stream:
            stream.write(b'wrong-artifact')
        wrong = list(args)
        wrong[wrong.index('-ArtifactZipPath') + 1] = wrong_artifact
        wrong_result = run(wrong + ['-AllowExistingArchive'], env=self.env, timeout=120)
        self.assertNotEqual(wrong_result.returncode, 0)
        self.assertIn('External artifact SHA-256', wrong_result.stdout + wrong_result.stderr)

        # A coherently resealed extracted package must still fail because the
        # externally anchored ZIP is unchanged.
        (self.package / 'PORTABLE_MANIFEST.txt').write_text('coherently tampered')
        manifest = '\n'.join(
            sha(p) + '  ' + p.relative_to(self.package).as_posix()
            for p in self.package.rglob('*')
            if p.is_file() and p.name != 'SHA256SUMS.txt'
        ) + '\n'
        (self.package / 'SHA256SUMS.txt').write_text(manifest, encoding='ascii')
        resealed = run(args + ['-AllowExistingArchive'], env=self.env, timeout=120)
        self.assertNotEqual(resealed.returncode, 0)
        self.assertIn('differs from externally anchored artifact', resealed.stdout + resealed.stderr)
        self.assertEqual(len(list(self.root.glob('local-harness-evidence-*.json'))), 1)

        # No external digest/artifact identity means no qualified acceptance.
        missing_anchor = [
            powershell, '-NoProfile', '-NonInteractive', '-File', script,
            '-ArchiveRoot', self.root, '-PlaylistUrl', SOURCE_URL, '-VideoUrl', VIDEO_URL,
            '-ExpectedCommit', commit
        ]
        absent = run(missing_anchor + ['-AllowExistingArchive'], env=self.env, timeout=30)
        self.assertNotEqual(absent.returncode, 0)
        self.assertEqual(len(list(self.root.glob('local-harness-evidence-*.json'))), 1)

    def test_malformed_journal_payload_is_rejected_before_any_write(self):
        journal = self.stage_transaction()
        data = json.loads(journal.read_text())
        import base64
        bad = b'{}'
        data['operations'][1]['after_base64'] = base64.b64encode(bad).decode()
        data['operations'][1]['after_sha256'] = hashlib.sha256(bad).hexdigest()
        journal.write_text(json.dumps(data))
        sources = self.root / 'State/ArchiveMode/sources.json'
        before = sources.read_bytes()
        self.command('ingest-pending', expect=1)
        self.assertEqual(sources.read_bytes(), before)
        self.assertTrue(journal.exists())

    def test_corrupt_history_cannot_be_extended_or_replaced(self):
        self.scan()
        history = self.root / 'Playlists/PLAUDIT/history.jsonl'
        history.write_bytes(b'not-json\n')
        before = self.canonical()
        self.command('scan', SOURCE_URL, expect=1)
        self.assertEqual(history.read_bytes(), b'not-json\n')
        self.assertEqual(self.canonical(), before)

    def test_external_downloader_state_leaf_links_are_refused(self):
        self.scan()
        state = self.root / 'State'
        outside = self.base / 'outside-state-leaf.txt'
        outside.write_text('outside sentinel\n')

        def link_file(link: Path):
            if link.exists() or link.is_symlink():
                link.unlink()
            if os.name == 'nt':
                result = subprocess.run(['cmd', '/c', 'mklink', str(link), str(outside)], capture_output=True, text=True)
                if result.returncode:
                    self.skipTest('Host does not permit file symbolic-link creation')
            else:
                link.symlink_to(outside)

        catalog = state / 'video-catalog.jsonl'
        link_file(catalog)
        before = outside.read_bytes()
        result = self.command('sync-item', VIDEO_URL, expect=1)
        self.assertIn('Unsafe external-tool state path refused', result.stderr)
        self.assertEqual(outside.read_bytes(), before)
        if catalog.exists() or catalog.is_symlink():
            catalog.unlink()

        archive = state / 'video-archive.txt'
        link_file(archive)
        before = outside.read_bytes()
        result = self.command('sync-item', VIDEO_URL, expect=1)
        self.assertIn('Unsafe external-tool state path refused', result.stderr)
        self.assertEqual(outside.read_bytes(), before)
        if archive.exists() or archive.is_symlink():
            archive.unlink()

        # Normal unlinked state leaves retain the existing sync path.
        self.command('sync-item', VIDEO_URL)

    def test_linked_state_directory_is_refused(self):
        outside = self.base / 'outside'
        outside.mkdir()
        self.root.mkdir()
        link = self.root / 'State'
        if os.name == 'nt':
            result = subprocess.run(['cmd', '/c', 'mklink', '/J', str(link), str(outside)], capture_output=True)
            if result.returncode:
                self.skipTest('Host does not permit junction creation')
            self.addCleanup(lambda: os.rmdir(link) if link.exists() else None)
        else:
            link.symlink_to(outside, target_is_directory=True)
        self.command('scan', SOURCE_URL, expect=1)
        self.assertEqual(list(outside.iterdir()), [])

if __name__ == '__main__':
    unittest.main(argv=[sys.argv[0], *test_args], verbosity=2)
