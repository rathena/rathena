"""
GPU-Accelerated Vector Search
Provides GPU acceleration for vector similarity search and embeddings
"""

import numpy as np
from typing import List, Tuple, Optional, Any
from loguru import logger


class GPUVectorSearch:
    """
    GPU-accelerated vector similarity search using FAISS-GPU
    
    Features:
    - GPU-accelerated similarity search
    - Batch embedding generation
    - Efficient memory management
    """
    
    def __init__(self, config: Any, gpu_manager: Any):
        """
        Initialize GPU vector search
        
        Args:
            config: Settings object with GPU configuration
            gpu_manager: GPUManager instance
        """
        self.config = config
        self.gpu_manager = gpu_manager
        self.use_gpu = (
            config.gpu_enabled and 
            config.vector_search_use_gpu and 
            gpu_manager.is_available() and
            gpu_manager.faiss_gpu_available
        )
        
        self.index = None
        self.dimension = None
        self.gpu_resources = None
        
        logger.info(f"GPU Vector Search initialized (GPU: {self.use_gpu})")
        
        if self.use_gpu:
            self._initialize_gpu_resources()
    
    def _initialize_gpu_resources(self) -> None:
        """Initialize FAISS GPU resources"""
        try:
            import faiss
            
            # Create GPU resources
            self.gpu_resources = faiss.StandardGpuResources()
            
            # Configure memory
            if self.config.gpu_memory_fraction < 1.0:
                # Set temporary memory limit
                temp_memory = int(
                    self.gpu_manager.get_info().total_memory * 
                    self.config.gpu_memory_fraction * 0.1  # 10% for temp memory
                )
                self.gpu_resources.setTempMemory(temp_memory)
            
            logger.info("✓ FAISS GPU resources initialized")
            
        except Exception as e:
            logger.error(f"Failed to initialize FAISS GPU resources: {e}")
            self.use_gpu = False
    
    def create_index(self, dimension: int, metric: str = "cosine") -> None:
        """
        Create vector index
        
        Args:
            dimension: Vector dimension
            metric: Distance metric (cosine, l2, ip)
        """
        self.dimension = dimension
        
        try:
            import faiss
            
            # Create CPU index first
            if metric == "cosine":
                # Normalize vectors for cosine similarity
                cpu_index = faiss.IndexFlatIP(dimension)
            elif metric == "l2":
                cpu_index = faiss.IndexFlatL2(dimension)
            elif metric == "ip":
                cpu_index = faiss.IndexFlatIP(dimension)
            else:
                raise ValueError(f"Unknown metric: {metric}")
            
            # Move to GPU if available
            if self.use_gpu:
                self.index = faiss.index_cpu_to_gpu(
                    self.gpu_resources,
                    0,  # GPU device 0
                    cpu_index
                )
                logger.info(f"✓ GPU index created (dimension={dimension}, metric={metric})")
            else:
                self.index = cpu_index
                logger.info(f"CPU index created (dimension={dimension}, metric={metric})")
                
        except Exception as e:
            logger.error(f"Failed to create index: {e}")
            raise
    
    def add_vectors(self, vectors: np.ndarray) -> None:
        """
        Add vectors to index
        
        Args:
            vectors: Numpy array of vectors (n_vectors, dimension)
        """
        if self.index is None:
            raise RuntimeError("Index not created. Call create_index() first.")
        
        try:
            # Ensure vectors are float32
            vectors = vectors.astype(np.float32)
            
            # Normalize for cosine similarity if needed
            if isinstance(self.index, type(self.index)) and "IP" in str(type(self.index)):
                faiss.normalize_L2(vectors)
            
            self.index.add(vectors)
            logger.debug(f"Added {len(vectors)} vectors to index")
            
        except Exception as e:
            logger.error(f"Failed to add vectors: {e}")
            raise
    
    def search(
        self,
        query_vectors: np.ndarray,
        k: int = 5
    ) -> Tuple[np.ndarray, np.ndarray]:
        """
        Search for nearest neighbors
        
        Args:
            query_vectors: Query vectors (n_queries, dimension)
            k: Number of nearest neighbors
            
        Returns:
            Tuple of (distances, indices)
        """
        if self.index is None:
            raise RuntimeError("Index not created. Call create_index() first.")
        
        try:
            # Ensure vectors are float32
            query_vectors = query_vectors.astype(np.float32)
            
            # Normalize for cosine similarity if needed
            if isinstance(self.index, type(self.index)) and "IP" in str(type(self.index)):
                import faiss
                faiss.normalize_L2(query_vectors)
            
            # Search
            distances, indices = self.index.search(query_vectors, k)
            
            logger.debug(f"Searched {len(query_vectors)} queries, k={k}")
            
            return distances, indices
            
        except Exception as e:
            logger.error(f"Search failed: {e}")
            raise
    
    def clear(self) -> None:
        """Clear the index"""
        if self.index is not None:
            self.index.reset()
            logger.debug("Index cleared")
    
    def get_stats(self) -> dict:
        """
        Get index statistics
        
        Returns:
            Dictionary with index stats
        """
        if self.index is None:
            return {"initialized": False}
        
        return {
            "initialized": True,
            "use_gpu": self.use_gpu,
            "dimension": self.dimension,
            "total_vectors": self.index.ntotal,
        }

