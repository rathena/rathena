import numpy
import sklearn
import xgboost
import tensorflow
import asyncpg
import asyncio

print("Numpy, scikit-learn, xgboost, tensorflow, and asyncpg imported successfully.")

async def test_asyncpg():
    try:
        conn = await asyncpg.connect(
            user="postgres",
            password="",
            database="ai_world_memory",
            host="localhost",
            port=5432,
        )
        print("PostgreSQL connection successful (asyncpg).")
        await conn.close()
    except Exception as e:
        print("PostgreSQL connection failed (asyncpg):", e)

asyncio.run(test_asyncpg())