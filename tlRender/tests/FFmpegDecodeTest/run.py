#!/usr/bin/env python3
"""Generate small fixtures and compare mrv2's hardware/automatic and software paths."""

import argparse
import os
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ffmpeg", default="ffmpeg")
    parser.add_argument("--decoder", required=True)
    parser.add_argument("--failure-decoder")
    parser.add_argument(
        "--require-hardware",
        action="store_true",
        help="Require VideoToolbox for standard ProRes/H264/HEVC fixtures",
    )
    args = parser.parse_args()

    fixtures = [
        ("h264", "libx264", "yuv420p", ["-g", "4", "-bf", "2"], True),
        ("hevc", "libx265", "yuv420p",
         ["-x265-params", "keyint=4:log-level=error"], True),
        ("prores", "prores_ks", "yuv422p10le", ["-profile:v", "3"], True),
        ("prores-444", "prores_ks", "yuv444p10le", ["-profile:v", "4"], True),
        ("prores-alpha", "prores_ks", "yuva444p10le", ["-profile:v", "4"], False),
        ("h264-10bit", "libx264", "yuv420p10le", ["-g", "4"], False),
        ("hevc-hdr", "libx265", "yuv420p10le",
         ["-x265-params", "keyint=4:log-level=error", "-color_primaries", "bt2020",
          "-color_trc", "smpte2084", "-colorspace", "bt2020nc"], True),
        ("hevc-444", "libx265", "yuv444p10le",
         ["-x265-params", "keyint=4:log-level=error"], False),
        ("mpeg4", "mpeg4", "yuv420p", [], False),
    ]

    failures = []
    passed = 0

    with tempfile.TemporaryDirectory(prefix="mrv2-decode-test-") as directory:
        for name, codec, pixel_format, extra, supported in fixtures:
            print(f"\n=== Testing fixture: {name} ===", flush=True)

            try:
                path = Path(directory) / (name + ".mov")
                source = "testsrc2=size=320x192:rate=24,format=" + pixel_format

                if name == "prores-alpha":
                    source += ",geq=a='X/W*1023':lum='p(X,Y)':cb='p(X,Y)':cr='p(X,Y)'"

                subprocess.run(
                    [
                        args.ffmpeg, "-hide_banner", "-loglevel", "error", "-y",
                        "-f", "lavfi", "-i", source,
                        "-frames:v", "12", "-c:v", codec, "-pix_fmt", pixel_format,
                        "-color_range", "tv", "-colorspace", "bt709",
                        "-color_primaries", "bt709", "-color_trc", "bt709",
                        "-metadata", "timecode=01:00:00:00",
                        *extra, str(path),
                    ],
                    check=True,
                    timeout=30,
                )

                expected = (
                    "VideoToolbox"
                    if args.require_hardware and supported
                    else "auto"
                )

                if name in ("prores-alpha", "mpeg4"):
                    expected = "Software"

                for rgb, memory, threads in [
                    ("0", "0", "0"),
                    ("1", "1", "4"),
                ]:
                    subprocess.run(
                        [
                            args.decoder, str(path), expected,
                            rgb, memory, threads,
                        ],
                        check=True,
                        timeout=30,
                    )

                if args.failure_decoder and name == "h264":
                    for variable, value, backend in [
                        ("MRV2_TEST_FAIL_HWDEVICE", "1", "Software"),
                        ("MRV2_TEST_FAIL_TRANSFER", "1", "Software"),
                        ("MRV2_TEST_FAIL_TRANSFER", "5", "fallback"),
                    ]:
                        print(
                            f"Injecting {variable}={value}",
                            flush=True,
                        )
                        subprocess.run(
                            [
                                args.failure_decoder, str(path), backend,
                                "0", "1", "4",
                            ],
                            env={**os.environ, variable: value},
                            check=True,
                            timeout=30,
                        )

                passed += 1
                print(f"PASS: {name}", flush=True)

            except (
                subprocess.CalledProcessError,
                subprocess.TimeoutExpired,
                OSError,
            ) as exc:
                failures.append((name, str(exc)))
                print(
                    f"FAIL: {name}: {exc}\n"
                    "Continuing with the next fixture...",
                    flush=True,
                )

    print("\n=== Test summary ===")
    print(f"Passed: {passed}/{len(fixtures)}")

    if failures:
        print(f"Failed: {len(failures)}")
        for name, error in failures:
            print(f"  {name}: {error}")
        return 1

    print("All fixtures passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
