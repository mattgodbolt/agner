/***
This is the original intrin.h modified to fix incompatibility with windows driver kit.
Much irrelevant content removed.
****/

#pragma once
#define __INTRIN_H_

#ifndef RC_INVOKED
#ifndef __midl

//#include <crtdefs.h>
//#include <setjmp.h>
//#include <stddef.h>

#ifndef _In_
#define _In_
#endif

#ifndef _Check_return_
#define _Check_return_
#endif


#if defined(__cplusplus)
extern "C" {
#endif

/*
** __MACHINE             : all compilers
** __MACHINEI            : Intel (32 bit x86) and X64
** __MACHINEX64          : X64 compiler only
** __MACHINEIA32         : 32 bit x86 arch only
** __MACHINEX86X_X64     : X86 Extended intrinsics supported on X64
** __MACHINEX86X_IA64    : X86 Extended intrinsics supported on IA64
** __MACHINEIA64         : IA64 compiler only
** __MACHINEW64          : WIN64(tm), 64 bit compilers only
** __MACHINEIW64         : IA32 + Win64 compilers only (__MACHINEI + __MACHINEW64)
** __MACHINESA           : ARM (StrongARM) only
** __MACHINEARMX         : ARM XSCALE intrinsics
** __MACHINECC           : Intel XSCALE Concan
** __MACHINECE           : common intrinsic functions for Windows CE
** __MACHINEZ            : nothing
*/

#define __MACHINEI            __MACHINE
#define __MACHINEX64          __MACHINE
#define __MACHINEIA32         __MACHINE
#define __MACHINEX86X_X64     __MACHINE
#define __MACHINEX86X_IA64    __MACHINE
#define __MACHINEIA64         __MACHINE
#define __MACHINEW64          __MACHINE
#define __MACHINEIW64         __MACHINE
#define __MACHINESA           __MACHINE
#define __MACHINEARMX         __MACHINE
#define __MACHINECC           __MACHINE
#define __MACHINECE           __MACHINE

/* No intrinsics available to pure managed code */
#if defined(_M_CEE_PURE)
#define __MACHINE(X)          __MACHINEZ(X)
#else
#define __MACHINE(X)          X;
#endif

#define __MACHINEZ(X)         /* NOTHING */

#if !(_M_IX86)
#undef __MACHINEIA32
#define __MACHINEIA32         __MACHINEZ
#endif

#if !(_M_IA64)
#undef __MACHINEIA64
#define __MACHINEIA64         __MACHINEZ
#endif

#if !(_M_AMD64)
#undef __MACHINEX64
#define __MACHINEX64          __MACHINEZ
#endif

#if !(_M_IA64 || _M_AMD64)
#undef __MACHINEW64
#define __MACHINEW64          __MACHINEZ
#endif

#if !(_M_IX86 || _M_AMD64 || _M_IA64)
#undef __MACHINEIW64
#define __MACHINEIW64         __MACHINEZ
#endif

#if !(_M_IX86 || _M_IA64)
#undef __MACHINEX86X_IA64
#define __MACHINEX86X_IA64    __MACHINEZ
#endif

#if !(_M_IX86 || _M_AMD64)
#undef __MACHINEX86X_X64
#define __MACHINEX86X_X64     __MACHINEZ
#endif

#if !(_M_ARM)
#undef  __MACHINESA
#undef  __MACHINEARMX
#undef  __MACHINECC
#define __MACHINESA           __MACHINEZ
#define __MACHINEARMX         __MACHINEZ
#define __MACHINECC           __MACHINEZ
#endif

#if !defined(_WIN32_WCE)
#undef __MACHINECE
#define __MACHINECE           __MACHINEZ
#endif

__MACHINE(void * __cdecl _alloca(size_t))
__MACHINE(int __cdecl abs(_In_ int))
__MACHINECE(__int64 __cdecl _abs64(__int64))
__MACHINE(_Check_return_ unsigned short __cdecl _byteswap_ushort(_In_ unsigned short value))
__MACHINE(_Check_return_ unsigned long __cdecl _byteswap_ulong(_In_ unsigned long value))
__MACHINE(_Check_return_ unsigned __int64 __cdecl _byteswap_uint64(_In_ unsigned __int64 value))
__MACHINECE(void __CacheRelease(void *))
__MACHINECE(void __CacheWriteback(void *))
__MACHINECE(_CRTIMP double ceil(double))
__MACHINEX64(_CRTIMP double ceil(double))
__MACHINECE(_CRTIMP double ceil(double))
__MACHINECE(double _CopyDoubleFromInt64(__int64))
__MACHINECE(float _CopyFloatFromInt32(__int32))
__MACHINECE(__int64 _CopyInt64FromDouble(double))
__MACHINECE(__int32 _CopyInt32FromFloat(float))
__MACHINECE(unsigned _CountLeadingOnes(long))
__MACHINECE(unsigned _CountLeadingOnes64(__int64))
__MACHINECE(unsigned _CountLeadingSigns(long))
__MACHINECE(unsigned _CountLeadingSigns64(__int64))
__MACHINECE(unsigned _CountLeadingZeros(long))
__MACHINECE(unsigned _CountLeadingZeros64(__int64))
__MACHINECE(unsigned _CountOneBits(long))
__MACHINECE(unsigned _CountOneBits64(__int64))
__MACHINE(void __cdecl __debugbreak(void))
__MACHINEI(void __cdecl _disable(void))
__MACHINEI(__int64 __emul(int,int))
__MACHINEI(unsigned __int64 __emulu(unsigned int,unsigned int))
__MACHINEI(void __cdecl _enable(void))
__MACHINECE(void __ICacheRefresh(void *))
//
__MACHINEI(int __cdecl _inp(unsigned short))
__MACHINEI(int __cdecl inp(unsigned short))
__MACHINEI(unsigned long __cdecl _inpd(unsigned short))
__MACHINEI(unsigned long __cdecl inpd(unsigned short))
__MACHINEI(unsigned short __cdecl _inpw(unsigned short))
__MACHINEI(unsigned short __cdecl inpw(unsigned short))
__MACHINECE(int _isnan(double))
__MACHINECE(int _isnanf(float))
__MACHINECE(int _isunordered(double,double))
__MACHINECE(int _isunorderedf(float,float))
__MACHINE(long __cdecl labs(_In_ long))
__MACHINE(_Check_return_ unsigned long __cdecl _lrotl(_In_ unsigned long,_In_ int))
__MACHINE(_Check_return_ unsigned long __cdecl _lrotr(_In_ unsigned long,_In_ int))
__MACHINEI(unsigned __int64  __ll_lshift(unsigned __int64,int))
__MACHINEI(__int64  __ll_rshift(__int64,int))
__MACHINECE(_CONST_RETURN void * __cdecl memchr(const void *,int,size_t))
//__MACHINE(_Check_return_ int __cdecl memcmp(_In_opt_bytecount_(_Size) const void *,_In_opt_bytecount_(_Size) const void *,_In_ size_t _Size))
//__MACHINE(void * __cdecl memcpy(_Out_opt_bytecapcount_(_Size) void *,_In_opt_bytecount_(_Size) const void *,_In_ size_t _Size))
//__MACHINE(void * __cdecl memset(_Out_opt_bytecapcount_(_Size) void *,_In_ int,_In_ size_t _Size))
__MACHINECE(long _MulHigh(long,long))
__MACHINECE(unsigned long _MulUnsignedHigh (unsigned long,unsigned long))
__MACHINEI(int __cdecl _outp(unsigned short,int))
__MACHINEI(int __cdecl outp(unsigned short,int))
__MACHINEI(unsigned long __cdecl _outpd(unsigned short,unsigned long))
__MACHINEI(unsigned long __cdecl outpd(unsigned short,unsigned long))
__MACHINEI(unsigned short __cdecl _outpw(unsigned short,unsigned short))
__MACHINEI(unsigned short __cdecl outpw(unsigned short,unsigned short))
__MACHINEI(void * _ReturnAddress(void))
__MACHINECE(void * _ReturnAddress(void))
__MACHINE(_Check_return_ unsigned int __cdecl _rotl(_In_ unsigned int,_In_ int))
__MACHINECE(_Check_return_ unsigned __int64 __cdecl _rotl64(_In_ unsigned __int64,_In_ int))
__MACHINE(_Check_return_ unsigned int __cdecl _rotr(_In_ unsigned int,_In_ int))
__MACHINECE(_Check_return_ unsigned __int64 __cdecl _rotr64(_In_ unsigned __int64,_In_ int))
//__MACHINE(char * __cdecl strcat(_Pre_cap_for_(_Source) _Prepost_z_ char *,_In_z_ const char * _Source))
//__MACHINE(_Check_return_ int __cdecl strcmp(_In_z_ const char *,_In_z_ const char *))
//__MACHINE(char * __cdecl strcpy(_Pre_cap_for_(_Source) _Post_z_ char *,_In_z_ const char * _Source))
//__MACHINE(_Check_return_ size_t __cdecl strlen(_In_z_ const char *))
//__MACHINECE(int __cdecl strncmp(const char *,const char *,size_t))
//__MACHINE(char * __cdecl _strset(_Inout_z_ char *,_In_ int))
//__MACHINE(char * __cdecl strset(_Inout_z_ char *,_In_ int))
__MACHINEI(unsigned __int64 __ull_rshift(unsigned __int64,int))
//__MACHINECE(wchar_t * __cdecl wcscat(_Pre_cap_for_(_Source) _Prepost_z_ wchar_t *,_In_z_ const wchar_t * _Source))
//__MACHINECE(_Check_return_ int __cdecl wcscmp(_In_z_ const wchar_t *,_In_z_ const wchar_t *))
//__MACHINECE(wchar_t * __cdecl wcscpy(_Pre_cap_for_(_Source) _Post_z_ wchar_t *,_In_z_ const wchar_t * _Source))
//__MACHINECE(_Check_return_ size_t __cdecl wcslen(_In_z_ const wchar_t *))
__MACHINECE(int __cdecl wcsncmp(const wchar_t *, const wchar_t *,size_t))
__MACHINECE(wchar_t * __cdecl wcsncpy(wchar_t *, const wchar_t *,size_t))
__MACHINECE(wchar_t * __cdecl _wcsset(_Inout_z_ wchar_t *, wchar_t))
__MACHINECE(void _WriteBarrier(void))
__MACHINEI(void * _AddressOfReturnAddress(void))

//
__MACHINEI(void _WriteBarrier(void))
__MACHINEI(void _ReadWriteBarrier(void))
__MACHINEX64(void __faststorefence(void))
__MACHINEX64(__int64 __mulh(__int64,__int64))
__MACHINEX64(unsigned __int64 __umulh(unsigned __int64,unsigned __int64))
__MACHINEX64(unsigned __int64 __readcr0(void))
__MACHINEX64(unsigned __int64 __readcr2(void))
__MACHINEX64(unsigned __int64 __readcr3(void))
__MACHINEX64(unsigned __int64 __readcr4(void))
__MACHINEX64(unsigned __int64 __readcr8(void))
__MACHINEIA32(unsigned long __readcr0(void))
__MACHINEIA32(unsigned long __readcr2(void))
__MACHINEIA32(unsigned long __readcr3(void))
__MACHINEIA32(unsigned long __readcr4(void))
__MACHINEIA32(unsigned long __readcr8(void))
__MACHINEX64(void __writecr0(unsigned __int64))
__MACHINEX64(void __writecr3(unsigned __int64))
__MACHINEX64(void __writecr4(unsigned __int64))
__MACHINEX64(void __writecr8(unsigned __int64))
__MACHINEIA32(void __writecr0(unsigned))
__MACHINEIA32(void __writecr3(unsigned))
__MACHINEIA32(void __writecr4(unsigned))
__MACHINEIA32(void __writecr8(unsigned))
__MACHINEX64(unsigned __int64 __readdr(unsigned int))
__MACHINEIA32(unsigned __readdr(unsigned int))
__MACHINEX64(void __writedr(unsigned int, unsigned __int64))
__MACHINEIA32(void __writedr(unsigned int, unsigned))
__MACHINEX64(unsigned __int64 __readeflags(void))
__MACHINEIA32(unsigned __readeflags(void))
__MACHINEX64(void __writeeflags(unsigned __int64))
__MACHINEIA32(void __writeeflags(unsigned))
__MACHINEI(void __wbinvd(void))
__MACHINEI(void __invlpg(void*))
__MACHINEI(unsigned __int64 __readmsr(unsigned long))
__MACHINEI(void __writemsr(unsigned long, unsigned __int64))
__MACHINEI(unsigned __int64 __rdtsc(void))
__MACHINEI(void __movsb(unsigned char *, unsigned char const *, size_t))
__MACHINEI(void __movsw(unsigned short *, unsigned short const *, size_t))
__MACHINEI(void __movsd(unsigned long *, unsigned long const *, size_t))
__MACHINEX64(void __movsq(unsigned long long *, unsigned long long const *, size_t))
__MACHINEX64(unsigned char __readgsbyte(unsigned long Offset))
__MACHINEX64(unsigned short __readgsword(unsigned long Offset))
__MACHINEX64(unsigned long __readgsdword(unsigned long Offset))
__MACHINEX64(unsigned __int64 __readgsqword(unsigned long Offset))
__MACHINEX64(void __writegsbyte(unsigned long Offset, unsigned char Data))
__MACHINEX64(void __writegsword(unsigned long Offset, unsigned short Data))
__MACHINEX64(void __writegsdword(unsigned long Offset, unsigned long Data))
__MACHINEX64(void __writegsqword(unsigned long Offset, unsigned __int64 Data))
__MACHINEI(unsigned char __inbyte(unsigned short Port))
__MACHINEI(unsigned short __inword(unsigned short Port))
__MACHINEI(unsigned long __indword(unsigned short Port))
__MACHINEI(void __outbyte(unsigned short Port, unsigned char Data))
__MACHINEI(void __outword(unsigned short Port, unsigned short Data))
__MACHINEI(void __outdword(unsigned short Port, unsigned long Data))
__MACHINEI(void __inbytestring(unsigned short Port, unsigned char *Buffer, unsigned long Count))
__MACHINEI(void __inwordstring(unsigned short Port, unsigned short *Buffer, unsigned long Count))
__MACHINEI(void __indwordstring(unsigned short Port, unsigned long *Buffer, unsigned long Count))
__MACHINEI(void __outbytestring(unsigned short Port, unsigned char *Buffer, unsigned long Count))
__MACHINEI(void __outwordstring(unsigned short Port, unsigned short *Buffer, unsigned long Count))
__MACHINEI(void __outdwordstring(unsigned short Port, unsigned long *Buffer, unsigned long Count))
__MACHINEI(unsigned int __getcallerseflags())
__MACHINEX64(unsigned char __vmx_vmclear(unsigned __int64*))
__MACHINEX64(unsigned char __vmx_vmlaunch(void))
__MACHINEX64(unsigned char __vmx_vmptrld(unsigned __int64*))
__MACHINEI(void __vmx_vmptrst(unsigned __int64 *))
__MACHINEX64(unsigned char __vmx_vmread(size_t, size_t*))
__MACHINEX64(unsigned char __vmx_vmresume(void))
__MACHINEX64(unsigned char __vmx_vmwrite(size_t, size_t))
__MACHINEI(void __vmx_off(void))
__MACHINEX64(unsigned char __vmx_on(unsigned __int64*))
__MACHINEI(void __svm_clgi(void))
__MACHINEI(void __svm_invlpga(void*, int))
__MACHINEI(void __svm_skinit(int))
__MACHINEI(void __svm_stgi(void))
__MACHINEI(void __svm_vmload(size_t))
__MACHINEI(void __svm_vmrun(size_t))
__MACHINEI(void __svm_vmsave(size_t))
__MACHINEI(void __halt(void))
__MACHINEI(void __sidt(void*))
__MACHINEI(void __lidt(void*))
__MACHINEI(void __ud2(void))
__MACHINEI(void __nop(void))
//
__MACHINEI(void __stosb(unsigned char *, unsigned char, size_t))
__MACHINEI(void __stosw(unsigned short *,  unsigned short, size_t))
__MACHINEI(void __stosd(unsigned long *,  unsigned long, size_t))
__MACHINEX64(void __stosq(unsigned __int64 *,  unsigned __int64, size_t))
__MACHINEIW64(unsigned char _bittest(long const *a, long b))
__MACHINEIW64(unsigned char _bittestandset(long *a, long b))
__MACHINEIW64(unsigned char _bittestandreset(long *a, long b))
__MACHINEIW64(unsigned char _bittestandcomplement(long *a, long b))
__MACHINEI(unsigned char _interlockedbittestandset(long volatile *a, long b))
__MACHINEI(unsigned char _interlockedbittestandreset(long volatile *a, long b))
__MACHINEW64(unsigned char _bittest64(__int64 const *a, __int64 b))
__MACHINEW64(unsigned char _bittestandset64(__int64 *a, __int64 b))
__MACHINEW64(unsigned char _bittestandreset64(__int64 *a, __int64 b))
__MACHINEW64(unsigned char _bittestandcomplement64(__int64 *a, __int64 b))
__MACHINEX64(unsigned char _interlockedbittestandset64(__int64 volatile *a, __int64 b))
__MACHINEX64(unsigned char _interlockedbittestandreset64(__int64 volatile *a, __int64 b))
__MACHINEI(void __cpuid(int a[4], int b))
__MACHINEI(void __cpuidex(int a[4], int b, int c))
__MACHINEI(unsigned __int64 __readpmc(unsigned long a))
__MACHINEI(unsigned long __segmentlimit(unsigned long a))

__MACHINEIA32(unsigned char __readfsbyte(unsigned long Offset))
__MACHINEIA32(unsigned short __readfsword(unsigned long Offset))
__MACHINEIA32(unsigned long __readfsdword(unsigned long Offset))
__MACHINEIA32(unsigned __int64 __readfsqword(unsigned long Offset))
__MACHINEIA32(void __writefsbyte(unsigned long Offset, unsigned char Data))
__MACHINEIA32(void __writefsword(unsigned long Offset, unsigned short Data))
__MACHINEIA32(void __writefsdword(unsigned long Offset, unsigned long Data))
__MACHINEIA32(void __writefsqword(unsigned long Offset, unsigned __int64 Data))


__MACHINE(_Check_return_ unsigned __int64 __cdecl _rotl64(_In_ unsigned __int64,_In_ int))
__MACHINE(_Check_return_ unsigned __int64 __cdecl _rotr64(_In_ unsigned __int64,_In_ int))
__MACHINE(__int64 __cdecl _abs64(__int64))

__MACHINEIW64(unsigned char _BitScanForward(unsigned long* Index, unsigned long Mask))
__MACHINEIW64(unsigned char _BitScanReverse(unsigned long* Index, unsigned long Mask))
__MACHINEW64(unsigned char _BitScanForward64(unsigned long* Index, unsigned __int64 Mask))
__MACHINEW64(unsigned char _BitScanReverse64(unsigned long* Index, unsigned __int64 Mask))
//__MACHINEIW64(_CRTIMP wchar_t * __cdecl wcscat( _Pre_cap_for_(_Source) _Prepost_z_ wchar_t *, _In_z_ const wchar_t * _Source))
//__MACHINEIW64(_Check_return_ _CRTIMP int __cdecl wcscmp(_In_z_ const wchar_t *,_In_z_  const wchar_t *))
//__MACHINEIW64(_CRTIMP wchar_t * __cdecl wcscpy(_Pre_cap_for_(_Source) _Post_z_ wchar_t *, _In_z_ const wchar_t * _Source))
//__MACHINEIW64(_Check_return_ _CRTIMP size_t __cdecl wcslen(_In_z_ const wchar_t *))
//#pragma warning(suppress: 4985)
//__MACHINEIW64(_CRTIMP wchar_t * __cdecl _wcsset(_Inout_z_ wchar_t *, wchar_t))
__MACHINEW64(unsigned __int64 __shiftleft128(unsigned __int64 LowPart, unsigned __int64 HighPart, unsigned char Shift))
__MACHINEW64(unsigned __int64 __shiftright128(unsigned __int64 LowPart, unsigned __int64 HighPart, unsigned char Shift))
__MACHINEW64(unsigned __int64 _umul128(unsigned __int64 multiplier, unsigned __int64 multiplicand, unsigned __int64 *highproduct))
__MACHINEW64(__int64 _mul128(__int64 multiplier, __int64 multiplicand, __int64 *highproduct))
__MACHINEI(void __int2c(void))
__MACHINEIW64(void _ReadBarrier(void))
__MACHINEIW64(unsigned char _rotr8(unsigned char value, unsigned char shift))
__MACHINEIW64(unsigned short _rotr16(unsigned short value, unsigned char shift))
__MACHINEIW64(unsigned char _rotl8(unsigned char value, unsigned char shift))
__MACHINEIW64(unsigned short _rotl16(unsigned short value, unsigned char shift))
__MACHINEIW64(short _InterlockedIncrement16(short volatile *Addend))
__MACHINEIW64(short _InterlockedDecrement16(short volatile *Addend))
__MACHINEIW64(short _InterlockedCompareExchange16(short volatile *Destination, short Exchange, short Comparand))
__MACHINEIW64(void __nvreg_save_fence(void))
__MACHINEIW64(void __nvreg_restore_fence(void))

__MACHINEX64(short _InterlockedCompareExchange16_np(short volatile *Destination, short Exchange, short Comparand))
__MACHINEX64(long _InterlockedCompareExchange_np (long *, long, long))
__MACHINEX64(__int64 _InterlockedCompareExchange64_np(__int64 *, __int64, __int64))
__MACHINEX64(void *_InterlockedCompareExchangePointer_np (void **, void *, void *))
__MACHINEX64(unsigned char _InterlockedCompareExchange128(__int64 volatile *, __int64, __int64, __int64 *))
__MACHINEX64(unsigned char _InterlockedCompareExchange128_np(__int64 volatile *, __int64, __int64, __int64 *))
__MACHINEX64(long _InterlockedAnd_np(long *, long))
__MACHINEX64(char _InterlockedAnd8_np(char *, char))
__MACHINEX64(short _InterlockedAnd16_np(short *, short))
__MACHINEX64(__int64 _InterlockedAnd64_np(__int64 *, __int64))
__MACHINEX64(long _InterlockedOr_np(long *, long))
__MACHINEX64(char _InterlockedOr8_np(char *, char))
__MACHINEX64(short _InterlockedOr16_np(short *, short))
__MACHINEX64(__int64 _InterlockedOr64_np(__int64 *, __int64))
__MACHINEX64(long _InterlockedXor_np(long *, long))
__MACHINEX64(char _InterlockedXor8_np(char *, char))
__MACHINEX64(short _InterlockedXor16_np(short *, short))
__MACHINEX64(__int64 _InterlockedXor64_np(__int64 *, __int64))

//
#if defined(_NO_PREFETCHW)
#if defined(_M_AMD64)

#define _InterlockedCompareExchange16 _InterlockedCompareExchange16_np
#define _InterlockedCompareExchange _InterlockedCompareExchange_np
#define _InterlockedCompareExchange64 _InterlockedCompareExchange64_np
#define _InterlockedCompareExchangePointer _InterlockedCompareExchangePointer_np
#define _InterlockedCompareExchange128 _InterlockedCompareExchange128_np
#define _InterlockedAnd _InterlockedAnd_np
#define _InterlockedAnd8 _InterlockedAnd8_np
#define _InterlockedAnd16 _InterlockedAnd16_np
#define _InterlockedAnd64 _InterlockedAnd64_np
#define _InterlockedOr _InterlockedOr_np
#define _InterlockedOr8 _InterlockedOr8_np
#define _InterlockedOr16 _InterlockedOr16_np
#define _InterlockedOr64 _InterlockedOr64_np
#define _InterlockedXor _InterlockedXor_np
#define _InterlockedXor8 _InterlockedXor8_np
#define _InterlockedXor16 _InterlockedXor16_np
#define _InterlockedXor64 _InterlockedXor64_np

#endif /* defined(_M_AMD64) */
#endif /* defined(_NO_PREFETCHW) */

#if defined(__cplusplus)
}
#endif
#endif
#endif
