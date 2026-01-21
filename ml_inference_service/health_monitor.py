"""
Health Monitor for ML Inference Service
Monitors GPU health, OOM detection, performance tracking
"""

import psutil
import logging
from typing import Dict, Any, Optional
import time


class HealthMonitor:
    """
    System health monitoring including GPU, memory, CPU
    Detects OOM conditions, performance degradation, resource exhaustion
    """
    
    def __init__(self, device: str = "cuda:0"):
        """
        Initialize health monitor
        
        Args:
            device: CUDA device to monitor
        """
        self.logger = logging.getLogger(__name__)
        self.device = device
        self.use_gpu = device.startswith('cuda')
        
        if self.use_gpu:
            self.device_id = int(device.split(':')[1]) if ':' in device else 0
        
        # Thresholds
        self.vram_critical_threshold = 0.95  # 95% usage is critical
        self.vram_warning_threshold = 0.85   # 85% usage is warning
        self.ram_critical_threshold = 0.90   # 90% RAM usage
        self.cpu_warning_threshold = 0.85    # 85% CPU usage
        
        # Health history
        self.last_check_time = time.time()
        self.health_history = []
        self.max_history_size = 100
        
        # OOM detection
        self.oom_detected = False
        self.last_oom_time = None
    
    def check_gpu_health(self) -> Dict[str, Any]:
        """
        Check GPU health and VRAM usage
        
        Returns:
            GPU health dictionary
        """
        if not self.use_gpu:
            return {
                'available': False,
                'reason': 'CPU mode',
                'vram_usage_percent': 0.0
            }
        
        try:
            import torch
            
            # Check availability
            if not torch.cuda.is_available():
                return {
                    'available': False,
                    'reason': 'CUDA not available',
                    'vram_usage_percent': 0.0
                }
            
            # Check device count
            device_count = torch.cuda.device_count()
            if self.device_id >= device_count:
                return {
                    'available': False,
                    'reason': f'Device {self.device_id} not found (count={device_count})',
                    'vram_usage_percent': 0.0
                }
            
            # Get memory info
            torch.cuda.synchronize(self.device_id)
            allocated = torch.cuda.memory_allocated(self.device_id)
            reserved = torch.cuda.memory_reserved(self.device_id)
            total = torch.cuda.get_device_properties(self.device_id).total_memory
            
            vram_usage_percent = allocated / total
            
            # Check for OOM risk
            if vram_usage_percent > self.vram_critical_threshold:
                status = 'critical'
                message = f'VRAM critically high ({vram_usage_percent*100:.1f}%)'
                
                if not self.oom_detected:
                    self.oom_detected = True
                    self.last_oom_time = time.time()
                    self.logger.critical(
                        f"OOM risk detected! VRAM usage: {vram_usage_percent*100:.1f}%",
                        extra={
                            'allocated_mb': allocated / (1024**2),
                            'total_mb': total / (1024**2),
                            'event_type': 'oom_risk'
                        }
                    )
            
            elif vram_usage_percent > self.vram_warning_threshold:
                status = 'warning'
                message = f'VRAM usage high ({vram_usage_percent*100:.1f}%)'
            
            else:
                status = 'healthy'
                message = f'VRAM usage normal ({vram_usage_percent*100:.1f}%)'
                self.oom_detected = False
            
            # Try simple GPU operation to verify functionality
            try:
                test_tensor = torch.randn(100, 100, device=f'cuda:{self.device_id}')
                result = test_tensor @ test_tensor
                torch.cuda.synchronize(self.device_id)
                del test_tensor, result
                functional = True
            except Exception as e:
                functional = False
                status = 'critical'
                message = f'GPU functional test failed: {e}'
            
            return {
                'available': True,
                'functional': functional,
                'status': status,
                'message': message,
                'device_id': self.device_id,
                'allocated_bytes': allocated,
                'allocated_mb': allocated / (1024**2),
                'allocated_gb': allocated / (1024**3),
                'reserved_bytes': reserved,
                'reserved_mb': reserved / (1024**2),
                'total_bytes': total,
                'total_mb': total / (1024**2),
                'total_gb': total / (1024**3),
                'vram_usage_percent': vram_usage_percent,
                'free_mb': (total - allocated) / (1024**2),
                'oom_detected': self.oom_detected
            }
        
        except Exception as e:
            self.logger.error(f"GPU health check error: {e}")
            return {
                'available': False,
                'reason': str(e),
                'vram_usage_percent': 0.0,
                'error': str(e)
            }
    
    def check_ram_health(self) -> Dict[str, Any]:
        """
        Check system RAM health
        
        Returns:
            RAM health dictionary
        """
        try:
            mem = psutil.virtual_memory()
            
            usage_percent = mem.percent / 100.0
            
            if usage_percent > self.ram_critical_threshold:
                status = 'critical'
                message = f'RAM critically high ({usage_percent*100:.1f}%)'
            elif usage_percent > 0.75:
                status = 'warning'
                message = f'RAM usage high ({usage_percent*100:.1f}%)'
            else:
                status = 'healthy'
                message = f'RAM usage normal ({usage_percent*100:.1f}%)'
            
            return {
                'status': status,
                'message': message,
                'total_gb': mem.total / (1024**3),
                'available_gb': mem.available / (1024**3),
                'used_gb': mem.used / (1024**3),
                'usage_percent': usage_percent,
                'free_gb': mem.free / (1024**3)
            }
        
        except Exception as e:
            self.logger.error(f"RAM health check error: {e}")
            return {
                'status': 'unknown',
                'error': str(e)
            }
    
    def check_cpu_health(self) -> Dict[str, Any]:
        """
        Check CPU health
        
        Returns:
            CPU health dictionary
        """
        try:
            cpu_percent = psutil.cpu_percent(interval=0.1)
            cpu_count = psutil.cpu_count()
            
            usage_percent = cpu_percent / 100.0
            
            if usage_percent > self.cpu_warning_threshold:
                status = 'warning'
                message = f'CPU usage high ({cpu_percent:.1f}%)'
            else:
                status = 'healthy'
                message = f'CPU usage normal ({cpu_percent:.1f}%)'
            
            return {
                'status': status,
                'message': message,
                'usage_percent': usage_percent,
                'cpu_count': cpu_count,
                'load_average': psutil.getloadavg()
            }
        
        except Exception as e:
            self.logger.error(f"CPU health check error: {e}")
            return {
                'status': 'unknown',
                'error': str(e)
            }
    
    def check_disk_health(self) -> Dict[str, Any]:
        """
        Check disk space health
        
        Returns:
            Disk health dictionary
        """
        try:
            disk = psutil.disk_usage('/opt/ml_monster_ai')
            
            usage_percent = disk.percent / 100.0
            
            if usage_percent > 0.90:
                status = 'critical'
                message = f'Disk space critically low ({disk.percent:.1f}%)'
            elif usage_percent > 0.80:
                status = 'warning'
                message = f'Disk space running low ({disk.percent:.1f}%)'
            else:
                status = 'healthy'
                message = f'Disk space sufficient ({disk.percent:.1f}%)'
            
            return {
                'status': status,
                'message': message,
                'total_gb': disk.total / (1024**3),
                'used_gb': disk.used / (1024**3),
                'free_gb': disk.free / (1024**3),
                'usage_percent': usage_percent
            }
        
        except Exception as e:
            self.logger.error(f"Disk health check error: {e}")
            return {
                'status': 'unknown',
                'error': str(e)
            }
    
    def perform_full_health_check(self) -> Dict[str, Any]:
        """
        Perform comprehensive health check
        
        Returns:
            Complete health status dictionary
        """
        health = {
            'timestamp': time.time(),
            'timestamp_iso': datetime.utcnow().isoformat() + 'Z',
            'gpu': self.check_gpu_health(),
            'ram': self.check_ram_health(),
            'cpu': self.check_cpu_health(),
            'disk': self.check_disk_health()
        }
        
        # Determine overall status
        statuses = [
            health['gpu'].get('status', 'unknown'),
            health['ram']['status'],
            health['cpu']['status'],
            health['disk']['status']
        ]
        
        if 'critical' in statuses:
            overall_status = 'critical'
        elif 'warning' in statuses:
            overall_status = 'warning'
        elif all(s == 'healthy' for s in statuses if s != 'unknown'):
            overall_status = 'healthy'
        else:
            overall_status = 'degraded'
        
        health['overall_status'] = overall_status
        
        # Update history
        self.health_history.append(health)
        if len(self.health_history) > self.max_history_size:
            self.health_history.pop(0)
        
        self.last_check_time = time.time()
        
        return health
    
    def get_health_summary(self) -> Dict[str, Any]:
        """
        Get summary of recent health checks
        
        Returns:
            Health summary dictionary
        """
        if not self.health_history:
            return {'status': 'no_data', 'message': 'No health checks performed yet'}
        
        latest = self.health_history[-1]
        
        return {
            'overall_status': latest['overall_status'],
            'last_check': latest['timestamp_iso'],
            'gpu_status': latest['gpu'].get('status', 'unknown'),
            'gpu_vram_percent': latest['gpu'].get('vram_usage_percent', 0.0) * 100,
            'ram_percent': latest['ram'].get('usage_percent', 0.0) * 100,
            'cpu_percent': latest['cpu'].get('usage_percent', 0.0) * 100,
            'disk_percent': latest['disk'].get('usage_percent', 0.0) * 100,
            'oom_detected': latest['gpu'].get('oom_detected', False),
            'checks_performed': len(self.health_history)
        }
