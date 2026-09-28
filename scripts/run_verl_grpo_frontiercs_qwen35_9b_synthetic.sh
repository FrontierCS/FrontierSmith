#!/bin/bash
# GRPO training for Qwen3.5-9B on Frontier-CS synthetic (VERL)
#
# Prerequisites:
#   1. source setup-env.sh   (creates .venv and installs deps if needed)
#   2. python scripts/prepare_synthetic_parquet.py --subset train
#   3. Frontier-CS judge on :8082 (cd Frontier-CS/algorithmic && ./run_judge.sh)
#   4. (optional) python scripts/prepare_alebench_parquet.py
#   5. 4+ GPUs (default 4; set NGPU=8 for 8-GPU machines)
#
# Usage:
#   bash scripts/run_verl_grpo_frontiercs_qwen35_9b_synthetic.sh
#   NGPU=8 TP=2 bash scripts/run_verl_grpo_frontiercs_qwen35_9b_synthetic.sh
#
# Start from scratch (backup existing checkpoints, do not delete):
#   FRESH_START=1 bash scripts/run_verl_grpo_frontiercs_qwen35_9b_synthetic.sh

set -x

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$PROJECT_ROOT"

# ── Environment ──────────────────────────────────────────────────────────────
if [ -f "$PROJECT_ROOT/.venv/bin/activate" ]; then
    source "$PROJECT_ROOT/.venv/bin/activate"
else
    echo "ERROR: .venv not found. Run 'source setup-env.sh' first."
    exit 1
fi

export PYTHONPATH="$PROJECT_ROOT/verl${PYTHONPATH:+:$PYTHONPATH}"
export HF_HOME="${HF_HOME:-$PROJECT_ROOT/.cache/huggingface}"
export HF_TOKEN="${HF_TOKEN:-$(cat "$HOME/.cache/huggingface/token" 2>/dev/null || true)}"
export VLLM_CACHE_DIR="${VLLM_CACHE_DIR:-$PROJECT_ROOT/.cache/vllm}"
export RAY_TMPDIR="${RAY_TMPDIR:-/tmp/ray}"
export ALE_BENCH_CACHE_DIR="${ALE_BENCH_CACHE_DIR:-$PROJECT_ROOT/.cache/ale-bench}"
export ALEBENCH_JUDGE_VERSION=${ALEBENCH_JUDGE_VERSION:-202301}

MODEL_PATH=${MODEL_PATH:-Qwen/Qwen3.5-9B}

TRAIN_DATA=${TRAIN_DATA:-$PROJECT_ROOT/data/frontiercs/train_synthetic_200.parquet}
VAL_DATA=${VAL_DATA:-$PROJECT_ROOT/data/frontiercs/full.parquet}
ALEBENCH_VAL_DATA=${ALEBENCH_VAL_DATA:-$PROJECT_ROOT/data/alebench/val.parquet}

TP=${TP:-2}
NGPU=${NGPU:-8}

# Ensure data exists
if [ ! -f "$TRAIN_DATA" ]; then
    echo "ERROR: $TRAIN_DATA not found."
    echo "Run: python scripts/prepare_synthetic_parquet.py --subset train"
    exit 1
fi

TRAIN_FILES="['$TRAIN_DATA']"

# Validate against Frontier-CS + ALE-Bench (when present).
VAL_LIST=()
if [ -f "$VAL_DATA" ]; then
    VAL_LIST+=("'$VAL_DATA'")
else
    VAL_LIST+=("'$TRAIN_DATA'")
fi
if [ -f "$ALEBENCH_VAL_DATA" ]; then
    VAL_LIST+=("'$ALEBENCH_VAL_DATA'")
    echo "[ALE-Bench] Including $ALEBENCH_VAL_DATA in validation."
else
    echo "[ALE-Bench] $ALEBENCH_VAL_DATA not found; run scripts/prepare_alebench_parquet.py to enable."
fi
VAL_FILES="[$(IFS=,; echo "${VAL_LIST[*]}")]"

export RAY_EXPERIMENTAL_NOSET_CUDA_VISIBLE_DEVICES=1

# ── Checkpoints & outputs (all in project dir) ──────────────────────────────
CKPT_DIR="$PROJECT_ROOT/checkpoints/verl_frontiercs_qwen35_9b_synthetic/qwen35_9b_grpo_synthetic"
ROLLOUT_DIR="$PROJECT_ROOT/outputs/rollout_data_synthetic"
mkdir -p "$CKPT_DIR" "$ROLLOUT_DIR"

FRESH_START_ARGS=""
if [ "${FRESH_START:-0}" = "1" ]; then
    if [ -d "$CKPT_DIR" ]; then
        BACKUP_DIR="${CKPT_DIR}_backup_$(date +%Y%m%d_%H%M%S)"
        echo "[FRESH_START] Backing up checkpoints: $CKPT_DIR -> $BACKUP_DIR"
        mv "$CKPT_DIR" "$BACKUP_DIR"
        echo "[FRESH_START] Backup done."
    fi
    FRESH_START_ARGS="trainer.resume_mode=disable"
    echo "[FRESH_START] Training will start from scratch (resume_mode=disable)."
fi

python -m verl.trainer.main_ppo \
    algorithm.adv_estimator=grpo \
    data.train_files="$TRAIN_FILES" \
    data.val_files="$VAL_FILES" \
    data.train_batch_size=8 \
    data.max_prompt_length=8192 \
    data.max_response_length=16000 \
    data.filter_overlong_prompts=True \
    data.truncation=error \
    data.prompt_key=prompt \
    actor_rollout_ref.model.path=$MODEL_PATH \
    actor_rollout_ref.model.use_remove_padding=True \
    +actor_rollout_ref.model.override_config.attn_implementation=sdpa \
    actor_rollout_ref.actor.optim.lr=1e-6 \
    actor_rollout_ref.actor.ppo_mini_batch_size=8 \
    actor_rollout_ref.actor.ppo_micro_batch_size_per_gpu=1 \
    actor_rollout_ref.actor.use_kl_loss=True \
    actor_rollout_ref.actor.kl_loss_coef=0.001 \
    actor_rollout_ref.actor.kl_loss_type=low_var_kl \
    actor_rollout_ref.actor.entropy_coeff=0 \
    actor_rollout_ref.model.enable_gradient_checkpointing=True \
    actor_rollout_ref.actor.fsdp_config.param_offload=True \
    actor_rollout_ref.rollout.tensor_model_parallel_size=$TP \
    actor_rollout_ref.rollout.log_prob_micro_batch_size_per_gpu=1 \
    actor_rollout_ref.rollout.name=vllm \
    actor_rollout_ref.rollout.checkpoint_engine.update_weights_bucket_megabytes=8192 \
    actor_rollout_ref.rollout.gpu_memory_utilization=0.6 \
    +actor_rollout_ref.rollout.engine_kwargs.vllm.attention_backend=FLASH_ATTN \
    actor_rollout_ref.rollout.n=8 \
    actor_rollout_ref.rollout.val_kwargs.n=5 \
    actor_rollout_ref.rollout.val_kwargs.do_sample=True \
    actor_rollout_ref.rollout.val_kwargs.temperature=1.0 \
    actor_rollout_ref.rollout.val_kwargs.top_p=1.0 \
    actor_rollout_ref.ref.log_prob_micro_batch_size_per_gpu=1 \
    actor_rollout_ref.ref.fsdp_config.param_offload=True \
    algorithm.use_kl_in_reward=False \
    trainer.critic_warmup=0 \
    trainer.logger='["console","wandb"]' \
    trainer.project_name=verl_frontiercs_qwen35_9b_synthetic \
    trainer.default_local_dir=$CKPT_DIR \
    trainer.rollout_data_dir=$ROLLOUT_DIR \
    trainer.experiment_name=qwen35_9b_grpo_synthetic \
    trainer.n_gpus_per_node=$NGPU \
    trainer.nnodes=1 \
    trainer.total_epochs=60 \
    trainer.save_freq=10 \
    trainer.test_freq=10 \
    $FRESH_START_ARGS \
    "$@"
