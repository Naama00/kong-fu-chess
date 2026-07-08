#!/usr/bin/env python3
import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUTPUT_FILE = ROOT / "generated_single_file.cpp"


def build_output(output_path: Path) -> None:
    source_path = ROOT / "scripts" / "platform_harness.cpp"
    output_path.write_text(source_path.read_text(encoding="utf-8"), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description="Build a single CPP file for the platform test harness")
    parser.add_argument("--output", default=str(OUTPUT_FILE), help="Path to the generated .cpp file")
    args = parser.parse_args()

    output_path = Path(args.output).resolve()
    build_output(output_path)
    print(output_path)


if __name__ == "__main__":
    main()
