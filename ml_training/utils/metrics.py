"""
ML Monster AI - Metrics Tracking
Enhanced ML Monster AI System v2.0
"""

import numpy as np
from typing import Dict, List, Any, Optional
from collections import defaultdict, deque
import time
import logging

logger = logging.getLogger(__name__)


class MetricsTracker:
    """
    Track training and evaluation metrics
    
    Features:
    - Running averages
    - Histograms
    - Time-series tracking
    - Export to various formats
    """
    
    def __init__(self, window_size: int = 100):
        """
        Initialize metrics tracker
        
        Args:
            window_size: Window for running averages
        """
        self.window_size = window_size
        self.metrics = defaultdict(lambda: deque(maxlen=window_size))
        self.full_history = defaultdict(list)
        self.start_time = time.time()
        
        logger.info(f"MetricsTracker initialized with window_size={window_size}")
    
    def add(self, name: str, value: float):
        """Add metric value"""
        self.metrics[name].append(value)
        self.full_history[name].append(value)
    
    def get_mean(self, name: str) -> float:
        """Get mean of recent values"""
        if name not in self.metrics or len(self.metrics[name]) == 0:
            return 0.0
        return np.mean(list(self.metrics[name]))
    
    def get_std(self, name: str) -> float:
        """Get standard deviation of recent values"""
        if name not in self.metrics or len(self.metrics[name]) == 0:
            return 0.0
        return np.std(list(self.metrics[name]))
    
    def get_summary(self, name: str) -> Dict[str, float]:
        """Get summary statistics"""
        if name not in self.metrics or len(self.metrics[name]) == 0:
            return {}
        
        values = list(self.metrics[name])
        
        return {
            'mean': np.mean(values),
            'std': np.std(values),
            'min': np.min(values),
            'max': np.max(values),
            'median': np.median(values),
            'count': len(values)
        }
    
    def get_all_summaries(self) -> Dict[str, Dict[str, float]]:
        """Get summaries for all metrics"""
        return {name: self.get_summary(name) for name in self.metrics.keys()}
    
    def reset(self):
        """Reset all metrics"""
        self.metrics.clear()
        self.full_history.clear()
        self.start_time = time.time()
    
    def get_elapsed_time(self) -> float:
        """Get elapsed time since initialization"""
        return time.time() - self.start_time
