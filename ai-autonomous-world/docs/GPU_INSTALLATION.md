# GPU Acceleration - Quick Installation Guide

## 🚀 Quick Start

### Step 1: Check Your Hardware

```bash
# For NVIDIA GPU
nvidia-smi

# For Apple Silicon
system_profiler SPHardwareDataType | grep "Chip"

# For AMD GPU
rocm-smi
```

---

## 📦 Installation by Platform

### NVIDIA CUDA (Linux/Windows)

#### 1. Install CUDA Toolkit
```bash
# Ubuntu/Debian
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get install cuda-toolkit-11-8

# Verify installation
nvcc --version
```

#### 2. Install Python Dependencies
```bash
cd ai-autonomous-world/ai-service

# Install PyTorch with CUDA 11.8
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118

# Install FAISS-GPU
pip install faiss-gpu

# Optional: Install vLLM for high-throughput inference
pip install vllm

# Optional: Install CuPy for GPU NumPy operations
pip install cupy-cuda11x
```

#### 3. Verify Installation
```bash
python -c "import torch; print(f'CUDA available: {torch.cuda.is_available()}')"
python -c "import torch; print(f'GPU: {torch.cuda.get_device_name(0)}')"
python -c "import faiss; print(f'FAISS-GPU: {hasattr(faiss, \"StandardGpuResources\")}')"
```

#### 4. Configure GPU
```bash
# Enable GPU acceleration
export GPU_ENABLED=true
export GPU_DEVICE=cuda
export GPU_MEMORY_FRACTION=0.8

# Enable GPU features
export LLM_USE_GPU=true
export VECTOR_SEARCH_USE_GPU=true
```

---

### Apple Silicon (macOS)

#### 1. Install Python Dependencies
```bash
cd ai-autonomous-world/ai-service

# Install PyTorch (includes MPS support)
pip install torch torchvision torchaudio

# Install FAISS (CPU version, GPU not available on macOS)
pip install faiss-cpu
```

#### 2. Verify Installation
```bash
python -c "import torch; print(f'MPS available: {torch.backends.mps.is_available()}')"
python -c "import faiss; print(f'FAISS version: {faiss.__version__}')"
```

#### 3. Configure GPU
```bash
# Enable GPU acceleration
export GPU_ENABLED=true
export GPU_DEVICE=mps
export GPU_MEMORY_FRACTION=0.8

# Enable GPU features (note: FAISS-GPU not available on macOS)
export LLM_USE_GPU=true
export VECTOR_SEARCH_USE_GPU=false  # Use CPU FAISS
export GPU_USE_FAISS_GPU=false
```

---

### AMD ROCm (Linux)

#### 1. Install ROCm
```bash
# Ubuntu 22.04
wget https://repo.radeon.com/amdgpu-install/5.4.2/ubuntu/jammy/amdgpu-install_5.4.50402-1_all.deb
sudo dpkg -i amdgpu-install_5.4.50402-1_all.deb
sudo amdgpu-install --usecase=rocm

# Verify installation
rocm-smi
```

#### 2. Install Python Dependencies
```bash
cd ai-autonomous-world/ai-service

# Install PyTorch with ROCm 5.4
pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/rocm5.4.2

# Install FAISS-GPU (if available for ROCm)
pip install faiss-gpu
```

#### 3. Verify Installation
```bash
python -c "import torch; print(f'CUDA available: {torch.cuda.is_available()}')"
python -c "import torch; print(f'ROCm version: {torch.version.hip}')"
```

#### 4. Configure GPU
```bash
# Enable GPU acceleration
export GPU_ENABLED=true
export GPU_DEVICE=rocm
export GPU_MEMORY_FRACTION=0.8

# Enable GPU features
export LLM_USE_GPU=true
export VECTOR_SEARCH_USE_GPU=true
```

---

## ✅ Verification

### Test GPU Manager
```bash
cd ai-autonomous-world/ai-service

python -c "
from config import settings
from utils import initialize_gpu

gpu_manager = initialize_gpu(settings)
print(f'GPU Available: {gpu_manager.is_available()}')
if gpu_manager.is_available():
    info = gpu_manager.get_info()
    print(f'Device: {info.device_name}')
    print(f'Type: {info.device_type}')
    print(f'Memory: {info.total_memory / 1024**3:.2f} GB')
"
```

### Test Vector Search
```bash
python -c "
from config import settings
from utils import GPUVectorSearch, get_gpu_manager
import numpy as np

gpu_manager = get_gpu_manager(settings)
vector_search = GPUVectorSearch(settings, gpu_manager)
print(f'Vector Search GPU: {vector_search.use_gpu}')
"
```

---

## 🔧 Troubleshooting

### Issue: "CUDA out of memory"
**Solution**: Reduce memory fraction
```bash
export GPU_MEMORY_FRACTION=0.5
export GPU_BATCH_SIZE=16
```

### Issue: "No GPU detected"
**Solution**: Check drivers and PyTorch installation
```bash
nvidia-smi  # Should show GPU
python -c "import torch; print(torch.cuda.is_available())"  # Should be True
```

### Issue: "vLLM not available"
**Solution**: vLLM is Linux-only
```bash
# On macOS/Windows, disable vLLM
export GPU_USE_VLLM=false
```

---

## 📚 Next Steps

1. Read the [GPU Acceleration Guide](GPU_ACCELERATION.md)
2. Review [Configuration Options](CONFIGURATION.md#gpu-acceleration-configuration)
3. Check [Performance Benchmarks](GPU_ACCELERATION.md#performance-benchmarks)

---

**Last Updated**: 2025-11-06  
**Version**: 1.0.0

