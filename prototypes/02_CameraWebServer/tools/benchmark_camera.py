"""Measure decoded MJPEG delivery, restoring temporary controls afterward.

Requires Pillow. Keep the scene, lighting, Wi-Fi and other clients unchanged.
Outputs may contain camera photographs; keep them private unless reviewed.
"""

import argparse
import io
import json
import statistics
import time
from pathlib import Path
from urllib.parse import urlencode, urlsplit, urlunsplit
from urllib.request import urlopen

from PIL import Image


SIZES = {
    6: (320, 240), 10: (640, 480), 11: (800, 600), 12: (1024, 768),
    13: (1280, 720), 14: (1280, 1024), 15: (1600, 1200),
    16: (1920, 1080), 19: (2048, 1536), 20: (2560, 1440),
    21: (2560, 1600), 23: (2560, 1920),
}


def get_json(base, path):
    with urlopen(base + path, timeout=12) as response:
        return json.load(response)


def control(base, name, value):
    query = urlencode({'var': name, 'val': value})
    with urlopen(base + '/control?' + query, timeout=12) as response:
        response.read()


def read_frame(stream):
    while True:
        line = stream.readline(4096)
        if not line:
            raise EOFError('Stream ended before the next frame')
        if line.startswith(b'--'):
            break
    headers = {}
    for _ in range(20):
        line = stream.readline(4096)
        if line in (b'\r\n', b'\n'):
            break
        if not line:
            raise EOFError('Incomplete frame headers')
        key, value = line.decode('ascii').split(':', 1)
        headers[key.lower()] = value.strip()
    else:
        raise ValueError('Too many frame headers')
    length = int(headers['content-length'])
    if not 0 < length <= 10_000_000:
        raise ValueError('Invalid frame length')
    data = stream.read(length)
    if len(data) != length:
        raise EOFError('Incomplete JPEG frame')
    return data, float(headers['x-timestamp'])


def measure(stream_url, size, seconds, output):
    arrivals, captured, lengths = [], [], []
    errors = []
    warmup = 0
    started = time.perf_counter()
    with urlopen(stream_url, timeout=12) as stream:
        while True:
            data, timestamp = read_frame(stream)
            arrived = time.perf_counter()
            try:
                with Image.open(io.BytesIO(data)) as image:
                    image.load()  # Fully decode; a JPEG header alone is insufficient.
                    if image.size != SIZES[size]:
                        raise ValueError(f'Expected {SIZES[size]}, got {image.size}')
            except (OSError, ValueError) as exc:
                errors.append(str(exc))
            else:
                if warmup < 5:
                    warmup += 1
                else:
                    arrivals.append(arrived)
                    captured.append(timestamp)
                    lengths.append(len(data))
                    if len(arrivals) == 5:
                        (output / f'{size}-{SIZES[size][0]}x{SIZES[size][1]}.jpg').write_bytes(data)
            if len(arrivals) >= 5 and arrivals[-1] - arrivals[0] >= seconds:
                break
            if arrived - started > seconds + 35:
                raise TimeoutError('Not enough valid frames within the time limit')
    elapsed = arrivals[-1] - arrivals[0]
    intervals = sorted(b - a for a, b in zip(arrivals, arrivals[1:]))
    capture_elapsed = captured[-1] - captured[0]
    return {
        'framesize': size, 'width': SIZES[size][0], 'height': SIZES[size][1],
        'decoded_frames': len(arrivals), 'warmup_frames': warmup,
        'seconds': round(elapsed, 3),
        'received_fps': round((len(arrivals) - 1) / elapsed, 2),
        'capture_timestamp_fps': round((len(arrivals) - 1) / capture_elapsed, 2) if capture_elapsed > 0 else None,
        'mean_jpeg_bytes': round(statistics.mean(lengths)),
        'p95_frame_interval_ms': round(intervals[min(len(intervals) - 1, int(len(intervals) * .95))] * 1000),
        'decode_errors': len(errors), 'error_examples': errors[:3],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('url')
    parser.add_argument('--sizes', nargs='+', type=int, choices=SIZES, default=[6, 10, 11, 12, 13, 15, 16, 23])
    parser.add_argument('--seconds', type=float, default=10)
    parser.add_argument('--quality', type=int, default=12)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    base = args.url.rstrip('/')
    parts = urlsplit(base)
    stream_url = urlunsplit((parts.scheme, f'{parts.hostname}:81', '/stream', '', ''))
    args.output.mkdir(parents=True, exist_ok=True)
    original = get_json(base, '/status')
    limits = get_json(base, '/settings')
    report = {'quality': args.quality, 'requested_seconds': args.seconds,
              'original_controls': {'framesize': original['framesize'], 'quality': original['quality']},
              'camera_settings': limits, 'measurements': []}
    try:
        control(base, 'quality', args.quality)
        for size in args.sizes:
            if size > limits['max_framesize']:
                result = {'framesize': size, 'error': 'Above the allocated buffer limit'}
            else:
                try:
                    control(base, 'framesize', size)
                    time.sleep(1)
                    result = measure(stream_url, size, args.seconds, args.output)
                except Exception as exc:
                    result = {'framesize': size, 'error': f'{type(exc).__name__}: {exc}'}
            report['measurements'].append(result)
            (args.output / 'results.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
            print(json.dumps(result), flush=True)
    finally:
        for name in ('framesize', 'quality'):
            control(base, name, original[name])
        print('Original resolution and quality restored; saved profile unchanged.', flush=True)


if __name__ == '__main__':
    main()
