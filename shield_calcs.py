

import numpy as np
import math

# =========================
# 1. Crowd Density Formula
# =========================
def crowd_density(num_people, ground_area):
    """
    Density = people per square meter
    """
    return num_people / max(ground_area, 1e-6)


# ======================================
# 2. Ground Area Approximation (Camera)
# ======================================
def ground_area(drone_height, camera_fov_rad):
    """
    A = (2h * tan(FOV/2))^2
    """
    width = 2 * drone_height * math.tan(camera_fov_rad / 2)
    return width ** 2


# ==================================
# 3. Person Speed Estimation
# ==================================
def person_speed(prev_pos, curr_pos, delta_t):
    """
    v = sqrt(dx^2 + dy^2) / dt
    """
    dx = curr_pos[0] - prev_pos[0]
    dy = curr_pos[1] - prev_pos[1]
    return math.sqrt(dx**2 + dy**2) / max(delta_t, 1e-6)


# ==================================
# 4. Speed Variance (Panic Indicator)
# ==================================
def speed_variance(speeds):
    """
    σ²(v)
    """
    return np.var(speeds)


# ==================================
# 5. Direction Entropy (Chaos Measure)
# ==================================
def direction_entropy(directions):
    """
    H(θ) = -Σ p(θ) log p(θ)
    """
    hist, _ = np.histogram(directions, bins=8, range=(-math.pi, math.pi))
    prob = hist / np.sum(hist + 1e-6)
    return -np.sum(prob * np.log(prob + 1e-6))


# ==================================
# 6. Stampede Risk Inference
# ==================================
def stampede_risk(density, speed_var, dir_entropy):
    """
    S = D × σ(v) × H(θ)
    """
    return density * speed_var * dir_entropy


# ==================================
# 7. Thermal Variance (Presence Confidence)
# ==================================
def thermal_variance(thermal_frame):
    """
    Var(T)
    """
    return np.var(thermal_frame)


# ==================================
# 8. Crowd Anomaly Score
# ==================================
def anomaly_score(current_features, baseline_features):
    """
    ||Xt − μ||
    """
    return np.linalg.norm(
        np.array(current_features) - np.array(baseline_features)
    )


# ==================================
# 9. Noise Level (Agitation)
# ==================================
def noise_db(audio_samples):
    """
    RMS → dB
    """
    rms = np.sqrt(np.mean(np.square(audio_samples)))
    return 20 * np.log10(rms + 1e-6)


# ==================================
# 10. Final Threat Score Aggregation
# ==================================
def threat_score(density, stampede, anomaly, noise,
                 w_density=0.3, w_stampede=0.4, w_anomaly=0.2, w_noise=0.1):
    """
    Threat = Σ wi × featurei
    """
    return (
        w_density * density +
        w_stampede * stampede +
        w_anomaly * anomaly +
        w_noise * noise
    )


# =========================
# Example Model Execution
# =========================
if __name__ == "__main__":
    # Example simulated inputs
    people = 42
    height = 25.0            # meters
    fov = math.radians(78)

    area = ground_area(height, fov)
    density = crowd_density(people, area)

    speeds = [1.2, 0.8, 2.3, 1.9, 3.1]
    speed_var = speed_variance(speeds)

    directions = [0.2, -1.1, 1.5, 2.8, -2.4]
    dir_entropy = direction_entropy(directions)

    stampede = stampede_risk(density, speed_var, dir_entropy)

    thermal = np.random.rand(64, 64) * 40
    thermal_var = thermal_variance(thermal)

    anomaly = anomaly_score(
        [density, speed_var, thermal_var],
        [0.3, 0.1, 5.0]
    )

    audio = np.random.randn(2048)
    noise = noise_db(audio)

    threat = threat_score(density, stampede, anomaly, noise)

    print("Threat Score:", round(threat, 3))
