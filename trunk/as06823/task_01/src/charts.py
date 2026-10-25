import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt


SIGNALS = {
    "step": ("Ступенчатое воздействие", "u(τ) = 1"),
    "impulse": ("Импульсное воздействие", "u(0) = 1, далее u(τ) = 0"),
    "harmonic": ("Гармоническое воздействие", "u(τ) = sin(τ)"),
}


def main():
    csv_path = Path(sys.argv[1] if len(sys.argv) > 1 else "results.csv")

    if not csv_path.is_absolute():
        csv_path = Path.cwd() / csv_path

    if not csv_path.is_file():
        print(f"Файл с результатами не найден: {csv_path}")
        return 1

    # Папка doc находится рядом с папкой src.
    src_dir = Path(__file__).resolve().parent
    doc_dir = src_dir.parent / "doc"
    doc_dir.mkdir(parents=True, exist_ok=True)

    with csv_path.open("r", newline="", encoding="utf-8") as file:
        rows = list(csv.DictReader(file))

    if not rows:
        print("CSV-файл не содержит результатов.")
        return 1

    signal_id = rows[0]["signal"]

    if signal_id not in SIGNALS:
        print(f"Неизвестный тип воздействия: {signal_id}")
        return 1

    if any(row["signal"] != signal_id for row in rows):
        print("Ошибка: CSV содержит несколько воздействий.")
        return 1

    title, formula = SIGNALS[signal_id]
    model_name = rows[0]["model"]

    # Значения входного сигнала и выхода модели.
    tau = [int(row["tau"]) for row in rows]
    inputs = [float(row["u"]) for row in rows]
    outputs = [float(row["y"]) for row in rows]

    print(f"Входное воздействие: {title}")
    print(f"Формула: {formula}")
    print(f"Модель: {model_name}")

    # Одно окно с двумя графиками.
    fig, (ax1, ax2) = plt.subplots(
        2, 1,
        figsize=(11, 8),
        sharex=True
    )

    # График 1: входное воздействие.
    ax1.plot(
        tau, inputs,
        color="tab:blue",
        marker="o",
        markersize=4,
        linewidth=1.8
    )

    ax1.set_title(f"1. Входное воздействие: {title}\n{formula}")
    ax1.set_ylabel("u(τ)")
    ax1.grid(True, alpha=0.3)
    ax1.axhline(0, color="black", linewidth=0.7)

    # График 2: выход модели и входное воздействие.
    ax2.plot(
        tau,
        outputs,
        color="tab:orange",
        marker="o",
        markersize=4,
        linewidth=1.8,
        label=f"Выход {model_name}"
    )

    ax2.set_title(
        f"2. Входное воздействие и выход модели {model_name}"
    )
    ax2.set_xlabel("Момент времени τ")
    ax2.set_ylabel("Выход модели y(τ+1)", color="tab:orange")
    ax2.tick_params(axis="y", labelcolor="tab:orange")
    ax2.grid(True, alpha=0.3)

    # Дополнительная линия входного сигнала на той же диаграмме.
    ax_input = ax2.twinx()

    ax_input.plot(
        tau,
        inputs,
        color="tab:blue",
        linestyle="--",
        marker="s",
        markersize=3,
        linewidth=1.6,
        label="Входное воздействие u(τ)"
    )

    ax_input.set_ylabel(
        "Входное воздействие u(τ)",
        color="tab:blue"
    )
    ax_input.tick_params(axis="y", labelcolor="tab:blue")

    # Общая легенда для обеих линий.
    lines1, labels1 = ax2.get_legend_handles_labels()
    lines2, labels2 = ax_input.get_legend_handles_labels()

    ax2.legend(
        lines1 + lines2,
        labels1 + labels2,
        loc="best"
    )

    model_slug = model_name.replace("Model ", "").replace(" ", "")
    image_path = doc_dir / f"module_{model_slug}_{signal_id}.png"

    fig.savefig(
        image_path,
        dpi=160,
        bbox_inches="tight"
    )

    print(f"\nОба графика сохранены в одном изображении:\n{image_path.resolve()}")

    plt.show()
    plt.close(fig)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())