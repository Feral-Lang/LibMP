#include "MP.hpp"

namespace fer
{

gmp_randstate_t rngstate;

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// VarMPInt /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

VarMPInt::VarMPInt(ModuleLoc loc, int64_t _val) : Var(loc, false, false)
{
	mpz_init_set_si(val, _val);
}
VarMPInt::VarMPInt(ModuleLoc loc, mpz_srcptr _val) : Var(loc, false, false)
{
	mpz_init_set(val, _val);
}
VarMPInt::VarMPInt(ModuleLoc loc, mpfr_srcptr _val) : Var(loc, false, false)
{
	mpz_init(val);
	mpfr_get_z(val, _val, mpfr_get_default_rounding_mode());
}
VarMPInt::VarMPInt(ModuleLoc loc, const char *_val) : Var(loc, false, false)
{
	mpz_init_set_str(val, _val, 0);
}
VarMPInt::~VarMPInt() { mpz_clear(val); }
Var *VarMPInt::onCopy(Interpreter &vm, ModuleLoc loc)
{
	return vm.makeVarWithRef<VarMPInt>(loc, val);
}
void VarMPInt::onSet(Interpreter &vm, Var *from) { mpz_set(val, as<VarMPInt>(from)->getPtr()); }

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// VarMPFlt /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

VarMPFlt::VarMPFlt(ModuleLoc loc, long double _val) : Var(loc, false, false)
{
	mpfr_init_set_ld(val, _val, mpfr_get_default_rounding_mode());
}
VarMPFlt::VarMPFlt(ModuleLoc loc, mpfr_srcptr _val) : Var(loc, false, false)
{
	mpfr_init_set(val, _val, mpfr_get_default_rounding_mode());
}
VarMPFlt::VarMPFlt(ModuleLoc loc, mpz_srcptr _val) : Var(loc, false, false)
{
	mpfr_init_set_z(val, _val, mpfr_get_default_rounding_mode());
}
VarMPFlt::VarMPFlt(ModuleLoc loc, const char *_val) : Var(loc, false, false)
{
	mpfr_init_set_str(val, _val, 0, mpfr_get_default_rounding_mode());
}
VarMPFlt::~VarMPFlt() { mpfr_clear(val); }
Var *VarMPFlt::onCopy(Interpreter &vm, ModuleLoc loc)
{
	return vm.makeVarWithRef<VarMPFlt>(loc, val);
}
void VarMPFlt::onSet(Interpreter &vm, Var *from)
{
	mpfr_set(val, as<VarMPFlt>(from)->getPtr(), mpfr_get_default_rounding_mode());
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// VarMPComplex ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

VarMPComplex::VarMPComplex(ModuleLoc loc) : Var(loc, false, false) { initBase(); }
VarMPComplex::VarMPComplex(ModuleLoc loc, int64_t real, int64_t imag) : Var(loc, false, false)
{
	initBase();
	mpc_set_si_si(val, real, imag, mpc_get_default_rounding_mode());
}
VarMPComplex::VarMPComplex(ModuleLoc loc, long double real, long double imag)
	: Var(loc, false, false)
{
	initBase();
	mpc_set_ld_ld(val, real, imag, mpc_get_default_rounding_mode());
}
VarMPComplex::VarMPComplex(ModuleLoc loc, mpfr_srcptr real, mpfr_srcptr imag)
	: Var(loc, false, false)
{
	initBase();
	mpc_set_fr_fr(val, real, imag, mpc_get_default_rounding_mode());
}
VarMPComplex::VarMPComplex(ModuleLoc loc, mpz_srcptr real, mpz_srcptr imag) : Var(loc, false, false)
{
	initBase();
	mpc_set_z_z(val, real, imag, mpc_get_default_rounding_mode());
}
VarMPComplex::VarMPComplex(ModuleLoc loc, mpc_srcptr _val) : Var(loc, false, false)
{
	initBase();
	mpc_set(val, _val, mpc_get_default_rounding_mode());
}
VarMPComplex::VarMPComplex(ModuleLoc loc, const char *_val) : Var(loc, false, false)
{
	initBase();
	mpc_set_str(val, _val, 0, mpc_get_default_rounding_mode());
}
VarMPComplex::~VarMPComplex() { mpc_clear(val); }

void VarMPComplex::initBase() { mpc_init2(val, 256); }

Var *VarMPComplex::onCopy(Interpreter &vm, ModuleLoc loc)
{
	return vm.makeVarWithRef<VarMPComplex>(loc, val);
}
void VarMPComplex::onSet(Interpreter &vm, Var *from)
{
	mpc_set(val, as<VarMPComplex>(from)->getPtr(), mpc_get_default_rounding_mode());
}

mpc_rnd_t mpc_get_default_rounding_mode() { return MPC_RNDNN; }

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *rngSeed(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	     const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPInt>()) {
		vm.fail(loc,
			"expected seed value to be a big int, found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	gmp_randseed(rngstate, as<VarMPInt>(args[1])->getPtr());
	return vm.getNil();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Int Functions //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *mpIntNewNative(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		    const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarStr>() && !args[1]->is<VarMPInt>() &&
	   !args[1]->is<VarMPFlt>())
	{
		vm.fail(loc,
			"argument 1 to bignum.newInt() must be of "
			"type 'int', 'str', 'bigint', or 'bigflt', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}

	if(args[1]->is<VarInt>()) {
		return vm.makeVar<VarMPInt>(loc, as<VarInt>(args[1])->getVal());
	}
	if(args[1]->is<VarStr>()) {
		return vm.makeVar<VarMPInt>(loc, as<VarStr>(args[1])->getVal().c_str());
	}
	if(args[1]->is<VarMPInt>()) {
		return vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[1])->getPtr());
	}
	return vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[1])->getSrcPtr());
}

#define ARITHI_FUNC(fn, name)                                                                  \
	Var *mpInt##fn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,                       \
		       const StringMap<AssnArgData> &assn_args)                                \
	{                                                                                      \
		if(args[1]->is<VarMPInt>()) {                                                  \
			VarMPInt *res =                                                        \
			vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());         \
			mpz_##name(res->getPtr(), res->getSrcPtr(),                            \
				   as<VarMPInt>(args[1])->getSrcPtr());                        \
			return res;                                                            \
		} else if(args[1]->is<VarMPFlt>()) {                                           \
			VarMPInt *res =                                                        \
			vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());         \
			mpz_t tmp;                                                             \
			mpz_init(tmp);                                                         \
			mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),                    \
				   mpfr_get_default_rounding_mode());                          \
			mpz_##name(res->getPtr(), res->getSrcPtr(), tmp);                      \
			mpz_clear(tmp);                                                        \
			return res;                                                            \
		}                                                                              \
		vm.fail(loc,                                                                   \
			"expected int or float argument for int " STRINGIFY(name) ", found: ", \
			vm.getTypeName(args[1]));                                              \
		return nullptr;                                                                \
	}

#define ARITHI_ASSN_FUNC(fn, name)                                                               \
	Var *mpIntAssn##fn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,                     \
			   const StringMap<AssnArgData> &assn_args)                              \
	{                                                                                        \
		if(args[1]->is<VarMPInt>()) {                                                    \
			mpz_##name(as<VarMPInt>(args[0])->getPtr(),                              \
				   as<VarMPInt>(args[0])->getSrcPtr(),                           \
				   as<VarMPInt>(args[1])->getSrcPtr());                          \
			return args[0];                                                          \
		} else if(args[1]->is<VarMPFlt>()) {                                             \
			mpz_t tmp;                                                               \
			mpz_init(tmp);                                                           \
			mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),                      \
				   mpfr_get_default_rounding_mode());                            \
			mpz_##name(as<VarMPInt>(args[0])->getPtr(),                              \
				   as<VarMPInt>(args[0])->getSrcPtr(), tmp);                     \
			mpz_clear(tmp);                                                          \
			return args[0];                                                          \
		}                                                                                \
		vm.fail(                                                                         \
		loc, "expected int or float argument for int " STRINGIFY(name) "-assn, found: ", \
		vm.getTypeName(args[1]));                                                        \
		return nullptr;                                                                  \
	}

#define LOGICI_FUNC(fn, name, sym)                                                         \
	Var *mpInt##fn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,                   \
		       const StringMap<AssnArgData> &assn_args)                            \
	{                                                                                  \
		if(args[1]->is<VarMPInt>()) {                                              \
			return mpz_cmp(as<VarMPInt>(args[0])->getSrcPtr(),                 \
				       as<VarMPInt>(args[1])->getSrcPtr()) sym 0           \
			       ? vm.getTrue()                                              \
			       : vm.getFalse();                                            \
		}                                                                          \
		vm.fail(loc, "expected int argument for int " STRINGIFY(name) ", found: ", \
			vm.getTypeName(args[1]));                                          \
		return nullptr;                                                            \
	}

ARITHI_FUNC(Add, add)
ARITHI_FUNC(Sub, sub)
ARITHI_FUNC(Mul, mul)
ARITHI_FUNC(Mod, mod)

ARITHI_ASSN_FUNC(Add, add)
ARITHI_ASSN_FUNC(Sub, sub)
ARITHI_ASSN_FUNC(Mul, mul)
ARITHI_ASSN_FUNC(Mod, mod)

LOGICI_FUNC(LT, lt, <)
LOGICI_FUNC(GT, gt, >)
LOGICI_FUNC(LE, le, <=)
LOGICI_FUNC(GE, ge, >=)

Var *mpIntEq(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	     const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		return mpz_cmp(as<VarMPInt>(args[0])->getSrcPtr(),
			       as<VarMPInt>(args[1])->getSrcPtr()) == 0
		       ? vm.getTrue()
		       : vm.getFalse();
	}
	return vm.getFalse();
}

Var *mpIntNe(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	     const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		return mpz_cmp(as<VarMPInt>(args[0])->getSrcPtr(),
			       as<VarMPInt>(args[1])->getSrcPtr()) != 0
		       ? vm.getTrue()
		       : vm.getFalse();
	}
	return vm.getTrue();
}

Var *mpIntDiv(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	      const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		// rhs == 0
		if(mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()) == 0) {
			vm.fail(loc, "division by zero");
			return nullptr;
		}
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_div(res->getPtr(), res->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr());
		return res;
	} else if(args[1]->is<VarMPFlt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		// rhs == 0
		if(mpz_get_ui(tmp) == 0) {
			vm.fail(loc, "division by zero");
			mpz_clear(tmp);
			return nullptr;
		}
		mpz_div(res->getPtr(), res->getSrcPtr(), tmp);
		mpz_clear(tmp);
		return res;
	}
	vm.fail(loc, "expected int or float argument for int " STRINGIFY(name) ", found: ",
		vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntAssnDiv(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		// rhs == 0
		if(mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()) == 0) {
			vm.fail(loc, "division by zero");
			return nullptr;
		}
		mpz_div(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			as<VarMPInt>(args[1])->getSrcPtr());
		return args[0];
	} else if(args[1]->is<VarMPFlt>()) {
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		// rhs == 0
		if(mpz_get_ui(tmp) == 0) {
			vm.fail(loc, "division by zero");
			mpz_clear(tmp);
			return nullptr;
		}
		mpz_div(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), tmp);
		mpz_clear(tmp);
		return args[0];
	}
	vm.fail(loc, "expected int or float argument for int " STRINGIFY(name) "-assn, found: ",
		vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntBAnd(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_and(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			as<VarMPInt>(args[1])->getSrcPtr());
		return res;
	}
	vm.fail(loc, "expected int argument for int bitwise and, found: ", vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntBOr(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	      const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_ior(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			as<VarMPInt>(args[1])->getSrcPtr());
		return res;
	}
	vm.fail(loc, "expected int argument for int bitwise or, found: ", vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntBXOr(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_xor(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			as<VarMPInt>(args[1])->getSrcPtr());
		return res;
	}
	vm.fail(loc, "expected int argument for int bitwise xor, found: ", vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntBNot(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
	mpz_com(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr());
	return res;
}

Var *mpIntBAndAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		   const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		mpz_and(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			as<VarMPInt>(args[1])->getSrcPtr());
		return args[0];
	}
	vm.fail(loc,
		"expected int argument for int "
		"bitwise and-assn, found: ",
		vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntBOrAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		mpz_ior(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			as<VarMPInt>(args[1])->getSrcPtr());
		return args[0];
	}
	vm.fail(loc,
		"expected int argument for int bitwise or-assn, found: ", vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntBXOrAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		   const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		mpz_xor(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			as<VarMPInt>(args[1])->getSrcPtr());
		return args[0];
	}
	vm.fail(loc, "expected int argument for int bitwise xor, found: ", vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntBNotAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		   const StringMap<AssnArgData> &assn_args)
{
	mpz_com(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr());
	return args[0];
}

Var *mpIntLShift(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_mul_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
		return res;
	} else if(args[1]->is<VarMPFlt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		mpz_mul_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), mpz_get_si(tmp));
		mpz_clear(tmp);
		return res;
	}
	vm.fail(loc,
		"expected bigint or float argument "
		"for bigint leftshift, found: ",
		vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntRShift(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_div_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
		return res;
	} else if(args[1]->is<VarMPFlt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		mpz_div_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), mpz_get_si(tmp));
		mpz_clear(tmp);
		return res;
	}
	vm.fail(loc,
		"expected int or float argument "
		"for int rightshift, found: ",
		vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntLShiftAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		     const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		mpz_mul_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
		return args[0];
	} else if(args[1]->is<VarMPFlt>()) {
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		mpz_mul_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			     mpz_get_si(tmp));
		mpz_clear(tmp);
		return args[0];
	}
	vm.fail(loc, "expected int or float argument for int leftshift-assign, found: ",
		vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntRShiftAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		     const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		mpz_div_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
		return args[0];
	} else if(args[1]->is<VarMPFlt>()) {
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		mpz_div_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
			     mpz_get_si(tmp));
		mpz_clear(tmp);
		return args[0];
	}
	vm.fail(loc, "expected int or float argument for int rightshift-assign, found: ",
		vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntPow(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	      const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_pow_ui(res->getPtr(), res->getSrcPtr(),
			   mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()));
		return res;
	} else if(args[1]->is<VarMPFlt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		mpz_pow_ui(res->getPtr(), res->getSrcPtr(), mpz_get_ui(tmp));
		mpz_clear(tmp);
		return res;
	}
	vm.fail(loc,
		"expected int or float argument for int power, found: ", vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntRoot(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPInt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_root(res->getPtr(), res->getSrcPtr(),
			 mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()));
		return res;
	} else if(args[1]->is<VarMPFlt>()) {
		VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
		mpz_t tmp;
		mpz_init(tmp);
		mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpfr_get_default_rounding_mode());
		mpz_root(res->getPtr(), res->getSrcPtr(), mpz_get_ui(tmp));
		mpz_clear(tmp);
		return res;
	}
	vm.fail(loc,
		"expected int or float argument for int root, found: ", vm.getTypeName(args[1]));
	return nullptr;
}

Var *mpIntPreInc(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	mpz_add_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
	return args[0];
}

Var *mpIntPostInc(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
	mpz_add_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
	return res;
}

Var *mpIntPreDec(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	mpz_sub_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
	return args[0];
}

Var *mpIntPostDec(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
	mpz_sub_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
	return res;
}

Var *mpIntUSub(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
	mpz_neg(res->getPtr(), res->getSrcPtr());
	return res;
}

Var *mpIntPopCnt(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	return vm.makeVar<VarMPInt>(loc, mpz_popcount(as<VarMPInt>(args[0])->getSrcPtr()));
}

Var *mpIntToInt(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		const StringMap<AssnArgData> &assn_args)
{
	return vm.makeVar<VarMPInt>(loc, mpz_get_si(as<VarMPInt>(args[0])->getPtr()));
}

Var *mpIntToStr(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		const StringMap<AssnArgData> &assn_args)
{
	typedef void (*gmp_freefunc_t)(void *, size_t);

	char *_res  = mpz_get_str(NULL, 10, as<VarMPInt>(args[0])->getSrcPtr());
	VarStr *res = vm.makeVar<VarStr>(loc, _res);

	gmp_freefunc_t freefunc;
	mp_get_memory_functions(NULL, NULL, &freefunc);
	freefunc(_res, strlen(_res) + 1);

	return res;
}

// Iterator

class VarMPIntIterator : public Var
{
	mpz_t begin, end, step, curr;
	bool started;
	bool reversed;

public:
	VarMPIntIterator(ModuleLoc loc);
	VarMPIntIterator(ModuleLoc loc, mpz_srcptr _begin, mpz_srcptr _end, mpz_srcptr _step);
	~VarMPIntIterator();

	Var *copy(ModuleLoc loc);
	void set(Var *from);

	bool next(mpz_ptr val);

	inline void setReversed(mpz_srcptr step) { reversed = mpz_cmp_si(step, 0) < 0; }
	inline mpz_ptr getBegin() { return begin; }
	inline mpz_ptr getEnd() { return end; }
	inline mpz_ptr getStep() { return step; }
	inline mpz_ptr getCurr() { return curr; }
};

VarMPIntIterator::VarMPIntIterator(ModuleLoc loc)
	: Var(loc, false, false), started(false), reversed(false)
{
	mpz_init(begin);
	mpz_init(end);
	mpz_init(step);
	mpz_init(curr);
}
VarMPIntIterator::VarMPIntIterator(ModuleLoc loc, mpz_srcptr _begin, mpz_srcptr _end,
				   mpz_srcptr _step)
	: Var(loc, false, false), started(false), reversed(mpz_cmp_si(_step, 0) < 0)
{
	mpz_init_set(begin, _begin);
	mpz_init_set(end, _end);
	mpz_init_set(step, _step);
	mpz_init_set(curr, _begin);
}
VarMPIntIterator::~VarMPIntIterator() { mpz_clears(begin, end, step, curr, NULL); }

Var *VarMPIntIterator::copy(ModuleLoc loc) { return new VarMPIntIterator(loc, begin, end, step); }
void VarMPIntIterator::set(Var *from)
{
	VarMPIntIterator *f = as<VarMPIntIterator>(from);
	mpz_set(begin, f->begin);
	mpz_set(end, f->end);
	mpz_set(step, f->step);
	mpz_set(curr, f->curr);
	started	 = f->started;
	reversed = f->reversed;
}

bool VarMPIntIterator::next(mpz_ptr val)
{
	if(reversed) {
		if(mpz_cmp(curr, end) <= 0) return false;
	} else {
		if(mpz_cmp(curr, end) >= 0) return false;
	}
	if(!started) {
		mpz_init(val);
		mpz_set(val, curr);
		started = true;
		return true;
	}
	mpz_t tmp;
	mpz_init(tmp);
	mpz_add(tmp, curr, step);
	if(reversed) {
		if(mpz_cmp(tmp, end) <= 0) {
			mpz_clear(tmp);
			return false;
		}
	} else {
		if(mpz_cmp(tmp, end) >= 0) {
			mpz_clear(tmp);
			return false;
		}
	}
	mpz_set(curr, tmp);
	mpz_init(val);
	mpz_set(val, curr);
	mpz_clear(tmp);
	return true;
}

Var *mpIntRange(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		const StringMap<AssnArgData> &assn_args)
{
	Var *lhs_base  = args[1];
	Var *rhs_base  = args.size() > 2 ? args[2] : nullptr;
	Var *step_base = args.size() > 3 ? args[3] : nullptr;

	if(!lhs_base->is<VarMPInt>()) {
		vm.fail(lhs_base->getLoc(),
			"expected argument 1 to be of type int, found: ", vm.getTypeName(lhs_base));
		return nullptr;
	}
	if(rhs_base && !rhs_base->is<VarMPInt>()) {
		vm.fail(rhs_base->getLoc(),
			"expected argument 2 to be of type int, found: ", vm.getTypeName(rhs_base));
		return nullptr;
	}
	if(step_base && !step_base->is<VarMPInt>()) {
		vm.fail(step_base->getLoc(), "expected argument 3 to be of type int, found: ",
			vm.getTypeName(step_base));
		return nullptr;
	}

	mpz_t begin, end, step;
	mpz_inits(begin, end, step, NULL);
	if(args.size() > 2) mpz_set(begin, as<VarMPInt>(lhs_base)->getSrcPtr());
	else mpz_set_si(begin, 0);
	if(rhs_base) mpz_set(end, as<VarMPInt>(rhs_base)->getSrcPtr());
	else mpz_set(end, as<VarMPInt>(lhs_base)->getSrcPtr());
	if(step_base) mpz_set(step, as<VarMPInt>(step_base)->getSrcPtr());
	else mpz_set_si(step, 1);
	VarMPIntIterator *res = vm.makeVar<VarMPIntIterator>(loc, begin, end, step);
	mpz_clears(begin, end, step, NULL);
	return res;
}

Var *getMPIntIteratorNext(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
			  const StringMap<AssnArgData> &assn_args)
{
	VarMPIntIterator *it = as<VarMPIntIterator>(args[0]);
	mpz_t _res;
	if(!it->next(_res)) {
		return vm.getNil();
	}
	VarMPInt *res = vm.makeVar<VarMPInt>(loc, _res);
	mpz_clear(_res);
	res->setLoadAsRef();
	return res;
}

// RNG

Var *rngSeedInt(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPInt>()) {
		vm.fail(loc,
			"expected seed value to be a big int, found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	gmp_randseed(rngstate, as<VarMPInt>(args[1])->getPtr());
	return vm.getNil();
}

// [0, to)
Var *rngGetInt(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPInt>()) {
		vm.fail(loc,
			"expected upper bound to be an big int, found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPInt *res = vm.makeVar<VarMPInt>(loc, 0);
	mpz_urandomm(res->getPtr(), rngstate, as<VarMPInt>(args[1])->getPtr());
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Float Functions /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *mpFltNewNative(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		    const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarFlt>() && !args[1]->is<VarStr>() && !args[1]->is<VarMPInt>() &&
	   !args[1]->is<VarMPFlt>())
	{
		vm.fail(loc,
			"argument 1 to bignum.newFlt() must be of "
			"type 'flt', 'str', 'bigint', or 'bigflt', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}

	if(args[1]->is<VarFlt>()) {
		return vm.makeVar<VarMPFlt>(loc, as<VarFlt>(args[1])->getVal());
	}
	if(args[1]->is<VarStr>()) {
		return vm.makeVar<VarMPFlt>(loc, as<VarStr>(args[1])->getVal().c_str());
	}
	if(args[1]->is<VarMPInt>()) {
		return vm.makeVar<VarMPFlt>(loc, as<VarMPInt>(args[1])->getPtr());
	}
	return vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[1])->getSrcPtr());
}

#define ARITHF_FUNC(fn, name, namez)                                                               \
	Var *mpFlt##fn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,                           \
		       const StringMap<AssnArgData> &assn_args)                                    \
	{                                                                                          \
		if(args[1]->is<VarMPInt>()) {                                                      \
			VarMPFlt *res =                                                            \
			vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());                \
			mpfr_##namez(res->getPtr(), res->getSrcPtr(),                              \
				     as<VarMPInt>(args[1])->getPtr(),                              \
				     mpfr_get_default_rounding_mode());                            \
			return res;                                                                \
		} else if(args[1]->is<VarMPFlt>()) {                                               \
			VarMPFlt *res =                                                            \
			vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());                \
			mpfr_##name(res->getPtr(), res->getSrcPtr(),                               \
				    as<VarMPFlt>(args[1])->getSrcPtr(),                            \
				    mpfr_get_default_rounding_mode());                             \
			return res;                                                                \
		}                                                                                  \
		vm.fail(loc, "expected mpInt or mpFlt arg for mpFlt " STRINGIFY(name) ", found: ", \
			vm.getTypeName(args[1]));                                                  \
		return nullptr;                                                                    \
	}

#define ARITHF_ASSN_FUNC(fn, name, namez)                                                          \
	Var *mpFltAssn##fn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,                       \
			   const StringMap<AssnArgData> &assn_args)                                \
	{                                                                                          \
		if(args[1]->is<VarMPInt>()) {                                                      \
			mpfr_##namez(                                                              \
			as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(),       \
			as<VarMPInt>(args[1])->getPtr(), mpfr_get_default_rounding_mode());        \
			return args[0];                                                            \
		} else if(args[1]->is<VarMPFlt>()) {                                               \
			mpfr_##name(                                                               \
			as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(),       \
			as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());     \
			return args[0];                                                            \
		}                                                                                  \
		vm.fail(loc,                                                                       \
			"expected mpInt or mpFlt arg for mpFlt " STRINGIFY(name) "-assn, found: ", \
			vm.getTypeName(args[1]));                                                  \
		return nullptr;                                                                    \
	}

#define LOGICF_FUNC(name, checksym)                                                       \
	Var *mpFlt##name(Interpreter &vm, ModuleLoc loc, Span<Var *> args,                \
			 const StringMap<AssnArgData> &assn_args)                         \
	{                                                                                 \
		if(args[1]->is<VarInt>()) {                                               \
			return mpfr_cmp_si(as<VarMPFlt>(args[0])->getPtr(),               \
					   as<VarInt>(args[1])->getVal()) checksym 0      \
			       ? vm.getTrue()                                             \
			       : vm.getFalse();                                           \
		}                                                                         \
		if(args[1]->is<VarFlt>()) {                                               \
			return mpfr_cmp_ld(as<VarMPFlt>(args[0])->getPtr(),               \
					   as<VarFlt>(args[1])->getVal()) checksym 0      \
			       ? vm.getTrue()                                             \
			       : vm.getFalse();                                           \
		}                                                                         \
		if(args[1]->is<VarMPInt>()) {                                             \
			return mpfr_cmp_z(as<VarMPFlt>(args[0])->getSrcPtr(),             \
					  as<VarMPInt>(args[1])->getSrcPtr()) checksym 0  \
			       ? vm.getTrue()                                             \
			       : vm.getFalse();                                           \
		}                                                                         \
		if(args[1]->is<VarMPFlt>()) {                                             \
			return mpfr_cmp(as<VarMPFlt>(args[0])->getSrcPtr(),               \
					as<VarMPFlt>(args[1])->getSrcPtr()) checksym 0    \
			       ? vm.getTrue()                                             \
			       : vm.getFalse();                                           \
		}                                                                         \
		vm.fail(loc, "expected mpFlt arg for mpFlt " STRINGIFY(name) ", found: ", \
			vm.getTypeName(args[1]));                                         \
		return nullptr;                                                           \
	}

ARITHF_FUNC(Add, add, add_z)
ARITHF_FUNC(Sub, sub, sub_z)
ARITHF_FUNC(Mul, mul, mul_z)
ARITHF_FUNC(Div, div, div_z)

ARITHF_ASSN_FUNC(Add, add, add_z)
ARITHF_ASSN_FUNC(Sub, sub, sub_z)
ARITHF_ASSN_FUNC(Mul, mul, mul_z)
ARITHF_ASSN_FUNC(Div, div, div_z)

LOGICF_FUNC(LT, <)
LOGICF_FUNC(GT, >)
LOGICF_FUNC(LE, <=)
LOGICF_FUNC(GE, >=)

Var *mpFltEQ(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	     const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPFlt>()) return vm.getFalse();
	return mpfr_cmp(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[1])->getPtr()) == 0
	       ? vm.getTrue()
	       : vm.getFalse();
}

Var *mpFltNE(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	     const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPFlt>()) return vm.getTrue();
	return mpfr_cmp(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[1])->getPtr()) != 0
	       ? vm.getTrue()
	       : vm.getFalse();
}

Var *mpFltPreInc(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	mpfr_add_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
		    mpfr_get_default_rounding_mode());
	return args[0];
}

Var *mpFltPostInc(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());
	mpfr_add_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
		    mpfr_get_default_rounding_mode());
	return res;
}

Var *mpFltPreDec(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	mpfr_sub_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
		    mpfr_get_default_rounding_mode());
	return args[0];
}

Var *mpFltPostDec(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());
	mpfr_sub_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
		    mpfr_get_default_rounding_mode());
	return res;
}

Var *mpFltUSub(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());
	mpfr_neg(res->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(),
		 mpfr_get_default_rounding_mode());
	return res;
}

Var *mpFltRound(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		const StringMap<AssnArgData> &assn_args)
{
	VarMPInt *res = vm.makeVar<VarMPInt>(loc, 0);
	mpfr_get_z(res->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), MPFR_RNDN);
	return res;
}

Var *mpFltPow(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	      const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPInt>()) {
		vm.fail(loc, "power must be an integer, found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0);
	mpfr_pow_si(res->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(),
		    mpz_get_si(as<VarMPInt>(args[1])->getPtr()), MPFR_RNDN);
	return res;
}

Var *mpFltRoot(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPInt>()) {
		vm.fail(loc, "root must be an integer, found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0);
#if MPFR_VERSION_MAJOR >= 4
	mpfr_rootn_ui(res->getPtr(), as<VarMPFlt>(args[0])->getPtr(),
		      mpz_get_ui(as<VarMPInt>(args[1])->getPtr()), MPFR_RNDN);
#else
	mpfr_root(res->getPtr(), as<VarMPFlt>(args[0])->getPtr(),
		  mpz_get_ui(INT(args[1])->getPtr()), MPFR_RNDN);
#endif // MPFR_VERSION_MAJOR
	return res;
}

Var *mpFltToFlt(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		const StringMap<AssnArgData> &assn_args)
{
	return vm.makeVar<VarFlt>(loc, mpfr_get_d(as<VarMPFlt>(args[0])->getPtr(), MPFR_RNDN));
}

Var *mpFltToStr(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		const StringMap<AssnArgData> &assn_args)
{
	mpfr_exp_t expo;
	char *_res  = mpfr_get_str(NULL, &expo, 10, 0, as<VarMPFlt>(args[0])->getSrcPtr(),
				   mpfr_get_default_rounding_mode());
	VarStr *res = vm.makeVar<VarStr>(loc, _res);
	mpfr_free_str(_res);
	if(res->getVal().empty() || expo == 0 || expo > 25) return res;
	auto last_zero_from_end = res->getVal().find_last_of("123456789");
	if(last_zero_from_end != String::npos) res->getVal().erase(last_zero_from_end + 1);
	if(expo > 0) {
		size_t sz = res->getVal().size();
		while(expo > sz) {
			res->getVal() += '0';
		}
		if(res->getVal()[0] == '-') ++expo;
		res->getVal().insert(expo, ".");
	} else {
		String pre_zero(-expo, '0');
		pre_zero.insert(pre_zero.begin(), '.');
		pre_zero.insert(pre_zero.begin(), '0');
		pre_zero += res->getVal();
		using namespace std;
		swap(res->getVal(), pre_zero);
	}
	return res;
}

// RNG

// [0.0, to]
Var *rngGetFlt(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
	       const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarMPFlt>()) {
		vm.fail(loc,
			"expected upper bound to be a big float, found: ", vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0.0);
	mpfr_urandom(res->getPtr(), rngstate, MPFR_RNDN);
	mpfr_mul(res->getPtr(), res->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr(), MPFR_RNDN);
	return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Complex Functions ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

Var *mpComplexNewNative(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
			const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarFlt>() && !args[1]->is<VarMPInt>() &&
	   !args[1]->is<VarMPFlt>())
	{
		vm.fail(loc,
			"argument 1 to mp.newComplex() must be of type "
			"'int', 'flt', 'mpInt', or 'mpFlt', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	if(!args[2]->is<VarInt>() && !args[2]->is<VarFlt>() && !args[2]->is<VarInt>() &&
	   !args[2]->is<VarFlt>())
	{
		vm.fail(loc,
			"argument 2 to mp.newComplex() must be of type "
			"'int', 'flt', 'mpInt', or 'mpFlt', found: ",
			vm.getTypeName(args[2]));
		return nullptr;
	}
	if(args[1]->getType() != args[2]->getType()) {
		vm.fail(loc, "the real and imaginary arguments must be of same time, found: (",
			vm.getTypeName(args[1]), ", ", vm.getTypeName(args[1]), ")");
		return nullptr;
	}

	Var *a1		  = args[1];
	Var *a2		  = args[2];
	VarMPComplex *res = nullptr;

	if(a1->is<VarInt>()) {
		res = vm.makeVarWithRef<VarMPComplex>(loc, as<VarInt>(a1)->getVal(),
						      as<VarInt>(a2)->getVal());
	} else if(a1->is<VarFlt>()) {
		res = vm.makeVarWithRef<VarMPComplex>(loc, as<VarFlt>(a1)->getVal(),
						      as<VarFlt>(a2)->getVal());
	} else if(a1->is<VarMPInt>()) {
		res = vm.makeVarWithRef<VarMPComplex>(loc, as<VarMPInt>(a1)->getSrcPtr(),
						      as<VarMPInt>(a2)->getSrcPtr());
	} else if(a1->is<VarMPFlt>()) {
		res = vm.makeVarWithRef<VarMPComplex>(loc, as<VarMPFlt>(a1)->getSrcPtr(),
						      as<VarMPFlt>(a2)->getSrcPtr());
	}

	return res;
}

#define LOGICC_FUNC(name, sym)                                                       \
	Var *mpComplex##name(Interpreter &vm, ModuleLoc loc, Span<Var *> args,       \
			     const StringMap<AssnArgData> &assn_args)                \
	{                                                                            \
		if(args[1]->is<VarMPComplex>()) {                                    \
			return mpc_cmp(as<VarMPComplex>(args[0])->getSrcPtr(),       \
				       as<VarMPComplex>(args[1])->getSrcPtr()) sym 0 \
			       ? vm.getTrue()                                        \
			       : vm.getFalse();                                      \
		}                                                                    \
		return vm.getFalse();                                                \
	}

LOGICC_FUNC(Lt, <)
LOGICC_FUNC(Gt, >)
LOGICC_FUNC(Le, <=)
LOGICC_FUNC(Ge, >=)

Var *mpComplexAdd(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc);
	if(args[1]->is<VarInt>()) {
		mpc_add_si(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarInt>(args[1])->getVal(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_add_fr(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarMPFlt>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_add(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			as<VarMPComplex>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	}
	return res;
}

Var *mpComplexSub(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc);
	if(args[1]->is<VarInt>()) {
		mpc_sub_ui(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarInt>(args[1])->getVal(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_sub_fr(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarMPFlt>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_sub(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			as<VarMPComplex>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	}
	return res;
}

Var *mpComplexMul(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc);
	if(args[1]->is<VarInt>()) {
		mpc_mul_si(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarInt>(args[1])->getVal(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_mul_fr(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarMPFlt>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_mul(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			as<VarMPComplex>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	}
	return res;
}

Var *mpComplexDiv(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc);
	if(args[1]->is<VarInt>()) {
		mpc_div_ui(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarInt>(args[1])->getVal(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_div_fr(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarMPFlt>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_div(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			as<VarMPComplex>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	}
	return res;
}

Var *mpComplexAddAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		      const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *base = as<VarMPComplex>(args[0]);
	if(args[1]->is<VarInt>()) {
		mpc_add_si(base->getPtr(), base->getSrcPtr(), as<VarInt>(args[1])->getVal(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_add_fr(base->getPtr(), base->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_add(base->getPtr(), base->getSrcPtr(), as<VarMPComplex>(args[1])->getSrcPtr(),
			mpc_get_default_rounding_mode());
	}
	return base;
}

Var *mpComplexSubAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		      const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *base = as<VarMPComplex>(args[0]);
	if(args[1]->is<VarInt>()) {
		mpc_sub_ui(base->getPtr(), base->getSrcPtr(), as<VarInt>(args[1])->getVal(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_sub_fr(base->getPtr(), base->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_sub(base->getPtr(), base->getSrcPtr(), as<VarMPComplex>(args[1])->getSrcPtr(),
			mpc_get_default_rounding_mode());
	}
	return base;
}

Var *mpComplexMulAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		      const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *base = as<VarMPComplex>(args[0]);
	if(args[1]->is<VarInt>()) {
		mpc_mul_si(base->getPtr(), base->getSrcPtr(), as<VarInt>(args[1])->getVal(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_mul_fr(base->getPtr(), base->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_mul(base->getPtr(), base->getSrcPtr(), as<VarMPComplex>(args[1])->getSrcPtr(),
			mpc_get_default_rounding_mode());
	}
	return base;
}

Var *mpComplexDivAssn(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		      const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarMPFlt>() && !args[1]->is<VarMPComplex>()) {
		vm.fail(loc,
			"argument 1 to complex arithmetic must be of "
			"type 'int', 'mpFlt', or 'complex', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *base = as<VarMPComplex>(args[0]);
	if(args[1]->is<VarInt>()) {
		mpc_div_ui(base->getPtr(), base->getSrcPtr(), as<VarInt>(args[1])->getVal(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPFlt>()) {
		mpc_div_fr(base->getPtr(), base->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr(),
			   mpc_get_default_rounding_mode());
	} else if(args[1]->is<VarMPComplex>()) {
		mpc_div(base->getPtr(), base->getSrcPtr(), as<VarMPComplex>(args[1])->getSrcPtr(),
			mpc_get_default_rounding_mode());
	}
	return base;
}

Var *mpComplexEq(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPComplex>()) {
		return mpc_cmp(as<VarMPComplex>(args[0])->getSrcPtr(),
			       as<VarMPComplex>(args[1])->getSrcPtr()) == 0
		       ? vm.getTrue()
		       : vm.getFalse();
	}
	return vm.getFalse();
}

Var *mpComplexNe(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		 const StringMap<AssnArgData> &assn_args)
{
	if(args[1]->is<VarMPComplex>()) {
		return mpc_cmp(as<VarMPComplex>(args[0])->getSrcPtr(),
			       as<VarMPComplex>(args[1])->getSrcPtr()) != 0
		       ? vm.getTrue()
		       : vm.getFalse();
	}
	return vm.getTrue();
}

Var *mpComplexPreInc(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		     const StringMap<AssnArgData> &assn_args)
{
	mpc_add_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
		   mpc_get_default_rounding_mode());
	return args[0];
}

Var *mpComplexPostInc(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		      const StringMap<AssnArgData> &assn_args)
{
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc, as<VarMPComplex>(args[0])->getSrcPtr());
	mpc_add_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
		   mpc_get_default_rounding_mode());
	return res;
}

Var *mpComplexPreDec(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		     const StringMap<AssnArgData> &assn_args)
{
	mpc_sub_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
		   mpc_get_default_rounding_mode());
	return args[0];
}

Var *mpComplexPostDec(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		      const StringMap<AssnArgData> &assn_args)
{
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc, as<VarMPComplex>(args[0])->getSrcPtr());
	mpc_sub_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
		   mpc_get_default_rounding_mode());
	return res;
}

Var *mpComplexUSub(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		   const StringMap<AssnArgData> &assn_args)
{
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc, as<VarMPComplex>(args[0])->getSrcPtr());
	mpc_neg(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
		mpc_get_default_rounding_mode());
	return res;
}

Var *mpComplexPow(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarFlt>() && !args[1]->is<VarMPInt>() &&
	   !args[1]->is<VarMPFlt>())
	{
		vm.fail(loc, "power must be an 'int' 'flt', 'mpInt' or 'mpFlt', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	VarMPComplex *res = vm.makeVar<VarMPComplex>(loc, as<VarMPComplex>(args[0])->getSrcPtr());
	if(args[1]->is<VarInt>())
		mpc_pow_si(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarInt>(args[1])->getVal(), mpc_get_default_rounding_mode());
	if(args[1]->is<VarFlt>())
		mpc_pow_ld(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarFlt>(args[1])->getVal(), mpc_get_default_rounding_mode());
	if(args[1]->is<VarMPInt>())
		mpc_pow_z(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			  as<VarMPInt>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	else if(args[1]->is<VarMPFlt>())
		mpc_pow_fr(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
			   as<VarMPFlt>(args[1])->getSrcPtr(), mpc_get_default_rounding_mode());
	return res;
}

Var *mpComplexAbs(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0.0);
	mpc_abs(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
		mpfr_get_default_rounding_mode());
	return res;
}

Var *mpComplexSet(Interpreter &vm, ModuleLoc loc, Span<Var *> args,
		  const StringMap<AssnArgData> &assn_args)
{
	if(!args[1]->is<VarInt>() && !args[1]->is<VarFlt>() && !args[1]->is<VarMPInt>() &&
	   !args[1]->is<VarMPFlt>())
	{
		vm.fail(loc,
			"argument 1 to mp.newComplex() must be of type "
			"'int', 'flt', 'mpInt', or 'mpFlt', found: ",
			vm.getTypeName(args[1]));
		return nullptr;
	}
	if(!args[2]->is<VarInt>() && !args[2]->is<VarFlt>() && !args[2]->is<VarInt>() &&
	   !args[2]->is<VarFlt>())
	{
		vm.fail(loc,
			"argument 2 to mp.newComplex() must be of type "
			"'int', 'flt', 'mpInt', or 'mpFlt', found: ",
			vm.getTypeName(args[2]));
		return nullptr;
	}
	if(args[1]->getType() != args[2]->getType()) {
		vm.fail(loc, "the real and imaginary arguments must be of same time, found: (",
			vm.getTypeName(args[1]), ", ", vm.getTypeName(args[1]), ")");
		return nullptr;
	}

	Var *a1		  = args[1];
	Var *a2		  = args[2];
	VarMPComplex *res = as<VarMPComplex>(args[0]);

	if(a1->is<VarInt>()) {
		mpc_set_si_si(res->getPtr(), as<VarInt>(a1)->getVal(), as<VarInt>(a2)->getVal(),
			      mpc_get_default_rounding_mode());
	} else if(a1->is<VarFlt>()) {
		mpc_set_ld_ld(res->getPtr(), as<VarFlt>(a1)->getVal(), as<VarFlt>(a2)->getVal(),
			      mpc_get_default_rounding_mode());
	} else if(a1->is<VarMPInt>()) {
		mpc_set_z_z(res->getPtr(), as<VarMPInt>(a1)->getSrcPtr(),
			    as<VarMPInt>(a2)->getSrcPtr(), mpc_get_default_rounding_mode());
	} else if(a1->is<VarMPFlt>()) {
		mpc_set_fr_fr(res->getPtr(), as<VarMPFlt>(a1)->getSrcPtr(),
			      as<VarMPFlt>(a2)->getSrcPtr(), mpc_get_default_rounding_mode());
	}

	return res;
}

INIT_MODULE(MP)
{
	gmp_randinit_default(rngstate);

	VarModule *mod = vm.getCurrModule();

	mod->addNativeFn(vm, "seed", rngSeed, 1);

	mod->addNativeFn(vm, "newIntNative", mpIntNewNative, 1);
	mod->addNativeFn(vm, "newFltNative", mpFltNewNative, 1);
	mod->addNativeFn(vm, "newComplexNative", mpComplexNewNative, 2);

	mod->addNativeFn(vm, "irange", mpIntRange, 1, true);
	mod->addNativeFn(vm, "getRandomIntNative", rngGetInt, 1);
	mod->addNativeFn(vm, "getRandomFltNative", rngGetFlt, 1);

	// Register the MPInt, MPFlt, MPComplex, and MPIntIterator types

	vm.registerType<VarMPInt>(loc, "MPInt");
	vm.registerType<VarMPFlt>(loc, "MPFlt");
	vm.registerType<VarMPComplex>(loc, "MPComplex");
	vm.registerType<VarMPIntIterator>(loc, "MPIntIterator");

	// MPInt functions

	vm.addNativeTypeFn<VarMPInt>(loc, "+", mpIntAdd, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "-", mpIntSub, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "*", mpIntMul, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "/", mpIntDiv, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "%", mpIntMod, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "<<", mpIntLShift, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, ">>", mpIntRShift, 1);

	vm.addNativeTypeFn<VarMPInt>(loc, "+=", mpIntAssnAdd, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "-=", mpIntAssnSub, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "*=", mpIntAssnMul, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "/=", mpIntAssnDiv, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "%=", mpIntAssnMod, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "<<=", mpIntLShiftAssn, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, ">>=", mpIntRShiftAssn, 1);

	vm.addNativeTypeFn<VarMPInt>(loc, "**", mpIntPow, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "//", mpIntRoot, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "++x", mpIntPreInc, 0);
	vm.addNativeTypeFn<VarMPInt>(loc, "x++", mpIntPostInc, 0);
	vm.addNativeTypeFn<VarMPInt>(loc, "--x", mpIntPreDec, 0);
	vm.addNativeTypeFn<VarMPInt>(loc, "x--", mpIntPostDec, 0);

	vm.addNativeTypeFn<VarMPInt>(loc, "u-", mpIntUSub, 0);

	vm.addNativeTypeFn<VarMPInt>(loc, "<", mpIntLT, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, ">", mpIntGT, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "<=", mpIntLE, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, ">=", mpIntGE, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "==", mpIntEq, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "!=", mpIntNe, 1);

	vm.addNativeTypeFn<VarMPInt>(loc, "&", mpIntBAnd, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "|", mpIntBOr, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "^", mpIntBXOr, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "~", mpIntBNot, 0);

	vm.addNativeTypeFn<VarMPInt>(loc, "&=", mpIntBAndAssn, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "|=", mpIntBOrAssn, 1);
	vm.addNativeTypeFn<VarMPInt>(loc, "^=", mpIntBXOrAssn, 1);

	vm.addNativeTypeFn<VarMPInt>(loc, "popcnt", mpIntPopCnt, 0);

	vm.addNativeTypeFn<VarMPInt>(loc, "int", mpIntToInt, 0);
	vm.addNativeTypeFn<VarMPInt>(loc, "str", mpIntToStr, 0);
	vm.addNativeTypeFn<VarMPIntIterator>(loc, "next", getMPIntIteratorNext, 0);

	// MPFloat functions

	vm.addNativeTypeFn<VarMPFlt>(loc, "+", mpFltAdd, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "-", mpFltSub, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "*", mpFltMul, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "/", mpFltDiv, 1);

	vm.addNativeTypeFn<VarMPFlt>(loc, "+=", mpFltAssnAdd, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "-=", mpFltAssnSub, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "*=", mpFltAssnMul, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "/=", mpFltAssnDiv, 1);

	vm.addNativeTypeFn<VarMPFlt>(loc, "++x", mpFltPreInc, 0);
	vm.addNativeTypeFn<VarMPFlt>(loc, "x++", mpFltPostInc, 0);
	vm.addNativeTypeFn<VarMPFlt>(loc, "--x", mpFltPreDec, 0);
	vm.addNativeTypeFn<VarMPFlt>(loc, "x--", mpFltPostDec, 0);

	vm.addNativeTypeFn<VarMPFlt>(loc, "u-", mpFltUSub, 0);

	vm.addNativeTypeFn<VarMPFlt>(loc, "round", mpFltRound, 0);

	vm.addNativeTypeFn<VarMPFlt>(loc, "**", mpFltPow, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "//", mpFltRoot, 1);

	vm.addNativeTypeFn<VarMPFlt>(loc, "<", mpFltLT, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, ">", mpFltGT, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "<=", mpFltLE, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, ">=", mpFltGE, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "==", mpFltEQ, 1);
	vm.addNativeTypeFn<VarMPFlt>(loc, "!=", mpFltNE, 1);

	vm.addNativeTypeFn<VarMPFlt>(loc, "flt", mpFltToFlt, 0);
	vm.addNativeTypeFn<VarMPFlt>(loc, "str", mpFltToStr, 0);

	// MPComplex functions

	vm.addNativeTypeFn<VarMPComplex>(loc, "+", mpComplexAdd, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "-", mpComplexSub, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "*", mpComplexMul, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "/", mpComplexDiv, 1);

	vm.addNativeTypeFn<VarMPComplex>(loc, "+=", mpComplexAddAssn, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "-=", mpComplexSubAssn, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "*=", mpComplexMulAssn, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "/=", mpComplexDivAssn, 1);

	vm.addNativeTypeFn<VarMPComplex>(loc, "==", mpComplexEq, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "!=", mpComplexNe, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "<", mpComplexLt, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, "<=", mpComplexLe, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, ">", mpComplexGt, 1);
	vm.addNativeTypeFn<VarMPComplex>(loc, ">=", mpComplexGe, 1);

	vm.addNativeTypeFn<VarMPComplex>(loc, "++x", mpComplexPreInc, 0);
	vm.addNativeTypeFn<VarMPComplex>(loc, "x++", mpComplexPostInc, 0);
	vm.addNativeTypeFn<VarMPComplex>(loc, "--x", mpComplexPreDec, 0);
	vm.addNativeTypeFn<VarMPComplex>(loc, "x--", mpComplexPostDec, 0);

	vm.addNativeTypeFn<VarMPComplex>(loc, "u-", mpComplexUSub, 0);

	vm.addNativeTypeFn<VarMPComplex>(loc, "**", mpComplexPow, 1);

	vm.addNativeTypeFn<VarMPComplex>(loc, "abs", mpComplexAbs, 0);
	vm.addNativeTypeFn<VarMPComplex>(loc, "set", mpComplexSet, 2);

	return true;
}

DEINIT_MODULE(MP) { gmp_randclear(rngstate); }

} // namespace fer