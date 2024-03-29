#ifndef SCRIPT_ENUMS_H
#define SCRIPT_ENUMS_H

extern int errors;
extern int warnings;

enum OpCode {
	// Everything between ASSIGN and PUSH are solely data read/write statements.
	ASSIGN,
	POP,
	// PUSH and everything above it take more complicated params.
	PUSH_THIS,
	POP_THIS,
	PUSH,
	OBJECT_ASSIGN,
	//OBJECT_PUSH,
	//OBJECT_POP,
	OBJECT_PEEK,
	ELEMENT_ASSIGN,
	ELEMENT_PEEK,
	ELEMENT_POP,
	//ELEMENT_PUSH,
	//ELEMENT_POKE,
	//PUSHL,
	//PUSHG,
	NTEST,
	TEST,
	ADD,
	SUB,
	MUL,
	DIV,
	DSIZE,
	MOD,
	NEG,
	NOT,
	OR,
	XOR,
	AND,
	LSHIFT,
	RSHIFT,
	CALL_WAIT,
	CALL_C,
	CALL,
	CALL_DLL,
	REF_CALL,
	REF_OBJECT_CALL,
	CALL_OBJECT,
	DISCARD,
	RET,
	NOP,
	//PUSH_STRING,
	CONCAT_STRING,
	APPEND,
	DUPE,
	SWAP,
	MAKE_LIST,
	//PUSH_EMPTY_LIST,
	LIST_ADD_ELEMENT,
	/*COMPARE_E,
	COMPARE_G,
	COMPARE_GE,
	COMPARE_L,
	COMPARE_LE,
	COMPARE_NE,
	//*/
	MAKE_COPY,
	COMPARE,
	COMPARE_S,
	COMPARE_SCI,
	SET_E,
	SET_G,
	SET_GE,
	SET_L,
	SET_LE,
	SET_NE,

	JMP_E,
	JMP_G,
	JMP_GE,
	JMP_L,
	JMP_LE,
	JMP_NE,

	PUSH_EMPTY_DICT,
	DICT_CREATE,

	//MAKE_RAW_OBJECT,
	MAKE_OBJECT,

	JMP,
	//PUSH_INT24,
	//RESIZE_STACK,
	// All 64 op codes are followed by a 64-bit value.
	//PUSH_INT64,
	//PUSH_DOUBLE64,
	FUNCTION64,
	// Everything below this isn't a real op code
	JMPDEST,
};

#define MAX_MINI_VALUE ( 0x1FFFF)
#define MIN_MINI_VALUE (-0x20000)

#define MAX_MID_VALUE ( (__int64)0x7FFFFFFF)
#define MIN_MID_VALUE (-(__int64)0x80000000)

// Not an enum, but nice to have here, too.
struct Op {
	union {
		//OpCode opCode:8;
		struct {
			unsigned int op:8;
			int data:24;
		};
		struct {
			unsigned int op:8;
			int value:18;
			unsigned int size:2;
			// 0
			// 1
			// 2
			// 3
			unsigned int type:4;
#define OP_NULL		0
#define OP_INT		1
#define OP_FLOAT	2
#define OP_STRING	3
#define OP_LOCAL	4
#define OP_GLOBAL	5
#define OP_OBJECT	6
#define OP_THIS_OBJECT 7
#define OP_ELEMENT	8
			// 0 - null
			// 1 - int
			// 2 - double
			// 3 - local
			// 4 - global
			// 5 - element
			// 6 - object
		} src;
		struct {
			unsigned int op:8;
			unsigned int function:15;
			unsigned int discard:1;
			unsigned int list:8;
		} call;
		struct {
			unsigned short low, high;
		};
	};
};

inline int OpSize(Op op) {
	if (op.op <= PUSH) return op.src.size;
	if (op.op < FUNCTION64) return 1;
	if (op.op > FUNCTION64) return 0;
	if (op.op == FUNCTION64) return 2;
	return 3;
}

inline int OpSize(OpCode op, int size) {
	if (op <= PUSH || (op >= ADD && op <= DSIZE) || op == COMPARE) {
		if (!size) return 1;
		return size;
	}
	if (op < FUNCTION64) return 1;
	if (op > FUNCTION64) return 0;
	if (op == FUNCTION64) return 2;
	return 3;
}




enum NodeType {
	NODE_NONE,
	NODE_LEAF,
	NODE_EMPTY,
	NODE_STATEMENT_LIST,
	NODE_STATEMENT,
	NODE_IF,
	NODE_IF_ELSE,
	NODE_EXPRESSION,
	NODE_SIMPLE_EXPRESSION,
	NODE_INT,
	NODE_DOUBLE,
	NODE_STRING,
	NODE_CONCAT_STRING,
	NODE_RETURN,

	NODE_DLL32,
	NODE_DLL64,
	NODE_DLL_FUNCTION,
	NODE_DLL_FUNCTION_SIMPLE,
	NODE_DLL_ARGS,
	NODE_DLL_ARG,

	NODE_DISCARD_FIRST,

	NODE_EMPTY_DICT,
	NODE_DICT_CREATE,
	//NODE_DICT_MERGE,

	NODE_LIST_ADD,
	NODE_LIST_ADD_NULL,
	NODE_LIST_CREATE_NULL,
	NODE_LIST_CREATE,
	NODE_EMPTY_LIST,
	NODE_SIZE,
	NODE_REF_CALL,
	NODE_STRUCT_REF_CALL,
	NODE_APPEND,

	NODE_CONTINUE,
	NODE_BREAK,

	NODE_NOT,
	NODE_NEG,
	NODE_BIN_NEG,

	NODE_E,
	NODE_G,
	NODE_GE,
	NODE_L,
	NODE_LE,
	NODE_NE,

	NODE_S_E,
	NODE_S_G,
	NODE_S_GE,
	NODE_S_L,
	NODE_S_LE,
	NODE_S_NE,

	NODE_SCI_E,
	NODE_SCI_G,
	NODE_SCI_GE,
	NODE_SCI_L,
	NODE_SCI_LE,
	NODE_SCI_NE,

	NODE_FUNCTION_ALIAS,

	//NODE_IDENTIFIER,
	//NODE_GLOBAL_DEREFERENCE,
	//NODE_GLOBAL_REFERENCE,
	NODE_GLOBAL_REFERENCE,
	NODE_DEREFERENCE,
	NODE_SUB_REFERENCE,
	NODE_LOCAL_REFERENCE,
	NODE_ELEMENT_ASSIGN,

	NODE_SIMPLE_ASSIGN,

	NODE_MUL_SIMPLE_ASSIGN,
	NODE_DIV_SIMPLE_ASSIGN,
	NODE_MOD_SIMPLE_ASSIGN,
	NODE_ADD_SIMPLE_ASSIGN,
	NODE_CONCAT_SIMPLE_ASSIGN,
	NODE_SUB_SIMPLE_ASSIGN,
	NODE_AND_SIMPLE_ASSIGN,
	NODE_XOR_SIMPLE_ASSIGN,
	NODE_OR_SIMPLE_ASSIGN,
	NODE_LSHIFT_SIMPLE_ASSIGN,
	NODE_RSHIFT_SIMPLE_ASSIGN,

	NODE_MUL_ELEMENT_ASSIGN,
	NODE_DIV_ELEMENT_ASSIGN,
	NODE_MOD_ELEMENT_ASSIGN,
	NODE_ADD_ELEMENT_ASSIGN,
	NODE_CONCAT_ELEMENT_ASSIGN,
	NODE_SUB_ELEMENT_ASSIGN,
	NODE_AND_ELEMENT_ASSIGN,
	NODE_XOR_ELEMENT_ASSIGN,
	NODE_OR_ELEMENT_ASSIGN,
	NODE_LSHIFT_ELEMENT_ASSIGN,
	NODE_RSHIFT_ELEMENT_ASSIGN,

	NODE_MAKE_COPY,

	NODE_BOOL_OR,
	NODE_BOOL_AND,

	NODE_ADD,
	NODE_SUB,
	NODE_MUL,
	NODE_DIV,
	NODE_MOD,
	NODE_AND,
	NODE_LSHIFT,
	NODE_RSHIFT,
	NODE_XOR,
	NODE_OR,
	NODE_FUNCTION_CALL,
	NODE_DISCARD_LAST,
	NODE_FUNCTION,
	NODE_FUNCTION_LIST,
	NODE_IDENTIFIER_LIST,
	NODE_FOR,
	NODE_WHILE,
	NODE_DO_WHILE,
	NODE_SIMPLE_PRE_INC,
	NODE_SIMPLE_PRE_DEC,
	NODE_SIMPLE_POST_INC,
	NODE_SIMPLE_POST_DEC,
	NODE_ELEMENT_PRE_INC,
	NODE_ELEMENT_PRE_DEC,
	NODE_ELEMENT_POST_INC,
	NODE_ELEMENT_POST_DEC,

	/*NODE_RAW_STRUCT,
	NODE_RAW_STRUCT_VALUE_LIST,
	NODE_INT_TYPE,
	NODE_DOUBLE_TYPE,
	NODE_FLOAT_TYPE,
	NODE_STRING_TYPE,
	NODE_CHAR_TYPE,
	//*/

	NODE_STRUCT_FUNCTION_CALL,
	NODE_STRUCT_THIS_FUNCTION_CALL,
	NODE_STRUCT_LIST,
	NODE_STRUCT_VALUES,
	NODE_STRUCT,
	NODE_STRUCT_REFERENCE,
	NODE_STRUCT_THIS_REFERENCE,
	NODE_IGNORE,
};




enum ScriptTokenType {
	//ZERO,ONE,STAR, PLUS,DOLLAR,S,E,B,
	// Values I want to keep, as they have data.  Others I can throw away.
	TOKEN_STRING,
	TOKEN_INT,
	TOKEN_DOUBLE,
	TOKEN_IDENTIFIER,
	// Values I dump.
	TOKEN_CONCAT,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_NEGATION,
	TOKEN_ASSIGN,
	TOKEN_FOR,

	/*TOKEN_RAW,
	TOKEN_INT_TYPE,
	TOKEN_DOUBLE_TYPE,
	TOKEN_FLOAT_TYPE,
	TOKEN_STRING_TYPE,
	TOKEN_CHAR_TYPE,
	//*/

	TOKEN_VAR,

	TOKEN_STRUCT,
	TOKEN_PERIOD,

	TOKEN_TILDE,

	TOKEN_E,
	TOKEN_G,
	TOKEN_GE,
	TOKEN_L,
	TOKEN_LE,
	TOKEN_NE,

	TOKEN_SCIE,
	TOKEN_SCIG,
	TOKEN_SCIGE,
	TOKEN_SCIL,
	TOKEN_SCILE,
	TOKEN_SCINE,

	TOKEN_SE,
	TOKEN_SG,
	TOKEN_SGE,
	TOKEN_SL,
	TOKEN_SLE,
	TOKEN_SNE,

	TOKEN_NOT,
	TOKEN_NEG,

	TOKEN_COLON,
	TOKEN_DOUBLE_COLON,

	TOKEN_ALIAS,

	TOKEN_INC,
	TOKEN_DEC,

	TOKEN_BOOL_AND,
	TOKEN_BOOL_OR,
	TOKEN_AND,
	TOKEN_XOR,
	TOKEN_OR,
	TOKEN_MOD,
	TOKEN_DO,
	TOKEN_ADD,
	TOKEN_SUB,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_LSHIFT,
	TOKEN_RSHIFT,

	TOKEN_ANDE,
	TOKEN_XORE,
	TOKEN_ORE,
	TOKEN_MODE,
	TOKEN_DOE,
	TOKEN_ADDE,
	TOKEN_SUBE,
	TOKEN_MULE,
	TOKEN_DIVE,
	TOKEN_LSHIFTE,
	TOKEN_RSHIFTE,
	TOKEN_CONCATE,

	TOKEN_EXTENDS,

	TOKEN_BREAK,
	TOKEN_CONTINUE,

	TOKEN_DLL,
	TOKEN_DLL32,
	TOKEN_DLL64,

	TOKEN_RETURN,
	TOKEN_COMMA,
	TOKEN_END_STATEMENT,
	TOKEN_OPEN_CODE,
	TOKEN_CLOSE_CODE,
	TOKEN_OPEN_BRACKET,
	TOKEN_CLOSE_BRACKET,
	TOKEN_OPEN_PAREN,
	TOKEN_CLOSE_PAREN,
	TOKEN_DOLLAR,
	TOKEN_APPEND,
	TOKEN_FUNCTION,
	TOKEN_SIZE,
	TOKEN_WHILE,
	TOKEN_ERROR,
	TOKEN_DICT,
	TOKEN_CALL,

	TOKEN_END,
	// Must be first PARSER value
	PARSER_SCRIPT,
	//PARSER_FUNCTION_CALL,
	//PARSER_FOR,
	PARSER_FOR_SUB_STATEMENT_LIST,
	PARSER_FOR_PRE_STATEMENT_LIST,
	PARSER_FOR_POST_STATEMENT_LIST,
	PARSER_STATEMENT_LIST,
	PARSER_STATEMENT_BLOCK,

	//PARSER_RAW_STRUCT,

	PARSER_DLL_ARG,
	PARSER_DLL_ARGS,
	PARSER_DLL_ARG_LIST,
	//PARSER_IF,
	//PARSER_IF_ELSE,
	PARSER_STATEMENT,
	PARSER_EXPRESSION,
	PARSER_SIMPLE_EXPRESSION,
	PARSER_SHIFT_EXPRESSION,
	PARSER_ADD_EXPRESSION,
	PARSER_MUL_EXPRESSION,
	PARSER_IDENTIFIER_EXPRESSION,
	PARSER_IDENTIFIER_EXPRESSION2,
	//PARSER_GLOBAL_INDEX_EXPRESSION,
	PARSER_ASSIGN_EXPRESSION,
	PARSER_FUNCTION,
	PARSER_FUNCTION_LIST,
	PARSER_IDENTIFIER_LIST,
	PARSER_MINI_DICT_EXPRESSION,
	PARSER_EMPTY_DICT,
	PARSER_DICT,
	PARSER_DICT_EXPRESSION,
	PARSER_SIZE,
	PARSER_EMPTY_LIST,
	PARSER_LIST,
	PARSER_LIST_EXPRESSION,
	PARSER_CONCAT_EXPRESSION,
	PARSER_APPEND_EXPRESSION,
	PARSER_EQUALS_EXPRESSION,
	PARSER_COMPARE_EXPRESSION,
	PARSER_COMPARE_STRING_EXPRESSION,
	PARSER_PREFIX_EXPRESSION,
	PARSER_AND_EXPRESSION,
	PARSER_XOR_EXPRESSION,
	PARSER_OR_EXPRESSION,
	PARSER_BOOL_OR_EXPRESSION,
	PARSER_BOOL_AND_EXPRESSION,
	PARSER_VALUE_LIST,
	//PARSER_RETURN,
	PARSER_SIMPLE_PREMOD_EXPRESSION,
	PARSER_SIMPLE_POSTMOD_EXPRESSION,
	PARSER_ELEMENT_PREMOD_EXPRESSION,
	PARSER_ELEMENT_POSTMOD_EXPRESSION,
	//PARSER_SIMPLE_EXPRESSION2,
	PARSER_DEREFERENCE_EXPRESSION,
	// Can be dereference *or* simple expression.
	PARSER_SIMPLE_DEREFERENCE_EXPRESSION,
	//PARSER_ELEMENT_PREMOD_IDENTIFIER,
	//PARSER_ELEMENT_PREPREMOD_EXPRESSION,

	PARSER_STRUCT,
	PARSER_STRUCT_LIST,
	PARSER_STRUCT_ENTRY,
	PARSER_STRUCT_VALUES,

	// Must be last
	PARSER_NONE
};



#endif
