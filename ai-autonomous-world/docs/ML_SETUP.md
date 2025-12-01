# ML Libraries Setup Guide

This guide covers the installation and configuration of PyTorch, Transformers, and Accelerate libraries for the AI Autonomous World project.

## 📋 Table of Contents

- [Overview](#overview)
- [Requirements](#requirements)
- [Installation](#installation)
  - [Standard Installation (CPU)](#standard-installation-cpu)
  - [GPU Installation (NVIDIA CUDA)](#gpu-installation-nvidia-cuda)
  - [Apple Silicon (M1/M2/M3)](#apple-silicon-m1m2m3)
  - [AMD ROCm (Linux)](#amd-rocm-linux)
  - [Cloud Deployment](#cloud-deployment)
- [Verification](#verification)
- [Troubleshooting](#troubleshooting)
- [Configuration](#configuration)
- [Best Practices](#best-practices)

---

## 🎯 Overview

The AI service requires these core ML libraries:

| Library | Version | Purpose |
|---------|---------|---------|
| **PyTorch** | ≥2.4.0 | Neural network training, reinforcement learning (PPO) |
| **Transformers** | ≥4.45.0 | LLM model loading, tokenization, NLP tasks |
| **Accelerate** | ≥0.34.0 | Multi-GPU support, distributed training, mixed precision |

**Total Size:**
- CPU-only: ~1.2GB
- GPU (CUDA): ~3.5GB

---

## ⚙️ Requirements

### System Requirements

**Minimum:**
- Python 3.10+
- 4GB RAM
- 5GB disk space

**Recommended:**
- Python 3.11+
- 8GB+ RAM
- 10GB+ disk space
- GPU: 8GB+ VRAM (for local inference)

### Software Prerequisites

```bash
# Verify Python version
python --version  # Should be 3.10 or higher

# Upgrade pip
pip install --upgrade pip setuptools wheel
```

---

## 📦 Installation

### Standard Installation (CPU)

For development or cloud deployments without GPU:

```bash
# Navigate to project directory
cd rathena-AI-world/ai-autonomous-world/ai-service

# Install all dependencies (includes PyTorch CPU)
pip install -r requirements.txt

# Or install ML libraries individually
pip install torch>=2.4.0 transformers>=4.45.0 accelerate>=0.34.0
```

**Note:** The default `torch` installation uses CPU-only binaries.

---

### GPU Installation (NVIDIA CUDA)

#### Prerequisites

1. **NVIDIA GPU** with CUDA Compute Capability 3.5+
2. **CUDA Toolkit** 11.8 or 12.1
3. **cuDNN** (included with PyTorch)

#### Installation Steps

```bash
# Step 1: Install CUDA-enabled PyTorch
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118

# For CUDA 12.1 (newer GPUs)
# pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu121

# Step 2: Install remaining dependencies
pip install transformers>=4.45.0 accelerate>=0.34.0

# Step 3: Install GPU-optimized requirements
pip install -r requirements-gpu.txt
```

#### Verify GPU Installation

```bash
python -c "import torch; print(f'CUDA available: {torch.cuda.is_available()}')"
python -c "import torch; print(f'CUDA version: {torch.version.cuda}')"
python -c "import torch; print(f'GPU count: {torch.cuda.device_count()}')"
```

---

### Apple Silicon (M1/M2/M3)

PyTorch supports Apple's Metal Performance Shaders (MPS) for GPU acceleration:

```bash
# Install PyTorch with MPS support (standard installation)
pip install torch>=2.4.0 transformers>=4.45.0 accelerate>=0.34.0

# Install remaining dependencies
pip install -r requirements.txt
```

#### Verify MPS Installation

```bash
python -c "import torch; print(f'MPS available: {torch.backends.mps.is_available()}')"
```

**Note:** MPS is automatically used when available. No additional configuration needed.

---

### AMD ROCm (Linux)

For AMD GPUs on Linux:

```bash
# Install ROCm-enabled PyTorch
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/rocm5.7

# Install remaining dependencies
pip install transformers>=4.45.0 accelerate>=0.34.0
pip install -r requirements-gpu.txt
```

**Supported AMD GPUs:** Radeon RX 6000/7000 series, Radeon Instinct, EPYC processors

---

### Cloud Deployment

For serverless or container-based deployments:

```bash
# Use cloud-optimized requirements (smaller footprint)
pip install -r requirements-cloud.txt

# Or minimal installation (no local ML training)
pip install -r requirements-minimal.txt
```

**Docker Installation:**

```dockerfile
FROM python:3.11-slim

WORKDIR /app

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Install ML libraries
COPY requirements-cloud.txt .
RUN pip install --no-cache-dir -r requirements-cloud.txt

COPY . .
```

---

## ✅ Verification

Run the verification script to check your installation:

```bash
python verify_ml_libs.py
```

### Expected Output

```
======================================================================
SYSTEM INFORMATION
======================================================================
Python version: 3.11.x
Platform: Linux-x.x.x
Architecture: x86_64

======================================================================
ML LIBRARY VERIFICATION
======================================================================

Core ML Libraries:
----------------------------------------------------------------------
✅ PyTorch 2.4.0
   Installation: GPU-enabled
   CUDA available: Yes
   CUDA version: 11.8
   GPU devices: 1
      - GPU 0: NVIDIA GeForce RTX 3090

✅ Transformers 4.45.0
   Location: /path/to/transformers
   Core components: AutoModel, AutoTokenizer available

✅ Accelerate 0.34.0
   Location: /path/to/accelerate
   Core components: Accelerator available

----------------------------------------------------------------------
Optional ML Libraries:
----------------------------------------------------------------------
✅ TensorFlow 2.20.0
✅ scikit-learn 1.7.2
✅ XGBoost 3.1.1
✅ FAISS 1.12.0
✅ sentence-transformers 5.1.2

======================================================================
SUMMARY
======================================================================
✅ All core ML libraries installed successfully!
```

---

## 🔧 Troubleshooting

### Issue: PyTorch Not Found

```bash
# Solution: Reinstall PyTorch
pip uninstall torch
pip install torch>=2.4.0
```

### Issue: CUDA Not Available (GPU Installation)

**Check NVIDIA driver:**
```bash
nvidia-smi
```

**Reinstall CUDA-enabled PyTorch:**
```bash
pip uninstall torch torchvision torchaudio
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118
```

### Issue: Out of Memory (OOM)

**Reduce batch size in configuration:**
```python
# config.yaml
ml:
  batch_size: 8  # Reduce from 32
  gradient_accumulation_steps: 4
```

**Enable memory-efficient attention:**
```python
import torch
torch.backends.cuda.enable_mem_efficient_sdp(True)
```

### Issue: Slow Performance (CPU)

**Enable oneDNN optimizations:**
```python
import torch
torch.set_num_threads(4)  # Adjust based on CPU cores
```

### Issue: Import Errors

```bash
# Clear pip cache and reinstall
pip cache purge
pip install --upgrade --force-reinstall torch transformers accelerate
```

---

## ⚙️ Configuration

### Environment Variables

Create or update `.env`:

```bash
# ML Configuration
ML_DEVICE=cuda  # Options: cuda, cpu, mps
ML_DTYPE=float16  # Options: float32, float16, bfloat16
ML_BATCH_SIZE=16
ML_MAX_LENGTH=512

# PyTorch Settings
PYTORCH_CUDA_ALLOC_CONF=max_split_size_mb:512
CUDA_VISIBLE_DEVICES=0  # GPU device IDs (comma-separated)

# Transformers Cache
HF_HOME=/path/to/huggingface/cache
TRANSFORMERS_CACHE=/path/to/transformers/cache
```

### Model Configuration

```yaml
# config.yaml
ml:
  device: cuda  # auto, cuda, cpu, mps
  dtype: float16  # float32, float16, bfloat16
  
  # Training
  batch_size: 16
  learning_rate: 5e-5
  max_epochs: 10
  
  # Inference
  max_length: 512
  temperature: 0.7
  top_p: 0.9
  
  # Memory Management
  gradient_checkpointing: true
  mixed_precision: true
  gradient_accumulation_steps: 4
```

---

## 🎯 Best Practices

### 1. **Version Pinning**

Always specify minimum versions in production:

```txt
torch>=2.4.0,<3.0.0
transformers>=4.45.0,<5.0.0
accelerate>=0.34.0,<1.0.0
```

### 2. **Memory Management**

```python
import torch
import gc

# Clear GPU memory after training
torch.cuda.empty_cache()
gc.collect()

# Enable gradient checkpointing for large models
model.gradient_checkpointing_enable()
```

### 3. **Performance Optimization**

```python
# Enable TF32 for Ampere GPUs (RTX 30xx/40xx)
torch.backends.cuda.matmul.allow_tf32 = True
torch.backends.cudnn.allow_tf32 = True

# Enable cuDNN auto-tuner
torch.backends.cudnn.benchmark = True
```

### 4. **Model Caching**

```python
from transformers import AutoModel

# Cache models locally
model = AutoModel.from_pretrained(
    "model-name",
    cache_dir="/path/to/cache",
    local_files_only=False  # Set True for offline mode
)
```

### 5. **Multi-GPU Training**

```python
from accelerate import Accelerator

accelerator = Accelerator()
model, optimizer, dataloader = accelerator.prepare(
    model, optimizer, dataloader
)
```

---

## 📚 Additional Resources

- **PyTorch Documentation:** https://pytorch.org/docs/stable/index.html
- **Transformers Documentation:** https://huggingface.co/docs/transformers
- **Accelerate Documentation:** https://huggingface.co/docs/accelerate
- **CUDA Toolkit:** https://developer.nvidia.com/cuda-toolkit
- **ROCm Documentation:** https://docs.amd.com/

---

## 🆘 Support

If you encounter issues:

1. **Check verification script:** `python verify_ml_libs.py`
2. **Review logs:** Check `/var/log/ai-service/` for errors
3. **GitHub Issues:** Report bugs or ask questions
4. **Discord Community:** Join for real-time support

---

## 📝 Changelog

### 2024-11-24
- Initial ML setup guide
- Added PyTorch 2.4.0+ support
- Added Transformers 4.45.0+ support
- Added Accelerate 0.34.0+ support
- Added GPU, Apple Silicon, and ROCm instructions

---

**Last Updated:** 2024-11-24  
**Maintained By:** AI Autonomous World Team