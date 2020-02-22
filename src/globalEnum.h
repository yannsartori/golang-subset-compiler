#ifndef GLOBAL_ENUM
#define GLOBAL_ENUM
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
	sliceType,
	identifierType,
	inferType,
} TypeType;

#endif
