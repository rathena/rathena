"""
Moral Alignment System: Dynamic Alignment, Moral Development, Redemption/Corruption, and Ethical Dilemmas

Implements dynamic alignment evolution, moral conflict handling, and alignment-driven decision-making for NPCs.
"""

from typing import Dict, Any, Optional
from datetime import datetime
from loguru import logger

ALIGNMENT_TYPES = [
    "lawful_good", "neutral_good", "chaotic_good",
    "lawful_neutral", "true_neutral", "chaotic_neutral",
    "lawful_evil", "neutral_evil", "chaotic_evil"
]

class MoralAlignment:
    def __init__(self, alignment: str = "true_neutral", spectrum: Optional[Dict[str, float]] = None):
        self.alignment = alignment if alignment in ALIGNMENT_TYPES else "true_neutral"
        self.spectrum = spectrum or {
            "altruism": 0.0,  # -1 (selfish) to +1 (altruistic)
            "lawful": 0.0,    # -1 (chaotic) to +1 (lawful)
            "honest": 0.0     # -1 (deceptive) to +1 (honest)
        }
        self.history = []
        self.last_update = datetime.utcnow()

    def update_from_action(self, action: Dict[str, Any]):
        """
        Updates alignment spectrum and type based on action context.
        Example action: {"type": "help", "target": "npc", "risk": 0.5, "self_cost": 0.2, "truthful": True}
        """
        delta = {"altruism": 0.0, "lawful": 0.0, "honest": 0.0}
        if action.get("type") == "help":
            delta["altruism"] += 0.1 * (1 - action.get("self_cost", 0))
        if action.get("type") == "betray":
            delta["altruism"] -= 0.2
            delta["honest"] -= 0.2
        if action.get("type") == "lie":
            delta["honest"] -= 0.2
        if action.get("type") == "obey_law":
            delta["lawful"] += 0.1
        if action.get("type") == "break_law":
            delta["lawful"] -= 0.1
        if action.get("truthful") is False:
            delta["honest"] -= 0.1
        # Apply deltas
        for k in self.spectrum:
            self.spectrum[k] = max(-1.0, min(1.0, self.spectrum[k] + delta.get(k, 0.0)))
        self.history.append({"timestamp": datetime.utcnow().isoformat(), "action": action, "delta": delta, "spectrum": dict(self.spectrum)})
        self.last_update = datetime.utcnow()
        self._update_alignment_type()
        logger.info(f"MoralAlignment updated: {self.spectrum}, type: {self.alignment}")

    def _update_alignment_type(self):
        # Simple mapping: can be made more sophisticated
        altruism = self.spectrum["altruism"]
        lawful = self.spectrum["lawful"]
        honest = self.spectrum["honest"]
        if altruism > 0.5:
            if lawful > 0.5:
                self.alignment = "lawful_good"
            elif lawful < -0.5:
                self.alignment = "chaotic_good"
            else:
                self.alignment = "neutral_good"
        elif altruism < -0.5:
            if lawful > 0.5:
                self.alignment = "lawful_evil"
            elif lawful < -0.5:
                self.alignment = "chaotic_evil"
            else:
                self.alignment = "neutral_evil"
        else:
            if lawful > 0.5:
                self.alignment = "lawful_neutral"
            elif lawful < -0.5:
                self.alignment = "chaotic_neutral"
            else:
                self.alignment = "true_neutral"

    def handle_moral_conflict(self, options: Dict[str, Any]) -> str:
        """
        Simulates a moral dilemma and returns the chosen option.
        Example options: {"help_friend": {"altruism": +0.2, "lawful": -0.1}, "obey_law": {"altruism": -0.1, "lawful": +0.2}}
        """
        # Simple heuristic: maximize sum of spectrum deltas weighted by current spectrum
        best_score = float("-inf")
        best_option = None
        for opt, delta in options.items():
            score = sum(self.spectrum[k] * delta.get(k, 0.0) for k in self.spectrum)
            if score > best_score:
                best_score = score
                best_option = opt
        logger.info(f"Moral conflict resolved: chose {best_option} (score={best_score})")
        return best_option

    def trigger_redemption(self):
        """
        Simulates a redemption arc: moves alignment toward good/honest/lawful.
        """
        self.spectrum["altruism"] = min(1.0, self.spectrum["altruism"] + 0.3)
        self.spectrum["honest"] = min(1.0, self.spectrum["honest"] + 0.2)
        self.spectrum["lawful"] = min(1.0, self.spectrum["lawful"] + 0.2)
        self._update_alignment_type()
        logger.info(f"Redemption arc: {self.spectrum}, type: {self.alignment}")

    def trigger_corruption(self):
        """
        Simulates a corruption arc: moves alignment toward evil/deceptive/chaotic.
        """
        self.spectrum["altruism"] = max(-1.0, self.spectrum["altruism"] - 0.3)
        self.spectrum["honest"] = max(-1.0, self.spectrum["honest"] - 0.2)
        self.spectrum["lawful"] = max(-1.0, self.spectrum["lawful"] - 0.2)
        self._update_alignment_type()
        logger.info(f"Corruption arc: {self.spectrum}, type: {self.alignment}")

    def to_dict(self):
        return {
            "alignment": self.alignment,
            "spectrum": dict(self.spectrum),
            "history": list(self.history),
            "last_update": self.last_update.isoformat()
        }