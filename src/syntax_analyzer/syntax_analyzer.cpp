//
// Created by Роман Балдин on 12.11.2024.
//
#include "syntax_analyzer.h"

checker *chc = new checker();

Syntax_analyzer::Syntax_analyzer() {
    analyzer_.get_lexemes();
    semstack_.set_checker(chc);
    get_lex();
    program();
}

bool isType(Lexeme &lex) {
    std::string name = lex.getName();
    if (name == "int" || name == "float" || name == "array" || name == "bool" ||
        name == "float" || name == "char" || name == "void" || name == "string") {
        return true;
    }
    return false;
}

void Syntax_analyzer::get_lex() { lex_ = analyzer_.get_lexeme(); }

Lexeme Syntax_analyzer::peek() { return analyzer_.peek(); }

Lexeme Syntax_analyzer::prev_lex() { return analyzer_.prev_lexeme(); }

void Syntax_analyzer::program() {
    if (lex_.getName() == "main") {
        get_lex();
        main();
    } else {
        while (lex_.getName() != "main") {
            function_definition();
            get_lex();
        }
        get_lex();
        main();
    }
}

void Syntax_analyzer::main() {
    if (lex_.getName() == "{") {
        get_lex();
        block();
    } else {
        throw lex_;
    }
}

void Syntax_analyzer::function_definition() {
    if (lex_.getName() != "function") {
        throw lex_;
    }
    get_lex();
    type();
    get_lex();
    name();
    get_lex();
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();

    chc->createScope();

    parameter_list();
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != "{") {
        throw lex_;
    }
    get_lex();
    block();
}

void Syntax_analyzer::type() {
    if (!isType(lex_)) {
        throw lex_;
    }
}

void Syntax_analyzer::name() {
    if (lex_.getType() != identifier) {
        throw lex_;
    }
}

void Syntax_analyzer::parameter_list() {
    if (lex_.getName() != "empty") {
        parameter();
        while (peek().getName() == ",") {
            get_lex();
            get_lex();
            parameter();
        }
    }
}

void Syntax_analyzer::parameter() {
    type();
    get_lex();
    name();
    chc->pushId(prev_lex().getType(), lex_.getName());
}

void Syntax_analyzer::block() {
    expression_list();
    chc->exitScope();
    get_lex();
    if (lex_.getName() != "}") {
        throw lex_;
    }
}

void Syntax_analyzer::expression_list() {
    all_operators();
    get_lex();
    if (lex_.getName() != ";") {
        throw lex_;
    }
    while (peek().getName() != "}") {
        get_lex();
        all_operators();
        get_lex();
        if (lex_.getName() != ";") {
            throw lex_;
        }
    }
}

void Syntax_analyzer::all_operators() {
    if (lex_.getName() == "for") {
        get_lex();
        for_operator();
    } else if (lex_.getName() == "while") {
        get_lex();
        while_operator();
    } else if (lex_.getName() == "if") {
        get_lex();
        if_conditional_statement();
    } else if (lex_.getName() == "switch") {
        get_lex();
        switch_conditional_statement();
    } else if (lex_.getName() == "return") {
        get_lex();
        expression();
    } else if (lex_.getName() == "print") {
        get_lex();
        output_operator();
    } else if (lex_.getName() == "break") {
        get_lex();
    } else if (lex_.getName() == "continue") {
        get_lex();
    } else if (lex_.getName() == "empty") {
    } else if (isType(lex_)) {
        initialization();
    } else {
        expression();
    }
}

void Syntax_analyzer::expression() { simple_expression(); }

void Syntax_analyzer::output_operator() {
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();
    function_args();
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
}

void Syntax_analyzer::while_operator() {
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();
    expression();
    semstack_.checkBool();
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != "{") {
        throw lex_;
    }
    get_lex();
    chc->createScope();
    block();
}

void Syntax_analyzer::simple_expression() { comma_expression(); }

void Syntax_analyzer::comma_expression() {
    assignment_operator();

    while (peek().getName() == ",") {
        get_lex();
        semstack_.push(lex_);
        get_lex();
        assignment_operator();
        semstack_.checkBin();
    }
}

void Syntax_analyzer::assignment_operator() {
    logical_or_expression();
    while (peek().getName() == "=") {
        get_lex();
        semstack_.push(lex_);
        get_lex();
        logical_or_expression();
        semstack_.checkBin();
    }
}

void Syntax_analyzer::logical_or_expression() {
    logical_and_expression();
    while (peek().getName() == "or" || peek().getName() == "||") {
        get_lex();
        semstack_.push(lex_);
        get_lex();
        logical_and_expression();
        semstack_.checkBin();
    }
}

void Syntax_analyzer::logical_and_expression() {
    relational_expression();
    while (peek().getName() == "and" || peek().getName() == "&&") {
        get_lex();
        semstack_.push(lex_);
        get_lex();
        relational_expression();
        semstack_.checkBin();
    }
}

void Syntax_analyzer::relational_expression() {
    additive_expression();
    while (peek().getName() == "==" || peek().getName() == "!=" ||
           peek().getName() == "<" || peek().getName() == ">" ||
           peek().getName() == "<=" || peek().getName() == ">=") {
        get_lex();
        semstack_.push(lex_);
        get_lex();
        additive_expression();
        semstack_.checkBin();
    }
}

void Syntax_analyzer::additive_expression() {
    multiplicative_expression();
    while (peek().getName() == "+" || peek().getName() == "-") {
        get_lex();
        semstack_.push(lex_);
        get_lex();
        multiplicative_expression();
        semstack_.checkBin();
    }
}

void Syntax_analyzer::multiplicative_expression() {
    unary_expression();
    while (peek().getName() == "*" || peek().getName() == "/") {
        get_lex();
        semstack_.push(lex_);
        get_lex();
        unary_expression();
        semstack_.checkBin();
    }
}

void Syntax_analyzer::unary_expression() {
    if (lex_.getName() == "+" || lex_.getName() == "-" ||
        lex_.getName() == "++" || lex_.getName() == "--" ||
        lex_.getName() == "not") {
        semstack_.push(lex_);
        get_lex();
        unary_expression();
        semstack_.checkUno();
    } else {
        primary_expression();
    }
}

void Syntax_analyzer::primary_expression() {
    if (lex_.getName() == "(") {
        get_lex();
        expression();
        get_lex();
        if (lex_.getName() != ")") {
            throw lex_;
        }
    } else if (lex_.getType() == identifier) {
        if (peek().getName() == "[") {
            get_lex();
            array_access();
        } else if (peek().getName() == "(") {
            get_lex();
            function_call();
        } else {
            semstack_.push(lex_);
        }
    } else if (lex_.getType() == string) {
        Lexeme lex("string", string);
        semstack_.push(lex);
    } else if (lex_.getName() == "true" || lex_.getName() == "false") {
        Lexeme lex("bool", booll);
        semstack_.push(lex);
    } else if (lex_.getType() == intt) {
        Lexeme lex("intt", intt);
        semstack_.push(lex);
    } else if (lex_.getType() == floatt) {
        Lexeme lex("floatt", floatt);
        semstack_.push(lex);
    } else if (lex_.getType() == lexemeType::literal) {
        Lexeme lex("literal", lexemeType::literal);
        semstack_.push(lex);
    } else {
        throw lex_;
    }
}

void Syntax_analyzer::array_access() {
    if (lex_.getName() != "[") {
        throw lex_;
    }
    get_lex();
    expression();
    get_lex();
    if (lex_.getName() != "]") {
        throw lex_;
    }
}

void Syntax_analyzer::literal() {
    if (lex_.getType() != lexemeType::literal) {
        throw lex_;
    }
}

void Syntax_analyzer::initialization() {
    if (lex_.getType() == identifier) {
        semstack_.push(lex_);
        get_lex();
        if (lex_.getType() != assignmentOperators) {
            throw lex_;
        }
        semstack_.push(Lexeme("=", assignmentOperators));
        expression();
        semstack_.checkBin();
    } else {
        type();
        if (lex_.getName() == "array") {
            get_lex();
            if (lex_.getType() != identifier) {
                throw lex_;
            }
            chc->pushId(array, lex_.getName());
            get_lex();
            if (lex_.getName() != "[") {
                throw lex_;
            }
            get_lex();
            if (lex_.getType() != intt) {
                throw lex_;
            }
            get_lex();
            if (lex_.getName() != "]") {
                throw lex_;
            }
        } else {
            get_lex();
            if (lex_.getType() != identifier) {
                throw lex_;
            }
            lexemeType tmp;
            std::string tmp_name = lex_.getName();
            if (prev_lex().getName() == "int") {
                tmp = intt;
                // chc->pushId(intt, lex_.getName());
            } else if (prev_lex().getName() == "float") {
                tmp = floatt;
                // chc->pushId(floatt, lex_.getName());
            } else if (prev_lex().getName() == "string") {
                tmp = string;
                // chc->pushId(string, lex_.getName());
            } else if (prev_lex().getName() == "bool") {
                tmp = booll;
                // chc->pushId(booll, lex_.getName());
            } else if (prev_lex().getName() == "char") {
                tmp = charr;
                // chc->pushId(charr, lex_.getName());
            }
            semstack_.push(lex_);
            get_lex();
            if (lex_.getName() != "=") {
                throw lex_;
            }
            semstack_.push(Lexeme("=", assignmentOperators));
            get_lex();
            expression();
            chc->pushId(tmp, tmp_name);
            semstack_.checkBin();
            semstack_.clear();
        }
    }
}

void Syntax_analyzer::if_conditional_statement() {
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();
    expression();
    semstack_.checkBool();
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != "{") {
        throw lex_;
    }
    get_lex();
    chc->createScope();
    block();
    if (peek().getName() == "else") {
        get_lex();
        get_lex();
        if (lex_.getName() != "{") {
            throw lex_;
        }
        get_lex();
        chc->createScope();
        block();
    }
}

void Syntax_analyzer::switch_conditional_statement() {
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();
    if (lex_.getType() != identifier && lex_.getType() != lexemeType::literal) {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != "{") {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != "case") {
        throw lex_;
    }
    get_lex();
    case_block();
    get_lex();
    while (lex_.getName() == "case") {
        get_lex();
        case_block();
        get_lex();
    }
}

void Syntax_analyzer::case_block() {
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();
    simple_expression();
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != "{") {
        throw lex_;
    }
    get_lex();
    chc->createScope();
    block();
}

void Syntax_analyzer::for_operator() {
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();
    chc->createScope();
    expression();
    semstack_.clear();
    get_lex();
    if (lex_.getName() != ";") {
        throw lex_;
    }
    get_lex();
    expression();
    semstack_.checkBool();
    get_lex();
    if (lex_.getName() != ";") {
        throw lex_;
    }
    get_lex();
    expression();
    semstack_.clear();
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
    get_lex();
    if (lex_.getName() != "{") {
        throw lex_;
    }
    get_lex();
    block();
}

void Syntax_analyzer::function_call() {
    if (lex_.getName() != "(") {
        throw lex_;
    }
    get_lex();
    function_args();
    get_lex();
    if (lex_.getName() != ")") {
        throw lex_;
    }
}

void Syntax_analyzer::function_args() {
    if (lex_.getName() == "empty") {
        return;
    }
    while (peek().getName() != ")") {
        if (lex_.getType() != lexemeType::literal && lex_.getType() != identifier) {
            throw lex_;
        }
        get_lex();
        if (lex_.getType() != comma) {
            throw lex_;
        }
        get_lex();
    }
    if (lex_.getType() != lexemeType::literal && lex_.getType() != identifier) {
        throw lex_;
    }
}
