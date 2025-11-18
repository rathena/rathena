"""
Agent Orchestrator - Coordinates multiple agents using CrewAI
Manages agent collaboration and task delegation
"""

from datetime import datetime
from typing import Dict, Any, List, Optional
from loguru import logger

import os
import psutil
import ctypes
import threading
import concurrent.futures
import time

from prometheus_client import Counter, Histogram, Gauge

from crewai import Crew, Process
from agents.base_agent import AgentContext, AgentResponse
from agents.dialogue_agent import DialogueAgent
from agents.decision_agent import DecisionAgent
from agents.decision_optimizer import DecisionOptimizer
from agents.memory_agent import MemoryAgent
from agents.world_agent import WorldAgent

class AgentOrchestrator:
    """
    Orchestrates multiple AI agents to handle complex NPC interactions
    Uses CrewAI for agent coordination and task management
    """
    def __init__(
        self,
        config: Dict[str, Any],
        npc_context: Optional[Dict[str, Any]] = None
    ):
        """
        Initialize the orchestrator with configuration and optional NPC context

        Args:
            config: Configuration dictionary with agent settings
            npc_context: Optional NPC context for initial state
        """
        self.config = config
        self.npc_context = npc_context or {}

        # Multi-threading and metrics setup
        self.max_workers = config.get("max_workers", os.cpu_count() or 4)
        self.executor = concurrent.futures.ThreadPoolExecutor(max_workers=self.max_workers)
        self.thread_affinity_enabled = config.get("thread_affinity_enabled", True)
        self.thread_pool_gauge = Gauge("ai_thread_pool_size", "Current thread pool size")
        self.cpu_util_gauge = Gauge("ai_cpu_utilization", "Current CPU utilization percent")
        self.agent_exec_histogram = Histogram("ai_agent_exec_time_seconds", "Agent execution time", ["agent_type"])
        self.agent_success_counter = Counter("ai_agent_success_total", "Agent success count", ["agent_type"])
        self.agent_failure_counter = Counter("ai_agent_failure_total", "Agent failure count", ["agent_type"])

        # Initialize agents with provided LLM provider or get default
        from llm.factory import get_llm_provider
        self.llm_provider = config.get("llm_provider") or get_llm_provider()
        self.dialogue_agent = DialogueAgent("dialogue_agent", self.llm_provider, config.get("dialogue", {}))
        self.decision_agent = DecisionAgent("decision_agent", self.llm_provider, config.get("decision", {}))
        self.memory_agent = MemoryAgent("memory_agent", self.llm_provider, config.get("memory", {}))
        self.world_agent = WorldAgent("world_agent", self.llm_provider, config.get("world", {}))
        self.ml_mode = config.get("ml_mode", "auto")
        self.optimizer = DecisionOptimizer(mode=self.ml_mode, fallback=None)

        logger.info("AgentOrchestrator initialized with {} agents and {} workers", 4, self.max_workers)
        self.thread_pool_gauge.set(self.max_workers)

        # Start dynamic scaling thread
        self._scaling_thread = threading.Thread(target=self._dynamic_scaling_loop, daemon=True)
        self._scaling_thread.start()

    def _set_thread_affinity(self):
        """Set CPU affinity for current thread (Linux only)"""
        if self.thread_affinity_enabled and hasattr(os, "sched_setaffinity"):
            try:
                cpu_id = threading.get_ident() % (os.cpu_count() or 1)
                os.sched_setaffinity(0, {cpu_id})
                logger.info(f"Set CPU affinity for thread {threading.get_ident()} to CPU {cpu_id}")
            except Exception as e:
                logger.warning(f"Failed to set thread affinity: {e}")

    def _dynamic_scaling_loop(self):
        """Dynamically scale thread pool based on CPU utilization"""
        while True:
            try:
                cpu_util = psutil.cpu_percent(interval=2)
                self.cpu_util_gauge.set(cpu_util)
                # Scale up if CPU < 60% and tasks are pending, scale down if > 90%
                if cpu_util < 60 and self.executor._max_workers < (os.cpu_count() or 4) * 2:
                    self.executor._max_workers += 1
                    self.thread_pool_gauge.set(self.executor._max_workers)
                    logger.info(f"Scaled up thread pool to {self.executor._max_workers} workers (CPU {cpu_util}%)")
                elif cpu_util > 90 and self.executor._max_workers > 2:
                    self.executor._max_workers -= 1
                    self.thread_pool_gauge.set(self.executor._max_workers)
                    logger.info(f"Scaled down thread pool to {self.executor._max_workers} workers (CPU {cpu_util}%)")
            except Exception as e:
                logger.warning(f"Dynamic scaling error: {e}")
            time.sleep(5)

    async def handle_player_interaction(
        self,
        npc_context: AgentContext,
        interaction_type: str,
        message: str = "",
    ) -> AgentResponse:
        """
        Handle player interaction by coordinating multiple agents (multi-threaded, monitored)

        Args:
            npc_context: AgentContext for the NPC
            interaction_type: Type of interaction (e.g., "dialogue", "trade")
            message: Player's input text

        Returns:
            AgentResponse containing the generated response
        """
        start_time = time.time()
        self._set_thread_affinity()
        try:
            # Store interaction in memory using the process method (threaded)
            def store_memory_task():
                try:
                    memory_context_obj = AgentContext(
                        npc_id=npc_context.npc_id,
                        npc_name=npc_context.npc_name,
                        personality=npc_context.personality,
                        goal_state=getattr(npc_context, "goal_state", None),
                        emotion_state=getattr(npc_context, "emotion_state", None),
                        memory_state=getattr(npc_context, "memory_state", None),
                        current_state={
                            "operation": "store",
                            "memory_data": {
                                "player_input": message,
                                "timestamp": datetime.now().isoformat(),
                                "interaction_type": interaction_type,
                            }
                        },
                        world_state=npc_context.world_state,
                        recent_events=npc_context.recent_events
                    )
                    return self.memory_agent.process(memory_context_obj)
                except Exception as e:
                    logger.warning("Failed to store interaction in memory: {}", e)
                    return None

            mem_future = self.executor.submit(store_memory_task)
            # Optionally: await mem_future.result() if process() is async

            # Create crew for dialogue generation
            crew = self.create_crew_for_task("dialogue")

            # Execute the crew to generate response (threaded)
            def crew_task():
                try:
                    npc_context_dict = {
                        "npc_id": getattr(npc_context, "npc_id", "unknown"),
                        "npc_name": getattr(npc_context, "npc_name", "Unknown NPC"),
                        "personality": getattr(npc_context, "personality", {}),
                        "goal_state": getattr(npc_context, "goal_state", None),
                        "emotion_state": getattr(npc_context, "emotion_state", None),
                        "memory_state": getattr(npc_context, "memory_state", None),
                        "current_state": getattr(npc_context, "current_state", {}),
                        "world_state": getattr(npc_context, "world_state", {}),
                        "recent_events": getattr(npc_context, "recent_events", [])
                    }
                    result = crew.kickoff(inputs={
                        "player_input": message,
                        "npc_context": npc_context_dict,
                        "interaction_type": interaction_type,
                    })
                    return result
                except Exception as e:
                    logger.error("Crew execution failed: {}", e)
                    return None

            crew_future = self.executor.submit(crew_task)
            result = crew_future.result(timeout=30)  # Wait for result

            if result:
                response = AgentResponse(
                    agent_type="dialogue",
                    success=True,
                    data={
                        "text": result.get("response", "I'm not sure how to respond to that."),
                        "emotional_state": result.get("emotional_state", "neutral"),
                        "actions": result.get("actions", [])
                    },
                    confidence=0.8,
                    metadata=result.get("metadata", {})
                )
                self.agent_success_counter.labels(agent_type="dialogue").inc()
                self.agent_exec_histogram.labels(agent_type="dialogue").observe(time.time() - start_time)
                return response
            else:
                # Fallback to direct dialogue agent (threaded)
                def fallback_task():
                    return self.dialogue_agent.process(
                        AgentContext(
                            npc_id=getattr(npc_context, "npc_id", "unknown"),
                            npc_name=getattr(npc_context, "npc_name", "Unknown NPC"),
                            personality=getattr(npc_context, "personality", {}),
                            goal_state=getattr(npc_context, "goal_state", None),
                            emotion_state=getattr(npc_context, "emotion_state", None),
                            memory_state=getattr(npc_context, "memory_state", None),
                            current_state={"player_message": message},
                            world_state=getattr(npc_context, "world_state", {}),
                            recent_events=getattr(npc_context, "recent_events", [])
                        )
                    )
                fallback_future = self.executor.submit(fallback_task)
                fallback_response = fallback_future.result(timeout=15)
                self.agent_failure_counter.labels(agent_type="dialogue").inc()
                self.agent_exec_histogram.labels(agent_type="dialogue").observe(time.time() - start_time)
                return AgentResponse(
                    agent_type="dialogue",
                    success=True,
                    data={
                        "text": getattr(fallback_response.data, "text", "I'm not sure how to respond to that."),
                        "emotional_state": getattr(fallback_response.data, "emotional_state", "neutral"),
                        "actions": getattr(fallback_response.data, "actions", [])
                    },
                    confidence=0.5,
                    metadata={"fallback": True}
                )

        except Exception as e:
            logger.error("Error in player interaction handling: {}", e)
            self.agent_failure_counter.labels(agent_type="dialogue").inc()
            self.agent_exec_histogram.labels(agent_type="dialogue").observe(time.time() - start_time)
            return AgentResponse(
                agent_type="dialogue",
                success=False,
                data={"text": "I'm having trouble processing that right now. Could you try again?"},
                confidence=0.0,
                reasoning=f"Error: {str(e)}"
            )

    async def handle_npc_action_decision(
        self,
        agent_context: AgentContext
    ) -> Dict[str, Any]:
        """
        Make decisions about NPC actions based on world state, using ML/LLM hybrid.

        Args:
            agent_context: AgentContext for the NPC

        Returns:
            Dictionary containing decided actions and their parameters
        """
        try:
            # Prepare ML features
            available_actions = self.decision_agent._get_available_actions(agent_context)
            features = self.decision_agent._extract_features(agent_context, available_actions)
            ml_decision = None
            if features is not None:
                try:
                    ml_pred = self.optimizer.predict(features)
                    action_idx = int(ml_pred[0]) if hasattr(ml_pred, "__getitem__") else 0
                    action_type = available_actions[action_idx]["type"] if action_idx < len(available_actions) else "idle"
                    ml_decision = {
                        "action_type": action_type,
                        "reasoning": f"ML model (mode={self.ml_mode}) selected action {action_type}",
                        "priority": 5
                    }
                except Exception as e:
                    logger.warning(f"ML model failed in orchestrator, will fallback to LLM: {e}")

            if ml_decision is not None:
                action_data = self.decision_agent._parse_decision(ml_decision, available_actions, agent_context)
                logger.info(f"Orchestrator decision (ML): {action_data.get('action_type')}")
                return action_data

            # Fallback to LLM/crew
            crew = self.create_crew_for_task("decision")
            result = await crew.kickoff(inputs={
                "world_state": agent_context.world_state,
                "npc_context": {
                    "npc_id": agent_context.npc_id,
                    "npc_name": agent_context.npc_name,
                    "personality": agent_context.personality,
                    "goal_state": getattr(agent_context, "goal_state", None),
                    "emotion_state": getattr(agent_context, "emotion_state", None),
                    "memory_state": getattr(agent_context, "memory_state", None),
                    "current_state": agent_context.current_state,
                    "recent_events": agent_context.recent_events,
                }
            })
            return result.get("actions", {})
        except Exception as e:
            logger.error("Error in NPC action decision: {}", e)
            return {"move": "stay", "interact": False}

    async def process_world_event(
        self,
        event_type: str,
        event_data: Dict[str, Any],
        affected_npcs: List[Dict[str, Any]]
    ) -> Dict[str, Any]:
        """
        Process world events and coordinate NPC responses

        Args:
            event_type: Type of world event
            event_data: Event-specific data
            affected_npcs: List of NPCs affected by the event

        Returns:
            Dictionary with NPC responses and world changes
        """
        try:
            crew = self.create_crew_for_task("world_event")
            result = await crew.kickoff(inputs={
                "event_type": event_type,
                "event_data": event_data,
                "affected_npcs": affected_npcs
            })
            return result
        except Exception as e:
            logger.error("Error processing world event: {}", e)
            return {"responses": {}, "world_changes": {}}

    def create_crew_for_task(self, task_type: str) -> Crew:
        """
        Create a CrewAI crew for a specific task type

        Args:
            task_type: Type of task ("dialogue", "decision", "world_event")

        Returns:
            Configured Crew instance
        """
        tasks_config = {
            "dialogue": {
                "description": "Generate appropriate dialogue response for NPC",
                "expected_output": "Natural language response with emotional context"
            },
            "decision": {
                "description": "Decide NPC actions based on world state",
                "expected_output": "Action decisions with parameters"
            },
            "world_event": {
                "description": "Process world events and coordinate NPC responses",
                "expected_output": "NPC responses and world state changes"
            }
        }

        config = tasks_config.get(task_type, tasks_config["dialogue"])

        return Crew(
            agents=[
                self.dialogue_agent.crew_agent,
                self.decision_agent.crew_agent,
                self.memory_agent.crew_agent,
                self.world_agent.crew_agent
            ],
            tasks=[{
                "description": config["description"],
                "expected_output": config["expected_output"],
                "agent": self.dialogue_agent.crew_agent
            }],
            process=Process.sequential,
            verbose=True
        )