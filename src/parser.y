%token TOKEN_LP TOKEN_RP TOKEN_LC TOKEN_RC TOKEN_LB TOKEN_RB TOKEN_COLON TOKEN_SEMICOLON TOKEN_COMMA TOKEN_AND TOKEN_OR TOKEN_EQ TOKEN_NEQ TOKEN_LT TOKEN_LEQ TOKEN_GT TOKEN_GEQ TOKEN_ADD TOKEN_SUB TOKEN_MUL TOKEN_DIV TOKEN_MOD TOKEN_ASSIGN TOKEN_ADD_ASSIGN TOKEN_SUB_ASSIGN TOKEN_MUL_ASSIGN TOKEN_DIV_ASSIGN TOKEN_MOD_ASSIGN TOKEN_BITAND_ASSIGN TOKEN_BITOR_ASSIGN TOKEN_XOR_ASSIGN TOKEN_LEFT_SHIFT_ASSIGN TOKEN_RIGHT_SHIFT_ASSIGN TOKEN_LOGIC_RIGHT_SHIFT_ASSIGN TOKEN_BITAND TOKEN_BITOR TOKEN_XOR TOKEN_LEFT_SHIFT TOKEN_RIGHT_SHIFT TOKEN_LOGIC_RIGHT_SHIFT TOKEN_ARRAY_PUSH TOKEN_ARRAY_POP TOKEN_CPL TOKEN_INC TOKEN_DEC TOKEN_NOT TOKEN_DOT TOKEN_IGNORE TOKEN_LITERAL_CHAR TOKEN_LITERAL_INT TOKEN_LITERAL_LONG TOKEN_LITERAL_FLOAT TOKEN_LITERAL_DOUBLE TOKEN_LITERAL_STRING TOKEN_REQUIRE TOKEN_CONST TOKEN_FUNCTION TOKEN_IF TOKEN_ELSE TOKEN_ELIF TOKEN_WHILE TOKEN_FOR TOKEN_SWITCH TOKEN_CASE TOKEN_RETURN TOKEN_BREAK TOKEN_CONTINUE TOKEN_NULL TOKEN_TRUE TOKEN_FALSE TOKEN_IDENTIFIER 

%start translation_unit

%%

translation_unit
    : postfix_expression
    ;
    
expression
    : assign_expression
    ;
    
assign_expression
    : lvalue_expression_list TOKEN_ASSIGN rvalue_expression_list
    | lvalue_expression TOKEN_ADD_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_SUB_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_MUL_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_DIV_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_MOD_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_BITAND_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_BITOR_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_XOR_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_LEFT_SHIFT_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_RIGHT_SHIFT_ASSIGN rvalue_expression
    | lvalue_expression TOKEN_LOGIC_RIGHT_SHIFT_ASSIGN rvalue_expression
    ;
    
lvalue_expression
    : logical_or_expression
    {
        /* check lvalue expression */
    }
    | TOKEN_IGNORE
    ;
    
rvalue_expression
    : logical_or_expression
    ;
    
logical_or_expression
    : logical_and_expression
    | logical_or_expression TOKEN_OR logical_and_expression
    ;

logical_and_expression
    : equality_expression
    | logical_and_expression TOKEN_AND equality_expression
    ;
    
equality_expression
    : relational_expression
    | equality_expression TOKEN_EQ relational_expression
    | equality_expression TOKEN_NEQ relational_expression
    ;
    
relational_expression
    : additive_expression
    | relational_expression TOKEN_GT additive_expression
    | relational_expression TOKEN_GEQ additive_expression
    | relational_expression TOKEN_LT additive_expression
    | relational_expression TOKEN_LEQ additive_expression
    ;
    
additive_expression
    : multiplicative_expression
    | additive_expression TOKEN_ADD multiplicative_expression
    | additive_expression TOKEN_SUB multiplicative_expression
    ;
    
multiplicative_expression
    : bitop_expression
    | multiplicative_expression TOKEN_MUL bitop_expression
    | multiplicative_expression TOKEN_DIV bitop_expression
    | multiplicative_expression TOKEN_MOD bitop_expression
    ;
    
bitop_expression
    : shift_bitop_expression
    | bitop_expression TOKEN_BITAND shift_bitop_expression
    | bitop_expression TOKEN_BITOR shift_bitop_expression
    | bitop_expression TOKEN_XOR shift_bitop_expression
    ;
    
shift_bitop_expression
    : unary_expression
    | shift_bitop_expression TOKEN_LEFT_SHIFT unary_expression
    | shift_bitop_expression TOKEN_RIGHT_SHIFT unary_expression
    | shift_bitop_expression TOKEN_LOGIC_RIGHT_SHIFT unary_expression
    ;
    
unary_expression
    : postfix_expression
    | TOKEN_ADD unary_expression
    | TOKEN_SUB unary_expression
    | TOKEN_NOT unary_expression
    | TOKEN_CPL unary_expression
    ;
    
postfix_expression
    : primary_expression
    | postfix_expression TOKEN_LB expression TOKEN_RB
    | postfix_expression TOKEN_ARRAY_PUSH expression
    | postfix_expression TOKEN_ARRAY_POP expression
    | postfix_expression TOKEN_DOT TOKEN_IDENTIFIER
    | postfix_expression TOKEN_LP argument_list TOKEN_RP
    | postfix_expression TOKEN_INC
    | postfix_expression TOKEN_DEC
    ;

primary_expression
    : TOKEN_NULL 
    | TOKEN_TRUE
    | TOKEN_FALSE
    | TOKEN_LITERAL_CHAR
    | TOKEN_LITERAL_INT
    | TOKEN_LITERAL_FLOAT
    | TOKEN_LITERAL_DOUBLE
    | TOKEN_LITERAL_STRING
    | TOKEN_IDENTIFIER
    | TOKEN_LP expression TOKEN_RP
    | function_definition
    | array_generate_expression
    | table_generate_expression
    ;

function_definition
    : TOKEN_FUNCTION TOKEN_LP TOKEN_RP block
    | TOKEN_FUNCTION TOKEN_LP parameter_list TOKEN_RP block
    | TOKEN_FUNCTION TOKEN_IDENTIFIER TOKEN_LP TOKEN_RP block
    | TOKEN_FUNCTION TOKEN_IDENTIFIER TOKEN_LP parameter_list TOKEN_RP block
    ;
    
parameter_list
    : TOKEN_IDENTIFIER
    | parameter_list TOKEN_COMMA TOKEN_IDENTIFIER
    ;
    
argument_list
    : expression
    | argument_list TOKEN_COMMA expression
    ;
    
array_generate_expression
    : TOKEN_LB TOKEN_RB
    | TOKEN_LB expression_list TOKEN_RB
    ;

expression_list
    : expression
    | expression_list TOKEN_COMMA expression
    ;
    
table_generate_expression
    : TOKEN_LC TOKEN_RC
    | TOKEN_LC table_pair_list TOKEN_RC
    ;
    
table_pair_list
    : table_pair
    | table_pair_list TOKEN_COMMA table_pair
    ;
    
table_pair 
    : TOKEN_IDENTIFIER TOKEN_COLON expression
    ;
    
block 
    :
    ;

%%
