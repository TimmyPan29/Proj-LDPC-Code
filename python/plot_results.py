#!/usr/bin/env python3
"""
plot_results.py -- Recreate the measured BER/BLER curves as PNG figures.

This is a Python/matplotlib port of the scripts under ``matlab/``; the data are
the simulation results reported in the project. Two experiments are plotted:

  * SPA, fixed Monte-Carlo budget, varying the iteration cap (50 / 100 / 150).
  * MSA, fixed 70 iterations, varying the error-block target (50 / 100 / 200) --
    a check that the BER/BLER estimate is stable w.r.t. the Monte-Carlo budget.

Usage:
    pip install matplotlib numpy
    python python/plot_results.py          # writes PNGs into figures/
"""
import os
import numpy as np
import matplotlib

matplotlib.use("Agg")  # headless-safe backend
import matplotlib.pyplot as plt

FIG_DIR = os.path.join(os.path.dirname(__file__), os.pardir, "figures")

STYLES = ["-o", "--+", "-*"]


def snr_axis(n, step=0.2):
    """Eb/N0 sweep starting at 0 dB with `n` points."""
    return step * np.arange(n)


# --- SPA, fixed block, varying iteration cap (SNR 0 .. 3.6 dB, 19 points) ---
SPA_BER = {
    "No. Ite = 50":  [1.09e-01, 1.03e-01, 9.80e-02, 9.56e-02, 8.49e-02, 8.16e-02, 7.75e-02, 7.40e-02, 6.71e-02, 5.98e-02, 4.80e-02, 4.22e-02, 3.86e-02, 2.54e-02, 3.54e-03, 1.77e-03, 5.59e-04, 1.86e-04, 3.88e-05],
    "No. Ite = 100": [1.09e-01, 1.05e-01, 9.80e-02, 9.53e-02, 8.48e-02, 8.18e-02, 7.74e-02, 7.41e-02, 6.73e-02, 5.98e-02, 4.75e-02, 4.19e-02, 2.15e-02, 1.08e-02, 4.34e-03, 1.49e-03, 3.98e-04, 1.12e-04, 2.63e-05],
    "No. Ite = 150": [1.09e-01, 1.05e-01, 9.81e-02, 9.54e-02, 8.49e-02, 8.17e-02, 7.75e-02, 7.40e-02, 6.72e-02, 5.98e-02, 4.70e-02, 4.22e-02, 2.48e-02, 9.98e-03, 4.13e-03, 1.19e-03, 2.77e-04, 6.98e-05, 1.60e-05],
}
SPA_BLER = {
    "No. Ite = 50":  [1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 9.43e-01, 8.20e-01, 7.58e-01, 5.00e-01, 2.20e-01, 6.94e-02, 3.46e-02, 1.02e-02, 3.34e-03, 7.64e-04],
    "No. Ite = 100": [1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 9.43e-01, 8.20e-01, 7.46e-01, 4.00e-01, 1.93e-01, 8.16e-02, 2.78e-02, 7.29e-03, 2.03e-03, 4.87e-04],
    "No. Ite = 150": [1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 9.43e-01, 8.06e-01, 7.46e-01, 4.72e-01, 1.88e-01, 7.91e-02, 2.26e-02, 5.34e-03, 1.34e-03, 2.87e-04],
}

# --- MSA, 70 iterations, varying error-block target (SNR 0 .. 4.6 / 4.4 dB) ---
MSA_BER = {
    "Error blocks = 50":  [1.52e-01, 1.47e-01, 1.40e-01, 1.37e-01, 1.31e-01, 1.26e-01, 1.25e-01, 1.22e-01, 1.19e-01, 1.18e-01, 1.14e-01, 1.10e-01, 1.09e-01, 1.00e-01, 9.81e-02, 8.57e-02, 7.18e-02, 5.10e-02, 2.58e-02, 1.24e-02, 4.05e-03, 7.64e-04, 1.65e-04, 1.57e-05],
    "Error blocks = 100": [1.50e-01, 1.44e-01, 1.39e-01, 1.37e-01, 1.29e-01, 1.28e-01, 1.24e-01, 1.22e-01, 1.20e-01, 1.18e-01, 1.11e-01, 1.10e-01, 1.09e-01, 1.05e-01, 9.96e-02, 9.05e-02, 6.77e-02, 5.39e-02, 2.75e-02, 1.23e-02, 3.95e-03, 9.75e-04, 1.51e-04, 1.76e-05],
    "Error blocks = 200": [1.496e-01, 1.439e-01, 1.406e-01, 1.375e-01, 1.324e-01, 1.310e-01, 1.247e-01, 1.222e-01, 1.218e-01, 1.168e-01, 1.146e-01, 1.122e-01, 1.087e-01, 1.086e-01, 9.734e-02, 9.226e-02, 7.614e-02, 4.691e-02, 2.756e-02, 1.293e-02, 3.348e-03, 9.958e-04, 1.71e-04],
}
MSA_BLER = {
    "Error blocks = 50":  [1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 9.80e-01, 8.77e-01, 6.94e-01, 5.00e-01, 2.62e-01, 1.29e-01, 4.45e-02, 8.24e-03, 1.75e-03, 1.78e-04],
    "Error blocks = 100": [1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 9.62e-01, 8.70e-01, 6.71e-01, 5.38e-01, 2.82e-01, 1.31e-01, 4.21e-02, 9.93e-03, 1.70e-03, 1.88e-04],
    "Error blocks = 200": [1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 1.00e+00, 9.95e-01, 9.48e-01, 9.01e-01, 7.69e-01, 4.85e-01, 2.77e-01, 1.34e-01, 3.50e-02, 1.02e-02, 1.77e-03],
}


def plot_curves(curves, title, ylabel, color, out_path):
    fig, ax = plt.subplots(figsize=(6.4, 4.6))
    for style, (label, data) in zip(STYLES, curves.items()):
        snr = snr_axis(len(data))
        ax.semilogy(snr, data, style, color=color, linewidth=1.2,
                    markersize=5, label=label)
    ax.set_xlabel("Eb/N0 (dB)")
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.grid(True, which="both", linestyle=":", linewidth=0.6)
    ax.legend()
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)
    print("wrote", out_path)


def main():
    os.makedirs(FIG_DIR, exist_ok=True)
    plot_curves(SPA_BER,  "SPA: BER vs Eb/N0 (varying iterations)",
                "BER",  "tab:red",  os.path.join(FIG_DIR, "spa_ber.png"))
    plot_curves(SPA_BLER, "SPA: BLER vs Eb/N0 (varying iterations)",
                "BLER", "tab:blue", os.path.join(FIG_DIR, "spa_bler.png"))
    plot_curves(MSA_BER,  "MSA (70 iter): BER vs Eb/N0",
                "BER",  "tab:red",  os.path.join(FIG_DIR, "msa_ber.png"))
    plot_curves(MSA_BLER, "MSA (70 iter): BLER vs Eb/N0",
                "BLER", "tab:blue", os.path.join(FIG_DIR, "msa_bler.png"))


if __name__ == "__main__":
    main()
