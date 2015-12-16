

#include "executor.h"
#include "environment.h"
#include "statement.h"
#include "evaluator.h"
#include "error.h"
#include "alloc.h"

#include <assert.h>

struct executor_s {
    environment_t env;
};

static executor_result_t __executor_if_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_elif_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_switch_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_for_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_while_statement__(environment_t env, statement_t stmt, bool toplevel);
static executor_result_t __executor_block_statement__(environment_t env, list_t block, bool toplevel);

executor_t executor_new(environment_t env)
{
    executor_t exec = (executor_t) mem_alloc(sizeof(struct executor_s));
    if (!exec) {
        return NULL;
    }

    exec->env = env;

    return exec;
}

void executor_free(executor_t exec)
{
    mem_free(exec);
}

void executor_run(executor_t exec)
{
    list_iter_t   iter;
    environment_t env;
    statements_t  stmts;
    statement_t   stmt;

    env = exec->env;

    assert(!stack_is_empty(env->statement_stack));

    stmts = stack_element(stack_pop(env->statement_stack), statements_t, link);

    list_for_each(stmts->stmts, iter) {
        stmt = list_element(iter, statement_t, link);
        switch (executor_statement(env, stmt, true)) {
        case EXECUTOR_RESULT_BREAK:
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "break outside loop");
            break;
        case EXECUTOR_RESULT_CONTINUE:
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "continue outside loop");
            break;
        case EXECUTOR_RESULT_RETURN:
            runtime_error("(%d, %d): %s", stmt->line, stmt->column, "return outside function");
            break;
        default:
            break;
        }
    }
}

executor_result_t executor_statement(environment_t env, statement_t stmt, bool toplevel)
{
    value_t value;

    switch (stmt->type) {
    case STATEMENT_TYPE_EXPRESSION:
        evaluator_expression(env, stmt->u.expr, toplevel);
        value = list_element(list_rbegin(env->stack), value_t, link);
        list_pop_back(env->stack);
        value_free(value);
        return EXECUTOR_RESULT_NORMAL;

    case STATEMENT_TYPE_IF:
        return __executor_if_statement__(env, stmt, toplevel);

    case STATEMENT_TYPE_SWITCH:
        return __executor_switch_statement__(env, stmt, toplevel);

    case STATEMENT_TYPE_WHILE:
        return __executor_while_statement__(env, stmt, toplevel);

    case STATEMENT_TYPE_FOR:
        return __executor_for_statement__(env, stmt, toplevel);

    case STATEMENT_TYPE_CONTINUE:
        return EXECUTOR_RESULT_CONTINUE;

    case STATEMENT_TYPE_BREAK:
        return EXECUTOR_RESULT_BREAK;

    case STATEMENT_TYPE_RETURN:
        if (stmt->u.return_expr) {
            evaluator_expression(env, stmt->u.return_expr, false);
        } else {
            environment_push_null(env);
        }
        return EXECUTOR_RESULT_RETURN;

    default:
        return EXECUTOR_RESULT_NORMAL;
    }

    return EXECUTOR_RESULT_NORMAL;
}

static executor_result_t __executor_block_statement__(environment_t env, list_t block, bool toplevel)
{
    list_iter_t iter;
    statement_t stmt;
    executor_result_t result;
    
    result = EXECUTOR_RESULT_NORMAL;
    list_for_each(block, iter) {
        stmt = list_element(iter, statement_t, link);
        result = executor_statement(env, stmt, toplevel);
        if (result != EXECUTOR_RESULT_NORMAL) {
            break;
        }
    }

    return result;
}

static executor_result_t __executor_if_statement__(environment_t env, statement_t stmt, bool toplevel)
{
    value_t condition_value;
    statement_if_t stmt_if;
    executor_result_t result = EXECUTOR_RESULT_NORMAL;

    stmt_if = stmt->u.if_stmt;

    evaluator_expression(env, stmt_if->condition, toplevel);

    condition_value = list_element(list_rbegin(env->stack), value_t, link);

    list_erase(env->stack, condition_value->link);

    if (condition_value->type != VALUE_TYPE_BOOL) {
        runtime_error("(%d, %d): %s cannot be converted to bool", 
                      stmt->line, 
                      stmt->column,
                      get_value_type_string(condition_value->type));
    }

    if (condition_value->u.bool_value) {
        result = __executor_block_statement__(env, stmt_if->if_block, toplevel);
    } else {
        result = __executor_elif_statement__(env, stmt, toplevel);
    }

    value_free(condition_value);
    return result;
}

static executor_result_t __executor_elif_statement__(environment_t env, statement_t stmt, bool toplevel)
{
    value_t condition_value;
    bool condition;
    statement_if_t stmt_if;
    statement_elif_t stmt_elif;
    list_iter_t iter;

    stmt_if = stmt->u.if_stmt;

    list_for_each(stmt_if->elifs, iter) {
        stmt_elif = list_element(iter, statement_elif_t, link);

        evaluator_expression(env, stmt_elif->condition, toplevel);

        condition_value = list_element(list_rbegin(env->stack), value_t, link);

        list_erase(env->stack, condition_value->link);

        if (condition_value->type != VALUE_TYPE_BOOL) {
            runtime_error("(%d, %d): %s cannot be converted to bool",
                          stmt->line,
                          stmt->column,
                          get_value_type_string(condition_value->type));
        }

        condition = condition_value->u.bool_value;

        value_free(condition_value);

        if (condition) {
            return __executor_block_statement__(env, stmt_elif->block, toplevel);
        }
    }

    if (!list_is_empty(stmt_if->else_block)) {
        return __executor_block_statement__(env, stmt_if->else_block, toplevel);
    }

    return EXECUTOR_RESULT_NORMAL;
}

static executor_result_t __executor_switch_statement__(environment_t env, statement_t stmt, bool toplevel)
{
    bool compare_result;
    executor_result_t result;
    value_t lvalue;
    value_t rvalue;
    list_iter_t iter;
    value_t compare_value;
    statement_switch_t stmt_switch;
    statement_switch_case_t stmt_case;

    stmt_switch = stmt->u.switch_stmt;

    evaluator_expression(env, stmt_switch->expr, toplevel);

    lvalue = list_element(list_rbegin(env->stack), value_t, link);

    list_for_each(stmt_switch->cases, iter) {
        stmt_case = list_element(iter, statement_switch_case_t, link);

        evaluator_expression(env, stmt_case->case_expr, toplevel);

        rvalue = list_element(list_rbegin(env->stack), value_t, link);

        evaluator_binary_value(env, stmt->line, stmt->column, EXPRESSION_TYPE_EQ, lvalue, rvalue);

        compare_value = list_element(list_rbegin(env->stack), value_t, link);

        list_pop_back(env->stack);  /* pop compare_result */
        list_pop_back(env->stack);  /* pop rvalue */

        compare_result = compare_value->u.bool_value;

        value_free(rvalue);
        value_free(compare_value);

        if (compare_result) {
            list_pop_back(env->stack); /* pop lvalue */

            value_free(lvalue);

            result = __executor_block_statement__(env, stmt_case->block, toplevel);
            if (result == EXECUTOR_RESULT_BREAK) {
                result = EXECUTOR_RESULT_NORMAL;
            }

            return result;
        }
    }

    list_pop_back(env->stack);
    value_free(lvalue);

    result = __executor_block_statement__(env, stmt_switch->default_block, toplevel);
    if (result == EXECUTOR_RESULT_BREAK) {
        result = EXECUTOR_RESULT_NORMAL;
    }

    return result;
}

static executor_result_t __executor_while_statement__(environment_t env, statement_t stmt, bool toplevel)
{
    bool condition;
    value_t condition_value;
    statement_while_t stmt_while;
    executor_result_t result = EXECUTOR_RESULT_NORMAL;

    stmt_while = stmt->u.while_stmt;

    while (true) {
        evaluator_expression(env, stmt_while->condition, toplevel);
        condition_value = list_element(list_rbegin(env->stack), value_t, link);
        list_pop_back(env->stack);

        if (condition_value->type != VALUE_TYPE_BOOL) {
            runtime_error("(%d, %d): %s cannot be converted to bool",
                          stmt->line,
                          stmt->column,
                          get_value_type_string(condition_value->type));
        }
       
        condition = condition_value->u.bool_value;

        value_free(condition_value);

        if (!condition) {
            break;
        }

        result = __executor_block_statement__(env, stmt_while->block, toplevel);
        if (result == EXECUTOR_RESULT_RETURN) {
            break;
        } else if (result == EXECUTOR_RESULT_BREAK) {
            result = EXECUTOR_RESULT_NORMAL;
            break;
        }
    }

    return EXECUTOR_RESULT_NORMAL;
}

static executor_result_t __executor_for_statement__(environment_t env, statement_t stmt, bool toplevel)
{
    value_t value;
    bool condition;
    statement_for_t stmt_for;
    executor_result_t result = EXECUTOR_RESULT_NORMAL;

    stmt_for = stmt->u.for_stmt;

    if (stmt_for->init) {
        evaluator_expression(env, stmt_for->init, toplevel);
        value = list_element(list_rbegin(env->stack), value_t, link);
        list_pop_back(env->stack);
        value_free(value);
    }

    while (true) {
        if (stmt_for->condition) {
            evaluator_expression(env, stmt_for->condition, toplevel);
            value = list_element(list_rbegin(env->stack), value_t, link);
            list_pop_back(env->stack);

            if (value->type != VALUE_TYPE_BOOL) {
                runtime_error("(%d, %d): %s cannot be converted to bool",
                              stmt->line,
                              stmt->column,
                              get_value_type_string(value->type));
            }

            condition = value->u.bool_value;

            value_free(value);

            if (!condition) {
                break;
            }
        }

        result = __executor_block_statement__(env, stmt_for->block, toplevel);
        if (result == EXECUTOR_RESULT_RETURN) {
            break;
        } else if (result == EXECUTOR_RESULT_BREAK) {
            result = EXECUTOR_RESULT_NORMAL;
            break;
        }

        if (stmt_for->post) {
            evaluator_expression(env, stmt_for->post, toplevel);
            
            value = list_element(list_rbegin(env->stack), value_t, link);
            
            list_pop_back(env->stack);

            value_free(value);
        }
    }

    return result;
}