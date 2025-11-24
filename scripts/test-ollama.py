#!/usr/bin/env python3
"""
Ollama Connectivity and Functionality Test Script

This script verifies that Ollama is:
1. Running and accessible
2. Has required models installed
3. Can generate responses
4. Performs within acceptable latency
"""

import sys
import time
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent / "ai-autonomous-world" / "ai-service"))

import requests
import json
from typing import Dict, Any, Optional


class Colors:
    """Terminal colors for pretty output"""
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    END = '\033[0m'


def print_success(message: str):
    print(f"{Colors.GREEN}✓{Colors.END} {message}")


def print_error(message: str):
    print(f"{Colors.RED}✗{Colors.END} {message}")


def print_warning(message: str):
    print(f"{Colors.YELLOW}⚠{Colors.END} {message}")


def print_info(message: str):
    print(f"{Colors.BLUE}ℹ{Colors.END} {message}")


def print_header(message: str):
    print(f"\n{Colors.BOLD}{message}{Colors.END}")
    print("=" * 60)


def test_ollama_service(base_url: str = "http://localhost:11434") -> bool:
    """Test if Ollama service is running"""
    print_header("Testing Ollama Service")
    
    try:
        response = requests.get(f"{base_url}/api/tags", timeout=5)
        if response.status_code == 200:
            print_success(f"Ollama service is running at {base_url}")
            return True
        else:
            print_error(f"Ollama returned status code: {response.status_code}")
            return False
    except requests.exceptions.ConnectionError:
        print_error(f"Cannot connect to Ollama at {base_url}")
        print_info("Make sure Ollama is running: ollama serve")
        return False
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        return False


def list_available_models(base_url: str = "http://localhost:11434") -> Dict[str, Any]:
    """List all available models"""
    print_header("Available Models")
    
    try:
        response = requests.get(f"{base_url}/api/tags", timeout=5)
        response.raise_for_status()
        
        data = response.json()
        models = data.get("models", [])
        
        if not models:
            print_warning("No models installed")
            print_info("Install a model: ollama pull llama2:13b")
            return {}
        
        model_dict = {}
        for model in models:
            name = model.get("name", "unknown")
            size = model.get("size", 0) / (1024 ** 3)  # Convert to GB
            modified = model.get("modified_at", "unknown")
            
            print_success(f"Model: {name}")
            print(f"  Size: {size:.2f} GB")
            print(f"  Modified: {modified}")
            
            model_dict[name] = model
        
        return model_dict
    
    except Exception as e:
        print_error(f"Failed to list models: {e}")
        return {}


def test_model_availability(
    model_name: str,
    base_url: str = "http://localhost:11434"
) -> bool:
    """Test if a specific model is available"""
    print_header(f"Testing Model: {model_name}")
    
    try:
        response = requests.get(f"{base_url}/api/tags", timeout=5)
        response.raise_for_status()
        
        data = response.json()
        models = data.get("models", [])
        
        available_models = [m.get("name") for m in models]
        
        if model_name in available_models:
            print_success(f"Model '{model_name}' is available")
            return True
        else:
            print_error(f"Model '{model_name}' not found")
            print_info(f"Available models: {', '.join(available_models)}")
            print_info(f"Install with: ollama pull {model_name}")
            return False
    
    except Exception as e:
        print_error(f"Failed to check model availability: {e}")
        return False


def test_generation(
    model_name: str,
    base_url: str = "http://localhost:11434",
    timeout: int = 120
) -> Optional[Dict[str, Any]]:
    """Test text generation with the model"""
    print_header(f"Testing Generation with {model_name}")
    
    prompt = "You are a medieval guard. Greet a player in one sentence."
    
    payload = {
        "model": model_name,
        "prompt": prompt,
        "stream": False,
        "options": {
            "temperature": 0.7,
            "num_predict": 100,
        }
    }
    
    try:
        print_info(f"Sending prompt: '{prompt}'")
        
        start_time = time.time()
        response = requests.post(
            f"{base_url}/api/generate",
            json=payload,
            timeout=timeout
        )
        end_time = time.time()
        
        response.raise_for_status()
        
        latency = (end_time - start_time) * 1000  # Convert to ms
        
        data = response.json()
        generated_text = data.get("response", "").strip()
        
        print_success("Generation successful!")
        print(f"  Response: \"{generated_text}\"")
        print(f"  Latency: {latency:.0f}ms")
        
        # Performance evaluation
        if latency < 500:
            print_success(f"Excellent latency ({latency:.0f}ms < 500ms)")
        elif latency < 1000:
            print_warning(f"Acceptable latency ({latency:.0f}ms < 1000ms)")
        else:
            print_error(f"High latency ({latency:.0f}ms > 1000ms)")
            print_info("Consider using a smaller model or GPU acceleration")
        
        return {
            "success": True,
            "text": generated_text,
            "latency_ms": latency,
            "model": model_name
        }
    
    except requests.exceptions.Timeout:
        print_error(f"Request timed out after {timeout}s")
        print_info("Try increasing timeout or using a smaller model")
        return None
    except Exception as e:
        print_error(f"Generation failed: {e}")
        return None


def test_chat_api(
    model_name: str,
    base_url: str = "http://localhost:11434",
    timeout: int = 120
) -> Optional[Dict[str, Any]]:
    """Test chat API with the model"""
    print_header(f"Testing Chat API with {model_name}")
    
    messages = [
        {"role": "system", "content": "You are a helpful medieval NPC."},
        {"role": "user", "content": "What is your name?"}
    ]
    
    payload = {
        "model": model_name,
        "messages": messages,
        "stream": False,
        "options": {
            "temperature": 0.7,
            "num_predict": 100,
        }
    }
    
    try:
        print_info("Testing chat conversation...")
        
        start_time = time.time()
        response = requests.post(
            f"{base_url}/api/chat",
            json=payload,
            timeout=timeout
        )
        end_time = time.time()
        
        if response.status_code == 404:
            print_warning("Chat API not supported (Ollama version < 0.1.0)")
            print_info("Generation API works fine for this use case")
            return None
        
        response.raise_for_status()
        
        latency = (end_time - start_time) * 1000
        
        data = response.json()
        message = data.get("message", {})
        generated_text = message.get("content", "").strip()
        
        print_success("Chat API test successful!")
        print(f"  Response: \"{generated_text}\"")
        print(f"  Latency: {latency:.0f}ms")
        
        return {
            "success": True,
            "text": generated_text,
            "latency_ms": latency,
            "model": model_name
        }
    
    except Exception as e:
        print_warning(f"Chat API test failed (falling back to generate API): {e}")
        return None


def test_provider_integration(model_name: str = "llama2:13b") -> bool:
    """Test the Ollama provider integration with AI service"""
    print_header("Testing AI Service Integration")
    
    try:
        # Import the provider
        from llm.providers.ollama_provider import OllamaProvider
        
        print_info("Creating OllamaProvider instance...")
        provider = OllamaProvider(
            model=model_name,
            base_url="http://localhost:11434",
            timeout=120.0,
            debug=True
        )
        
        print_success("Provider created successfully")
        
        # Test generation through provider
        print_info("Testing provider.generate()...")
        
        import asyncio
        
        async def test_async():
            result = await provider.generate(
                messages=[
                    {"role": "system", "content": "You are a medieval guard."},
                    {"role": "user", "content": "Hello!"}
                ],
                temperature=0.7,
                max_tokens=100
            )
            return result
        
        result = asyncio.run(test_async())
        
        if result and "text" in result:
            print_success("Provider integration test successful!")
            print(f"  Generated: \"{result['text'][:100]}...\"")
            print(f"  Tokens used: {result.get('tokens_used', 'N/A')}")
            return True
        else:
            print_error("Provider returned invalid result")
            return False
    
    except ImportError as e:
        print_error(f"Cannot import OllamaProvider: {e}")
        print_info("Make sure you're running from the correct directory")
        return False
    except Exception as e:
        print_error(f"Provider integration test failed: {e}")
        return False


def print_recommendations(test_results: Dict[str, bool]):
    """Print recommendations based on test results"""
    print_header("Recommendations")
    
    if not test_results.get("service"):
        print("1. Start Ollama service:")
        print("   $ ollama serve")
        print()
    
    if not test_results.get("model"):
        print("2. Install recommended model:")
        print("   $ ollama pull llama2:13b")
        print()
    
    if test_results.get("generation"):
        print_success("All tests passed! Ollama is ready to use.")
        print()
        print("Next steps:")
        print("1. Update ai-service/.env:")
        print("   DEFAULT_LLM_PROVIDER=ollama")
        print("   OLLAMA_MODEL=llama2:13b")
        print()
        print("2. Start AI service:")
        print("   $ cd ai-autonomous-world/ai-service")
        print("   $ python main.py")
        print()
        print("3. Test NPC interaction:")
        print("   $ curl -X POST http://localhost:8000/api/npc/dialogue \\")
        print("     -H 'Content-Type: application/json' \\")
        print("     -d '{\"npc_id\": \"guard_001\", \"player_message\": \"Hello!\"}'")
    else:
        print_warning("Some tests failed. Review errors above and retry.")


def main():
    """Main test execution"""
    print(f"\n{Colors.BOLD}Ollama Connectivity Test{Colors.END}")
    print("=" * 60)
    
    base_url = "http://localhost:11434"
    model_name = "llama2:13b"
    
    test_results = {}
    
    # Test 1: Service availability
    test_results["service"] = test_ollama_service(base_url)
    if not test_results["service"]:
        print_recommendations(test_results)
        sys.exit(1)
    
    # Test 2: List models
    models = list_available_models(base_url)
    
    # Test 3: Check specific model
    test_results["model"] = test_model_availability(model_name, base_url)
    if not test_results["model"] and models:
        # Try first available model
        model_name = list(models.keys())[0]
        print_info(f"Trying with available model: {model_name}")
        test_results["model"] = True
    
    if not test_results["model"]:
        print_recommendations(test_results)
        sys.exit(1)
    
    # Test 4: Generation
    gen_result = test_generation(model_name, base_url)
    test_results["generation"] = gen_result is not None
    
    # Test 5: Chat API (optional)
    chat_result = test_chat_api(model_name, base_url)
    test_results["chat"] = chat_result is not None
    
    # Test 6: Provider integration (optional, requires imports)
    try:
        test_results["integration"] = test_provider_integration(model_name)
    except:
        print_info("Skipping provider integration test (run from ai-service directory)")
        test_results["integration"] = None
    
    # Summary
    print_header("Test Summary")
    print(f"Service Running:     {Colors.GREEN + '✓' if test_results['service'] else Colors.RED + '✗'}{Colors.END}")
    print(f"Model Available:     {Colors.GREEN + '✓' if test_results['model'] else Colors.RED + '✗'}{Colors.END}")
    print(f"Generation Works:    {Colors.GREEN + '✓' if test_results['generation'] else Colors.RED + '✗'}{Colors.END}")
    print(f"Chat API Works:      {Colors.GREEN + '✓' if test_results.get('chat') else Colors.YELLOW + '~'}{Colors.END}")
    if test_results.get("integration") is not None:
        print(f"Provider Integration: {Colors.GREEN + '✓' if test_results['integration'] else Colors.RED + '✗'}{Colors.END}")
    
    # Recommendations
    print_recommendations(test_results)
    
    # Exit code
    if all([test_results["service"], test_results["model"], test_results["generation"]]):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(f"\n{Colors.YELLOW}Test interrupted by user{Colors.END}")
        sys.exit(130)
    except Exception as e:
        print_error(f"Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)