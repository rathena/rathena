def test_env_debug():
    from config import settings
    print("AZURE_OPENAI_API_KEY:", repr(settings.azure_openai_api_key))
    assert settings.azure_openai_api_key is not None and settings.azure_openai_api_key.strip() != ""