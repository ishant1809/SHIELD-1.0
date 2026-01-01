from dataclasses import dataclass
from typing import Dict, List
import math
import time


@dataclass
class SensorInput:
    crowd_density: int
    movement_compression: int
    noise_level: int
    anomaly_flag: bool

    thermal_max: float
    thermal_variance: float
    night_mode: bool

    altitude_cm: int
    altitude_variance: float

    route_a_congestion: int
    route_b_congestion: int
    route_c_congestion: int


@dataclass
class RouteStatus:
    state: str
    congestion: int


@dataclass
class DecisionOutput:
    threat_score: int
    threat_level: str
    thermal_alert: bool
    altitude_alert: bool
    night_operation: bool
    routes: Dict[str, RouteStatus]
    final_decision: str
    explanation: str
    timestamp: float


class ShieldDecisionModel:

    def __init__(self):
        self.event_log: List[DecisionOutput] = []

    def compute_threat_score(self, d: SensorInput) -> int:
        anomaly_score = 100 if d.anomaly_flag else 0

        thermal_score = 0
        if d.thermal_max > 45:
            thermal_score += 60
        if d.thermal_variance > 8:
            thermal_score += 40

        altitude_score = min(100, int(d.altitude_variance * 10))

        base = (
            0.30 * d.crowd_density +
            0.25 * d.movement_compression +
            0.15 * d.noise_level +
            0.10 * anomaly_score +
            0.10 * thermal_score +
            0.10 * altitude_score
        )

        if d.night_mode:
            base += 5

        return min(100, round(base))

    def classify_threat(self, score: int) -> str:
        if score < 30:
            return "NOMINAL"
        if score < 70:
            return "ELEVATED"
        return "CRITICAL"

    def classify_route(self, congestion: int) -> RouteStatus:
        if congestion >= 70:
            return RouteStatus("REROUTE", congestion)
        if congestion >= 40:
            return RouteStatus("DELAYED", congestion)
        return RouteStatus("SAFE", congestion)

    def detect_thermal_alert(self, d: SensorInput) -> bool:
        return d.thermal_max > 50 or d.thermal_variance > 10

    def detect_altitude_instability(self, d: SensorInput) -> bool:
        return d.altitude_variance > 6

    def decide_routes(self, d: SensorInput) -> Dict[str, RouteStatus]:
        return {
            "Route A": self.classify_route(d.route_a_congestion),
            "Route B": self.classify_route(d.route_b_congestion),
            "Route C": self.classify_route(d.route_c_congestion)
        }

    def choose_route(self, routes: Dict[str, RouteStatus]) -> str:
        reroutes = [r for r in routes.values() if r.state == "REROUTE"]
        if len(reroutes) == 3:
            return "ALL ROUTES BLOCKED"
        return min(routes.items(), key=lambda r: r[1].congestion)[0]

    def explain(self, score, level, thermal, altitude, night, decision) -> str:
        parts = []
        parts.append(f"Threat={level}({score})")
        if thermal:
            parts.append("Thermal anomaly detected")
        if altitude:
            parts.append("Altitude instability detected")
        if night:
            parts.append("Night surveillance mode active")
        parts.append(f"Decision={decision}")
        return " | ".join(parts)

    def decide(self, d: SensorInput) -> DecisionOutput:
        score = self.compute_threat_score(d)
        level = self.classify_threat(score)

        thermal_alert = self.detect_thermal_alert(d)
        altitude_alert = self.detect_altitude_instability(d)

        routes = self.decide_routes(d)
        decision = self.choose_route(routes)

        explanation = self.explain(
            score,
            level,
            thermal_alert,
            altitude_alert,
            d.night_mode,
            decision
        )

        output = DecisionOutput(
            threat_score=score,
            threat_level=level,
            thermal_alert=thermal_alert,
            altitude_alert=altitude_alert,
            night_operation=d.night_mode,
            routes=routes,
            final_decision=decision,
            explanation=explanation,
            timestamp=time.time()
        )

        self.event_log.append(output)
        return output

    def history(self) -> List[DecisionOutput]:
        return self.event_log


if __name__ == "__main__":
    model = ShieldDecisionModel()

    sample = SensorInput(
        crowd_density=68,
        movement_compression=75,
        noise_level=62,
        anomaly_flag=True,

        thermal_max=52.3,
        thermal_variance=11.4,
        night_mode=True,

        altitude_cm=120,
        altitude_variance=7.2,

        route_a_congestion=88,
        route_b_congestion=95,
        route_c_congestion=46
    )

    decision = model.decide(sample)

    print("Threat Score:", decision.threat_score)
    print("Threat Level:", decision.threat_level)
    print("Thermal Alert:", decision.thermal_alert)
    print("Altitude Alert:", decision.altitude_alert)
    print("Night Mode:", decision.night_operation)
    print("Routes:")
    for k, v in decision.routes.items():
        print(k, v.state, v.congestion)
    print("Final Decision:", decision.final_decision)
    print("Explanation:", decision.explanation)
