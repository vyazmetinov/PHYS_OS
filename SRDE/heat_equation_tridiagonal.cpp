#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {

std::vector<double> make_grid(int cells, double alpha) {
    std::vector<double> x(cells + 1);
    if (alpha <= 10.0) {
        for (int i = 0; i <= cells; ++i) {
            x[i] = static_cast<double>(i) / static_cast<double>(cells);
        }
        return x;
    }

    // Stretching that refines near both boundaries.
    constexpr double beta = 3.5;
    for (int i = 0; i <= cells; ++i) {
        const double xi = static_cast<double>(i) / static_cast<double>(cells);
        const double s = std::tanh(beta * (2.0 * xi - 1.0)) / std::tanh(beta);
        x[i] = 0.5 * (1.0 + s);
    }
    return x;
}

double exact_solution(double x, double alpha) {
    if (std::abs(alpha) < 1e-12) {
        return x;
    }
    // Stable equivalent form to avoid overflow for large alpha.
    const double e_minus_alpha = std::exp(-alpha);
    return (std::exp(alpha * (x - 1.0)) - e_minus_alpha) / (1.0 - e_minus_alpha);
}

std::vector<double> solve_tridiagonal(
    const std::vector<double>& lower,
    const std::vector<double>& diag,
    const std::vector<double>& upper,
    const std::vector<double>& rhs) {
    const int n = static_cast<int>(diag.size());
    if (n == 0 || static_cast<int>(rhs.size()) != n || static_cast<int>(lower.size()) != n - 1 ||
        static_cast<int>(upper.size()) != n - 1) {
        throw std::runtime_error("Invalid tridiagonal system sizes");
    }

    std::vector<double> cprime(n - 1, 0.0);
    std::vector<double> dprime(n, 0.0);

    if (std::abs(diag[0]) < 1e-14) {
        throw std::runtime_error("Zero pivot in tridiagonal solver");
    }
    cprime[0] = upper[0] / diag[0];
    dprime[0] = rhs[0] / diag[0];

    for (int i = 1; i < n; ++i) {
        const double denom = diag[i] - lower[i - 1] * cprime[i - 1];
        if (std::abs(denom) < 1e-14) {
            throw std::runtime_error("Zero pivot in tridiagonal solver");
        }
        if (i < n - 1) {
            cprime[i] = upper[i] / denom;
        }
        dprime[i] = (rhs[i] - lower[i - 1] * dprime[i - 1]) / denom;
    }

    std::vector<double> solution(n, 0.0);
    solution[n - 1] = dprime[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        solution[i] = dprime[i] - cprime[i] * solution[i + 1];
    }
    return solution;
}

std::tuple<std::vector<double>, std::vector<double>> solve_temperature_profile(int cells, double alpha) {
    const double t_left = 0.0;
    const double t_right = 1.0;

    const std::vector<double> x = make_grid(cells, alpha);
    const int unknowns = cells - 1;

    std::vector<double> lower(unknowns - 1, 0.0);
    std::vector<double> diag(unknowns, 0.0);
    std::vector<double> upper(unknowns - 1, 0.0);
    std::vector<double> rhs(unknowns, 0.0);

    for (int i = 1; i <= unknowns; ++i) {
        const double h_minus = x[i] - x[i - 1];
        const double h_plus = x[i + 1] - x[i];

        const double a = 2.0 / (h_minus * (h_minus + h_plus)) + alpha / (h_minus + h_plus);
        const double b = -2.0 / (h_minus * h_plus);
        const double c = 2.0 / (h_plus * (h_minus + h_plus)) - alpha / (h_minus + h_plus);

        diag[i - 1] = b;
        if (i > 1) {
            lower[i - 2] = a;
        } else {
            rhs[i - 1] -= a * t_left;
        }

        if (i < unknowns) {
            upper[i - 1] = c;
        } else {
            rhs[i - 1] -= c * t_right;
        }
    }

    const std::vector<double> inner = solve_tridiagonal(lower, diag, upper, rhs);
    std::vector<double> t(cells + 1, 0.0);
    t.front() = t_left;
    t.back() = t_right;
    for (int i = 1; i <= unknowns; ++i) {
        t[i] = inner[i - 1];
    }

    return {x, t};
}

void run_case(double alpha, int cells) {
    const auto [x, t] = solve_temperature_profile(cells, alpha);

    double max_err = 0.0;
    for (int i = 0; i <= cells; ++i) {
        const double e = std::abs(t[i] - exact_solution(x[i], alpha));
        max_err = std::max(max_err, e);
    }

    std::cout << "alpha = " << alpha << ", cells = " << cells << ", max|T - Texact| = " << max_err << '\n';
    std::cout << "  x          T_num       T_exact      abs_err\n";
    for (int i = 0; i <= cells; i += std::max(1, cells / 10)) {
        const double exact = exact_solution(x[i], alpha);
        const double err = std::abs(t[i] - exact);
        std::cout << std::fixed << std::setprecision(6) << "  " << std::setw(8) << x[i] << "  " << std::setw(10)
                  << t[i] << "  " << std::setw(10) << exact << "  " << std::setw(10) << err << '\n';
    }
    std::cout << '\n';
}

}  // namespace

int main() {
    try {
        run_case(1.0, 120);
        run_case(100.0, 500);
        run_case(1000.0, 1200);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
