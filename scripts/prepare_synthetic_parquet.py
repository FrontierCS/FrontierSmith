#!/usr/bin/env python3
"""Build a Parquet of the FrontierSmith training or parity problems for VERL.

Each problem directory under ``Frontier-CS/algorithmic/problems/`` contributes a
single row.  ``data_source`` is set to ``frontiercs`` and ``ground_truth`` is the
problem directory name, so the existing Frontier-CS judge handles them
unchanged.

Usage:
    python scripts/prepare_synthetic_parquet.py
    python scripts/prepare_synthetic_parquet.py --subset train
    python scripts/prepare_synthetic_parquet.py --output-dir data/frontiercs --output-name train_synthetic.parquet
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

import pandas as pd

PROJECT_ROOT = Path(__file__).resolve().parent.parent
PROBLEMS_DIR = PROJECT_ROOT / "Frontier-CS" / "algorithmic" / "problems"

TRAIN_MANIFEST = PROJECT_ROOT / "data" / "sample_lists" / "frontiersmith_train_200.json"
PARITY_MANIFEST = PROJECT_ROOT / "data" / "sample_lists" / "frontiersmith_parity_10.json"


def build_prompt(statement: str) -> list[dict]:
    return [
        {
            "role": "user",
            "content": (
                "You are a competitive programmer. Solve the following problem in C++. "
                "Output ONLY the C++ code wrapped in ```cpp and ```. No explanation.\n\n"
                f"{statement}\n\nGenerate solution code:"
            ),
        }
    ]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--subset", choices=("parity", "train"), default="parity")
    parser.add_argument("--output-dir", type=Path, default=PROJECT_ROOT / "data" / "frontiercs")
    parser.add_argument("--output-name", type=str)
    args = parser.parse_args()

    if args.subset == "train":
        manifest_path, expected_count = TRAIN_MANIFEST, 200
        output_name = args.output_name or "train_synthetic_200.parquet"
    else:
        manifest_path, expected_count = PARITY_MANIFEST, 10
        output_name = args.output_name or "train_synthetic.parquet"

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    problem_ids = [entry["problem_id"] for entry in manifest["problems"]]
    if len(problem_ids) != expected_count or len(set(problem_ids)) != expected_count:
        raise SystemExit(f"The {args.subset} manifest must contain {expected_count} unique problem IDs.")

    rows: list[dict] = []
    missing: list[str] = []
    for pid in problem_ids:
        stmt = PROBLEMS_DIR / pid / "statement.txt"
        if not stmt.is_file():
            missing.append(pid)
            continue
        rows.append(
            {
                "prompt": build_prompt(stmt.read_text(encoding="utf-8")),
                "reward_model": {"ground_truth": pid},
                "data_source": "frontiercs",
            }
        )

    if missing:
        raise SystemExit("Missing statement.txt for: " + ", ".join(missing))

    args.output_dir.mkdir(parents=True, exist_ok=True)
    out_path = args.output_dir / output_name
    pd.DataFrame(rows).to_parquet(out_path, index=False)
    print(f"Saved {len(rows)} problems -> {out_path}")


if __name__ == "__main__":
    main()
