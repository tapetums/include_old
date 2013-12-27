// UString.hpp

//---------------------------------------------------------------------------//
//
// 文字コード相互変換ユーティリティ
//   Copyright (C) 2013 tapetums
//
//---------------------------------------------------------------------------//

typedef unsigned char  char8_t;
typedef unsigned short char16_t;
typedef unsigned int   char32_t;

//---------------------------------------------------------------------------//

char*     __stdcall toMBCSz(const char*     s);
char*     __stdcall toMBCSz(const wchar_t*  s);
char*     __stdcall toMBCSz(const char8_t*  s);
char*     __stdcall toMBCSz(const char16_t* s);
char*     __stdcall toMBCSz(const char32_t* s);

wchar_t*  __stdcall toUnicodez(const char*     s);
wchar_t*  __stdcall toUnicodez(const wchar_t*  s);
wchar_t*  __stdcall toUnicodez(const char8_t*  s);
wchar_t*  __stdcall toUnicodez(const char16_t* s);
wchar_t*  __stdcall toUnicodez(const char32_t* s);

char8_t*  __stdcall toUTF8z(const char*     s);
char8_t*  __stdcall toUTF8z(const wchar_t*  s);
char8_t*  __stdcall toUTF8z(const char8_t*  s);
char8_t*  __stdcall toUTF8z(const char16_t* s);
char8_t*  __stdcall toUTF8z(const char32_t* s);

char16_t* __stdcall toUTF16z(const char*     s);
char16_t* __stdcall toUTF16z(const wchar_t*  s);
char16_t* __stdcall toUTF16z(const char8_t*  s);
char16_t* __stdcall toUTF16z(const char16_t* s);
char16_t* __stdcall toUTF16z(const char32_t* s);

char32_t* __stdcall toUTF32z(const char*     s);
char32_t* __stdcall toUTF32z(const wchar_t*  s);
char32_t* __stdcall toUTF32z(const char8_t*  s);
char32_t* __stdcall toUTF32z(const char16_t* s);
char32_t* __stdcall toUTF32z(const char32_t* s);

//---------------------------------------------------------------------------//

#if defined (_UNICODE) || defined (UNICODE)
    #define toTextz toUnicodez
#else
    #define toTextz toMBCSz
#endif

//---------------------------------------------------------------------------//

// UString.hpp