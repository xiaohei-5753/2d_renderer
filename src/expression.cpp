#include "../include/expression.h"
#include <cmath>
#include <cctype>
#include <sstream>
#include <iostream>

using namespace std;

static bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '^';
}

static int opPrecedence(char op) {
    switch (op) {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        case '^': return 3;
        default: return 0;
    }
}

void ExpressionParser::skipWhitespace(const string& expr, int& pos) {
    while (pos < expr.size() && isspace(expr[pos])) pos++;
}

unique_ptr<Expression> ExpressionParser::parsePrimary(const string& expr, int& pos) {
    skipWhitespace(expr, pos);
    if (pos >= expr.size()) return nullptr;
    
    if (expr[pos] == 'x' || expr[pos] == 'y') {
        char var = expr[pos];
        pos++;
        return make_unique<VariableExpr>(var);
    }
    
    // Parse number
    if (isdigit(expr[pos]) || expr[pos] == '.' || expr[pos] == '-') {
        size_t end;
        double value = stod(expr.substr(pos), &end);
        pos += end;
        return make_unique<ConstantExpr>(value);
    }
    
    // TODO: handle parentheses? Not in original.
    return nullptr;
}

unique_ptr<Expression> ExpressionParser::parseFactor(const string& expr, int& pos) {
    skipWhitespace(expr, pos);
    unique_ptr<Expression> left = parsePrimary(expr, pos);
    if (!left) return nullptr;
    
    skipWhitespace(expr, pos);
    while (pos < expr.size() && expr[pos] == '^') {
        char op = expr[pos];
        pos++;
        unique_ptr<Expression> right = parseFactor(expr, pos);
        if (!right) return left;
        left = make_unique<BinaryExpr>(op, move(left), move(right));
        skipWhitespace(expr, pos);
    }
    
    return left;
}

unique_ptr<Expression> ExpressionParser::parseTerm(const string& expr, int& pos) {
    unique_ptr<Expression> left = parseFactor(expr, pos);
    if (!left) return nullptr;
    
    skipWhitespace(expr, pos);
    while (pos < expr.size() && (expr[pos] == '*' || expr[pos] == '/')) {
        char op = expr[pos];
        pos++;
        unique_ptr<Expression> right = parseFactor(expr, pos);
        if (!right) return left;
        left = make_unique<BinaryExpr>(op, move(left), move(right));
        skipWhitespace(expr, pos);
    }
    
    return left;
}

unique_ptr<Expression> ExpressionParser::parseExpression(const string& expr, int& pos) {
    unique_ptr<Expression> left = parseTerm(expr, pos);
    if (!left) return nullptr;
    
    skipWhitespace(expr, pos);
    while (pos < expr.size() && (expr[pos] == '+' || expr[pos] == '-')) {
        char op = expr[pos];
        pos++;
        unique_ptr<Expression> right = parseTerm(expr, pos);
        if (!right) return left;
        left = make_unique<BinaryExpr>(op, move(left), move(right));
        skipWhitespace(expr, pos);
    }
    
    return left;
}

unique_ptr<Expression> ExpressionParser::parse(const string& expr) {
    int pos = 0;
    unique_ptr<Expression> result = parseExpression(expr, pos);
    skipWhitespace(expr, pos);
    if (pos < expr.size()) {
        cerr << "ExpressionParser: extra characters after expression: " << expr.substr(pos) << endl;
    }
    return result;
}

vector<unique_ptr<Expression>> ExpressionParser::parsePenSpec(const string& penSpec) {
    vector<unique_ptr<Expression>> result;
    istringstream ss(penSpec);
    string token;
    
    bool isExpression = false;
    if (!penSpec.empty() && penSpec[0] == '~') {
        isExpression = true;
        ss >> token; // consume "~"
    }
    
    for (int i = 0; i < 7; i++) {
        if (!(ss >> token)) {
            cerr << "Pen specification has less than 7 components" << endl;
            // Fill with zero
            result.push_back(make_unique<ConstantExpr>(0.0));
            continue;
        }
        
        if (isExpression) {
            result.push_back(parse(token));
        } else {
            // Constant value 0..255
            double value = stod(token);
            result.push_back(make_unique<ConstantExpr>(value / 255.0));
        }
    }
    
    return result;
}

PenResult evaluatePen(const string& penSpec, const Point2D& point) {
    auto exprs = ExpressionParser::parsePenSpec(penSpec);
    PenResult result;
    
    if (exprs.size() >= 7) {
        result.r = exprs[0]->evaluate(point);
        result.g = exprs[1]->evaluate(point);
        result.b = exprs[2]->evaluate(point);
        result.a = exprs[3]->evaluate(point);
        result.lr = exprs[4]->evaluate(point);
        result.lg = exprs[5]->evaluate(point);
        result.lb = exprs[6]->evaluate(point);
        
        // Clamp to [0,1] as in original
        result.r = max(0.0f, min(1.0f, result.r));
        result.g = max(0.0f, min(1.0f, result.g));
        result.b = max(0.0f, min(1.0f, result.b));
        result.a = max(0.0f, min(1.0f, result.a));
        result.lr = max(0.0f, min(1.0f, result.lr));
        result.lg = max(0.0f, min(1.0f, result.lg));
        result.lb = max(0.0f, min(1.0f, result.lb));
    } else {
        // Default black transparent
        result = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    }
    
    return result;
}