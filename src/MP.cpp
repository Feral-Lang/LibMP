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
Var *VarMPInt::onCopy(MemoryManager &mem, ModuleLoc loc)
{
    return Var::makeVarWithRef<VarMPInt>(mem, loc, val);
}
void VarMPInt::onSet(MemoryManager &mem, Var *from) { mpz_set(val, as<VarMPInt>(from)->getPtr()); }

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// VarMPFlt /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

VarMPFlt::VarMPFlt(ModuleLoc loc, double _val) : Var(loc, false, false)
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
Var *VarMPFlt::onCopy(MemoryManager &mem, ModuleLoc loc)
{
    return Var::makeVarWithRef<VarMPFlt>(mem, loc, val);
}
void VarMPFlt::onSet(MemoryManager &mem, Var *from)
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
VarMPComplex::VarMPComplex(ModuleLoc loc, double real, double imag) : Var(loc, false, false)
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

Var *VarMPComplex::onCopy(MemoryManager &mem, ModuleLoc loc)
{
    return Var::makeVarWithRef<VarMPComplex>(mem, loc, val);
}
void VarMPComplex::onSet(MemoryManager &mem, Var *from)
{
    mpc_set(val, as<VarMPComplex>(from)->getPtr(), mpc_get_default_rounding_mode());
}

mpc_rnd_t mpc_get_default_rounding_mode() { return MPC_RNDNN; }

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

FERAL_FUNC(rngSeed, 1, false,
           "  fn(seed) -> Nil\n"
           "Provides a `seed` number to the random number generator.")
{
    EXPECT(VarMPInt, args[1], "seed value");
    gmp_randseed(rngstate, as<VarMPInt>(args[1])->getPtr());
    return vm.getNil();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// Int Functions //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

FERAL_FUNC(mpIntNewNative, 1, false,
           "  fn(value) -> MPInt\n"
           "Creates and returns a new MPInt with `value`.\n"
           "Here `value` can be any of Int / Str / MPInt / MPFlt")
{
    EXPECT4(VarInt, VarStr, VarMPInt, VarMPFlt, args[1], "initial value");
    if(args[1]->is<VarInt>()) { return vm.makeVar<VarMPInt>(loc, as<VarInt>(args[1])->getVal()); }
    if(args[1]->is<VarStr>()) {
        return vm.makeVar<VarMPInt>(loc, as<VarStr>(args[1])->getVal().c_str());
    }
    if(args[1]->is<VarMPInt>()) {
        return vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[1])->getPtr());
    }
    return vm.makeVar<VarMPInt>(loc, as<VarMPFlt>(args[1])->getSrcPtr());
}

#define ARITHI_FUNC(fn, name)                                                                  \
    FERAL_FUNC(mpInt##fn, 1, false,                                                            \
               "  var.fn(other) -> MPInt\n"                                                    \
               "Applies arithmetic-" STRINGIFY(                                                \
                   name) " on `var` and `other` and returns a new MPInt with the result.")     \
    {                                                                                          \
        EXPECT2(VarMPInt, VarMPFlt, args[1], "big int " STRINGIFY(name));                      \
        if(args[1]->is<VarMPInt>()) {                                                          \
            VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());     \
            mpz_##name(res->getPtr(), res->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr());   \
            return res;                                                                        \
        }                                                                                      \
        VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());         \
        mpz_t tmp;                                                                             \
        mpz_init(tmp);                                                                         \
        mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode()); \
        mpz_##name(res->getPtr(), res->getSrcPtr(), tmp);                                      \
        mpz_clear(tmp);                                                                        \
        return res;                                                                            \
    }

#define ARITHI_ASSN_FUNC(fn, name)                                                             \
    FERAL_FUNC(mpIntAssn##fn, 1, false,                                                        \
               "  var.fn(other) -> var\n"                                                      \
               "Applies arithmetic-" STRINGIFY(                                                \
                   name) " on `var` with `other` and returns the updated `var`.")              \
    {                                                                                          \
        EXPECT_NO_CONST(args[0], "var");                                                       \
        EXPECT2(VarMPInt, VarMPFlt, args[1], "big int " STRINGIFY(name) "-assn");              \
        if(args[1]->is<VarMPInt>()) {                                                          \
            mpz_##name(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),    \
                       as<VarMPInt>(args[1])->getSrcPtr());                                    \
            return args[0];                                                                    \
        }                                                                                      \
        mpz_t tmp;                                                                             \
        mpz_init(tmp);                                                                         \
        mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode()); \
        mpz_##name(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), tmp);  \
        mpz_clear(tmp);                                                                        \
        return args[0];                                                                        \
    }

#define LOGICI_FUNC(fn, name, sym)                                                             \
    FERAL_FUNC(mpInt##fn, 1, false,                                                            \
               "  var.fn(other) -> Bool\n"                                                     \
               "Applies logical '" STRINGIFY(                                                  \
                   name) "' between `var` and `other` and returns the resulting Bool.")        \
    {                                                                                          \
        EXPECT(VarMPInt, args[1], "big int logical " STRINGIFY(name));                         \
        return mpz_cmp(as<VarMPInt>(args[0])->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr()) \
                       sym 0                                                                   \
                   ? vm.getTrue()                                                              \
                   : vm.getFalse();                                                            \
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

FERAL_FUNC(mpIntEQ, 1, false,
           "  var.fn(other) -> Bool\n"
           "Returns `true` if `var` and `other` are equal.")
{
    if(args[1]->is<VarMPInt>()) {
        return mpz_cmp(as<VarMPInt>(args[0])->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr()) == 0
                   ? vm.getTrue()
                   : vm.getFalse();
    }
    return vm.getFalse();
}

FERAL_FUNC(mpIntNE, 1, false,
           "  var.fn(other) -> Bool\n"
           "Returns `true` if `var` and `other` are not equal.")
{
    if(args[1]->is<VarMPInt>()) {
        return mpz_cmp(as<VarMPInt>(args[0])->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr()) != 0
                   ? vm.getTrue()
                   : vm.getFalse();
    }
    return vm.getTrue();
}

FERAL_FUNC(mpIntDiv, 1, false,
           "  var.fn(other) -> MPInt\n"
           "Divides `var` by `other` and returns a new MPInt with the result.")
{
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int division");
    if(args[1]->is<VarMPInt>()) {
        // rhs == 0
        if(mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()) == 0) {
            vm.fail(loc, "division by zero");
            return nullptr;
        }
        VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
        mpz_div(res->getPtr(), res->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr());
        return res;
    }
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
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

FERAL_FUNC(mpIntAssnDiv, 1, false,
           "  var.fn(other) -> var\n"
           "Divides `var` by `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int division");
    if(args[1]->is<VarMPInt>()) {
        // rhs == 0
        if(mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()) == 0) {
            vm.fail(loc, "division by zero");
            return nullptr;
        }
        mpz_div(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
                as<VarMPInt>(args[1])->getSrcPtr());
        return args[0];
    }
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
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

FERAL_FUNC(mpIntBAnd, 1, false,
           "  var.fn(other) -> MPInt\n"
           "Applies bitwise AND operation between `var` and `other` and returns a new MPInt with "
           "the result.")
{
    EXPECT(VarMPInt, args[1], "big int bitwise AND");
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_and(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr());
    return res;
}

FERAL_FUNC(mpIntBOr, 1, false,
           "  var.fn(other) -> MPInt\n"
           "Applies bitwise OR operation between `var` and `other` and returns a new MPInt with "
           "the result.")
{
    EXPECT(VarMPInt, args[1], "big int bitwise OR");
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_ior(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr());
    return res;
}

FERAL_FUNC(mpIntBXOr, 1, false,
           "  var.fn(other) -> MPInt\n"
           "Applies bitwise XOR operation between `var` and `other` and returns a new MPInt with "
           "the result.")
{
    EXPECT(VarMPInt, args[1], "big int bitwise XOR");
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_xor(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), as<VarMPInt>(args[1])->getSrcPtr());
    return res;
}

FERAL_FUNC(mpIntBNot, 0, false,
           "  var.fn() -> MPInt\n"
           "Applies bitwise NOT operation on `var` and returns a new MPInt with the result.")
{
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_com(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr());
    return res;
}

FERAL_FUNC(mpIntAssnBAnd, 1, false,
           "  var.fn(other) -> var\n"
           "Applies bitwise AND operation between `var` and `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT(VarMPInt, args[1], "big int bitwise AND-assn");
    mpz_and(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
            as<VarMPInt>(args[1])->getSrcPtr());
    return args[0];
}

FERAL_FUNC(mpIntAssnBOr, 1, false,
           "  var.fn(other) -> var\n"
           "Applies bitwise OR operation between `var` and `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT(VarMPInt, args[1], "big int bitwise OR-assn");
    mpz_ior(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
            as<VarMPInt>(args[1])->getSrcPtr());
    return args[0];
}

FERAL_FUNC(mpIntAssnBXOr, 1, false,
           "  var.fn(other) -> var\n"
           "Applies bitwise XOR operation between `var` and `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT(VarMPInt, args[1], "big int bitwise XOR-assn");
    mpz_xor(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
            as<VarMPInt>(args[1])->getSrcPtr());
    return args[0];
}

FERAL_FUNC(mpIntAssnBNot, 0, false,
           "  var.fn(other) -> var\n"
           "Applies bitwise NOT operation on `var` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    mpz_com(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr());
    return args[0];
}

FERAL_FUNC(
    mpIntLShift, 1, false,
    "  var.fn(other) -> MPInt\n"
    "Applies left shift operation on `var` using `other` and returns a new MPInt with the result.")
{
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int left-shift");
    if(args[1]->is<VarMPInt>()) {
        VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
        mpz_mul_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
                     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
        return res;
    }
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
    mpz_mul_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), mpz_get_si(tmp));
    mpz_clear(tmp);
    return res;
}

FERAL_FUNC(
    mpIntRShift, 1, false,
    "  var.fn(other) -> MPInt\n"
    "Applies right shift operation on `var` using `other` and returns a new MPInt with the result.")
{
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int right-shift");
    if(args[1]->is<VarMPInt>()) {
        VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
        mpz_div_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
                     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
        return res;
    }
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
    mpz_div_2exp(res->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), mpz_get_si(tmp));
    mpz_clear(tmp);
    return res;
}

FERAL_FUNC(mpIntAssnLShift, 1, false,
           "  var.fn(other) -> var\n"
           "Applies left shift operation on `var` using `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int left-shift-assn");
    if(args[1]->is<VarMPInt>()) {
        mpz_mul_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
                     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
        return args[0];
    }
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
    mpz_mul_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
                 mpz_get_si(tmp));
    mpz_clear(tmp);
    return args[0];
}

FERAL_FUNC(mpIntAssnRShift, 1, false,
           "  var.fn(other) -> var\n"
           "Applies right shift operation on `var` using `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int right-shift-assn");
    if(args[1]->is<VarMPInt>()) {
        mpz_div_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
                     mpz_get_si(as<VarMPInt>(args[1])->getSrcPtr()));
        return args[0];
    }
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
    mpz_div_2exp(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(),
                 mpz_get_si(tmp));
    mpz_clear(tmp);
    return args[0];
}

FERAL_FUNC(mpIntPreInc, 0, false,
           "  var.fn() -> var\n"
           "Applies pre-increment on `var` and returns `var` itself.")
{
    mpz_add_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
    return args[0];
}

FERAL_FUNC(mpIntPostInc, 0, false,
           "  var.fn() -> MPInt\n"
           "Applies post-increment on `var` and returns a new MPInt with `var` - 1 as the result.")
{
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_add_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
    return res;
}

FERAL_FUNC(mpIntPreDec, 0, false,
           "  var.fn() -> var\n"
           "Applies pre-decrement on `var` and returns `var` itself.")
{
    mpz_sub_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
    return args[0];
}

FERAL_FUNC(mpIntPostDec, 0, false,
           "  var.fn() -> MPInt\n"
           "Applies post-decrement on `var` and returns a new MPInt with `var` + 1 as the result.")
{
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_sub_ui(as<VarMPInt>(args[0])->getPtr(), as<VarMPInt>(args[0])->getSrcPtr(), 1);
    return res;
}

FERAL_FUNC(mpIntUSub, 0, false,
           "  var.fn() -> MPInt\n"
           "Returns the negative equivalent of `var` as a new MPInt.")
{
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_neg(res->getPtr(), res->getSrcPtr());
    return res;
}

FERAL_FUNC(mpIntPow, 1, false,
           "  var.fn(other) -> MPInt\n"
           "Raises `var` to the power of `other` and returns a new MPInt with the result.")
{
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int power");
    if(args[1]->is<VarMPInt>()) {
        VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
        mpz_pow_ui(res->getPtr(), res->getSrcPtr(), mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()));
        return res;
    }
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
    mpz_pow_ui(res->getPtr(), res->getSrcPtr(), mpz_get_ui(tmp));
    mpz_clear(tmp);
    return res;
}

FERAL_FUNC(mpIntRoot, 1, false,
           "  var.fn(other) -> MPInt\n"
           "Lowers `var` to the root of `other` and returns a new MPInt with the result.")
{
    EXPECT2(VarMPInt, VarMPFlt, args[1], "big int root");
    if(args[1]->is<VarMPInt>()) {
        VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
        mpz_root(res->getPtr(), res->getSrcPtr(), mpz_get_ui(as<VarMPInt>(args[1])->getSrcPtr()));
        return res;
    }
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, as<VarMPInt>(args[0])->getSrcPtr());
    mpz_t tmp;
    mpz_init(tmp);
    mpfr_get_z(tmp, as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());
    mpz_root(res->getPtr(), res->getSrcPtr(), mpz_get_ui(tmp));
    mpz_clear(tmp);
    return res;
}

FERAL_FUNC(mpIntPopCnt, 0, false,
           "  var.fn() -> MPInt\n"
           "Returns the number of set bits in `var` as a new MPInt.")
{
    return vm.makeVar<VarMPInt>(loc, mpz_popcount(as<VarMPInt>(args[0])->getSrcPtr()));
}

FERAL_FUNC(mpIntToInt, 0, false,
           "  var.fn() -> Int\n"
           "Converts `var` from MPInt to Int and returns the value.")
{
    return vm.makeVar<VarMPInt>(loc, mpz_get_si(as<VarMPInt>(args[0])->getPtr()));
}

FERAL_FUNC(mpIntToStr, 0, false,
           "  var.fn() -> Str\n"
           "Converts `var` from MPInt to Str and returns the value.")
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
    started  = f->started;
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

FERAL_FUNC(mpIntRange, 1, true,
           "  fn(start, end, step) -> MPIntIterator\n"
           "Creates an iterator which starts at `start`, ends at `end` (exclusive), and "
           "increments/decrements by `step`.\n"
           "If `end` and `step` are not provided, `start` becomes 0, `end` becomes the provided "
           "`start`, and `step` becomes 1.")
{
    Var *lhsBase  = args[1];
    Var *rhsBase  = args.size() > 2 ? args[2] : nullptr;
    Var *stepBase = args.size() > 3 ? args[3] : nullptr;

    EXPECT(VarMPInt, lhsBase, "range start");
    if(rhsBase) { EXPECT(VarMPInt, rhsBase, "range end"); }
    if(stepBase) { EXPECT(VarMPInt, stepBase, "range step"); }

    mpz_t begin, end, step;
    mpz_inits(begin, end, step, NULL);
    if(args.size() > 2) mpz_set(begin, as<VarMPInt>(lhsBase)->getSrcPtr());
    else mpz_set_si(begin, 0);
    if(rhsBase) mpz_set(end, as<VarMPInt>(rhsBase)->getSrcPtr());
    else mpz_set(end, as<VarMPInt>(lhsBase)->getSrcPtr());
    if(stepBase) mpz_set(step, as<VarMPInt>(stepBase)->getSrcPtr());
    else mpz_set_si(step, 1);
    VarMPIntIterator *res = vm.makeVar<VarMPIntIterator>(loc, begin, end, step);
    mpz_clears(begin, end, step, NULL);
    return res;
}

FERAL_FUNC(getMPIntIteratorNext, 0, false,
           "  var.fn() -> MPInt\n"
           "Fetch the next MPInt from the MPIntIterator `var`.\n"
           "This function is mainly used by for-in loop.")
{
    VarMPIntIterator *it = as<VarMPIntIterator>(args[0]);
    mpz_t _res;
    if(!it->next(_res)) { return vm.getNil(); }
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, _res);
    mpz_clear(_res);
    res->setLoadAsRef();
    return res;
}

// RNG

FERAL_FUNC(mpIntRngGet, 1, false,
           "  fn(upto) -> MPInt\n"
           "Returns a random number between [0, `upto`).")
{
    EXPECT(VarMPInt, args[1], "upper limit");
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, 0);
    mpz_urandomm(res->getPtr(), rngstate, as<VarMPInt>(args[1])->getPtr());
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Float Functions /////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

FERAL_FUNC(mpFltNewNative, 1, false,
           "  fn(value) -> MPFlt\n"
           "Creates and returns a new MPFlt with `value`.\n"
           "Here `value` can be any of Flt / Str / MPInt / MPFlt")
{
    EXPECT4(VarFlt, VarStr, VarMPInt, VarMPFlt, args[1], "initial value");
    if(args[1]->is<VarFlt>()) { return vm.makeVar<VarMPFlt>(loc, as<VarFlt>(args[1])->getVal()); }
    if(args[1]->is<VarStr>()) {
        return vm.makeVar<VarMPFlt>(loc, as<VarStr>(args[1])->getVal().c_str());
    }
    if(args[1]->is<VarMPInt>()) {
        return vm.makeVar<VarMPFlt>(loc, as<VarMPInt>(args[1])->getPtr());
    }
    return vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[1])->getSrcPtr());
}

#define ARITHF_FUNC(fn, name, namez)                                                       \
    FERAL_FUNC(mpFlt##fn, 1, false,                                                        \
               "  var.fn(other) -> MPFlt\n"                                                \
               "Applies arithmetic-" STRINGIFY(                                            \
                   name) " on `var` and `other` and returns a new MPFlt with the result.") \
    {                                                                                      \
        EXPECT2(VarMPInt, VarMPFlt, args[1], "big float " STRINGIFY(name));                \
        if(args[1]->is<VarMPInt>()) {                                                      \
            VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());    \
            mpfr_##namez(res->getPtr(), res->getSrcPtr(), as<VarMPInt>(args[1])->getPtr(), \
                         mpfr_get_default_rounding_mode());                                \
            return res;                                                                    \
        }                                                                                  \
        VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());        \
        mpfr_##name(res->getPtr(), res->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr(),   \
                    mpfr_get_default_rounding_mode());                                     \
        return res;                                                                        \
    }

#define ARITHF_ASSN_FUNC(fn, name, namez)                                                     \
    FERAL_FUNC(mpFltAssn##fn, 1, false,                                                       \
               "  var.fn(other) -> var\n"                                                     \
               "Applies arithmetic-" STRINGIFY(                                               \
                   name) " on `var` with `other` and returns the updated `var`.")             \
    {                                                                                         \
        EXPECT_NO_CONST(args[0], "var");                                                      \
        EXPECT2(VarMPInt, VarMPFlt, args[1], "big float " STRINGIFY(name) "-assn");           \
        if(args[1]->is<VarMPInt>()) {                                                         \
            mpfr_##namez(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), \
                         as<VarMPInt>(args[1])->getPtr(), mpfr_get_default_rounding_mode());  \
            return args[0];                                                                   \
        }                                                                                     \
        mpfr_##name(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(),      \
                    as<VarMPFlt>(args[1])->getSrcPtr(), mpfr_get_default_rounding_mode());    \
        return args[0];                                                                       \
    }

#define LOGICF_FUNC(fn, name, checksym)                                                         \
    FERAL_FUNC(mpFlt##fn, 1, false,                                                             \
               "  var.fn(other) -> Bool\n"                                                      \
               "Applies logical '" STRINGIFY(                                                   \
                   name) "' between `var` and `other` and returns the resulting Bool.")         \
    {                                                                                           \
        EXPECT4(VarInt, VarFlt, VarMPInt, VarMPFlt, args[1],                                    \
                "big float logical " STRINGIFY(name));                                          \
        if(args[1]->is<VarInt>()) {                                                             \
            return mpfr_cmp_si(as<VarMPFlt>(args[0])->getPtr(), as<VarInt>(args[1])->getVal())  \
                           checksym 0                                                           \
                       ? vm.getTrue()                                                           \
                       : vm.getFalse();                                                         \
        } else if(args[1]->is<VarFlt>()) {                                                      \
            return mpfr_cmp_ld(as<VarMPFlt>(args[0])->getPtr(), as<VarFlt>(args[1])->getVal())  \
                           checksym 0                                                           \
                       ? vm.getTrue()                                                           \
                       : vm.getFalse();                                                         \
        } else if(args[1]->is<VarMPInt>()) {                                                    \
            return mpfr_cmp_z(as<VarMPFlt>(args[0])->getSrcPtr(),                               \
                              as<VarMPInt>(args[1])->getSrcPtr()) checksym 0                    \
                       ? vm.getTrue()                                                           \
                       : vm.getFalse();                                                         \
        }                                                                                       \
        return mpfr_cmp(as<VarMPFlt>(args[0])->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr()) \
                       checksym 0                                                               \
                   ? vm.getTrue()                                                               \
                   : vm.getFalse();                                                             \
    }

ARITHF_FUNC(Add, add, add_z)
ARITHF_FUNC(Sub, sub, sub_z)
ARITHF_FUNC(Mul, mul, mul_z)
ARITHF_FUNC(Div, div, div_z)

ARITHF_ASSN_FUNC(Add, add, add_z)
ARITHF_ASSN_FUNC(Sub, sub, sub_z)
ARITHF_ASSN_FUNC(Mul, mul, mul_z)
ARITHF_ASSN_FUNC(Div, div, div_z)

LOGICF_FUNC(LT, lt, <)
LOGICF_FUNC(GT, gt, >)
LOGICF_FUNC(LE, le, <=)
LOGICF_FUNC(GE, ge, >=)

FERAL_FUNC(mpFltEQ, 1, false,
           "  var.fn(other) -> Bool\n"
           "Returns `true` if `var` and `other` are equal.")
{
    if(!args[1]->is<VarMPFlt>()) return vm.getFalse();
    return mpfr_cmp(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[1])->getPtr()) == 0
               ? vm.getTrue()
               : vm.getFalse();
}

FERAL_FUNC(mpFltNE, 1, false,
           "  var.fn(other) -> Bool\n"
           "Returns `true` if `var` and `other` are not equal.")
{
    if(!args[1]->is<VarMPFlt>()) return vm.getTrue();
    return mpfr_cmp(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[1])->getPtr()) != 0
               ? vm.getTrue()
               : vm.getFalse();
}

FERAL_FUNC(mpFltPreInc, 0, false,
           "  var.fn() -> var\n"
           "Applies pre-increment on `var` and returns `var` itself.")
{
    mpfr_add_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
                mpfr_get_default_rounding_mode());
    return args[0];
}

FERAL_FUNC(mpFltPostInc, 0, false,
           "  var.fn() -> MPFlt\n"
           "Applies post-increment on `var` and returns a new MPFlt with `var` - 1 as the result.")
{
    VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());
    mpfr_add_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
                mpfr_get_default_rounding_mode());
    return res;
}

FERAL_FUNC(mpFltPreDec, 0, false,
           "  var.fn() -> var\n"
           "Applies pre-decrement on `var` and returns `var` itself.")
{
    mpfr_sub_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
                mpfr_get_default_rounding_mode());
    return args[0];
}

FERAL_FUNC(mpFltPostDec, 0, false,
           "  var.fn() -> MPFlt\n"
           "Applies post-decrement on `var` and returns a new MPFlt with `var` + 1 as the result.")
{
    VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());
    mpfr_sub_ui(as<VarMPFlt>(args[0])->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), 1,
                mpfr_get_default_rounding_mode());
    return res;
}

FERAL_FUNC(mpFltUSub, 0, false,
           "  var.fn() -> MPFlt\n"
           "Returns the negative equivalent of `var` as a new MPFlt.")
{
    VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, as<VarMPFlt>(args[0])->getPtr());
    mpfr_neg(res->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), mpfr_get_default_rounding_mode());
    return res;
}

FERAL_FUNC(mpFltRound, 0, false,
           "  var.fn() -> MPInt\n"
           "Rounds `var` to the closest whole number and returns it as a new MPInt.")
{
    VarMPInt *res = vm.makeVar<VarMPInt>(loc, 0);
    mpfr_get_z(res->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(), MPFR_RNDN);
    return res;
}

FERAL_FUNC(mpFltPow, 1, false,
           "  var.fn(other) -> MPFlt\n"
           "Raises `var` to the power of `other` and returns a new MPFlt with the result.")
{
    EXPECT(VarMPInt, args[1], "power");
    VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0);
    mpfr_pow_si(res->getPtr(), as<VarMPFlt>(args[0])->getSrcPtr(),
                mpz_get_si(as<VarMPInt>(args[1])->getPtr()), MPFR_RNDN);
    return res;
}

FERAL_FUNC(mpFltRoot, 1, false,
           "  var.fn(other) -> MPFlt\n"
           "Lowers `var` to the root of `other` and returns a new MPFlt with the result.")
{
    EXPECT(VarMPInt, args[1], "root");
    VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0);
#if MPFR_VERSION_MAJOR >= 4
    mpfr_rootn_ui(res->getPtr(), as<VarMPFlt>(args[0])->getPtr(),
                  mpz_get_ui(as<VarMPInt>(args[1])->getPtr()), MPFR_RNDN);
#else
    mpfr_root(res->getPtr(), as<VarMPFlt>(args[0])->getPtr(), mpz_get_ui(INT(args[1])->getPtr()),
              MPFR_RNDN);
#endif // MPFR_VERSION_MAJOR
    return res;
}

FERAL_FUNC(mpFltToFlt, 0, false,
           "  var.fn() -> Flt\n"
           "Converts `var` from MPInt to Flt and returns the value.")
{
    return vm.makeVar<VarFlt>(loc, mpfr_get_d(as<VarMPFlt>(args[0])->getPtr(), MPFR_RNDN));
}

FERAL_FUNC(mpFltToStr, 0, false,
           "  var.fn() -> Str\n"
           "Converts `var` from MPFlt to Str and returns the value.")
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
        while(expo > sz) { res->getVal() += '0'; }
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

FERAL_FUNC(mpFltRngGet, 1, false,
           "  fn(upto) -> MPFlt\n"
           "Returns a random number between [0.0, `upto`].")
{
    EXPECT(VarMPFlt, args[1], "upper bound");
    VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0.0);
    mpfr_urandom(res->getPtr(), rngstate, MPFR_RNDN);
    mpfr_mul(res->getPtr(), res->getSrcPtr(), as<VarMPFlt>(args[1])->getSrcPtr(), MPFR_RNDN);
    return res;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////// Complex Functions ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

FERAL_FUNC(mpComplexNewNative, 2, false,
           "  fn(real, virtual) -> MPComplex\n"
           "Creates and returns a new MPFlt using `real` and `virtual` value.\n"
           "Here `real` and `virtual` can be either of MPInt / MPFlt")
{
    EXPECT4(VarInt, VarFlt, VarMPInt, VarMPFlt, args[1], "real value");
    EXPECT4(VarInt, VarFlt, VarMPInt, VarMPFlt, args[2], "virtual value");
    if(args[1]->getType() != args[2]->getType()) {
        vm.fail(loc, "the real and imaginary arguments must be of same time, found: (",
                vm.getTypeName(args[1]), ", ", vm.getTypeName(args[1]), ")");
        return nullptr;
    }

    Var *a1           = args[1];
    Var *a2           = args[2];
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

#define LOGICC_FUNC(fn, name, sym)                                                      \
    FERAL_FUNC(mpComplex##fn, 1, false,                                                 \
               "  var.fn(other) -> Bool\n"                                              \
               "Applies logical '" STRINGIFY(                                           \
                   name) "' between `var` and `other` and returns the resulting Bool.") \
    {                                                                                   \
        if(args[1]->is<VarMPComplex>()) {                                               \
            return mpc_cmp(as<VarMPComplex>(args[0])->getSrcPtr(),                      \
                           as<VarMPComplex>(args[1])->getSrcPtr()) sym 0                \
                       ? vm.getTrue()                                                   \
                       : vm.getFalse();                                                 \
        }                                                                               \
        return vm.getFalse();                                                           \
    }

LOGICC_FUNC(LT, lt, <)
LOGICC_FUNC(GT, gt, >)
LOGICC_FUNC(LE, le, <=)
LOGICC_FUNC(GE, ge, >=)

FERAL_FUNC(
    mpComplexAdd, 1, false,
    "  var.fn(other) -> MPComplex\n"
    "Applies arithmetic-add on `var` and `other` and returns a new MPComplex with the result.")
{
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex addition");
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

FERAL_FUNC(
    mpComplexSub, 1, false,
    "  var.fn(other) -> MPComplex\n"
    "Applies arithmetic-sub on `var` and `other` and returns a new MPComplex with the result.")
{
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex subtraction");
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

FERAL_FUNC(
    mpComplexMul, 1, false,
    "  var.fn(other) -> MPComplex\n"
    "Applies arithmetic-mul on `var` and `other` and returns a new MPComplex with the result.")
{
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex multiplication");
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

FERAL_FUNC(
    mpComplexDiv, 1, false,
    "  var.fn(other) -> MPComplex\n"
    "Applies arithmetic-div on `var` and `other` and returns a new MPComplex with the result.")
{
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex division");
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

FERAL_FUNC(mpComplexAssnAdd, 1, false,
           "  var.fn(other) -> var\n"
           "Applies arithmetic-add on `var` with `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex addition-assn");
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

FERAL_FUNC(mpComplexAssnSub, 1, false,
           "  var.fn(other) -> var\n"
           "Applies arithmetic-sub on `var` with `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex subtraction-assn");
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

FERAL_FUNC(mpComplexAssnMul, 1, false,
           "  var.fn(other) -> var\n"
           "Applies arithmetic-mul on `var` with `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex multiplication-assn");
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

FERAL_FUNC(mpComplexAssnDiv, 1, false,
           "  var.fn(other) -> var\n"
           "Applies arithmetic-div on `var` with `other` and returns the updated `var`.")
{
    EXPECT_NO_CONST(args[0], "var");
    EXPECT3(VarInt, VarMPFlt, VarMPComplex, args[1], "complex division-assn");
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

FERAL_FUNC(mpComplexEQ, 1, false,
           "  var.fn(other) -> Bool\n"
           "Returns `true` if `var` and `other` are equal.")
{
    if(args[1]->is<VarMPComplex>()) {
        return mpc_cmp(as<VarMPComplex>(args[0])->getSrcPtr(),
                       as<VarMPComplex>(args[1])->getSrcPtr()) == 0
                   ? vm.getTrue()
                   : vm.getFalse();
    }
    return vm.getFalse();
}

FERAL_FUNC(mpComplexNE, 1, false,
           "  var.fn(other) -> Bool\n"
           "Returns `true` if `var` and `other` are not equal.")
{
    if(args[1]->is<VarMPComplex>()) {
        return mpc_cmp(as<VarMPComplex>(args[0])->getSrcPtr(),
                       as<VarMPComplex>(args[1])->getSrcPtr()) != 0
                   ? vm.getTrue()
                   : vm.getFalse();
    }
    return vm.getTrue();
}

FERAL_FUNC(mpComplexPreInc, 0, false,
           "  var.fn() -> var\n"
           "Applies pre-increment on `var` and returns `var` itself.")
{
    mpc_add_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
               mpc_get_default_rounding_mode());
    return args[0];
}

FERAL_FUNC(
    mpComplexPostInc, 0, false,
    "  var.fn() -> MPComplex\n"
    "Applies post-increment on `var` and returns a new MPComplex with `var` - 1 as the result.")
{
    VarMPComplex *res = vm.makeVar<VarMPComplex>(loc, as<VarMPComplex>(args[0])->getSrcPtr());
    mpc_add_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
               mpc_get_default_rounding_mode());
    return res;
}

FERAL_FUNC(mpComplexPreDec, 0, false,
           "  var.fn() -> var\n"
           "Applies pre-decrement on `var` and returns `var` itself.")
{
    mpc_sub_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
               mpc_get_default_rounding_mode());
    return args[0];
}

FERAL_FUNC(
    mpComplexPostDec, 0, false,
    "  var.fn() -> MPComplex\n"
    "Applies post-decrement on `var` and returns a new MPComplex with `var` + 1 as the result.")
{
    VarMPComplex *res = vm.makeVar<VarMPComplex>(loc, as<VarMPComplex>(args[0])->getSrcPtr());
    mpc_sub_ui(as<VarMPComplex>(args[0])->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), 1,
               mpc_get_default_rounding_mode());
    return res;
}

FERAL_FUNC(mpComplexUSub, 0, false,
           "  var.fn() -> MPComplex\n"
           "Returns the negative equivalent of `var` as a new MPComplex.")
{
    VarMPComplex *res = vm.makeVar<VarMPComplex>(loc, as<VarMPComplex>(args[0])->getSrcPtr());
    mpc_neg(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(), mpc_get_default_rounding_mode());
    return res;
}

FERAL_FUNC(mpComplexPow, 1, false,
           "  var.fn(other) -> MPComplex\n"
           "Raises `var` to the power of `other` and returns a new MPComplex with the result.")
{
    EXPECT4(VarInt, VarFlt, VarMPInt, VarMPFlt, args[1], "complex power");
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

FERAL_FUNC(mpComplexAbs, 0, false,
           "  var.fn() -> MPFlt\n"
           "Returns the absolute float value of `var` as a new MPFlt.")
{
    VarMPFlt *res = vm.makeVar<VarMPFlt>(loc, 0.0);
    mpc_abs(res->getPtr(), as<VarMPComplex>(args[0])->getSrcPtr(),
            mpfr_get_default_rounding_mode());
    return res;
}

FERAL_FUNC(mpComplexSet, 2, false,
           "  var.fn(real, virtual) -> var\n"
           "Updates the `real` and `virtual` parts of the MPComplex `var` and returns itself.")
{
    EXPECT4(VarInt, VarFlt, VarMPInt, VarMPFlt, args[1], "real value");
    EXPECT4(VarInt, VarFlt, VarMPInt, VarMPFlt, args[2], "virtual value");
    if(args[1]->getType() != args[2]->getType()) {
        vm.fail(loc, "the real and imaginary arguments must be of same time, found: (",
                vm.getTypeName(args[1]), ", ", vm.getTypeName(args[1]), ")");
        return nullptr;
    }

    Var *a1           = args[1];
    Var *a2           = args[2];
    VarMPComplex *res = as<VarMPComplex>(args[0]);

    if(a1->is<VarInt>()) {
        mpc_set_si_si(res->getPtr(), as<VarInt>(a1)->getVal(), as<VarInt>(a2)->getVal(),
                      mpc_get_default_rounding_mode());
    } else if(a1->is<VarFlt>()) {
        mpc_set_ld_ld(res->getPtr(), as<VarFlt>(a1)->getVal(), as<VarFlt>(a2)->getVal(),
                      mpc_get_default_rounding_mode());
    } else if(a1->is<VarMPInt>()) {
        mpc_set_z_z(res->getPtr(), as<VarMPInt>(a1)->getSrcPtr(), as<VarMPInt>(a2)->getSrcPtr(),
                    mpc_get_default_rounding_mode());
    } else if(a1->is<VarMPFlt>()) {
        mpc_set_fr_fr(res->getPtr(), as<VarMPFlt>(a1)->getSrcPtr(), as<VarMPFlt>(a2)->getSrcPtr(),
                      mpc_get_default_rounding_mode());
    }

    return res;
}

INIT_MODULE(MP)
{
    gmp_randinit_default(rngstate);

    VarModule *mod = vm.getCurrModule();

    mod->addNativeFn(vm, "seed", rngSeed);

    mod->addNativeFn(vm, "newIntNative", mpIntNewNative);
    mod->addNativeFn(vm, "newFltNative", mpFltNewNative);
    mod->addNativeFn(vm, "newComplexNative", mpComplexNewNative);

    mod->addNativeFn(vm, "irange", mpIntRange);
    mod->addNativeFn(vm, "getRandomIntNative", mpIntRngGet);
    mod->addNativeFn(vm, "getRandomFltNative", mpFltRngGet);

    // Register the MPInt, MPFlt, MPComplex, and MPIntIterator types

    vm.registerType<VarMPInt>(loc, "MPInt", "GNU Multiprecision - Big Int type.");
    vm.registerType<VarMPFlt>(loc, "MPFlt", "GNU Multiprecision - Big Flt type.");
    vm.registerType<VarMPComplex>(loc, "MPComplex", "GNU Multiprecision - Complex type.");
    vm.registerType<VarMPIntIterator>(loc, "MPIntIterator", "Iterator for Big Int.");

    // MPInt functions

    vm.addNativeTypeFn<VarMPInt>(loc, "+", mpIntAdd);
    vm.addNativeTypeFn<VarMPInt>(loc, "-", mpIntSub);
    vm.addNativeTypeFn<VarMPInt>(loc, "*", mpIntMul);
    vm.addNativeTypeFn<VarMPInt>(loc, "/", mpIntDiv);
    vm.addNativeTypeFn<VarMPInt>(loc, "%", mpIntMod);
    vm.addNativeTypeFn<VarMPInt>(loc, "<<", mpIntLShift);
    vm.addNativeTypeFn<VarMPInt>(loc, ">>", mpIntRShift);

    vm.addNativeTypeFn<VarMPInt>(loc, "+=", mpIntAssnAdd);
    vm.addNativeTypeFn<VarMPInt>(loc, "-=", mpIntAssnSub);
    vm.addNativeTypeFn<VarMPInt>(loc, "*=", mpIntAssnMul);
    vm.addNativeTypeFn<VarMPInt>(loc, "/=", mpIntAssnDiv);
    vm.addNativeTypeFn<VarMPInt>(loc, "%=", mpIntAssnMod);
    vm.addNativeTypeFn<VarMPInt>(loc, "<<=", mpIntAssnLShift);
    vm.addNativeTypeFn<VarMPInt>(loc, ">>=", mpIntAssnRShift);

    vm.addNativeTypeFn<VarMPInt>(loc, "**", mpIntPow);
    vm.addNativeTypeFn<VarMPInt>(loc, "//", mpIntRoot);
    vm.addNativeTypeFn<VarMPInt>(loc, "++x", mpIntPreInc);
    vm.addNativeTypeFn<VarMPInt>(loc, "x++", mpIntPostInc);
    vm.addNativeTypeFn<VarMPInt>(loc, "--x", mpIntPreDec);
    vm.addNativeTypeFn<VarMPInt>(loc, "x--", mpIntPostDec);

    vm.addNativeTypeFn<VarMPInt>(loc, "u-", mpIntUSub);

    vm.addNativeTypeFn<VarMPInt>(loc, "<", mpIntLT);
    vm.addNativeTypeFn<VarMPInt>(loc, ">", mpIntGT);
    vm.addNativeTypeFn<VarMPInt>(loc, "<=", mpIntLE);
    vm.addNativeTypeFn<VarMPInt>(loc, ">=", mpIntGE);
    vm.addNativeTypeFn<VarMPInt>(loc, "==", mpIntEQ);
    vm.addNativeTypeFn<VarMPInt>(loc, "!=", mpIntNE);

    vm.addNativeTypeFn<VarMPInt>(loc, "&", mpIntBAnd);
    vm.addNativeTypeFn<VarMPInt>(loc, "|", mpIntBOr);
    vm.addNativeTypeFn<VarMPInt>(loc, "^", mpIntBXOr);
    vm.addNativeTypeFn<VarMPInt>(loc, "~", mpIntBNot);

    vm.addNativeTypeFn<VarMPInt>(loc, "&=", mpIntAssnBAnd);
    vm.addNativeTypeFn<VarMPInt>(loc, "|=", mpIntAssnBOr);
    vm.addNativeTypeFn<VarMPInt>(loc, "^=", mpIntAssnBXOr);

    vm.addNativeTypeFn<VarMPInt>(loc, "popcnt", mpIntPopCnt);

    vm.addNativeTypeFn<VarMPInt>(loc, "int", mpIntToInt);
    vm.addNativeTypeFn<VarMPInt>(loc, "str", mpIntToStr);
    vm.addNativeTypeFn<VarMPIntIterator>(loc, "next", getMPIntIteratorNext);

    // MPFloat functions

    vm.addNativeTypeFn<VarMPFlt>(loc, "+", mpFltAdd);
    vm.addNativeTypeFn<VarMPFlt>(loc, "-", mpFltSub);
    vm.addNativeTypeFn<VarMPFlt>(loc, "*", mpFltMul);
    vm.addNativeTypeFn<VarMPFlt>(loc, "/", mpFltDiv);

    vm.addNativeTypeFn<VarMPFlt>(loc, "+=", mpFltAssnAdd);
    vm.addNativeTypeFn<VarMPFlt>(loc, "-=", mpFltAssnSub);
    vm.addNativeTypeFn<VarMPFlt>(loc, "*=", mpFltAssnMul);
    vm.addNativeTypeFn<VarMPFlt>(loc, "/=", mpFltAssnDiv);

    vm.addNativeTypeFn<VarMPFlt>(loc, "++x", mpFltPreInc);
    vm.addNativeTypeFn<VarMPFlt>(loc, "x++", mpFltPostInc);
    vm.addNativeTypeFn<VarMPFlt>(loc, "--x", mpFltPreDec);
    vm.addNativeTypeFn<VarMPFlt>(loc, "x--", mpFltPostDec);

    vm.addNativeTypeFn<VarMPFlt>(loc, "u-", mpFltUSub);

    vm.addNativeTypeFn<VarMPFlt>(loc, "round", mpFltRound);

    vm.addNativeTypeFn<VarMPFlt>(loc, "**", mpFltPow);
    vm.addNativeTypeFn<VarMPFlt>(loc, "//", mpFltRoot);

    vm.addNativeTypeFn<VarMPFlt>(loc, "<", mpFltLT);
    vm.addNativeTypeFn<VarMPFlt>(loc, ">", mpFltGT);
    vm.addNativeTypeFn<VarMPFlt>(loc, "<=", mpFltLE);
    vm.addNativeTypeFn<VarMPFlt>(loc, ">=", mpFltGE);
    vm.addNativeTypeFn<VarMPFlt>(loc, "==", mpFltEQ);
    vm.addNativeTypeFn<VarMPFlt>(loc, "!=", mpFltNE);

    vm.addNativeTypeFn<VarMPFlt>(loc, "flt", mpFltToFlt);
    vm.addNativeTypeFn<VarMPFlt>(loc, "str", mpFltToStr);

    // MPComplex functions

    vm.addNativeTypeFn<VarMPComplex>(loc, "+", mpComplexAdd);
    vm.addNativeTypeFn<VarMPComplex>(loc, "-", mpComplexSub);
    vm.addNativeTypeFn<VarMPComplex>(loc, "*", mpComplexMul);
    vm.addNativeTypeFn<VarMPComplex>(loc, "/", mpComplexDiv);

    vm.addNativeTypeFn<VarMPComplex>(loc, "+=", mpComplexAssnAdd);
    vm.addNativeTypeFn<VarMPComplex>(loc, "-=", mpComplexAssnSub);
    vm.addNativeTypeFn<VarMPComplex>(loc, "*=", mpComplexAssnMul);
    vm.addNativeTypeFn<VarMPComplex>(loc, "/=", mpComplexAssnDiv);

    vm.addNativeTypeFn<VarMPComplex>(loc, "==", mpComplexEQ);
    vm.addNativeTypeFn<VarMPComplex>(loc, "!=", mpComplexNE);
    vm.addNativeTypeFn<VarMPComplex>(loc, "<", mpComplexLT);
    vm.addNativeTypeFn<VarMPComplex>(loc, "<=", mpComplexLE);
    vm.addNativeTypeFn<VarMPComplex>(loc, ">", mpComplexGT);
    vm.addNativeTypeFn<VarMPComplex>(loc, ">=", mpComplexGE);

    vm.addNativeTypeFn<VarMPComplex>(loc, "++x", mpComplexPreInc);
    vm.addNativeTypeFn<VarMPComplex>(loc, "x++", mpComplexPostInc);
    vm.addNativeTypeFn<VarMPComplex>(loc, "--x", mpComplexPreDec);
    vm.addNativeTypeFn<VarMPComplex>(loc, "x--", mpComplexPostDec);

    vm.addNativeTypeFn<VarMPComplex>(loc, "u-", mpComplexUSub);

    vm.addNativeTypeFn<VarMPComplex>(loc, "**", mpComplexPow);

    vm.addNativeTypeFn<VarMPComplex>(loc, "abs", mpComplexAbs);
    vm.addNativeTypeFn<VarMPComplex>(loc, "set", mpComplexSet);

    return true;
}

DEINIT_MODULE(MP) { gmp_randclear(rngstate); }

} // namespace fer