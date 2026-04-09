#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cmath>

struct Point2D {
    double x, y;
    Point2D(double x = 0.0, double y = 0.0) : x(x), y(y) {}
};

class Expression {
public:
    virtual ~Expression() = default;
    virtual double evaluate(const Point2D& p) const = 0;
    virtual std::string toString() const = 0;
};

class ConstantExpr : public Expression {
    double value;
public:
    ConstantExpr(double v) : value(v) {}
    double evaluate(const Point2D& p) const override { return value; }
    std::string toString() const override { return std::to_string(value); }
};

class VariableExpr : public Expression {
    char varName; // 'x' or 'y'
public:
    VariableExpr(char name) : varName(name) {}
    double evaluate(const Point2D& p) const override {
        if (varName == 'x') return p.x;
        if (varName == 'y') return p.y;
        return 0.0;
    }
    std::string toString() const override { return std::string(1, varName); }
};

class BinaryExpr : public Expression {
    char op; // '+', '-', '*', '/', '^'
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
public:
    BinaryExpr(char op, std::unique_ptr<Expression> l, std::unique_ptr<Expression> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}
    
    double evaluate(const Point2D& p) const override {
        double lv = left->evaluate(p);
        double rv = right->evaluate(p);
        switch (op) {
            case '+': return lv + rv;
            case '-': return lv - rv;
            case '*': return lv * rv;
            case '/': return lv / rv;
            case '^': return std::pow(lv, rv);
            default: return 0.0;
        }
    }
    
    std::string toString() const override {
        return "(" + left->toString() + " " + op + " " + right->toString() + ")";
    }
};

class ExpressionParser {
public:
    // Parse a single expression string (e.g., "x^2 + y*0.5")
    static std::unique_ptr<Expression> parse(const std::string& expr);
    
    // Parse a pen specification string (7 space-separated expressions or values)
    // Returns vector of 7 expressions. If first token is "~", treat as expressions.
    static std::vector<std::unique_ptr<Expression>> parsePenSpec(const std::string& penSpec);
    
private:
    static std::unique_ptr<Expression> parseExpression(const std::string& expr, int& pos);
    static std::unique_ptr<Expression> parseTerm(const std::string& expr, int& pos);
    static std::unique_ptr<Expression> parseFactor(const std::string& expr, int& pos);
    static std::unique_ptr<Expression> parsePrimary(const std::string& expr, int& pos);
    static void skipWhitespace(const std::string& expr, int& pos);
};

// Helper function to evaluate a pen specification at a given point
// Returns Pixel values (r,g,b,a,lr,lg,lb) in range [0,1]
struct PenResult {
    float r, g, b, a;
    float lr, lg, lb;
};

PenResult evaluatePen(const std::string& penSpec, const Point2D& point);