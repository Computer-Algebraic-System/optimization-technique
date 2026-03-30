#pragma once

class optimization::NLPP : public LPP {
public:
    NLPP(const Optimization type, const algebra::SimplePolynomial& objective, const std::vector<algebra::Inequation>& constraints,
         const std::vector<algebra::Inequation>& restrictions) : LPP(type, objective, constraints, restrictions) {
        for (algebra::Inequation& constraint : this->constraints) {
            constraint -= constraint.rhs;
        }
    }

    std::map<algebra::Variable, algebra::Fraction> optimize() const {
        GLOBAL_FORMATTING << *this;
        const int size = constraints.size();
        algebra::SimplePolynomial lagrange_multiplier = objective;
        std::vector<algebra::Variable> lagrange_variables;
        std::vector<algebra::Equation> equations;
        std::set<algebra::Variable> seen;
        lagrange_variables.reserve(size);

        for (int i = 0; i < size; i++) {
            lagrange_variables.emplace_back("L" + std::to_string(i + 1));
            lagrange_multiplier -= lagrange_variables.back() * constraints[i].lhs;
        }
        GLOBAL_FORMATTING << lagrange_multiplier;

        for (const algebra::Variable& variable :
             std::array{objective.terms, lagrange_variables} | std::views::join | std::views::filter([&seen](const algebra::Variable& var) -> bool {
                 return !var.is_fraction() && !seen.contains(var.variables[0].name);
             })) {
            equations.push_back(lagrange_multiplier.differentiate(variable.variables[0].name) == 0);
            const auto itr = std::ranges::find_if(equations.back().lhs.terms, [](const algebra::Variable& var) -> bool { return var.is_fraction(); });

            if (itr != equations.back().lhs.terms.end()) {
                equations.back() -= *itr;
            }
            seen.emplace(variable.variables[0].name);
        }
        tensor::solve_linear_system(equations);
        return {};
    }
};
