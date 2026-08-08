#!/usr/bin/env python3
"""Generate platform icon files from the single assets/icon.png source."""

from argparse import ArgumentParser
from pathlib import Path

from PIL import Image


def main() -> None:
    parser = ArgumentParser()
    parser.add_argument("source", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--format", choices=("ico", "png"), required=True)
    args = parser.parse_args()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with Image.open(args.source) as source:
        image = source.convert("RGBA")
        if args.format == "ico":
            image.save(
                args.output,
                format="ICO",
                sizes=((16, 16), (24, 24), (32, 32), (48, 48),
                       (64, 64), (128, 128), (256, 256)),
            )
        else:
            image.resize((512, 512), Image.Resampling.LANCZOS).save(
                args.output, format="PNG", optimize=True
            )


if __name__ == "__main__":
    main()
