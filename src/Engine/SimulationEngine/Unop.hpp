
using namespace z3;
static inline int CountLeadingZeros32(uint32_t n) {
#if defined(_MSC_VER)
	unsigned long result = 0;  // NOLINT(runtime/int)
	if (_BitScanReverse(&result, n)) {
		return 31 - result;
	}
	return 32;
#elif defined(__GNUC__)

	// Handle 0 as a special case because __builtin_clz(0) is undefined.
	if (n == 0) {
		return 32;
	}
	return __builtin_clz(n);
#else
	return CountLeadingZeros32Slow(n);
#endif
}

static inline int MostSignificantBit32(uint32_t n) {
	return 32 - CountLeadingZeros32(n);
}

static inline int CountLeadingZeros64(uint64_t n) {
#if defined(_MSC_VER) && defined(_M_X64)
	// MSVC does not have __builtin_clzll. Use _BitScanReverse64.
	unsigned long result = 0;  // NOLINT(runtime/int)
	if (_BitScanReverse64(&result, n)) {
		return 63 - result;
	}
	return 64;
#elif defined(_MSC_VER)
	// MSVC does not have __builtin_clzll. Compose two calls to _BitScanReverse
	unsigned long result = 0;  // NOLINT(runtime/int)
	if ((n >> 32) && _BitScanReverse(&result, n >> 32)) {
		return 31 - result;
	}
	if (_BitScanReverse(&result, n)) {
		return 63 - result;
	}
	return 64;
#elif defined(__GNUC__)

	// Handle 0 as a special case because __builtin_clzll(0) is undefined.
	if (n == 0) {
		return 64;
	}
	return __builtin_clzll(n);
#else
	return CountLeadingZeros64Slow(n);
#endif
}

static inline int MostSignificantBit64(uint64_t n) {
	return 64 - CountLeadingZeros64(n);
}

inline Z3_ast bool2bv(Z3_context ctx,Z3_ast ast) {
	Z3_inc_ref(ctx, ast);
	Z3_sort sort = Z3_mk_bv_sort(ctx, 1);
	Z3_inc_ref(ctx, (Z3_ast)sort);
	Z3_ast zero = Z3_mk_int(ctx, 0, sort);
	Z3_inc_ref(ctx, zero);
	Z3_ast one = Z3_mk_int(ctx, 1, sort);
	Z3_inc_ref(ctx, one);
	Z3_ast result = Z3_mk_ite(ctx, ast, one, zero);
	Z3_inc_ref(ctx, result);
	Z3_dec_ref(ctx, (Z3_ast)sort);
	Z3_dec_ref(ctx, one);
	Z3_dec_ref(ctx, zero);
	Z3_dec_ref(ctx, ast);
	return result;
}


template<typename T1, typename T2>
inline Variable Iop_to(Variable &a) { vassert(a.bitn == (sizeof(T1) * 8)); return Variable((T2)(((T1)a)),a); }
template<int T1, int T2>
inline Variable Iop_ZEXT(Variable &a) { vassert(a.bitn == T1); return Variable(a, Z3_mk_zero_ext(a, T2 - T1, a), T2); }

template<int T1, int T2>
inline Variable Iop_SEXT(Variable &a) { vassert(a.bitn == T1); return Variable(a, Z3_mk_sign_ext(a, T2 - T1, a), T2); }

inline Variable State::T_Unop(IROp op, IRExpr* arg1) {
	Variable a = tIRExpr(arg1);

	//a.tostr();

	if (a.symbolic()) goto dosymbol;
	switch (op) {
	case Iop_1Uto8:vassert(a.bitn == 1); return Variable((UChar)((UChar)a & 0b1 ? 1 : 0), a);
	case Iop_1Uto32:vassert(a.bitn == 1); return Variable((UShort)((UChar)a & 0b1 ? 1 : 0), a);
	case Iop_1Uto64:vassert(a.bitn == 1); return Variable((ULong)((UChar)a & 0b1 ? 1 : 0), a);
	case Iop_1Sto8:vassert(a.bitn == 1); return Variable((Char)((UChar)a & 0b1 ? -1 : 0), a);
	case Iop_1Sto16:vassert(a.bitn == 1); return Variable((Short)((UChar)a & 0b1 ? -1 : 0), a);
	case Iop_1Sto32:vassert(a.bitn == 1); return Variable((Int)((UChar)a & 0b1 ? -1 : 0), a);
	case Iop_1Sto64:vassert(a.bitn == 1); return Variable((Long)((UChar)a & 0b1 ? -1 : 0), a);


	case Iop_32to1:vassert(a.bitn == 32); return Variable(((UChar)a & 0b1 ? 1 : 0), a, 1);
	case Iop_64to1:vassert(a.bitn == 64); return Variable(((UChar)a & 0b1 ? 1 : 0), a, 1);

	case Iop_Not1:return Variable(((UChar)a & 0b1 ? 0 : 1), a, 1);
	case Iop_Not8:return Variable((UChar)(~(UChar)a), a);
	case Iop_Not16:return Variable((UShort)(~(UShort)a), a);
	case Iop_Not32:return Variable((UInt)(~(UInt)a), a);
	case Iop_Not64:return Variable((ULong)(~(ULong)a), a);
	case Iop_NotV256:return Variable(_mm256_not_si256(a), a);
	case Iop_NotV128:return Variable(_mm_not_si128(a), a);

	case Iop_8Sto16:return Iop_to<Char, Short>(a);
	case Iop_8Sto32:return Iop_to<Char, Int>(a);
	case Iop_8Sto64:return Iop_to<Char, Long>(a);
	case Iop_8Uto16:return Iop_to<UChar, UShort>(a);
	case Iop_8Uto32:return Iop_to<UChar, UInt>(a);
	case Iop_8Uto64:return Iop_to<UChar, ULong>(a);

	case Iop_16Sto32:return Iop_to<Short, Int>(a);
	case Iop_16Sto64:return Iop_to<Short, Long>(a);
	case Iop_16Uto32:return Iop_to<UShort, UInt>(a);
	case Iop_16Uto64:return Iop_to<UShort, ULong>(a);

	case Iop_32Sto64:return Iop_to<Int, Long>(a);
	case Iop_32Uto64:return Iop_to<UInt, ULong>(a);
	case Iop_32UtoV128:return Variable(_mm_set_epi32(0, 0, 0, a), a);
	case Iop_64UtoV128:return Variable(_mm_set_epi64x(0, a), a);

	case Iop_64to32:return Iop_to<ULong, UInt>(a);
	case Iop_64to16:return Iop_to<ULong, UShort>(a);
	case Iop_64to8:return Iop_to<ULong, UChar>(a);
	case Iop_64HIto32:return Variable(a.get<Int, 4>(), a);
	case Iop_32to16:return Iop_to<UInt, UShort>(a);
	case Iop_32to8:return Iop_to<UInt, UChar>(a);
	case Iop_32HIto16:return Variable(a.get<UShort, 2>(), a);


	case Iop_V128to32:return Variable(((__m128i)a).m128i_u32[0], a);//OK
	case Iop_V128to64:return Variable(((__m128i)a).m128i_u64[0], a);//OK
	case Iop_128to64:return Variable(((__m128i)a).m128i_u64[0], a);//OK
	case Iop_V128HIto64:return Variable(((__m128i)a).m128i_u64[1], a);//OK
	case Iop_V256toV128_0:return Variable( GET16(((__m256i)a).m256i_u64), a);
	case Iop_V256toV128_1:return Variable( GET16(((__m256i)a).m256i_u64+2), a);
	case Iop_128HIto64:return Variable(((__m128i)a).m128i_u64[1], a);//OK
	case Iop_16HIto8:return Variable(a.get<UChar,1>(), a);//OK
	case Iop_GetMSBs8x16: return Variable((UShort)_mm_movemask_epi8(a), a);//OK pmovmskb
	case Iop_Clz32:vassert(a.bitn == 32); return Variable((UInt)CountLeadingZeros32(a), a);
	case Iop_Clz64:vassert(a.bitn == 64); return Variable((ULong)CountLeadingZeros64(a), a);
	case Iop_Ctz32: {
		vassert(a.bitn == 32);
		unsigned long result = 0;
		_BitScanForward(&result, (UInt)a);
		return Variable((UInt)result, a);
	};//ok
	case Iop_Ctz64: {
		vassert(a.bitn == 64);
		unsigned long result = 0;
		_BitScanForward64(&result, (ULong)a);
		return Variable((ULong)result, a); 
	};//ok
	case Iop_Abs64Fx2:
	case Iop_Neg64Fx2:
	case Iop_RSqrtEst64Fx2:
	case Iop_RecipEst64Fx2:

	case Iop_Sqrt64F0x2:

	case Iop_Sqrt32Fx8:
	case Iop_RSqrtEst32Fx8:
	case Iop_RecipEst32Fx8:

	case Iop_Sqrt64Fx4:

	case Iop_RecipEst32Fx4:
	case Iop_RoundF32x4_RM:
	case Iop_RoundF32x4_RP:
	case Iop_RoundF32x4_RN:
	case Iop_RoundF32x4_RZ:
	case Iop_RecipEst32Ux4:
	case Iop_Abs32Fx4:
	case Iop_Neg32Fx4:
	case Iop_RSqrtEst32Fx4:

	case Iop_RecipEst32Fx2:
	case Iop_RecipEst32Ux2:
	case Iop_Abs32Fx2:
	case Iop_Neg32Fx2:
	case Iop_RSqrtEst32Fx2:

	case Iop_Sqrt32F0x4:
	case Iop_RSqrtEst32F0x4:
	case Iop_RecipEst32F0x4:

	case Iop_Dup8x16:
	case Iop_Dup16x8:
	case Iop_Dup32x4:
	case Iop_Reverse1sIn8_x16:
	case Iop_Reverse8sIn16_x8:
	case Iop_Reverse8sIn32_x4:
	case Iop_Reverse16sIn32_x4:
	case Iop_Reverse8sIn64_x2:
	case Iop_Reverse16sIn64_x2:
	case Iop_Reverse32sIn64_x2:

	case Iop_ZeroHI64ofV128:
	case Iop_ZeroHI96ofV128:
	case Iop_ZeroHI112ofV128:
	case Iop_ZeroHI120ofV128:

	case Iop_F128HItoF64:  /* F128 -> high half of F128 */
	case Iop_D128HItoD64:  /* D128 -> high half of D128 */
	case Iop_F128LOtoF64:  /* F128 -> low  half of F128 */
	case Iop_D128LOtoD64:  /* D128 -> low  half of D128 */

	case Iop_NegF128:
	case Iop_AbsF128:

	case Iop_I32StoF128: /* signed I32 -> F128 */
	case Iop_I64StoF128: /* signed I64 -> F128 */
	case Iop_I32UtoF128: /* unsigned I32 -> F128 */
	case Iop_I64UtoF128: /* unsigned I64 -> F128 */
	case Iop_F32toF128:  /* F32 -> F128 */
	case Iop_F64toF128:  /* F64 -> F128 */
	case Iop_I32StoD128: /* signed I64 -> D128 */
	case Iop_I64StoD128: /* signed I64 -> D128 */
	case Iop_I32UtoD128: /* unsigned I32 -> D128 */
	case Iop_I64UtoD128: /* unsigned I64 -> D128 */

	case Iop_F16toF64:goto FAILD;
	case Iop_F32toF64:return Variable((double)((float)a), a);
	case Iop_I32StoF64:return Variable((double)((Int)a), a);
	case Iop_I32UtoF64:return Variable((double)((UInt)a), a);
	case Iop_NegF64:
	case Iop_AbsF64:
	case Iop_RSqrtEst5GoodF64:
	case Iop_RoundF64toF64_NEAREST:
	case Iop_RoundF64toF64_NegINF:
	case Iop_RoundF64toF64_PosINF:
	case Iop_RoundF64toF64_ZERO:
	case Iop_D32toD64:
	case Iop_I32StoD64:
	case Iop_I32UtoD64:
	case Iop_ExtractExpD64:    /* D64  -> I64 */
	case Iop_ExtractExpD128:   /* D128 -> I64 */
	case Iop_ExtractSigD64:    /* D64  -> I64 */
	case Iop_ExtractSigD128:   /* D128 -> I64 */
	case Iop_DPBtoBCD:
	case Iop_BCDtoDPB:

	case Iop_D64toD128:

	case Iop_TruncF64asF32:
	case Iop_NegF32:
	case Iop_AbsF32:
	case Iop_F16toF32:

	case Iop_Dup8x8:
	case Iop_Dup16x4:
	case Iop_Dup32x2:
	case Iop_Reverse8sIn16_x4:
	case Iop_Reverse8sIn32_x2:
	case Iop_Reverse16sIn32_x2:
	case Iop_Reverse8sIn64_x1:
	case Iop_Reverse16sIn64_x1:
	case Iop_Reverse32sIn64_x1:
	case Iop_V256to64_0: case Iop_V256to64_1:
	case Iop_V256to64_2: case Iop_V256to64_3:

	case Iop_GetMSBs8x8:


	case Iop_ReinterpF64asI64:
	case Iop_ReinterpI64asF64:
	case Iop_ReinterpI32asF32:
	case Iop_ReinterpF32asI32:
	case Iop_ReinterpI64asD64:
	case Iop_ReinterpD64asI64:

	case Iop_CmpNEZ8x8:
	case Iop_Cnt8x8:
	case Iop_Clz8x8:
	case Iop_Cls8x8:
	case Iop_Abs8x8:

	case Iop_CmpNEZ8x16:
	case Iop_Cnt8x16:
	case Iop_Clz8x16:
	case Iop_Cls8x16:
	case Iop_Abs8x16:

	case Iop_CmpNEZ16x4:
	case Iop_Clz16x4:
	case Iop_Cls16x4:
	case Iop_Abs16x4:

	case Iop_CmpNEZ16x8:
	case Iop_Clz16x8:
	case Iop_Cls16x8:
	case Iop_Abs16x8:

	case Iop_CmpNEZ32x2:
	case Iop_Clz32x2:
	case Iop_Cls32x2:
	case Iop_Abs32x2:

	case Iop_CmpNEZ32x4:
	case Iop_Clz32x4:
	case Iop_Cls32x4:
	case Iop_Abs32x4:
	case Iop_RSqrtEst32Ux4:

	case Iop_CmpwNEZ32:

	case Iop_CmpwNEZ64:

	case Iop_CmpNEZ64x2:
	case Iop_CipherSV128:
	case Iop_Clz64x2:
	case Iop_Abs64x2:

	case Iop_PwBitMtxXpose64x2:

	case Iop_NarrowUn16to8x8:
	case Iop_NarrowUn32to16x4:
	case Iop_NarrowUn64to32x2:
	case Iop_QNarrowUn16Sto8Sx8:
	case Iop_QNarrowUn16Sto8Ux8:
	case Iop_QNarrowUn16Uto8Ux8:
	case Iop_QNarrowUn32Sto16Sx4:
	case Iop_QNarrowUn32Sto16Ux4:
	case Iop_QNarrowUn32Uto16Ux4:
	case Iop_QNarrowUn64Sto32Sx2:
	case Iop_QNarrowUn64Sto32Ux2:
	case Iop_QNarrowUn64Uto32Ux2:

	case Iop_Widen8Sto16x8:
	case Iop_Widen8Uto16x8:
	case Iop_Widen16Sto32x4:
	case Iop_Widen16Uto32x4:
	case Iop_Widen32Sto64x2:
	case Iop_Widen32Uto64x2:

	case Iop_PwAddL32Ux2:
	case Iop_PwAddL32Sx2:

	case Iop_PwAddL16Ux4:
	case Iop_PwAddL16Sx4:

	case Iop_PwAddL8Ux8:
	case Iop_PwAddL8Sx8:

	case Iop_PwAddL32Ux4:
	case Iop_PwAddL32Sx4:

	case Iop_PwAddL16Ux8:
	case Iop_PwAddL16Sx8:

	case Iop_PwAddL8Ux16:
	case Iop_PwAddL8Sx16:

	case Iop_I64UtoF32:
	default:
	}
	goto FAILD;

dosymbol:
	switch (op) {
	case Iop_1Uto8: return Iop_ZEXT<1, 8>(a);
	case Iop_1Uto32:return Iop_ZEXT<1, 32>(a);
	case Iop_1Uto64:return Iop_ZEXT<1, 64>(a);
	case Iop_1Sto8: return Iop_SEXT<1, 8>(a);
	case Iop_1Sto16:return Iop_SEXT<1, 16>(a);
	case Iop_1Sto32:return Iop_SEXT<1, 32>(a);
	case Iop_1Sto64:return Iop_SEXT<1, 64>(a);

	case Iop_32to1:vassert(a.bitn == 32); return Variable(a, Z3_mk_extract(a, 0, 0, a), 1);
	case Iop_64to1:vassert(a.bitn == 64); return Variable(a, Z3_mk_extract(a, 0, 0, a), 1);

	case Iop_Not1: return Variable(a, Z3_mk_bvnot(a, a), 1);
	case Iop_Not8: return Variable(a, Z3_mk_bvnot(a, a), 8);
	case Iop_Not16:return Variable(a, Z3_mk_bvnot(a, a), 16);
	case Iop_Not32:return Variable(a, Z3_mk_bvnot(a, a), 32);
	case Iop_Not64:return Variable(a, Z3_mk_bvnot(a, a), 64);

	case Iop_8Sto16:return Iop_SEXT<8, 16>(a);
	case Iop_8Sto32:return Iop_SEXT<8, 32>(a);
	case Iop_8Sto64:return Iop_SEXT<8, 64>(a);
	case Iop_8Uto16:return Iop_ZEXT<8, 16>(a);
	case Iop_8Uto32:return Iop_ZEXT<8, 32>(a);
	case Iop_8Uto64:return Iop_ZEXT<8, 64>(a);

	case Iop_16Sto32:return Iop_SEXT<16, 32>(a);
	case Iop_16Sto64:return Iop_SEXT<16, 64>(a);
	case Iop_16Uto32:return Iop_ZEXT<16, 32>(a);
	case Iop_16Uto64:return Iop_ZEXT<16, 64>(a);

	case Iop_32Sto64:return Iop_SEXT<32, 64>(a);
	case Iop_32Uto64:return Iop_ZEXT<32, 64>(a);
	/*case Iop_32UtoV128:return Variable(ast2z3_vec32x4(zext((expr)a, 128)), 128);
	case Iop_64UtoV128:return Variable(ast2z3_vec64x2(zext((expr)a, 128)), 128);*/

	case Iop_32to8:return Variable(a, Z3_mk_extract(a, 7, 0, a), 8);
	case Iop_64to8:return Variable(a, Z3_mk_extract(a, 7, 0, a), 8);
	case Iop_64to16:return Variable(a, Z3_mk_extract(a, 15, 0, a), 16);
	case Iop_32to16:return Variable(a, Z3_mk_extract(a, 15, 0, a), 16);
	case Iop_64to32:return Variable(a, Z3_mk_extract(a, 31, 0, a), 32);
	case Iop_V128to32:return Variable(a, Z3_mk_extract(a, 31, 0, a), 32);
	case Iop_V128to64:return Variable(a, Z3_mk_extract(a, 63, 0, a), 64);
	case Iop_128to64:return Variable(a, Z3_mk_extract(a, 63, 0, a), 64);
	case Iop_16HIto8:return Variable(a, Z3_mk_extract(a, 15, 8, a), 8);
	case Iop_32HIto16:return Variable(a, Z3_mk_extract(a, 31, 16, a), 16);
	case Iop_64HIto32:return Variable(a, Z3_mk_extract(a, 63, 32, a), 32);
	case Iop_V128HIto64:return Variable(a, Z3_mk_extract(a, 127, 64, a), 64);
	case Iop_128HIto64:return Variable(a, Z3_mk_extract(a, 127, 64, a), 64);
	}


FAILD:
	ppIROp(op);
	vpanic("tIRType");
}

