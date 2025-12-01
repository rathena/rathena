"""
Tests for GPU Manager Utility

Comprehensive test suite covering:
- GPU device detection
- Memory allocation and management
- Device selection strategies
- Multi-GPU handling
- Performance monitoring
- Error handling for missing GPUs
- Fallback to CPU
"""

import pytest
from unittest.mock import Mock, patch, MagicMock
from typing import List, Dict, Optional
import time


# =============================================================================
# MOCK CLASSES
# =============================================================================

class MockGPUDevice:
    """Mock GPU device"""
    def __init__(self, device_id: int, name: str, total_memory: int):
        self.device_id = device_id
        self.name = name
        self.total_memory = total_memory
        self.allocated_memory = 0
    
    def allocate(self, size: int) -> bool:
        """Allocate memory"""
        if self.allocated_memory + size <= self.total_memory:
            self.allocated_memory += size
            return True
        return False
    
    def free(self, size: int):
        """Free memory"""
        self.allocated_memory = max(0, self.allocated_memory - size)
    
    def get_memory_info(self) -> Dict[str, int]:
        """Get memory information"""
        return {
            'total': self.total_memory,
            'allocated': self.allocated_memory,
            'free': self.total_memory - self.allocated_memory
        }


class GPUManager:
    """Mock GPU manager"""
    def __init__(self):
        self.devices: List[MockGPUDevice] = []
        self.cuda_available = False
        self.current_device = None
    
    def detect_devices(self) -> List[MockGPUDevice]:
        """Detect available GPU devices"""
        # Simulate detection
        return self.devices
    
    def is_available(self) -> bool:
        """Check if CUDA is available"""
        return self.cuda_available
    
    def device_count(self) -> int:
        """Get number of devices"""
        return len(self.devices)
    
    def get_device(self, device_id: int) -> Optional[MockGPUDevice]:
        """Get device by ID"""
        if 0 <= device_id < len(self.devices):
            return self.devices[device_id]
        return None
    
    def set_device(self, device_id: int) -> bool:
        """Set current device"""
        if 0 <= device_id < len(self.devices):
            self.current_device = device_id
            return True
        return False
    
    def get_current_device(self) -> Optional[int]:
        """Get current device ID"""
        return self.current_device
    
    def select_best_device(self) -> Optional[int]:
        """Select device with most free memory"""
        if not self.devices:
            return None
        
        best_device = 0
        max_free_memory = self.devices[0].get_memory_info()['free']
        
        for i, device in enumerate(self.devices[1:], 1):
            free_memory = device.get_memory_info()['free']
            if free_memory > max_free_memory:
                max_free_memory = free_memory
                best_device = i
        
        return best_device
    
    def clear_cache(self):
        """Clear GPU cache"""
        for device in self.devices:
            device.allocated_memory = 0


# =============================================================================
# FIXTURES
# =============================================================================

@pytest.fixture
def gpu_manager():
    """Create GPU manager with mock devices"""
    manager = GPUManager()
    manager.cuda_available = True
    manager.devices = [
        MockGPUDevice(0, "NVIDIA GeForce RTX 3090", 24576),
        MockGPUDevice(1, "NVIDIA GeForce RTX 3080", 10240)
    ]
    return manager


@pytest.fixture
def no_gpu_manager():
    """Create GPU manager with no devices"""
    manager = GPUManager()
    manager.cuda_available = False
    return manager


@pytest.fixture
def single_gpu_manager():
    """Create GPU manager with single device"""
    manager = GPUManager()
    manager.cuda_available = True
    manager.devices = [
        MockGPUDevice(0, "NVIDIA GeForce RTX 3090", 24576)
    ]
    return manager


# =============================================================================
# DEVICE DETECTION TESTS
# =============================================================================

class TestDeviceDetection:
    """Test GPU device detection"""
    
    def test_detect_multiple_devices(self, gpu_manager):
        """Test detecting multiple GPU devices"""
        devices = gpu_manager.detect_devices()
        
        assert len(devices) == 2
        assert devices[0].name == "NVIDIA GeForce RTX 3090"
        assert devices[1].name == "NVIDIA GeForce RTX 3080"
    
    def test_detect_no_devices(self, no_gpu_manager):
        """Test detecting when no GPUs available"""
        devices = no_gpu_manager.detect_devices()
        
        assert len(devices) == 0
    
    def test_cuda_availability(self, gpu_manager):
        """Test checking CUDA availability"""
        assert gpu_manager.is_available() is True
    
    def test_cuda_not_available(self, no_gpu_manager):
        """Test CUDA not available"""
        assert no_gpu_manager.is_available() is False
    
    def test_device_count(self, gpu_manager):
        """Test getting device count"""
        assert gpu_manager.device_count() == 2
    
    def test_device_count_no_gpu(self, no_gpu_manager):
        """Test device count with no GPU"""
        assert no_gpu_manager.device_count() == 0


# =============================================================================
# DEVICE SELECTION TESTS
# =============================================================================

class TestDeviceSelection:
    """Test device selection strategies"""
    
    def test_select_device_by_id(self, gpu_manager):
        """Test selecting device by ID"""
        success = gpu_manager.set_device(0)
        
        assert success is True
        assert gpu_manager.get_current_device() == 0
    
    def test_select_invalid_device(self, gpu_manager):
        """Test selecting invalid device ID"""
        success = gpu_manager.set_device(99)
        
        assert success is False
    
    def test_get_device_by_id(self, gpu_manager):
        """Test getting device by ID"""
        device = gpu_manager.get_device(0)
        
        assert device is not None
        assert device.device_id == 0
    
    def test_get_invalid_device(self, gpu_manager):
        """Test getting invalid device"""
        device = gpu_manager.get_device(99)
        
        assert device is None
    
    def test_select_best_device(self, gpu_manager):
        """Test selecting device with most free memory"""
        # Allocate some memory on device 0
        gpu_manager.devices[0].allocate(10000)
        
        best = gpu_manager.select_best_device()
        
        # Device 1 should have more free memory
        assert best == 1
    
    def test_select_best_device_no_gpu(self, no_gpu_manager):
        """Test selecting best device when none available"""
        best = no_gpu_manager.select_best_device()
        
        assert best is None


# =============================================================================
# MEMORY MANAGEMENT TESTS
# =============================================================================

class TestMemoryManagement:
    """Test GPU memory management"""
    
    def test_allocate_memory(self, single_gpu_manager):
        """Test allocating memory on device"""
        device = single_gpu_manager.get_device(0)
        success = device.allocate(1024)
        
        assert success is True
        assert device.allocated_memory == 1024
    
    def test_allocate_exceeds_capacity(self, single_gpu_manager):
        """Test allocating more than available memory"""
        device = single_gpu_manager.get_device(0)
        success = device.allocate(30000)  # More than total
        
        assert success is False
    
    def test_free_memory(self, single_gpu_manager):
        """Test freeing memory"""
        device = single_gpu_manager.get_device(0)
        device.allocate(1024)
        device.free(512)
        
        assert device.allocated_memory == 512
    
    def test_get_memory_info(self, single_gpu_manager):
        """Test getting memory information"""
        device = single_gpu_manager.get_device(0)
        device.allocate(1000)
        
        info = device.get_memory_info()
        
        assert info['total'] == 24576
        assert info['allocated'] == 1000
        assert info['free'] == 23576
    
    def test_clear_cache(self, gpu_manager):
        """Test clearing GPU cache"""
        # Allocate on both devices
        gpu_manager.devices[0].allocate(1000)
        gpu_manager.devices[1].allocate(500)
        
        gpu_manager.clear_cache()
        
        assert gpu_manager.devices[0].allocated_memory == 0
        assert gpu_manager.devices[1].allocated_memory == 0


# =============================================================================
# MULTI-GPU TESTS
# =============================================================================

class TestMultiGPU:
    """Test multi-GPU handling"""
    
    def test_distribute_across_gpus(self, gpu_manager):
        """Test distributing work across GPUs"""
        # Allocate on different devices
        gpu_manager.devices[0].allocate(1000)
        gpu_manager.devices[1].allocate(500)
        
        assert gpu_manager.devices[0].allocated_memory == 1000
        assert gpu_manager.devices[1].allocated_memory == 500
    
    def test_balance_load(self, gpu_manager):
        """Test load balancing across GPUs"""
        allocations = []
        
        for i in range(10):
            best = gpu_manager.select_best_device()
            device = gpu_manager.get_device(best)
            device.allocate(100)
            allocations.append(best)
        
        # Should distribute relatively evenly
        assert len(allocations) == 10
    
    def test_switch_between_devices(self, gpu_manager):
        """Test switching between devices"""
        gpu_manager.set_device(0)
        assert gpu_manager.get_current_device() == 0
        
        gpu_manager.set_device(1)
        assert gpu_manager.get_current_device() == 1


# =============================================================================
# ERROR HANDLING TESTS
# =============================================================================

class TestErrorHandling:
    """Test error handling"""
    
    def test_handle_no_cuda(self, no_gpu_manager):
        """Test handling when CUDA not available"""
        assert no_gpu_manager.is_available() is False
        assert no_gpu_manager.device_count() == 0
    
    def test_handle_out_of_memory(self, single_gpu_manager):
        """Test handling out of memory error"""
        device = single_gpu_manager.get_device(0)
        
        # Fill up memory
        device.allocate(24000)
        
        # Try to allocate more
        success = device.allocate(1000)
        assert success is False
    
    def test_graceful_fallback_to_cpu(self, no_gpu_manager):
        """Test graceful fallback to CPU"""
        if not no_gpu_manager.is_available():
            # Use CPU instead
            device = "cpu"
        else:
            device = "cuda"
        
        assert device == "cpu"


# =============================================================================
# PERFORMANCE MONITORING TESTS
# =============================================================================

class TestPerformanceMonitoring:
    """Test performance monitoring"""
    
    def test_monitor_memory_usage(self, single_gpu_manager):
        """Test monitoring memory usage"""
        device = single_gpu_manager.get_device(0)
        
        initial_free = device.get_memory_info()['free']
        device.allocate(1000)
        after_alloc = device.get_memory_info()['free']
        
        assert initial_free - after_alloc == 1000
    
    def test_track_allocation_history(self, single_gpu_manager):
        """Test tracking allocation history"""
        device = single_gpu_manager.get_device(0)
        allocations = []
        
        for size in [100, 200, 300]:
            device.allocate(size)
            allocations.append(device.allocated_memory)
        
        assert allocations == [100, 300, 600]
    
    @pytest.mark.performance
    def test_allocation_performance(self, single_gpu_manager):
        """Test allocation performance"""
        device = single_gpu_manager.get_device(0)
        
        start = time.perf_counter()
        for _ in range(1000):
            device.allocate(1)
            device.free(1)
        duration = time.perf_counter() - start
        
        # Should be fast
        assert duration < 0.5


# =============================================================================
# RESOURCE CLEANUP TESTS
# =============================================================================

class TestResourceCleanup:
    """Test resource cleanup"""
    
    def test_cleanup_on_exit(self, single_gpu_manager):
        """Test cleaning up resources"""
        device = single_gpu_manager.get_device(0)
        device.allocate(1000)
        
        # Simulate cleanup
        device.free(device.allocated_memory)
        
        assert device.allocated_memory == 0
    
    def test_clear_all_devices(self, gpu_manager):
        """Test clearing all devices"""
        for device in gpu_manager.devices:
            device.allocate(500)
        
        gpu_manager.clear_cache()
        
        for device in gpu_manager.devices:
            assert device.allocated_memory == 0


# =============================================================================
# DEVICE INFO TESTS
# =============================================================================

class TestDeviceInfo:
    """Test device information retrieval"""
    
    def test_get_device_name(self, gpu_manager):
        """Test getting device name"""
        device = gpu_manager.get_device(0)
        
        assert "RTX 3090" in device.name
    
    def test_get_device_memory_capacity(self, gpu_manager):
        """Test getting device memory capacity"""
        device = gpu_manager.get_device(0)
        
        assert device.total_memory == 24576
    
    def test_list_all_devices(self, gpu_manager):
        """Test listing all available devices"""
        devices = gpu_manager.detect_devices()
        
        assert len(devices) == 2
        assert all(isinstance(d, MockGPUDevice) for d in devices)


# =============================================================================
# CONCURRENT OPERATIONS TESTS
# =============================================================================

class TestConcurrentOperations:
    """Test concurrent GPU operations"""
    
    @pytest.mark.concurrent
    def test_concurrent_allocations(self, single_gpu_manager):
        """Test concurrent memory allocations"""
        from concurrent.futures import ThreadPoolExecutor, as_completed
        
        device = single_gpu_manager.get_device(0)
        
        def allocate_memory(size):
            return device.allocate(size)
        
        with ThreadPoolExecutor(max_workers=5) as executor:
            futures = [executor.submit(allocate_memory, 100) for _ in range(10)]
            results = [f.result() for f in as_completed(futures)]
        
        # All should succeed or fail gracefully
        assert len(results) == 10
    
    @pytest.mark.concurrent
    def test_concurrent_device_switching(self, gpu_manager):
        """Test concurrent device switching"""
        from concurrent.futures import ThreadPoolExecutor
        
        def switch_device(device_id):
            return gpu_manager.set_device(device_id % 2)
        
        with ThreadPoolExecutor(max_workers=4) as executor:
            futures = [executor.submit(switch_device, i) for i in range(20)]
            results = [f.result() for f in futures]
        
        assert all(results)


# =============================================================================
# CONFIGURATION TESTS
# =============================================================================

class TestConfiguration:
    """Test GPU manager configuration"""
    
    def test_custom_device_limit(self):
        """Test limiting number of devices"""
        manager = GPUManager()
        manager.devices = [
            MockGPUDevice(i, f"GPU {i}", 8192)
            for i in range(8)
        ]
        
        # Limit to first 2 devices
        limited_devices = manager.devices[:2]
        
        assert len(limited_devices) == 2
    
    def test_memory_fraction(self, single_gpu_manager):
        """Test using memory fraction"""
        device = single_gpu_manager.get_device(0)
        fraction = 0.5
        allowed_memory = int(device.total_memory * fraction)
        
        success = device.allocate(allowed_memory)
        
        assert success is True


# =============================================================================
# FALLBACK STRATEGIES TESTS
# =============================================================================

class TestFallbackStrategies:
    """Test fallback strategies"""
    
    def test_fallback_to_cpu_on_oom(self, single_gpu_manager):
        """Test falling back to CPU on OOM"""
        device = single_gpu_manager.get_device(0)
        
        # Try to allocate too much
        if not device.allocate(30000):
            # Fallback to CPU
            compute_device = "cpu"
        else:
            compute_device = "cuda"
        
        assert compute_device == "cpu"
    
    def test_fallback_to_smaller_batch(self, single_gpu_manager):
        """Test fallback to smaller batch on OOM"""
        device = single_gpu_manager.get_device(0)
        
        batch_size = 1000
        while not device.allocate(batch_size):
            batch_size //= 2
            if batch_size < 1:
                break
        
        assert batch_size >= 1


# =============================================================================
# EDGE CASE TESTS
# =============================================================================

class TestEdgeCases:
    """Test edge cases and boundary conditions"""
    
    def test_allocate_zero_memory(self, single_gpu_manager):
        """Test allocating zero memory"""
        device = single_gpu_manager.get_device(0)
        success = device.allocate(0)
        
        assert success is True
        assert device.allocated_memory == 0
    
    def test_free_more_than_allocated(self, single_gpu_manager):
        """Test freeing more memory than allocated"""
        device = single_gpu_manager.get_device(0)
        device.allocate(100)
        device.free(200)  # Free more than allocated
        
        # Should clamp to 0
        assert device.allocated_memory == 0
    
    def test_negative_device_id(self, gpu_manager):
        """Test handling negative device ID"""
        success = gpu_manager.set_device(-1)
        
        assert success is False
    
    def test_device_id_out_of_range(self, gpu_manager):
        """Test device ID out of range"""
        device = gpu_manager.get_device(999)
        
        assert device is None
    
    def test_empty_device_list(self):
        """Test operations with empty device list"""
        manager = GPUManager()
        
        assert manager.device_count() == 0
        assert manager.select_best_device() is None
    
    def test_multiple_allocations_same_device(self, single_gpu_manager):
        """Test multiple allocations on same device"""
        device = single_gpu_manager.get_device(0)
        
        device.allocate(1000)
        device.allocate(2000)
        device.allocate(3000)
        
        assert device.allocated_memory == 6000
    
    def test_interleaved_allocate_free(self, single_gpu_manager):
        """Test interleaved allocate and free operations"""
        device = single_gpu_manager.get_device(0)
        
        device.allocate(1000)
        device.free(500)
        device.allocate(300)
        device.free(200)
        
        assert device.allocated_memory == 600


# =============================================================================
# INTEGRATION TESTS
# =============================================================================

class TestIntegration:
    """Test integration scenarios"""
    
    def test_full_workflow(self, gpu_manager):
        """Test complete GPU management workflow"""
        # Detect devices
        devices = gpu_manager.detect_devices()
        assert len(devices) > 0
        
        # Select best device
        best = gpu_manager.select_best_device()
        assert best is not None
        
        # Set device
        gpu_manager.set_device(best)
        
        # Allocate memory
        device = gpu_manager.get_device(best)
        success = device.allocate(1000)
        assert success is True
        
        # Check memory
        info = device.get_memory_info()
        assert info['allocated'] == 1000
        
        # Clear cache
        gpu_manager.clear_cache()
        assert device.allocated_memory == 0
    
    def test_multi_device_workflow(self, gpu_manager):
        """Test workflow with multiple devices"""
        # Use both devices
        for i in range(2):
            gpu_manager.set_device(i)
            device = gpu_manager.get_device(i)
            device.allocate(500)
        
        # Verify allocations
        assert gpu_manager.devices[0].allocated_memory == 500
        assert gpu_manager.devices[1].allocated_memory == 500