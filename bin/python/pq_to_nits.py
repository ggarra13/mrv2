import numpy as np

def pq_to_nits(v):
    """
    Converts a normalized PQ value (0.0 to 1.0) to absolute Luminance (Nits).
    Based on SMPTE ST 2084.
    """
    # SMPTE ST 2084 Constants
    m1 = 2610 / 16384
    m2 = (2523 / 4096) * 128
    c1 = 3424 / 4096
    c2 = (2413 / 4096) * 32
    c3 = (2392 / 4096) * 32

    # Clip to 0 to avoid errors with power
    v = np.maximum(v, 0.0)
    
    # PQ Decode
    v_pow = np.power(v, 1.0 / m2)
    numerator = np.maximum(v_pow - c1, 0.0)
    denominator = c2 - c3 * v_pow
    
    # Calculate Nits (multiplied by the 10,000 nit peak)
    l_normalized = np.power(numerator / denominator, 1.0 / m1)
    return 10000.0 * l_normalized

# Verification Table
test_vals = [0.0, 0.5, 0.508, 0.5819, 0.7518, 1.0]
print(f"{'Input (V)':<12} | {'Output (Nits)':<15}")
print("-" * 30)
for val in test_vals:
    print(f"{val:<12} | {pq_to_nits(val):<15.2f}")
