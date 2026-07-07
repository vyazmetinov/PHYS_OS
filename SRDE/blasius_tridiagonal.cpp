#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

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

    std::vector<double> x(n, 0.0);
    x[n - 1] = dprime[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        x[i] = dprime[i] - cprime[i] * x[i + 1];
    }
    return x;
}

std::vector<double> integrate_f(const std::vector<double>& u, double h) {
    const int n = static_cast<int>(u.size()) - 1;
    std::vector<double> f(n + 1, 0.0);
    for (int i = 1; i <= n; ++i) {
        f[i] = f[i - 1] + 0.5 * h * (u[i - 1] + u[i]);
    }
    return f;
}

struct Result {
    std::vector<double> eta;
    std::vector<double> f;
    std::vector<double> u;
};

Result solve_blasius(int n = 2000, double eta_max = 12.0, int max_iter = 200, double tol = 1e-9) {
    const double h = eta_max / static_cast<double>(n);
    std::vector<double> eta(n + 1, 0.0);
    for (int i = 0; i <= n; ++i) {
        eta[i] = i * h;
    }

    // Initial profile satisfying u(0)=0 and u(eta_max)=1.
    std::vector<double> u(n + 1, 0.0);
    for (int i = 0; i <= n; ++i) {
        u[i] = 1.0 - std::exp(-eta[i]);
    }
    u[0] = 0.0;
    u[n] = 1.0;

    for (int iter = 0; iter < max_iter; ++iter) {
        const std::vector<double> f = integrate_f(u, h);

        const int m = n - 1;  // inner unknowns u_1..u_{n-1}
        std::vector<double> lower(m - 1, 0.0);
        std::vector<double> diag(m, 0.0);
        std::vector<double> upper(m - 1, 0.0);
        std::vector<double> rhs(m, 0.0);

        for (int i = 1; i <= n - 1; ++i) {
            const double fi = f[i];
            const double ai = 1.0 / (h * h) - fi / (4.0 * h);
            const double bi = -2.0 / (h * h);
            const double ci = 1.0 / (h * h) + fi / (4.0 * h);

            const int row = i - 1;
            diag[row] = bi;
            if (i > 1) {
                lower[row - 1] = ai;
            } else {
                rhs[row] -= ai * 0.0;  // u(0)=0
            }

            if (i < n - 1) {
                upper[row] = ci;
            } else {
                rhs[row] -= ci * 1.0;  // u(eta_max)=1
            }
        }

        std::vector<double> inner = solve_tridiagonal(lower, diag, upper, rhs);
        std::vector<double> u_new(n + 1, 0.0);
        u_new[0] = 0.0;
        u_new[n] = 1.0;
        for (int i = 1; i <= n - 1; ++i) {
            u_new[i] = inner[i - 1];
        }

        double max_delta = 0.0;
        for (int i = 0; i <= n; ++i) {
            max_delta = std::max(max_delta, std::abs(u_new[i] - u[i]));
        }
        u.swap(u_new);

        if (max_delta < tol) {
            return {eta, integrate_f(u, h), u};
        }
    }

    return {eta, integrate_f(u, h), u};
}

}  // namespace

int main() {
    try {
        const Result r = solve_blasius();

        const int n = static_cast<int>(r.eta.size()) - 1;
        const double h = r.eta[1] - r.eta[0];
        const double fpp0 = (-3.0 * r.u[0] + 4.0 * r.u[1] - r.u[2]) / (2.0 * h);

        std::cout << std::fixed << std::setprecision(8);
        std::cout << "Blasius solution by iterative tridiagonal sweep\n";
        std::cout << "eta_max = " << r.eta.back() << ", nodes = " << n + 1 << '\n';
        std::cout << "f''(0) ≈ " << fpp0 << " (reference ~0.33205734)\n\n";
        std::cout << "eta        f(eta)      u(eta)=f'(eta)\n";

        const int stride = n / 20;
        for (int i = 0; i <= n; i += std::max(1, stride)) {
            std::cout << std::setw(7) << r.eta[i] << "   " << std::setw(10) << r.f[i] << "   " << std::setw(10)
                      << r.u[i] << '\n';
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
