#!/usr/bin/env python3
"""
ML Monster AI - ONNX Export Script
Enhanced ML Monster AI System v2.0

Export all trained PyTorch models to ONNX FP16 format for production inference

Usage:
    python export_to_onnx.py --all
    python export_to_onnx.py --archetype aggressive --model-type combat_dqn
"""

import sys
import os
import argparse
import logging
from pathlib import Path
import torch
import torch.onnx
import numpy as np
from typing import Dict, List, Optional

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from models.model_factory import ModelFactory
from models.model_architectures import estimate_model_size

logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


def export_model_to_onnx(
    model: torch.nn.Module,
    archetype: str,
    model_type: str,
    example_input: torch.Tensor,
    output_dir: str = '/opt/ml_monster_ai/models',
    use_fp16: bool = True,
    opset_version: int = 17
) -> str:
    """
    Export PyTorch model to ONNX FP16
    
    Args:
        model: PyTorch model
        archetype: Model archetype
        model_type: Model type
        example_input: Example input tensor
        output_dir: Output directory
        use_fp16: Convert to FP16
        opset_version: ONNX opset version
    
    Returns:
        Path to exported ONNX file
    """
    model.eval()
    
    # Convert to FP16 if requested
    if use_fp16:
        model = model.half()
        example_input = example_input.half()
    
    # Create output directory
    arch_dir = Path(output_dir) / archetype
    arch_dir.mkdir(parents=True, exist_ok=True)
    
    # Output path
    precision_suffix = '_fp16' if use_fp16 else '_fp32'
    output_path = arch_dir / f"{model_type}{precision_suffix}.onnx"
    
    # Determine input/output names based on model type
    if model_type in ['movement_ppo', 'team_coordination']:
        # Actor-critic models have multiple outputs
        output_names = ['action_logits', 'value']
    elif model_type == 'soft_actor_critic':
        output_names = ['q1', 'q2']
    else:
        output_names = ['output']
    
    # Export to ONNX
    torch.onnx.export(
        model,
        example_input,
        str(output_path),
        export_params=True,
        opset_version=opset_version,
        do_constant_folding=True,
        input_names=['state'],
        output_names=output_names,
        dynamic_axes={
            'state': {0: 'batch_size'},
            output_names[0]: {0: 'batch_size'}
        },
        verbose=False
    )
    
    # Verify export
    import onnx
    onnx_model = onnx.load(str(output_path))
    onnx.checker.check_model(onnx_model)
    
    # Get file size
    file_size_mb = os.path.getsize(output_path) / (1024 ** 2)
    
    logger.info(f"✓ Exported {archetype}/{model_type} to ONNX")
    logger.info(f"  Path: {output_path}")
    logger.info(f"  Size: {file_size_mb:.2f} MB")
    logger.info(f"  Precision: {'FP16' if use_fp16 else 'FP32'}")
    logger.info(f"  Opset: {opset_version}")
    
    return str(output_path)


def get_example_input(
    model_type: str,
    state_dim: int = 64,
    device: str = 'cuda',
    sequence_length: int = 10
) -> torch.Tensor:
    """
    Get example input for model export
    
    Args:
        model_type: Type of model
        state_dim: State dimension
        device: Device
        sequence_length: Sequence length (for sequence models)
    
    Returns:
        Example input tensor
    """
    if model_type in ['team_coordination', 'temporal_transformer', 'pattern_recognition']:
        # Sequence models
        example_input = torch.randn(1, sequence_length, state_dim, device=device)
    else:
        # Single state models
        example_input = torch.randn(1, state_dim, device=device)
    
    return example_input


def export_all_models(
    archetypes: Optional[List[str]] = None,
    model_types: Optional[List[str]] = None,
    output_dir: str = '/opt/ml_monster_ai/models',
    model_dir: str = '/opt/ml_monster_ai/models',
    device: str = 'cuda:0',
    use_fp16: bool = True
) -> Dict[str, List[str]]:
    """
    Export all trained models to ONNX
    
    Args:
        archetypes: List of archetypes (None = all)
        model_types: List of model types (None = all)
        output_dir: Output directory for ONNX files
        model_dir: Directory with trained PyTorch models
        device: Device for export
        use_fp16: Convert to FP16
    
    Returns:
        Dictionary mapping archetype -> list of exported model paths
    """
    if archetypes is None:
        archetypes = ModelFactory.ARCHETYPES
    
    if model_types is None:
        model_types = ModelFactory.MODEL_TYPES
    
    total = len(archetypes) * len(model_types)
    current = 0
    
    exported_models = {arch: [] for arch in archetypes}
    failed_exports = []
    
    logger.info(f"Exporting {total} models to ONNX...")
    logger.info(f"Output directory: {output_dir}")
    logger.info(f"Precision: {'FP16' if use_fp16 else 'FP32'}\n")
    
    total_size_mb = 0.0
    
    for archetype in archetypes:
        for model_type in model_types:
            current += 1
            
            try:
                logger.info(f"[{current}/{total}] Exporting {archetype}/{model_type}...")
                
                # Load PyTorch model
                model = ModelFactory.load_model(
                    archetype=archetype,
                    model_type=model_type,
                    version='latest',
                    device=device,
                    base_dir=model_dir
                )
                
                # Get example input
                example_input = get_example_input(model_type, device=device)
                
                # Export to ONNX
                onnx_path = export_model_to_onnx(
                    model=model,
                    archetype=archetype,
                    model_type=model_type,
                    example_input=example_input,
                    output_dir=output_dir,
                    use_fp16=use_fp16
                )
                
                exported_models[archetype].append(onnx_path)
                
                # Track total size
                file_size_mb = os.path.getsize(onnx_path) / (1024 ** 2)
                total_size_mb += file_size_mb
                
                # Clean up
                del model
                torch.cuda.empty_cache()
            
            except FileNotFoundError as e:
                logger.warning(f"✗ {archetype}/{model_type} - Model not found (skipping)")
                failed_exports.append(f"{archetype}/{model_type}")
            
            except Exception as e:
                logger.error(f"✗ {archetype}/{model_type} - Export failed: {e}")
                failed_exports.append(f"{archetype}/{model_type}")
    
    # Summary
    logger.info(f"\n{'='*60}")
    logger.info(f"ONNX EXPORT SUMMARY")
    logger.info(f"{'='*60}")
    logger.info(f"Total Exported: {current - len(failed_exports)}/{total}")
    logger.info(f"Failed: {len(failed_exports)}/{total}")
    logger.info(f"Total Size: {total_size_mb:.2f} MB ({total_size_mb/1024:.2f} GB)")
    
    if failed_exports:
        logger.warning(f"\nFailed exports:")
        for model_name in failed_exports:
            logger.warning(f"  - {model_name}")
    
    logger.info(f"\nONNX models saved to: {output_dir}")
    
    return exported_models


def verify_onnx_model(onnx_path: str, device: str = 'cuda:0') -> bool:
    """
    Verify ONNX model can be loaded and executed
    
    Args:
        onnx_path: Path to ONNX model
        device: Device for verification
    
    Returns:
        True if model is valid
    """
    try:
        import onnxruntime as ort
        
        # Set providers based on device
        if device.startswith('cuda'):
            providers = ['CUDAExecutionProvider', 'CPUExecutionProvider']
        else:
            providers = ['CPUExecutionProvider']
        
        # Create session
        session = ort.InferenceSession(onnx_path, providers=providers)
        
        # Get input shape
        input_shape = session.get_inputs()[0].shape
        
        # Create dummy input
        dummy_input = np.random.randn(*[d if isinstance(d, int) else 1 for d in input_shape]).astype(np.float32)
        
        # Run inference
        outputs = session.run(None, {'state': dummy_input})
        
        logger.info(f"✓ ONNX model verified: {onnx_path}")
        logger.info(f"  Input shape: {input_shape}")
        logger.info(f"  Output shapes: {[o.shape for o in outputs]}")
        
        return True
    
    except Exception as e:
        logger.error(f"✗ ONNX model verification failed: {e}")
        return False


def optimize_onnx_model(onnx_path: str) -> str:
    """
    Optimize ONNX model for inference
    
    Args:
        onnx_path: Path to ONNX model
    
    Returns:
        Path to optimized model
    """
    try:
        import onnx
        from onnxruntime.transformers import optimizer
        
        # Load model
        model = onnx.load(onnx_path)
        
        # Optimize
        optimized_model = optimizer.optimize_model(
            onnx_path,
            model_type='bert',  # Generic transformer optimization
            num_heads=0,  # Auto-detect
            hidden_size=0  # Auto-detect
        )
        
        # Save optimized model
        optimized_path = onnx_path.replace('.onnx', '_optimized.onnx')
        optimized_model.save_model_to_file(optimized_path)
        
        # Check size reduction
        original_size = os.path.getsize(onnx_path) / (1024 ** 2)
        optimized_size = os.path.getsize(optimized_path) / (1024 ** 2)
        reduction = (1 - optimized_size / original_size) * 100
        
        logger.info(f"✓ Model optimized: {reduction:.1f}% size reduction")
        logger.info(f"  {original_size:.2f} MB → {optimized_size:.2f} MB")
        
        return optimized_path
    
    except Exception as e:
        logger.warning(f"Optimization failed: {e}, using original model")
        return onnx_path


def parse_args():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description='Export models to ONNX')
    
    parser.add_argument(
        '--archetypes',
        nargs='+',
        choices=ModelFactory.ARCHETYPES,
        help='Archetypes to export'
    )
    
    parser.add_argument(
        '--model-types',
        nargs='+',
        choices=ModelFactory.MODEL_TYPES,
        help='Model types to export'
    )
    
    parser.add_argument(
        '--all',
        action='store_true',
        help='Export all models'
    )
    
    parser.add_argument(
        '--model-dir',
        type=str,
        default='/opt/ml_monster_ai/models',
        help='Directory with trained models'
    )
    
    parser.add_argument(
        '--output-dir',
        type=str,
        default='/opt/ml_monster_ai/models',
        help='Output directory for ONNX models'
    )
    
    parser.add_argument(
        '--device',
        type=str,
        default='cuda:0',
        help='Device for export'
    )
    
    parser.add_argument(
        '--fp32',
        action='store_true',
        help='Export as FP32 instead of FP16'
    )
    
    parser.add_argument(
        '--verify',
        action='store_true',
        help='Verify exported models'
    )
    
    parser.add_argument(
        '--optimize',
        action='store_true',
        help='Optimize ONNX models'
    )
    
    return parser.parse_args()


def main():
    """Main export entry point"""
    args = parse_args()
    
    # Determine what to export
    if args.all:
        archetypes = ModelFactory.ARCHETYPES
        model_types = ModelFactory.MODEL_TYPES
    else:
        archetypes = args.archetypes or ModelFactory.ARCHETYPES
        model_types = args.model_types or ModelFactory.MODEL_TYPES
    
    use_fp16 = not args.fp32
    
    logger.info(f"{'='*60}")
    logger.info(f"ONNX EXPORT")
    logger.info(f"{'='*60}")
    logger.info(f"Archetypes: {archetypes}")
    logger.info(f"Model Types: {model_types}")
    logger.info(f"Total: {len(archetypes) * len(model_types)} models")
    logger.info(f"Precision: {'FP16' if use_fp16 else 'FP32'}")
    logger.info(f"Device: {args.device}")
    logger.info(f"{'='*60}\n")
    
    # Export all models
    exported = export_all_models(
        archetypes=archetypes,
        model_types=model_types,
        output_dir=args.output_dir,
        model_dir=args.model_dir,
        device=args.device,
        use_fp16=use_fp16
    )
    
    # Verify if requested
    if args.verify:
        logger.info(f"\nVerifying exported models...")
        verified = 0
        failed = 0
        
        for archetype in archetypes:
            for onnx_path in exported[archetype]:
                if verify_onnx_model(onnx_path, args.device):
                    verified += 1
                else:
                    failed += 1
        
        logger.info(f"Verification: {verified} passed, {failed} failed")
    
    # Optimize if requested
    if args.optimize:
        logger.info(f"\nOptimizing models...")
        for archetype in archetypes:
            optimized_paths = []
            for onnx_path in exported[archetype]:
                optimized_path = optimize_onnx_model(onnx_path)
                optimized_paths.append(optimized_path)
            exported[archetype] = optimized_paths
    
    logger.info(f"\nExport complete!")
    return 0


if __name__ == '__main__':
    sys.exit(main())
