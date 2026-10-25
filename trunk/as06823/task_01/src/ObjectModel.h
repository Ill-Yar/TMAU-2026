#pragma once

#include <cmath>
#include <stdexcept>
#include <string>

class Model {
public:
    virtual ~Model() = default;
    virtual std::string name() const = 0;

    double advance(double input, double dt) {
        const double next = calculateNext(input, dt);
        if (!std::isfinite(next)) {
            throw std::runtime_error(
                "Модель вернула некорректное значение. Проверьте коэффициенты.");
        }
        yPrev2_ = yPrev_;
        yPrev_ = y_;
        y_ = next;
        return y_;
    }

    double value() const noexcept { return y_; }

protected:
    virtual double calculateNext(double input, double dt) = 0;

    double y_ = 0.0;       
    double yPrev_ = 0.0;   
    double yPrev2_ = 0.0;  
};

class Model16 final : public Model {
public:
    Model16(double a1, double a2, double a3, double b)
        : a1_(a1), a2_(a2), a3_(a3), b_(b) {}

    std::string name() const override { return "Model 1.6"; }

protected:
    double calculateNext(double input, double /*dt*/) override {
        return a1_ * y_ + a2_ * yPrev_ + a3_ * yPrev2_ + b_ * input;
    }

private:
    double a1_;
    double a2_;
    double a3_;
    double b_;
};

class Model28 final : public Model {
public:
    Model28(double a, double b, double c)
        : a_(a), b_(b), c_(c) {}

    std::string name() const override { return "Model 2.8"; }

protected:
    double calculateNext(double input, double /*dt*/) override {
        return a_ * y_ * (1.0 - y_) + b_ * input
             + c_ * std::sin(yPrev_ * input);
    }

private:
    double a_;
    double b_;
    double c_;
};

class Model310 final : public Model {
public:
    Model310(double a, double b) : a_(a), b_(b) {}

    std::string name() const override { return "Model 3.10"; }

protected:
    double calculateNext(double input, double dt) override {
        if (dt <= 0.0) {
            throw std::invalid_argument("Шаг интегрирования dt должен быть положительным.");
        }
        const double derivative = -a_ * y_ + b_ + input;
        return y_ + dt * derivative;
    }

private:
    double a_;
    double b_;
};
