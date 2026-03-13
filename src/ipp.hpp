#pragma once

class optimization::IPP : public LPP {
public:
    using LPP::LPP;

    std::variant<std::vector<std::map<algebra::Variable, algebra::Fraction>>, std::string> optimize_branch_bound(const std::string& path) const {
        int i = 1;
        algebra::Fraction optimal = type == Optimization::MAXIMIZE ? -algebra::inf : algebra::inf;
        std::queue<IPP> queue;
        std::vector<std::map<algebra::Variable, algebra::Fraction>> res;
        queue.push(*this);

        while (!queue.empty()) {
            IPP current = queue.front();
            std::variant<std::vector<std::map<algebra::Variable, algebra::Fraction>>, std::string> result =
                current.optimize_graphical(path + "/graph" + std::to_string(i++) + ".png");
            queue.pop();

            if (const std::vector<std::map<algebra::Variable, algebra::Fraction>>* solution =
                    std::get_if<std::vector<std::map<algebra::Variable, algebra::Fraction>>>(&result)) {
                bool is_fractional = false;

                for (const std::map<algebra::Variable, algebra::Fraction>& ans : *solution) {
                    for (const auto& [variable, fraction] : ans) {
                        if (fraction.denominator != 1 && variable.variables.front().name != "z") {
                            current.LPP::constraints.push_back(variable <= static_cast<int64_t>(fraction));
                            queue.push(current);
                            current.LPP::constraints.back() = variable >= static_cast<int64_t>(fraction) + 1;
                            queue.push(current);
                            is_fractional = true;
                        }
                        if (GLOBAL_FORMATTING.verbose) {
                            *GLOBAL_FORMATTING.out << variable << '=' << fraction << " ";
                        }
                    }
                    if (GLOBAL_FORMATTING.verbose) {
                        *GLOBAL_FORMATTING.out << std::endl;
                    }
                }
                if (!is_fractional) {
                    const algebra::Fraction& value = solution->front().at(algebra::Variable("z"));

                    if (type == Optimization::MAXIMIZE && optimal < value || type == Optimization::MINIMIZE && optimal > value) {
                        optimal = value;
                        res = *solution;
                    }
                }
            } else {
                if (GLOBAL_FORMATTING.verbose) {
                    *GLOBAL_FORMATTING.out << std::get<std::string>(result);
                }
            }
        }
        return res;
    }
};
