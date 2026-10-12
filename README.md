<p align="center">
  <img src="assets/logo.png" alt="FrontierSmith Logo" width="200"/>
</p>

<h1 align="center">FrontierSmith</h1>

<h3 align="center">
Synthetic Open-ended Problem Generation
</h3>

<p align="center">
  <a href="https://arxiv.org/abs/2605.14445"><img src="https://img.shields.io/badge/arXiv-2605.14445-b31b1b?logo=arxiv&logoColor=white" alt="arXiv"></a>
  <a href="https://frontier-cs.org/blog/frontiersmith"><img src="https://img.shields.io/badge/Blog-frontier--cs.org-1f6feb" alt="Blog"></a>
  <a href="https://huggingface.co/runyuanhe/qwen35-9b-frontiersmith"><img src="https://img.shields.io/badge/%F0%9F%A4%97%20HuggingFace-Model-yellow" alt="HuggingFace Model"></a>
  <a href="https://github.com/FrontierCS/Frontier-CS"><img src="https://img.shields.io/badge/Frontier--CS-Official_Repo-blue?logo=github" alt="Frontier-CS"></a>
  <img src="https://img.shields.io/badge/Synthetic_Problems-200-green" alt="Synthetic Problems">
  <img src="https://img.shields.io/badge/Python-3.11+-yellow?logo=python&logoColor=white" alt="Python">
  <img src="https://img.shields.io/badge/Docker-24+-2496ED?logo=docker&logoColor=white" alt="Docker">
</p>


https://github.com/user-attachments/assets/b91ee8fb-d5e0-40b3-8a6c-cd1b3b915ac2

## Overview

**FrontierSmith** is the synthetic open-ended problem-generation pipeline. This repository contains training code, evaluation code, and **200 synthetic training problems**, including the **10 problems** used in the paper's parity experiment. The problem release includes only statements, test inputs and answers, checkers, and judge configurations.

> The orchestrator and LLM-driven test/checker generators are intentionally withheld.

---

## Repository Structure

```
FrontierSmith/
├── README.md
├── requirements.txt
├── setup-env.sh                          # one-shot environment bootstrap
├── verl/                                 # vendored VERL framework (editable install)
├── ALE-Bench/                            # ALE-Bench validator (third-party)
├── Frontier-CS/
│   ├── algorithmic/
│   │   ├── problems/                     # 200 problems, including the parity subset
│   │   │   └── frontiersmith_{0..199}/
│   │   ├── Dockerfile / server.js / judge/ / scripts/
│   │   └── ...
│   ├── src/ pyproject.toml
│   └── README.md
├── harbor/
│   └── adapters/frontier-cs-algorithm/   # Harbor adapter
├── scripts/                              # training / evaluation / data-prep
└── data/
    ├── frontiercs/train_synthetic_200.parquet # ready-to-use training data
    └── sample_lists/                     # reproducibility manifests
```

---

## Synthetic Problems

### Training set (200 problems)

The complete 200-problem training set is in
`Frontier-CS/algorithmic/problems/frontiersmith_{0..199}/`, with **1,998
input/answer pairs**. The problem IDs and training row order are recorded in
[`frontiersmith_train_200.json`](data/sample_lists/frontiersmith_train_200.json).

### Parity subset (10 of the 200 problems)

All 10 previously released examples are included in the training set with
the same statements, test data, and scoring behavior. The duplicate example directories
have been removed. The [parity manifest](data/sample_lists/frontiersmith_parity_10.json)
records their canonical IDs below. They correspond to problems **306–315** in
the [Frontier-CS main repository](https://github.com/FrontierCS/Frontier-CS):

| ID | Frontier-CS ID | Name |
|:---|:---------------|:-----|
| `frontiersmith_180` | 306 | Scorched Bridges Campaign |
| `frontiersmith_167` | 307 | Farmwide Teleport Pad Deployment |
| `frontiersmith_56` | 308 | Metallic Pink Resonator Layout |
| `frontiersmith_151` | 309 | Park Ranger Shift Balancing |
| `frontiersmith_114` | 310 | Prime Resonance Retuning |
| `frontiersmith_143` | 311 | Mobile Relay Layout |
| `frontiersmith_139` | 312 | Archipelago Relay Network Design |
| `frontiersmith_107` | 313 | Resonant Bay Layout |
| `frontiersmith_184` | 314 | Duff's Defensive Lineup |
| `frontiersmith_50` | 315 | Quadratic Witness Packing |

Each problem directory contains:

```
chk.cc           # custom checker
config.yaml      # judge configuration
statement.txt    # problem statement
testdata/        # *.in / *.ans pairs
```

---

## Environment Setup

```bash
source setup-env.sh             # creates .venv, installs all deps
source setup-env.sh --skip      # activate existing env quickly
```

External services:

```bash
hf auth login          # to download Qwen3.5-9B / 27B weights
wandb login            # optional, for training logs
```

### Tested Versions

| Package      | Version           | Notes                                   |
|:-------------|:------------------|:----------------------------------------|
| Python       | 3.11              | `apt install python3.11 python3.11-dev` |
| torch        | 2.11.0+cu130      | pulled by vllm                          |
| vllm         | 0.20.0            |                                         |
| transformers | 5.7.0             | Qwen3.5 needs >= 5.2.0                  |
| verl         | 0.8.0.dev (local) | editable install from `verl/`           |
| ray          | 2.55.1            |                                         |

---

## Datasets

### Frontier-CS Algorithmic Track (172 problems, public)

Not redistributed. Use the [official release](https://github.com/FrontierCS/Frontier-CS) to populate `Frontier-CS/algorithmic/problems/<numeric_id>/`.

### HardTest (sampled, public)

```bash
python scripts/download_hardtest.py
python scripts/install_hardtest_frontier_packages.py
python scripts/split_hardtest_by_difficulty.py
python scripts/sample_hardtest_problems.py --n 200 --seed 42 \
       -o results/hardtest_hard_sampled_200.json
```

The exact 200-problem manifest is at `data/sample_lists/hardtest_hard_sampled_200.json`.

### Synthetic Problems (200 training, including 10 parity examples)

The ready-to-use [200-problem training Parquet](data/frontiercs/train_synthetic_200.parquet)
is included in this repository. To rebuild it from the statements and manifest:

```bash
python scripts/prepare_synthetic_parquet.py --subset train
```

This writes `data/frontiercs/train_synthetic_200.parquet` with judge IDs matching
the released `frontiersmith_0` through `frontiersmith_199` directories.

The 30-problem mixed sample list (10 from each of HardTest, Frontier-CS, synthetic) is at `data/sample_lists/harbor_sample_30.jsonl`.

### Harbor + Claude Code Reproduction

The 10 FrontierSmith parity problems can be loaded into a local Harbor dataset with
the bundled Frontier-CS algorithm adapter, then run with Harbor's standard
`claude-code` agent.

Prerequisites:

- Docker Desktop or another Docker daemon reachable by the current user.
- `uv` and the Harbor CLI (`uv tool install harbor`, or `pip install harbor`).
- A Claude Code compatible Anthropic key exported as `ANTHROPIC_API_KEY`.
- If your machine needs a proxy, configure both the shell and Docker Desktop
  for it, for example `http://127.0.0.1:7897`.

Load the FrontierSmith tasks:

```bash
export FRONTIERSMITH_ROOT="$(pwd)"
export FRONTIER_CS_ALGORITHMIC_PATH="$FRONTIERSMITH_ROOT/Frontier-CS/algorithmic"

cd "$FRONTIERSMITH_ROOT/harbor/adapters/frontier-cs-algorithm"
uv sync
uv run frontier-cs-algorithm \
  --source "$FRONTIERSMITH_ROOT/Frontier-CS" \
  --output-dir "$FRONTIERSMITH_ROOT/datasets/frontiersmith-sample" \
  --include-non-numeric \
  --task-ids \
    frontiersmith_180 frontiersmith_167 frontiersmith_56 frontiersmith_151 frontiersmith_114 \
    frontiersmith_143 frontiersmith_139 frontiersmith_107 frontiersmith_184 frontiersmith_50 \
  --overwrite
cd "$FRONTIERSMITH_ROOT"
```

This creates 10 Harbor task directories under
`datasets/frontiersmith-sample/frontier-cs-algorithm-frontiersmith_*`.

Run a smoke test with Claude Code on one task:

```bash
harbor run \
  -p datasets/frontiersmith-sample \
  -a claude-code \
  -m anthropic/claude-opus-4-6 \
  -l 1 \
  -n 1 \
  --jobs-dir jobs/frontiersmith-claude-smoke \
  --yes
```

Run all 10 tasks:

```bash
harbor run \
  -p datasets/frontiersmith-sample \
  -a claude-code \
  -m anthropic/claude-opus-4-6 \
  -n 1 \
  --jobs-dir jobs/frontiersmith-claude \
  --yes
```

Results are written under the selected `jobs/` directory. Each task expects
the agent to write `/app/solution.cpp`; the verifier posts that solution to
the Frontier-CS judge sidecar and records the reward in
`reward.txt` / `reward.json`.

### ALE-Bench (validation only)

```bash
python scripts/prepare_alebench_parquet.py
```

---

## Services

### Frontier-CS Judge

```bash
cd Frontier-CS/algorithmic
docker build -t frontiercs-judge .
./run_judge.sh                  # listens on http://localhost:8082
```

### ALE-Bench Judge (optional)

```bash
cd ALE-Bench
bash scripts/docker_build_202301.sh $(id -u) $(id -g)
```

---

## Data Preparation

```bash
python scripts/prepare_frontiercs_parquet.py             # Frontier-CS 172 → parquet
python scripts/prepare_hardtest_hard_sample_parquet.py    # HardTest 200 → parquet
python scripts/prepare_synthetic_parquet.py --subset train # 200 training → parquet
python scripts/prepare_synthetic_parquet.py                # 10 parity → parquet
python scripts/prepare_alebench_parquet.py                # ALE-Bench validation
python scripts/prepare_random_reward_train_parquet.py     # Random-reward
```

---

## Training

All scripts use `python -m verl.trainer.main_ppo` with Hydra overrides.

```bash
# Main 9B GRPO run
bash scripts/run_verl_grpo_frontiercs_qwen35_9b.sh

# Multi-GPU
NGPU=8 TP=2 bash scripts/run_verl_grpo_frontiercs_qwen35_9b.sh
```

| Variable      | Default                         | Description                          |
|:--------------|:--------------------------------|:-------------------------------------|
| `MODEL_PATH`  | `Qwen/Qwen3.5-9B`              | HF model id or local path           |
| `TRAIN_DATA`  | `data/frontiercs/train.parquet` | training parquet                     |
| `VAL_DATA`    | `data/frontiercs/full.parquet`  | validation parquet                   |
| `NGPU`        | `4`                             | GPUs per node                        |
| `TP`          | `1`                             | tensor parallel size for vLLM        |
| `FRESH_START` | `0`                             | set `1` to start from scratch        |

### Experiment Scripts

| Script | Purpose |
|:-------|:--------|
| `run_verl_grpo_frontiercs_qwen35_9b.sh` | 9B on Frontier-CS (172 problems) |
| `run_verl_grpo_frontiercs_qwen35_9b_no_validation.sh` | above, validation disabled |
| `run_verl_grpo_frontiercs_qwen35_9b_alebench.sh` | 9B with ALE-Bench validation |
| `run_verl_grpo_frontiercs_qwen35_9b_hardtest.sh` | 9B on HardTest 200 |
| `run_verl_grpo_frontiercs_qwen35_9b_synthetic.sh` | 9B + synthetic mix |
| `run_verl_grpo_frontiercs_qwen35_9b_nofilter.sh` | ablation (no filtering) |
| `run_verl_grpo_frontiercs_qwen35_9b_randomreward.sh` | random-reward control |

---

## Evaluation

```bash
# Start vLLM server
bash scripts/start_vllm_server.sh

# Base model / checkpoint sweeps
bash scripts/eval_base_model_frontiercs.sh
bash scripts/eval_frontiercs_checkpoints.sh

# Single-shot evaluation
python scripts/eval_frontiercs_via_vllm.py
python scripts/run_qwen_frontiercs.py
python scripts/run_merged_model.py

# VERL inference
bash scripts/run_verl_inference_server.sh
bash scripts/run_verl_inference_from_ckpt.sh
bash scripts/run_verl_inference_from_model.sh

# Convert FSDP shards → HF model
python scripts/merge_fsdp_to_hf.py --ckpt-dir <...> --output-dir <...>
```

### Citing Us
```
@article{he2026frontiersmith,
  title={FrontierSmith: Synthesizing Open-Ended Coding Problems at Scale},
  author={He, Runyuan and Mang, Qiuyang and Zhou, Shang and Liu, Kaiyuan and Li, Hanchen and Mao, Huanzhi and Zhang, Qizheng and Li, Zerui and Peng, Bo and Cheng, Lufeng and others},
  journal={arXiv preprint arXiv:2605.14445},
  year={2026}
}
```
