#include "ObjectModel.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
constexpr double kDt = 0.1;

struct Signal {
    std::string id;
    std::string name;
    std::string formula;
};

struct ResultRow {
    std::string signalId;
    int step;
    int tau;
    double time;
    double input;
    std::string model;
    double output;
};

int readChoice(const std::string& prompt, int min, int max) {
    std::cout << prompt;

    int choice = 0;
    if (!(std::cin >> choice) || choice < min || choice > max) {
        throw std::invalid_argument("Неверный выбор.");
    }

    return choice;
}

int readSteps() {
    return readChoice(
        "Введите количество шагов n (1-100000): ",
        1, 100000
    );
}

double readCoefficient(const std::string& name) {
    std::cout << "Введите коэффициент " << name << ": ";

    double value = 0.0;

    if (!(std::cin >> value) || !std::isfinite(value)) {
        throw std::invalid_argument(
            "Коэффициент должен быть конечным числом."
        );
    }

    return value;
}

std::unique_ptr<Model> createModel(int choice) {
    switch (choice) {
    case 1: {
        std::cout << "\nПараметры Model 1.6\n";

        double a1 = readCoefficient("a1");
        double a2 = readCoefficient("a2");
        double a3 = readCoefficient("a3");
        double b = readCoefficient("b");

        return std::make_unique<Model16>(a1, a2, a3, b);
    }

    case 2: {
        std::cout << "\nПараметры Model 2.8\n";

        double a = readCoefficient("a");
        double b = readCoefficient("b");
        double c = readCoefficient("c");

        return std::make_unique<Model28>(a, b, c);
    }

    case 3: {
        std::cout << "\nПараметры Model 3.10\n";

        double a = readCoefficient("a");
        double b = readCoefficient("b");

        return std::make_unique<Model310>(a, b);
    }

    default:
        throw std::invalid_argument("Неизвестная модель.");
    }
}

double controlInput(const std::string& id, int tau) {
    if (id == "step") {
        return 1.0;
    }

    if (id == "impulse") {
        return tau == 0 ? 1.0 : 0.0;
    }

    if (id == "harmonic") {
        return std::sin(static_cast<double>(tau));
    }

    throw std::invalid_argument("Неизвестное воздействие.");
}

void printTable(
    const Signal& signal,
    const std::string& modelName,
    const std::vector<ResultRow>& rows
) {
    std::cout << "\nВходное воздействие: "
              << signal.name << '\n';

    std::cout << "Формула: " << signal.formula << '\n';
    std::cout << "Модель: " << modelName << "\n\n";

    std::cout << std::left
              << std::setw(8) << "Шаг"
              << std::setw(8) << "tau"
              << std::setw(12) << "Время"
              << std::setw(12) << "u(tau)"
              << std::setw(14) << "y(tau)"
              << '\n';

    std::cout << std::string(54, '-') << '\n';

    for (const auto& row : rows) {
        std::cout << std::fixed << std::setprecision(4)
                  << std::left
                  << std::setw(8) << row.step
                  << std::setw(8) << row.tau
                  << std::setw(12) << row.time
                  << std::setw(12) << row.input
                  << std::setw(14) << row.output
                  << '\n';
    }
}

void writeCsv(
    const std::string& fileName,
    const std::vector<ResultRow>& rows
) {
    std::ofstream out(fileName);

    if (!out) {
        throw std::runtime_error(
            "Не удалось создать файл: " + fileName
        );
    }

    out.imbue(std::locale::classic());

    out << "signal,step,tau,time,u,model,y\n";
    out << std::fixed << std::setprecision(8);

    for (const auto& row : rows) {
        out << row.signalId << ','
            << row.step << ','
            << row.tau << ','
            << row.time << ','
            << row.input << ','
            << row.model << ','
            << row.output << '\n';
    }

    if (!out) {
        throw std::runtime_error("Ошибка записи CSV.");
    }
}
} // namespace

int main() {
    try {
        std::cin.imbue(std::locale::classic());
        std::cout.imbue(std::locale::classic());

        const int n = readSteps();

        const int modelChoice = readChoice(
            "\nВыберите модель:\n"
            "1. Model 1.6 - линейная система третьего порядка\n"
            "2. Model 2.8 - нелинейная логистическая модель\n"
            "3. Model 3.10 - дифференциальное уравнение\n"
            "Ваш выбор: ",
            1, 3
        );

        const std::vector<Signal> signals = {
            {"step", "Ступенчатое", "u(tau) = 1"},
            {"impulse", "Импульсное",
             "u(0) = 1, при tau > 0: u(tau) = 0"},
            {"harmonic", "Гармоническое",
             "u(tau) = sin(tau)"}
        };

        const int signalChoice = readChoice(
            "\nВыберите входное воздействие:\n"
            "1. Ступенчатое (u(tau) = 1)\n"
            "2. Импульсное (u(0) = 1, далее 0)\n"
            "3. Гармоническое (u(tau) = sin(tau))\n"
            "Ваш выбор: ",
            1, 3
        );

        const Signal& signal =
            signals[static_cast<std::size_t>(signalChoice - 1)];

        // Создаём только выбранную модель.
        auto model = createModel(modelChoice);
        const std::string modelName = model->name();

        std::cout << "\nВыполняется симуляция.\n"
                  << "Модель: " << modelName << '\n'
                  << "Воздействие: " << signal.name << '\n'
                  << "Формула: " << signal.formula << '\n';

        std::vector<ResultRow> rows;
        rows.reserve(static_cast<std::size_t>(n));

        for (int tau = 0; tau < n; ++tau) {
            const double u = controlInput(signal.id, tau);
            const double y = model->advance(u, kDt);

            rows.push_back({
                signal.id,
                tau + 1,
                tau,
                (tau + 1) * kDt,
                u,
                modelName,
                y
            });
        }

        printTable(signal, modelName, rows);
        writeCsv("results.csv", rows);

        std::cout << "\nРезультаты сохранены в results.csv\n"
                  << "Теперь будет предложено построить график.\n";

        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "\nОшибка: " << ex.what() << '\n';
        return 1;
    }
}