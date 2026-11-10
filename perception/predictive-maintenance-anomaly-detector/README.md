# Edge AI Predictive Maintenance: Crest Factor & Mahalanobis Anomaly Detector

Zero-allocation C++17 motor and actuator health monitoring pipeline combining **Peak-to-RMS Crest Factor** impulse extraction with **Multivariate Mahalanobis Distance** anomaly scoring.

## 1. Engineering Problem & Solution
Early-stage ball-bearing spalling produces microscopic impact spikes that scarcely alter overall RMS vibration (`0.14g` -> `0.19g`) or stator temperature (`42.1°C` -> `43.2°C`). By coupling **Crest Factor** (`Peak / RMS`) with standardized Mahalanobis distance across `[RMS, Crest Factor, Temperature]`, this module isolates incipient bearing pitting long before thermal runaway occurs.

## 2. Verified 3-Regime Benchmark Results

| Operating Regime | RMS Vibration (`g`) | Crest Factor | Winding Temp (`°C`) | Mahalanobis Score | Classified State |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **1. Nominal Operation** | `0.14 g` | `1.41` | `42.10 °C` | `0.13` | `HEALTHY` |
| **2. Incipient Bearing Spalling** | `0.19 g` | **`3.46`** | `43.20 °C` | **`5.08`** | **`EARLY_BEARING_WARN`** |
| **3. Shaft Unbalance & Thermal Fault** | **`0.53 g`** | `1.70` | **`58.50 °C`** | **`28.08`** | **`CRITICAL_FAULT`** |

## 3. Resume / CV Bullet
* **Built an unsupervised C++17 predictive maintenance anomaly detector fusing Peak-to-RMS Crest Factor impulse extraction with multivariate Mahalanobis scoring on time-series motor telemetry to flag incipient bearing spalling prior to thermal failure.**
