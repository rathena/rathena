#!/usr/bin/env python3
"""
Verify ML Library Installation

This script checks that PyTorch, Transformers, and Accelerate are properly installed
and provides detailed information about the installation status and GPU availability.

Usage:
    python verify_ml_libs.py
"""

import sys
from typing import Tuple


def check_torch() -> Tuple[bool, str]:
    """Check PyTorch installation and CUDA/GPU availability."""
    try:
        import torch
        
        version = torch.__version__
        cuda_available = torch.cuda.is_available()
        mps_available = hasattr(torch.backends, 'mps') and torch.backends.mps.is_available()
        
        info = [
            f"✅ PyTorch {version}",
            f"   Installation: {'GPU-enabled' if (cuda_available or mps_available) else 'CPU-only'}"
        ]
        
        if cuda_available:
            info.append(f"   CUDA available: Yes")
            info.append(f"   CUDA version: {torch.version.cuda}")
            info.append(f"   GPU devices: {torch.cuda.device_count()}")
            for i in range(torch.cuda.device_count()):
                info.append(f"      - GPU {i}: {torch.cuda.get_device_name(i)}")
        elif mps_available:
            info.append(f"   MPS (Apple Silicon) available: Yes")
        else:
            info.append(f"   GPU support: Not available (CPU-only)")
        
        return True, "\n".join(info)
    except ImportError as e:
        return False, f"❌ PyTorch not installed: {e}"
    except Exception as e:
        return False, f"❌ PyTorch error: {e}"


def check_transformers() -> Tuple[bool, str]:
    """Check Transformers library installation."""
    try:
        import transformers
        
        version = transformers.__version__
        info = [
            f"✅ Transformers {version}",
            f"   Location: {transformers.__file__}"
        ]
        
        # Check for key components
        try:
            from transformers import AutoModel, AutoTokenizer
            info.append("   Core components: AutoModel, AutoTokenizer available")
        except ImportError:
            info.append("   ⚠️  Warning: Some core components may be missing")
        
        return True, "\n".join(info)
    except ImportError as e:
        return False, f"❌ Transformers not installed: {e}"
    except Exception as e:
        return False, f"❌ Transformers error: {e}"


def check_accelerate() -> Tuple[bool, str]:
    """Check Accelerate library installation."""
    try:
        import accelerate
        
        version = accelerate.__version__
        info = [
            f"✅ Accelerate {version}",
            f"   Location: {accelerate.__file__}"
        ]
        
        # Check for key components
        try:
            from accelerate import Accelerator
            info.append("   Core components: Accelerator available")
        except ImportError:
            info.append("   ⚠️  Warning: Some core components may be missing")
        
        return True, "\n".join(info)
    except ImportError as e:
        return False, f"❌ Accelerate not installed: {e}"
    except Exception as e:
        return False, f"❌ Accelerate error: {e}"


def check_optional_libs() -> str:
    """Check optional ML libraries."""
    info = []
    
    # Check TensorFlow
    try:
        import tensorflow as tf
        info.append(f"✅ TensorFlow {tf.__version__}")
    except ImportError:
        info.append("❌ TensorFlow not installed")
    
    # Check scikit-learn
    try:
        import sklearn
        info.append(f"✅ scikit-learn {sklearn.__version__}")
    except ImportError:
        info.append("❌ scikit-learn not installed")
    
    # Check XGBoost
    try:
        import xgboost as xgb
        info.append(f"✅ XGBoost {xgb.__version__}")
    except ImportError:
        info.append("❌ XGBoost not installed")
    
    # Check FAISS
    try:
        import faiss
        info.append(f"✅ FAISS {faiss.__version__}")
    except ImportError:
        info.append("❌ FAISS not installed")
    
    # Check sentence-transformers
    try:
        import sentence_transformers
        info.append(f"✅ sentence-transformers {sentence_transformers.__version__}")
    except ImportError:
        info.append("❌ sentence-transformers not installed")
    
    return "\n".join(info)


def print_system_info():
    """Print system information."""
    import platform
    
    print("=" * 70)
    print("SYSTEM INFORMATION")
    print("=" * 70)
    print(f"Python version: {sys.version}")
    print(f"Platform: {platform.platform()}")
    print(f"Architecture: {platform.machine()}")
    print()


def main():
    """Main verification function."""
    print_system_info()
    
    print("=" * 70)
    print("ML LIBRARY VERIFICATION")
    print("=" * 70)
    print()
    
    # Check core ML libraries
    print("Core ML Libraries:")
    print("-" * 70)
    
    torch_ok, torch_info = check_torch()
    print(torch_info)
    print()
    
    transformers_ok, transformers_info = check_transformers()
    print(transformers_info)
    print()
    
    accelerate_ok, accelerate_info = check_accelerate()
    print(accelerate_info)
    print()
    
    # Check optional libraries
    print("-" * 70)
    print("Optional ML Libraries:")
    print("-" * 70)
    print(check_optional_libs())
    print()
    
    # Summary
    print("=" * 70)
    print("SUMMARY")
    print("=" * 70)
    
    all_ok = torch_ok and transformers_ok and accelerate_ok
    
    if all_ok:
        print("✅ All core ML libraries installed successfully!")
        print()
        print("You can now use:")
        print("  - PyTorch for neural network training")
        print("  - Transformers for LLM/NLP tasks")
        print("  - Accelerate for distributed training")
        print()
        sys.exit(0)
    else:
        print("❌ Some core ML libraries are missing")
        print()
        print("Installation instructions:")
        print("  pip install torch>=2.4.0 transformers>=4.45.0 accelerate>=0.34.0")
        print()
        print("For GPU support (NVIDIA CUDA):")
        print("  pip install torch torchvision torchaudio --index-url https://download.pytorch.org/whl/cu118")
        print()
        print("For more details, see: docs/ML_SETUP.md")
        print()
        sys.exit(1)


if __name__ == "__main__":
    main()