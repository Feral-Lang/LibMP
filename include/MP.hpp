#pragma once

#include <gmp.h>
#include <mpc.h>
#include <mpfr.h>
#include <VM/Interpreter.hpp>

namespace fer
{

extern gmp_randstate_t rngstate;

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// MPInt class //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

class VarMPInt : public Var
{
    mpz_t val;

    Var *onCopy(MemoryManager &mem, ModuleLoc loc) override;
    void onSet(MemoryManager &mem, Var *from) override;

public:
    VarMPInt(ModuleLoc loc, int64_t _val);
    VarMPInt(ModuleLoc loc, mpz_srcptr _val);
    VarMPInt(ModuleLoc loc, mpfr_srcptr _val);
    VarMPInt(ModuleLoc loc, const char *_val);
    ~VarMPInt();

    inline mpz_ptr getPtr() { return val; }
    // mpz_srcptr is basically 'const mpz_ptr'
    inline mpz_srcptr getSrcPtr() { return val; }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////// MPFlt class //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

class VarMPFlt : public Var
{
    mpfr_t val;

    Var *onCopy(MemoryManager &mem, ModuleLoc loc) override;
    void onSet(MemoryManager &mem, Var *from) override;

public:
    VarMPFlt(ModuleLoc loc, double _val);
    VarMPFlt(ModuleLoc loc, mpfr_srcptr _val);
    VarMPFlt(ModuleLoc loc, mpz_srcptr _val);
    VarMPFlt(ModuleLoc loc, const char *_val);
    ~VarMPFlt();

    inline mpfr_ptr getPtr() { return val; }
    // mpfr_srcptr is basically 'const mpfr_ptr'
    inline mpfr_srcptr getSrcPtr() { return val; }
};

//////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// MPComplex class ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////

class VarMPComplex : public Var
{
    mpc_t val;

    Var *onCopy(MemoryManager &mem, ModuleLoc loc) override;
    void onSet(MemoryManager &mem, Var *from) override;

public:
    VarMPComplex(ModuleLoc loc);
    VarMPComplex(ModuleLoc loc, int64_t real, int64_t imag);
    VarMPComplex(ModuleLoc loc, double real, double imag);
    VarMPComplex(ModuleLoc loc, mpfr_srcptr real, mpfr_srcptr imag);
    VarMPComplex(ModuleLoc loc, mpz_srcptr real, mpz_srcptr imag);
    VarMPComplex(ModuleLoc loc, mpc_srcptr _val);
    VarMPComplex(ModuleLoc loc, const char *_val);
    ~VarMPComplex();

    void initBase();

    inline mpc_ptr getPtr() { return val; }
    // mpc_srcptr is basically 'const mpc_ptr'
    inline mpc_srcptr getSrcPtr() { return val; }
};

mpc_rnd_t mpc_get_default_rounding_mode();

} // namespace fer