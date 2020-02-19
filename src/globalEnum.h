#ifndef GLOBAL_ENUM
#define GLOBAL_ENUM

typedef enum {
    StmtKindBlock,
    StmtKindExpression,
    StmtKindAssignment,
   
    StmtKindDeclaration, //TODO
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
	StmtKindFallthrough



}StatementKind;



typedef enum{
    inLoop,
    inSwitchStatement,
    inFunction,
    outside
}State;



//Yann's defintion

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
	expKindTypeCast
} ExpressionKind;



#endif