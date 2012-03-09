#include <iostream>
#include <sstream>
#include <string>
#include "parserlib.hpp"
using namespace std;
using namespace parserlib;


/**** GRAMMAR DECLARATIONS ****/


extern rule exp, add, mul;


rule ws = *expr(' ');


rule digit = range('0', '9');


rule num = +digit >> -('.' >> +digit >> -(set("eE") >> -set("+-") >> +digit));


rule val = num
         | '(' >> exp >> ')';


rule mul_op = '*' >> mul;
rule div_op = '/' >> mul;
rule mul = val >> -(mul_op | div_op);


rule add_op = '+' >> add;
rule sub_op = '-' >> add;
rule add = mul >> -(add_op | sub_op);


rule exp = add;


/**** AST DECLARATIONS ****/


class expr_t : public ast_container {
public:
    virtual double eval() const = 0;
};


class num_t : public expr_t {
public:
    virtual void construct(const pos &b, const pos &e, ast_stack &st) {
        stringstream stream;
        for(input::iterator it = b.m_it; it != e.m_it; ++it) {
            stream << (char)*it;
        }
        stream >> m_value;
    }

    virtual double eval() const {
        return m_value;
    }

private:
    double m_value;
};


class binary_expr_t : public expr_t {
public:
    ast_ptr<expr_t> left, right;
};


class add_t : public binary_expr_t {
public:
    virtual double eval() const {
        return left->eval() + right->eval();
    }
};


class sub_t : public binary_expr_t {
public:
    virtual double eval() const {
        return left->eval() - right->eval();
    }
};


class mul_t : public binary_expr_t {
public:
    virtual double eval() const {
        return left->eval() * right->eval();
    }
};


class div_t_ : public binary_expr_t {
public:
    virtual double eval() const {
        return left->eval() / right->eval();
    }
};


/**** GRAMMAR-AST CONNECTIONS ****/


ast<num_t> ast_num(num);
ast<add_t> ast_add(add_op);
ast<sub_t> ast_sub(sub_op);
ast<mul_t> ast_mul(mul_op);
ast<div_t_> ast_div(div_op);


/**** MAIN ****/


//main
int main() {
	for(;;) {
		string s;
		cout << "enter a math expression (+-*/, floats, parentheses) or enter to exit:\n";
		getline(cin, s);
		if (s.empty()) break;

		//convert the string to input
		input i(s.begin(), s.end());

		//parse
		error_list el;
		expr_t *r = dynamic_cast<expr_t *>(parse(i, exp, ::ws, el));

		//on success
		if (r) {
			double v = r->eval();
			cout << "success; result = " << v << "\n";
			delete r;
		}

		//on error
		else {
			cout << "errors: \n";
			for(error_list::iterator it = el.begin();
				it != el.end();
				++it)
			{
				error &err = *it;
				cout << "line " << err.m_begin.m_line << ", col " << err.m_begin.m_col << ": ";
				wcout << err.m_msg << endl;
			}
		}

		//next input
		cout << endl;
	}
    return 0;
}