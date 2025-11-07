"""
GPU Manager Module
Handles GPU detection, initialization, and management for AI operations
"""

import os
import platform
from typing import Optional, Dict, Any, Tuple
from dataclasses import dataclass
from loguru import logger


@dataclass
class GPUInfo:
    """GPU information dataclass"""
    available: bool
    device_type: str  # cuda, mps, rocm, cpu
    device_name: Optional[str] = None
    device_count: int = 0
    total_memory: Optional[int] = None  # bytes
    compute_capability: Optional[Tuple[int, int]] = None
    driver_version: Optional[str] = None
    cuda_version: Optional[str] = None


class GPUManager:
    """
    GPU Manager for detecting and managing GPU resources
    
    Supports:
    - NVIDIA CUDA (Linux, Windows)
    - Apple Metal Performance Shaders (macOS)
    - AMD ROCm (Linux)
    - CPU fallback
    """
    
    def __init__(self, config: Optional[Any] = None):
        """
        Initialize GPU Manager
        
        Args:
            config: Settings object with GPU configuration
        """
        self.config = config
        self.gpu_info: Optional[GPUInfo] = None
        self.device = None
        self.torch_available = False
        self.cupy_available = False
        self.faiss_gpu_available = False
        
        logger.info("Initializing GPU Manager...")
        
        # Detect GPU availability
        self._detect_gpu()
        
        # Initialize GPU if enabled
        if config and config.gpu_enabled:
            self._initialize_gpu()
        else:
            logger.info("GPU acceleration disabled in configuration")
    
    def _detect_gpu(self) -> None:
        """Detect available GPU and gather information"""
        logger.info("Detecting GPU hardware...")
        
        # Try CUDA (NVIDIA)
        if self._detect_cuda():
            return
        
        # Try MPS (Apple Silicon)
        if self._detect_mps():
            return
        
        # Try ROCm (AMD)
        if self._detect_rocm():
            return
        
        # Fallback to CPU
        logger.info("No GPU detected, using CPU")
        self.gpu_info = GPUInfo(
            available=False,
            device_type="cpu",
            device_name="CPU",
            device_count=os.cpu_count() or 1
        )
    
    def _detect_cuda(self) -> bool:
        """Detect NVIDIA CUDA GPU"""
        try:
            import torch
            self.torch_available = True
            
            if torch.cuda.is_available():
                device_count = torch.cuda.device_count()
                device_name = torch.cuda.get_device_name(0)
                total_memory = torch.cuda.get_device_properties(0).total_memory
                compute_capability = torch.cuda.get_device_capability(0)
                
                # Get CUDA version
                cuda_version = torch.version.cuda
                
                self.gpu_info = GPUInfo(
                    available=True,
                    device_type="cuda",
                    device_name=device_name,
                    device_count=device_count,
                    total_memory=total_memory,
                    compute_capability=compute_capability,
                    cuda_version=cuda_version
                )
                
                logger.info(f"✓ CUDA GPU detected: {device_name}")
                logger.info(f"  - Device Count: {device_count}")
                logger.info(f"  - Total Memory: {total_memory / 1024**3:.2f} GB")
                logger.info(f"  - Compute Capability: {compute_capability}")
                logger.info(f"  - CUDA Version: {cuda_version}")
                
                return True
        except ImportError:
            logger.debug("PyTorch not available, skipping CUDA detection")
        except Exception as e:
            logger.debug(f"CUDA detection failed: {e}")
        
        return False
    
    def _detect_mps(self) -> bool:
        """Detect Apple Metal Performance Shaders (MPS)"""
        if platform.system() != "Darwin":
            return False
        
        try:
            import torch
            self.torch_available = True
            
            if hasattr(torch.backends, 'mps') and torch.backends.mps.is_available():
                self.gpu_info = GPUInfo(
                    available=True,
                    device_type="mps",
                    device_name="Apple Silicon GPU",
                    device_count=1
                )
                
                logger.info("✓ Apple MPS GPU detected")
                logger.info("  - Device: Apple Silicon GPU")
                
                return True
        except ImportError:
            logger.debug("PyTorch not available, skipping MPS detection")
        except Exception as e:
            logger.debug(f"MPS detection failed: {e}")
        
        return False

    def _detect_rocm(self) -> bool:
        """Detect AMD ROCm GPU"""
        try:
            import torch
            self.torch_available = True

            # ROCm uses torch.cuda API
            if torch.cuda.is_available() and hasattr(torch.version, 'hip') and torch.version.hip:
                device_count = torch.cuda.device_count()
                device_name = torch.cuda.get_device_name(0)

                self.gpu_info = GPUInfo(
                    available=True,
                    device_type="rocm",
                    device_name=device_name,
                    device_count=device_count
                )

                logger.info(f"✓ ROCm GPU detected: {device_name}")
                logger.info(f"  - Device Count: {device_count}")

                return True
        except (ImportError, AttributeError):
            logger.debug("PyTorch with ROCm not available")
        except Exception as e:
            logger.debug(f"ROCm detection failed: {e}")

        return False

    def _initialize_gpu(self) -> None:
        """Initialize GPU with configuration settings"""
        if not self.gpu_info or not self.gpu_info.available:
            if self.config.gpu_fallback_to_cpu:
                logger.warning("GPU not available, falling back to CPU")
                return
            else:
                raise RuntimeError("GPU enabled but not available, and fallback to CPU is disabled")

        logger.info(f"Initializing GPU: {self.gpu_info.device_type}")

        try:
            # Set device based on configuration
            device_type = self.config.gpu_device
            if device_type == "auto":
                device_type = self.gpu_info.device_type

            # Initialize PyTorch device
            if self.torch_available:
                import torch

                if device_type == "cuda":
                    self.device = torch.device("cuda:0")

                    # Configure memory settings
                    if self.config.gpu_memory_fraction < 1.0:
                        torch.cuda.set_per_process_memory_fraction(
                            self.config.gpu_memory_fraction,
                            device=0
                        )
                        logger.info(f"  - Memory Fraction: {self.config.gpu_memory_fraction}")

                    # Set memory growth
                    if self.config.gpu_allow_growth:
                        # PyTorch uses dynamic allocation by default
                        logger.info("  - Memory Growth: Enabled (default)")

                elif device_type == "mps":
                    self.device = torch.device("mps")
                    logger.info("  - Using Apple MPS backend")

                elif device_type == "rocm":
                    self.device = torch.device("cuda:0")  # ROCm uses CUDA API
                    logger.info("  - Using ROCm backend")

                else:
                    self.device = torch.device("cpu")
                    logger.info("  - Using CPU backend")

                logger.info(f"✓ PyTorch device initialized: {self.device}")

            # Check for optional GPU libraries
            self._check_optional_libraries()

            # Log memory usage if enabled
            if self.config.gpu_log_memory_usage:
                self._log_memory_usage()

        except Exception as e:
            logger.error(f"GPU initialization failed: {e}")
            if self.config.gpu_fallback_to_cpu:
                logger.warning("Falling back to CPU")
                self.device = None
            else:
                raise

    def _check_optional_libraries(self) -> None:
        """Check availability of optional GPU libraries"""
        logger.info("Checking optional GPU libraries...")

        # Check CuPy
        if self.config.gpu_use_cupy:
            try:
                import cupy
                self.cupy_available = True
                logger.info("  ✓ CuPy available")
            except ImportError:
                logger.info("  ✗ CuPy not available (optional)")

        # Check FAISS-GPU
        if self.config.gpu_use_faiss_gpu:
            try:
                import faiss
                if hasattr(faiss, 'StandardGpuResources'):
                    self.faiss_gpu_available = True
                    logger.info("  ✓ FAISS-GPU available")
                else:
                    logger.info("  ✗ FAISS-GPU not available (CPU version installed)")
            except ImportError:
                logger.info("  ✗ FAISS not available (optional)")

        # Check vLLM
        if self.config.gpu_use_vllm:
            try:
                import vllm
                logger.info("  ✓ vLLM available")
            except ImportError:
                logger.info("  ✗ vLLM not available (optional)")

    def _log_memory_usage(self) -> None:
        """Log current GPU memory usage"""
        if not self.torch_available or not self.gpu_info or not self.gpu_info.available:
            return

        try:
            import torch

            if self.gpu_info.device_type == "cuda":
                allocated = torch.cuda.memory_allocated(0) / 1024**3
                reserved = torch.cuda.memory_reserved(0) / 1024**3
                total = self.gpu_info.total_memory / 1024**3 if self.gpu_info.total_memory else 0

                logger.info(f"GPU Memory Usage:")
                logger.info(f"  - Allocated: {allocated:.2f} GB")
                logger.info(f"  - Reserved: {reserved:.2f} GB")
                logger.info(f"  - Total: {total:.2f} GB")
                logger.info(f"  - Utilization: {(allocated/total*100):.1f}%")
        except Exception as e:
            logger.debug(f"Failed to log memory usage: {e}")

    def get_device(self) -> Optional[Any]:
        """
        Get the current GPU device

        Returns:
            PyTorch device or None if CPU
        """
        return self.device

    def is_available(self) -> bool:
        """
        Check if GPU is available and initialized

        Returns:
            True if GPU is available, False otherwise
        """
        return self.gpu_info is not None and self.gpu_info.available and self.device is not None

    def get_info(self) -> Optional[GPUInfo]:
        """
        Get GPU information

        Returns:
            GPUInfo object or None
        """
        return self.gpu_info

    def clear_cache(self) -> None:
        """Clear GPU cache to free memory"""
        if not self.is_available():
            return

        try:
            import torch
            if self.gpu_info.device_type == "cuda":
                torch.cuda.empty_cache()
                logger.info("GPU cache cleared")
        except Exception as e:
            logger.warning(f"Failed to clear GPU cache: {e}")

    def synchronize(self) -> None:
        """Synchronize GPU operations"""
        if not self.is_available():
            return

        try:
            import torch
            if self.gpu_info.device_type == "cuda":
                torch.cuda.synchronize()
        except Exception as e:
            logger.warning(f"Failed to synchronize GPU: {e}")

    def get_memory_stats(self) -> Dict[str, Any]:
        """
        Get detailed GPU memory statistics

        Returns:
            Dictionary with memory statistics
        """
        if not self.is_available() or not self.torch_available:
            return {"available": False}

        try:
            import torch

            if self.gpu_info.device_type == "cuda":
                return {
                    "available": True,
                    "allocated_gb": torch.cuda.memory_allocated(0) / 1024**3,
                    "reserved_gb": torch.cuda.memory_reserved(0) / 1024**3,
                    "total_gb": self.gpu_info.total_memory / 1024**3 if self.gpu_info.total_memory else 0,
                    "free_gb": (self.gpu_info.total_memory - torch.cuda.memory_allocated(0)) / 1024**3
                               if self.gpu_info.total_memory else 0
                }
        except Exception as e:
            logger.debug(f"Failed to get memory stats: {e}")

        return {"available": False}


# Global GPU manager instance (initialized on import)
_gpu_manager: Optional[GPUManager] = None


def get_gpu_manager(config: Optional[Any] = None) -> GPUManager:
    """
    Get or create global GPU manager instance

    Args:
        config: Settings object with GPU configuration

    Returns:
        GPUManager instance
    """
    global _gpu_manager

    if _gpu_manager is None:
        _gpu_manager = GPUManager(config)

    return _gpu_manager


def initialize_gpu(config: Any) -> GPUManager:
    """
    Initialize GPU manager with configuration

    Args:
        config: Settings object with GPU configuration

    Returns:
        GPUManager instance
    """
    global _gpu_manager
    _gpu_manager = GPUManager(config)
    return _gpu_manager

    def _detect_rocm(self) -> bool:
        """Detect AMD ROCm GPU"""
        try:
            import torch
            self.torch_available = True
            
            # ROCm uses torch.cuda API
            if torch.cuda.is_available() and "rocm" in torch.version.hip:
                device_count = torch.cuda.device_count()
                device_name = torch.cuda.get_device_name(0)
                
                self.gpu_info = GPUInfo(
                    available=True,
                    device_type="rocm",
                    device_name=device_name,
                    device_count=device_count
                )
                
                logger.info(f"✓ ROCm GPU detected: {device_name}")
                logger.info(f"  - Device Count: {device_count}")
                
                return True
        except (ImportError, AttributeError):
            logger.debug("PyTorch with ROCm not available")
        except Exception as e:
            logger.debug(f"ROCm detection failed: {e}")
        
        return False

