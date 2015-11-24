

#include "statement.h"
#include "alloc.h"

statement_t __statement_new__(statement_type_t type, long line, long column)
{
    statement_t stmt = (statement_t) mem_alloc(sizeof(struct statement_s));
    if (!stmt) {
        return NULL;
    }

    stmt->type   = type;
    stmt->line   = line;
    stmt->column = column;

    return stmt;
}

statement_t statement_new_require(long line, long column, cstring_t package_name)
{
    statement_t stmt = __statement_new__(STATEMENT_TYPE_REQUIRE, line, column);

    stmt->u.package_name = package_name;

    return stmt;
}

statement_t statement_new_expression(long line, long column, expression_t expr)
{
    statement_t stmt = __statement_new__(STATEMENT_TYPE_EXPRESSION, line, column);

    stmt->u.expr = expr;

    return stmt;
}

void statement_free(statement_t stmt)
{
    switch (stmt->type) {
    case STATEMENT_TYPE_REQUIRE:
        cstring_free(stmt->u.package_name);
        break;
    case STATEMENT_TYPE_EXPRESSION:
        expression_free(stmt->u.expr);
        break;
    default:
        break;
    }

    mem_free(stmt);
}