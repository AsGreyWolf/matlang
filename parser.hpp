#ifndef PARSER_HPP
#define PARSER_HPP
#include "object.hpp"
#include <cctype>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <stdexcept>
#include <string>

namespace matlang {

struct parse_error : std::exception {
	size_t i_;
	std::string line_;
	parse_error(size_t i, const std::string &name)
	    : i_{i}, line_{name + " expected at position " + std::to_string(i)} {}
	const char *what() const throw() override { return line_.c_str(); }
};
class parser {
	std::map<std::string, object> vars;
	std::list<object> temp_vars;

	size_t implicit_space(size_t start, const std::string &line) {
		size_t i = start;
		while (i < line.size() && std::isspace(line[i]))
			i++;
		return i;
	}
	size_t parse_index(size_t start, const std::string &line, size_t &result) {
		size_t i = start;
		std::string num;
		if (i >= line.size() || !isdigit(line[i]))
			throw parse_error(i, "index");
		if (line[i] == '0') {
			i++;
			result = 0;
			return i;
		}
		while (i < line.size() && isdigit(line[i])) {
			num.push_back(line[i]);
			i++;
		}
		if (num.size() > 9)
			throw parse_error(i, "index");
		size_t err;
		result = stoull(num, &err);
		return i;
	}
	size_t parse_float(size_t start, const std::string &line, double &result) {
		size_t i = start;
		std::string num;
		if (i < line.size() && line[i] == '-') {
			num.push_back('-');
			i++;
		}
		if (i >= line.size() || !isdigit(line[i]))
			throw parse_error(i, "float");
		if (i < line.size() && line[i] == '0') {
			num.push_back('0');
			i++;
		} else
			while (i < line.size() && isdigit(line[i])) {
				num.push_back(line[i]);
				i++;
			}
		if (i < line.size() && line[i] == '.') {
			num.push_back(line[i]);
			i++;
			while (i < line.size() && isdigit(line[i])) {
				num.push_back(line[i]);
				i++;
			}
		}
		size_t err;
		result = stod(num, &err);
		return i;
	}
	size_t parse_identifier(size_t start, const std::string &line,
	                        std::string &result) {
		size_t i = start;
		std::string name;
		if (i >= line.size() || !(isalpha(line[i]) || line[i] == '_'))
			throw parse_error(i, "identifier");
		while (i < line.size() && (isalnum(line[i]) || line[i] == '_')) {
			name.push_back(line[i]);
			i++;
		}
		result = move(name);
		return i;
	}
	size_t parse_slice(size_t start, const std::string &line, slice &result) {
		size_t i = start;
		result.clear();
		while (true) {
			size_t id;
			i = parse_index(i, line, id);
			result.push_back(id);
			i = implicit_space(i, line);
			if (i >= line.size() || line[i] != ',')
				break;
			i++;
			i = implicit_space(i, line);
		}
		return i;
	}
	size_t parse_object(size_t start, const std::string &line, object &result) {
		size_t i = start;
		if (i >= line.size())
			throw parse_error(i, "object");
		if (line[i] == '[') {
			object::container_impl container;
			i++;
			while (true) {
				i = implicit_space(i, line);
				object o;
				i = eval_expression(i, line, o);
				container.push_back(std::move(o));
				i = implicit_space(i, line);
				if (i >= line.size() || line[i] != ',')
					break;
				i++;
			}
			if (i >= line.size() || line[i] != ']')
				throw parse_error(i, "]");
			i++;
			result = object(move(container));
		} else {
			double d;
			i = parse_float(i, line, d);
			result = object(d);
		}
		return i;
	}
	struct view_link {
		std::string name;
		std::vector<slice> sl;
	};
	size_t parse_view_link(size_t start, const std::string &line,
	                       view_link &result) {
		size_t i = start;
		i = parse_identifier(i, line, result.name);
		while (i < line.size() && line[i] == '[') {
			i++;
			i = implicit_space(i, line);
			slice sl;
			i = parse_slice(i, line, sl);
			result.sl.push_back(sl);
			if (i >= line.size() || line[i] != ']')
				throw parse_error(i, "]");
			i++;
		}
		return i;
	}
	using operand = std::variant<view_link, object>;
	size_t parse_operand(size_t start, const std::string &line, operand &result) {
		size_t i = start;
		try {
			view_link vl;
			i = parse_view_link(i, line, vl);
			result = std::move(vl);
		} catch (parse_error &) {
			try {
				object obj;
				i = parse_object(i, line, obj);
				result = std::move(obj);
			} catch (parse_error &err) {
				std::cerr << err.what() << std::endl;
				throw parse_error(i, "operand");
			}
		}
		return i;
	}
	object get(object op) { return std::move(op); }
	object get(object &o, std::vector<slice> sl, size_t dim = 0) {
		if (sl.size() == dim)
			return o;
		if (*std::max_element(sl[dim].begin(), sl[dim].end()) >= o.size())
			throw std::invalid_argument("index out of bounds");
		if (dim == sl.size() - 1)
			if (sl[dim].size() == 1) {
				return o[sl[dim][0]];
			} else
				return o[std::move(sl[dim])].copy();
		if (sl[dim].size() == 1) {
			size_t id = sl[dim][0];
			if (!o.flat())
				return get(o[id], move(sl), dim + 1);
			throw std::invalid_argument("invalid dimension");
		}
		throw std::invalid_argument("slices are allowed only on a final dimension");
	}
	object get(view_link op) {
		auto vari = vars.find(op.name);
		if (vari == vars.end())
			throw std::invalid_argument(op.name + " is not defined");
		return get(vari->second, std::move(op.sl));
	}
	object get(operand op) {
		return std::visit([this](auto a) { return get(std::move(a)); },
		                  std::move(op));
	}
	object_view get_view(object op) {
		temp_vars.push_back(std::move(op));
		return temp_vars.back().view();
	}
	object_view get_view(object &o, std::vector<slice> sl, size_t dim = 0) {
		if (sl.size() == dim)
			return o.view();
		if (*std::max_element(sl[dim].begin(), sl[dim].end()) >= o.size())
			throw std::invalid_argument("index out of bounds");
		if (dim == sl.size() - 1)
			if (sl[dim].size() == 1 && !o[sl[dim][0]].flat()) {
				return o[sl[dim][0]].view();
			} else
				return o[std::move(sl[dim])];
		if (sl[dim].size() == 1) {
			int id = sl[dim][0];
			if (!o[id].flat())
				return get_view(o[id], std::move(sl), dim + 1);
			throw std::invalid_argument("invalid dimension");
		}
		throw std::invalid_argument("slices are allowed only on a final dimension");
	}
	object_view get_view(view_link op) {
		auto vari = vars.find(op.name);
		if (vari == vars.end())
			throw std::invalid_argument(op.name + " is not defined");
		return get_view(vari->second, std::move(op.sl));
	}
	object_view get_view(operand op) {
		return std::visit([this](auto a) { return get_view(std::move(a)); },
		                  std::move(op));
	}
	std::map<char, size_t> priority{{'-', 1},  {'+', 1},  {'*', 3}, {')', 0},
	                                {-'(', 0}, {-'-', 4}, {-'+', 4}};
	std::map<char, bool> unary{
	    {'(', true}, {'-', true}, {'+', true}, {'*', false}, {')', false}};
	std::map<char, bool> binary{
	    {'(', false}, {'-', true}, {'+', true}, {'*', true}, {')', true}};
	std::map<char, std::function<object(object)>> unary_evaluators{
	    {-'-',
	     [&](auto a) {
		     std::cout << a << " unary-" << std::endl;
		     return object(-1) * a;
	     }},
	    {-'+', [&](auto a) {
		     std::cout << a << " unary+" << std::endl;
		     return object(1) * a;
	     }}};
	std::map<char, std::function<object(object, object)>> binary_evaluators{
	    {'-',
	     [&](auto a, auto b) {
		     std::cout << a << " - " << b << std::endl;
		     return a - b;
	     }},
	    {'+',
	     [&](auto a, auto b) {
		     std::cout << a << " + " << b << std::endl;
		     return a + b;
	     }},
	    {'*', [&](auto a, auto b) {
		     std::cout << a << " * " << b << std::endl;
		     return a * b;
	     }}};
	void eval_impl(std::stack<operand> &operands, std::stack<char> &operators,
	               size_t min_priority, parse_error err) {
		while (!operands.empty() && !operators.empty() &&
		       ((priority[operators.top()] >= min_priority || min_priority == 0) &&
		        (operators.top() != -'(' || min_priority != 0))) {
			char oprtr = operators.top();
			operators.pop();
			auto oprnd = get(move(operands.top()));
			operands.pop();
			if (oprtr > 0) {
				auto oprnd2 = get(move(operands.top()));
				operands.pop();
				operands.push(
				    binary_evaluators[oprtr](std::move(oprnd2), std::move(oprnd)));
			} else {
				operands.push(unary_evaluators[oprtr](std::move(oprnd)));
			}
		}
		if (operands.empty() || operators.empty() ||
		    (operators.top() != -'(' && min_priority == 0))
			throw err;
		if (min_priority == 0)
			operators.pop();
	}
	size_t eval_expression(size_t start, const std::string &line, object &result) {
		size_t i = start;
		std::stack<operand> operands;
		std::stack<char> operators;
		operators.push(-'(');
		operand temp;
		int state = 0; // 0: operator, 1: operand
		while (i < line.size() && line[i] != ';' && line[i] != ',' &&
		       line[i] != ']') {
			char c = line[i];
			if (unary[c] || binary[c]) {
				if (state == 0 && !unary[c])
					throw parse_error(i, "expression error");
				if (state == 1 && !binary[c])
					throw parse_error(i, "expression error");
				if (c == ')') {
					eval_impl(operands, operators, 0, parse_error(i, "expression error"));
					state = 1;
				} else {
					char oprtr = state == 0 ? -c : c;
					if (c != '(')
						eval_impl(operands, operators, priority[oprtr],
						          parse_error(i, "expression error"));
					operators.push(oprtr);
					state = 0;
				}
				i++;
			} else {
				i = parse_operand(i, line, temp);
				operands.push(move(temp));
				state = 1;
			}
			i = implicit_space(i, line);
		}
		eval_impl(operands, operators, 0, parse_error(i, "expression error"));
		if (operands.size() != 1)
			throw parse_error(i, "expression error");
		result = get(move(operands.top()));
		return i;
	}

public:
	void clear() { temp_vars.clear(); }
	size_t eval(size_t start, const std::string &line, object &ov) {
		size_t i = start;
		i = implicit_space(i, line);
		view_link lvalue;
		i = parse_view_link(i, line, lvalue);
		i = implicit_space(i, line);
		if (i < line.size() && line[i] == '=') {
			i++;
			i = implicit_space(i, line);
			object result;
			i = eval_expression(i, line, result);
			if (i >= line.size() || line[i] != ';')
				throw parse_error(i, ";");
			i++;
			if (lvalue.sl.size() != 0) {
				auto src = get_view(move(operand(lvalue)));
				if (src.size() == 1)
					src[0] = result;
				else
					ops_impl::update(src, result);
			} else {
				vars[lvalue.name] = result;
			}
			ov = std::move(result);
			return i;
		}
		i = implicit_space(i, line);
		if (i >= line.size() || line[i] != ';')
			throw parse_error(i, ";");
		i++;
		ov = get(std::move(lvalue));
		return i;
	}
};
} // namespace matlang

#endif /* end of include guard: PARSER_HPP */
