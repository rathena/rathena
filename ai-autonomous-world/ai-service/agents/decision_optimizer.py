"""
DecisionOptimizer: Hybrid ML Model for Agent Decision-Making (CPU/GPU, Runtime Selection, Fallback)

Implements a unified interface for agent decision-making using scikit-learn/XGBoost (CPU) and PyTorch/TensorFlow (GPU).
Supports runtime mode selection, device detection, model training, inference, and fallback to LLM or rule-based logic.
"""

import os
import numpy as np

# CPU ML imports
from sklearn.ensemble import RandomForestClassifier
from xgboost import XGBClassifier

# GPU ML imports
import torch
import torch.nn as nn
import torch.nn.functional as F

# TensorFlow (for alternative GPU/CPU)
import tensorflow as tf

class TorchDecisionNet(nn.Module):
    def __init__(self, input_dim, output_dim):
        super().__init__()
        self.fc1 = nn.Linear(input_dim, 64)
        self.fc2 = nn.Linear(64, 32)
        self.fc3 = nn.Linear(32, output_dim)

    def forward(self, x):
        x = F.relu(self.fc1(x))
        x = F.relu(self.fc2(x))
        return self.fc3(x)

class DecisionOptimizer:
    def __init__(self, mode="auto", fallback=None):
        """
        mode: "auto", "cpu", "gpu", "sklearn", "xgboost", "torch", "tf"
        fallback: callable for fallback logic (e.g., LLM or rule-based)
        """
        self.mode = mode
        self.fallback = fallback
        self.device = "cuda" if torch.cuda.is_available() and mode in ("auto", "gpu", "torch") else "cpu"
        self.cpu_model = None
        self.gpu_model = None
        self.tf_model = None
        self.input_dim = 10
        self.output_dim = 5
        self._init_models()

    def _init_models(self):
        # CPU models
        self.cpu_model = RandomForestClassifier(n_estimators=100)
        self.xgb_model = XGBClassifier(n_estimators=100, tree_method="hist")
        # GPU model (PyTorch)
        self.gpu_model = TorchDecisionNet(self.input_dim, self.output_dim).to(self.device)
        # TensorFlow model
        self.tf_model = tf.keras.Sequential([
            tf.keras.layers.Input(shape=(self.input_dim,)),
            tf.keras.layers.Dense(64, activation="relu"),
            tf.keras.layers.Dense(32, activation="relu"),
            tf.keras.layers.Dense(self.output_dim)
        ])
        self.tf_model.compile(optimizer="adam", loss="mse")

    def train(self, X, y):
        """
        Trains all models on the provided data.
        X: np.ndarray of shape (n_samples, input_dim)
        y: np.ndarray of shape (n_samples,)
        """
        # CPU
        self.cpu_model.fit(X, y)
        self.xgb_model.fit(X, y)
        # GPU (PyTorch)
        X_tensor = torch.tensor(X, dtype=torch.float32).to(self.device)
        y_tensor = torch.tensor(y, dtype=torch.long).to(self.device)
        optimizer = torch.optim.Adam(self.gpu_model.parameters(), lr=0.001)
        for epoch in range(10):
            optimizer.zero_grad()
            outputs = self.gpu_model(X_tensor)
            loss = F.cross_entropy(outputs, y_tensor)
            loss.backward()
            optimizer.step()
        # TensorFlow
        self.tf_model.fit(X, y, epochs=10, verbose=0)

    def predict(self, X, runtime_mode=None):
        """
        Predicts using the selected runtime mode.
        X: np.ndarray of shape (n_samples, input_dim)
        runtime_mode: override self.mode for this call
        Returns: np.ndarray of predictions
        """
        mode = runtime_mode or self.mode
        try:
            if mode in ("auto", "cpu", "sklearn"):
                return self.cpu_model.predict(X)
            elif mode == "xgboost":
                return self.xgb_model.predict(X)
            elif mode in ("auto", "gpu", "torch"):
                X_tensor = torch.tensor(X, dtype=torch.float32).to(self.device)
                with torch.no_grad():
                    outputs = self.gpu_model(X_tensor)
                    return outputs.argmax(dim=1).cpu().numpy()
            elif mode == "tf":
                preds = self.tf_model.predict(X)
                return preds.argmax(axis=1)
            else:
                raise ValueError(f"Unknown mode: {mode}")
        except Exception as e:
            if self.fallback:
                return self.fallback(X)
            raise e

    def save_models(self, path="models/"):
        os.makedirs(path, exist_ok=True)
        import joblib
        joblib.dump(self.cpu_model, os.path.join(path, "rf_cpu_model.pkl"))
        joblib.dump(self.xgb_model, os.path.join(path, "xgb_cpu_model.pkl"))
        torch.save(self.gpu_model.state_dict(), os.path.join(path, "torch_gpu_model.pt"))
        self.tf_model.save(os.path.join(path, "tf_model.keras"))

    def load_models(self, path="models/"):
        import joblib
        self.cpu_model = joblib.load(os.path.join(path, "rf_cpu_model.pkl"))
        self.xgb_model = joblib.load(os.path.join(path, "xgb_cpu_model.pkl"))
        self.gpu_model.load_state_dict(torch.load(os.path.join(path, "torch_gpu_model.pt"), map_location=self.device))
        self.tf_model = tf.keras.models.load_model(os.path.join(path, "tf_model.keras"))

# Example usage:
if __name__ == "__main__":
    optimizer = DecisionOptimizer(mode="auto")
    # Generate dummy data
    X = np.random.rand(100, 10)
    y = np.random.randint(0, 5, size=100)
    optimizer.train(X, y)
    preds = optimizer.predict(X)
    print("Predictions:", preds)
    optimizer.save_models()
    optimizer.load_models()