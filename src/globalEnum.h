#ifndef GLOBAL_ENUM
#define GLOBAL_ENUM

typedef struct Stmt Stmt;
typedef struct switchCaseClause switchCaseClause;

typedef struct Exp Exp;
typedef struct ExpList ExpList; // for arguments

typedef struct RootNode RootNode;
typedef struct TopDeclarationNode TopDeclarationNode;
typedef struct VarDeclNode VarDeclNode;
typedef struct TypeDeclNode TypeDeclNode;
typedef struct FuncDeclNode FuncDeclNode;
typedef struct TypeHolderNode TypeHolderNode;
typedef struct IdChain IdChain;

typedef struct Context Context;
typedef struct SymbolTable SymbolTable;
typedef struct TypeTable TypeTable;
typedef struct STEntry STEntry;
typedef struct TTEntry TTEntry;
typedef struct TTEntryList TTEntryList;
typedef struct EntryTupleList EntryTupleList;
typedef struct PolymorphicEntry PolymorphicEntry;

typedef enum {
    StmtKindBlock,
    StmtKindExpression,
    StmtKindAssignment,
   
    StmtKindShortDeclaration, //TODO
    StmtKindSimpleStatement, //TODO

    StmtKindPrint,
    StmtKindPrintln,
    StmtKindIf,
    StmtKindReturn,
    StmtKindElse,
    StmtKindSwitch,
    StmtKindInfLoop,
    StmtKindWhileLoop,
    StmtKindThreePartLoop,

    StmtKindBreak,
    StmtKindContinue,
	StmtKindFallthrough,

	StmtKindTypeDeclaration,
	StmtKindVarDeclaration,
}StatementKind;



typedef enum{
    inLoop,
    inSwitchStatement,
    inFunction,
    outside
}State;



//Yann's defintion


extern int printTokens;

typedef enum {
	expKindIdentifier,
	expKindIntLit,
	expKindFloatLit,
	expKindRuneLit,
	expKindRawStringLit,
	expKindInterpretedStringLit,
	expKindAddition,
	expKindSubtraction,
	expKindMultiplication,
	expKindDivision,
	expKindMod,
	expKindBitAnd,
	expKindBitOr,
	expKindBitNotUnary,
	expKindBitNotBinary,
	expKindBitShiftLeft,
	expKindBitShiftRight,
	expKindBitAndNot,
	expKindLogicAnd,
	expKindLogicOr,
	expKindEQ,
	expKindGreater,
	expKindLess,
	expKindLogicNot,
	expKindNEQ,
	expKindLEQ,
	expKindGEQ,
	expKindUnaryPlus,
	expKindUnaryMinus,
	expKindFuncCall,
	expKindIndexing,
	expKindFieldSelect,
	expKindAppend,
	expKindLength,
	expKindCapacity,
} ExpressionKind;

typedef enum {
	funcDeclType,
	typeDeclType,
	variDeclType
} TopDeclarationType;

typedef enum {
	arrayType,
	structType,
	funcType, //TODO decide if this is the way
	sliceType,
	identifierType,
	inferType,
	badType,
} TypeType;
typedef enum {
	baseInt,
	baseFloat64,
	baseRune,
	baseString,
	baseBool,
	baseStruct,
} BaseType;
#endif
