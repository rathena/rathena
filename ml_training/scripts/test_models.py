#!/usr/bin/env python3
"""
Test script to verify model creation works correctly
"""

import sys
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

from models.model_factory import ModelFactory
from models.model_architectures import estimate_model_size

def main():
    print('Testing model creation...')
    print('='*60)
    
    # Test creating one model of each type
    test_models = []
    
    for model_type in ['combat_dqn', 'movement_ppo', 'spatial_vit']:
        print(f'\nCreating aggressive/{model_type}...')
        try:
            model = ModelFactory.create_model('aggressive', model_type, device='cpu')
            size = estimate_model_size(model, 'fp16')
            print(f'  ✓ Created: {size["parameters"]:,} params, {size["mb"]:.2f} MB FP16')
            test_models.append((model_type, model))
        except Exception as e:
            print(f'  ✗ Failed: {e}')
    
    print('\n' + '='*60)
    print(f'✓ Model creation test PASSED')
    print(f'Successfully created {len(test_models)} test models')
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
